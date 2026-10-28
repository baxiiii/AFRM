// test_calculations.cpp - formulas (R1-R4), rounding and numeric validation.

#include <gtest/gtest.h>
#include "pch.h"

#include <functional>
#include <limits>
#include <vector>

#include "C:\Users\ibrah\OneDrive\Documents\University\REPOs\AFRM\AFRM\src\AFRM.Core\Calculations.h"
#include "test_helpers.h"

using namespace afrm;
using namespace afrm_test;

TEST(Calculations, REQ_CALC_01_BaseFlightValues) {
    const FuelCalculation c = calculate_fuel(make_base_flight());
    EXPECT_NEAR(c.trip_fuel_to_go_kg, 3600.0, kTolerance);
    EXPECT_NEAR(c.alternate_fuel_kg, 1200.0, kTolerance);
    EXPECT_NEAR(c.contingency_fuel_kg, 180.0, kTolerance);
    EXPECT_NEAR(c.final_reserve_fuel_kg, 1080.0, kTolerance);
    EXPECT_NEAR(c.predicted_fuel_at_destination_kg, 2460.0, kTolerance);
    EXPECT_NEAR(c.protected_fuel_kg, 2460.0, kTolerance);           // 180 + 1200 + 1080
    EXPECT_NEAR(c.diversion_protected_fuel_kg, 2280.0, kTolerance);  // 1200 + 1080
}

TEST(Calculations, REQ_CALC_01_DifferentSpeedAndFlow) {
    FlightRecord flight = make_base_flight();
    flight.trip_distance_nm = 900.0;
    flight.distance_flown_nm = 0.0;
    flight.ground_speed_knots = 450.0;       // 2 h
    flight.cruise_flow_kg_per_hour = 2700.0;
    EXPECT_NEAR(calculate_fuel(flight).trip_fuel_to_go_kg, 5400.0, kTolerance);
}

TEST(Calculations, REQ_CALC_01_NothingLeftToFly) {
    FlightRecord flight = make_base_flight();
    flight.distance_flown_nm = flight.trip_distance_nm;
    const FuelCalculation c = calculate_fuel(flight);
    EXPECT_NEAR(c.trip_fuel_to_go_kg, 0.0, kTolerance);
    EXPECT_NEAR(c.predicted_fuel_at_destination_kg, flight.fuel_on_board_kg, kTolerance);
}

TEST(Calculations, REQ_CALC_01_InvalidInputsRejected) {
    constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();
    constexpr double kInf = std::numeric_limits<double>::infinity();

    struct InvalidCase {
        const char* name;
        std::function<void(FlightRecord&)> mutate;
    };
    const std::vector<InvalidCase> cases = {
        {"empty flight id", [](FlightRecord& r) { r.flight_id.clear(); }},
        {"negative trip distance", [](FlightRecord& r) { r.trip_distance_nm = -1.0; }},
        {"negative alternate distance", [](FlightRecord& r) { r.alternate_distance_nm = -1.0; }},
        {"negative distance flown", [](FlightRecord& r) { r.distance_flown_nm = -1.0; }},
        {"distance flown exceeds trip", [](FlightRecord& r) { r.distance_flown_nm = 1000.5; }},
        {"negative fuel on board", [](FlightRecord& r) { r.fuel_on_board_kg = -1.0; }},
        {"zero speed", [](FlightRecord& r) { r.ground_speed_knots = 0.0; }},
        {"negative speed", [](FlightRecord& r) { r.ground_speed_knots = -400.0; }},
        {"zero cruise flow", [](FlightRecord& r) { r.cruise_flow_kg_per_hour = 0.0; }},
        {"zero holding flow", [](FlightRecord& r) { r.holding_flow_kg_per_hour = 0.0; }},
        {"NaN fuel on board", [kNaN](FlightRecord& r) { r.fuel_on_board_kg = kNaN; }},
        {"infinite trip distance", [kInf](FlightRecord& r) { r.trip_distance_nm = kInf; }},
    };

    for (const InvalidCase& test_case : cases) {
        SCOPED_TRACE(test_case.name);
        FlightRecord flight = make_base_flight();
        test_case.mutate(flight);
        const ValidationResult validation = validate_flight_record(flight);
        EXPECT_FALSE(validation.is_valid);
        EXPECT_FALSE(validation.message.empty());
    }
}

TEST(Calculations, REQ_CALC_01_BoundaryButValidInputsAccepted) {
    FlightRecord flight = make_base_flight();
    flight.fuel_on_board_kg = 0.0;
    flight.alternate_distance_nm = 0.0;
    flight.distance_flown_nm = 0.0;
    EXPECT_TRUE(validate_flight_record(flight).is_valid);
    EXPECT_TRUE(validate_flight_record(make_base_flight()).is_valid);
}

// ---- Rounding -------------------------------------------------------------

TEST(Calculations, REQ_CALC_01_RoundUpToWholeKg) {
    EXPECT_EQ(round_up_to_whole_kg(150.2), 151.0);
    EXPECT_EQ(round_up_to_whole_kg(150.0), 150.0);
    EXPECT_EQ(round_up_to_whole_kg(0.1), 1.0);
    EXPECT_EQ(round_up_to_whole_kg(0.0), 0.0);
    EXPECT_EQ(round_up_to_whole_kg(150.0 + 1e-10), 150.0);  // floating-point noise ignored
    EXPECT_EQ(round_up_to_whole_kg(150.0 + 1e-8), 151.0);   // a real excess rounds up
}

TEST(Calculations, REQ_CALC_01_RequiredFuelRoundedUpTripFuelAndPfdNot) {
    // Cruise flow 2401: trip fuel 3601.5; alternate 1200.5; contingency max(180.075, 180) = 180.075;
    // final reserve 1080. Rounded up: 1201, 181, 1080.
    FlightRecord flight = make_base_flight();
    flight.cruise_flow_kg_per_hour = 2401.0;
    flight.fuel_on_board_kg = 6062.5;

    const FuelCalculation c = calculate_fuel(flight);
    EXPECT_DOUBLE_EQ(c.alternate_fuel_kg, 1201.0);
    EXPECT_DOUBLE_EQ(c.contingency_fuel_kg, 181.0);
    EXPECT_DOUBLE_EQ(c.final_reserve_fuel_kg, 1080.0);
    EXPECT_DOUBLE_EQ(c.diversion_protected_fuel_kg, 2281.0);
    EXPECT_DOUBLE_EQ(c.protected_fuel_kg, 2462.0);  // sum of the rounded values

    EXPECT_NEAR(c.trip_fuel_to_go_kg, 3601.5, kTolerance);  // not rounded
    EXPECT_NEAR(c.predicted_fuel_at_destination_kg, 2461.0, kTolerance);
}

// ---- Contingency (R2) -----------------------------------------------------

TEST(Calculations, REQ_CONT_01_BothBranchesAndCrossover) {
    FlightRecord flight = make_base_flight();

    // Crossover: 5% of 3600 = 180 = 5 minutes of holding (2160 / 12).
    EXPECT_DOUBLE_EQ(calculate_fuel(flight).contingency_fuel_kg, 180.0);

    flight.distance_flown_nm = 0.0;  // trip fuel 6000, 5% = 300 wins
    EXPECT_DOUBLE_EQ(calculate_fuel(flight).contingency_fuel_kg, 300.0);

    flight.distance_flown_nm = 900.0;  // trip fuel 600, 5% = 30, holding 180 wins
    EXPECT_DOUBLE_EQ(calculate_fuel(flight).contingency_fuel_kg, 180.0);
}

// ---- Final reserve (R3) ---------------------------------------------------

TEST(Calculations, REQ_RESERVE_01_ThirtyMinutesOfHolding) {
    EXPECT_DOUBLE_EQ(calculate_fuel(make_base_flight()).final_reserve_fuel_kg, 1080.0);

    FlightRecord flight = make_base_flight();
    flight.holding_flow_kg_per_hour = 2001.0;  // 1000.5 kg rounds up
    EXPECT_DOUBLE_EQ(calculate_fuel(flight).final_reserve_fuel_kg, 1001.0);
}

// ---- Predicted fuel at destination (R4) -----------------------------------

TEST(Calculations, REQ_PFD_01_FuelOnBoardMinusTripFuelToGo) {
    FlightRecord flight = make_base_flight();
    EXPECT_NEAR(calculate_fuel(flight).predicted_fuel_at_destination_kg, 2460.0, kTolerance);

    flight.fuel_on_board_kg = 3000.0;  // short of trip fuel
    EXPECT_NEAR(calculate_fuel(flight).predicted_fuel_at_destination_kg, -600.0, kTolerance);
}