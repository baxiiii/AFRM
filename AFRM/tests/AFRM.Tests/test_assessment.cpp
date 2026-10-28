// test_assessment.cpp - R5 (four status bands, exact thresholds) and the PFD-to-status link.

#include <gtest/gtest.h>
#include "pch.h"
#include <limits>
#include <string>

#include "Assessment.h"
#include "Calculations.h"
#include "test_helpers.h"

using namespace afrm;
using namespace afrm_test;

namespace {

    // Hand-built so thresholds can be hit exactly: protected 2460, diversion 2280, final reserve 1080.
    FuelCalculation make_calculation(double pfd_kg) {
        FuelCalculation c;
        c.contingency_fuel_kg = 180.0;
        c.alternate_fuel_kg = 1200.0;
        c.final_reserve_fuel_kg = 1080.0;
        c.diversion_protected_fuel_kg = 2280.0;
        c.protected_fuel_kg = 2460.0;
        c.predicted_fuel_at_destination_kg = pfd_kg;
        return c;
    }

    FuelStatus status_for_pfd(double pfd_kg) {
        return assess_fuel_status(make_calculation(pfd_kg)).status;
    }

}  // namespace

TEST(Assessment, REQ_STATUS_01_StatusLabelsAreExact) {
    EXPECT_EQ(std::string(to_string(FuelStatus::Nominal)), "NOMINAL");
    EXPECT_EQ(std::string(to_string(FuelStatus::Caution)), "CAUTION");
    EXPECT_EQ(std::string(to_string(FuelStatus::MinimumFuel)), "MINIMUM FUEL");
    EXPECT_EQ(std::string(to_string(FuelStatus::FuelEmergency)), "FUEL EMERGENCY");
}

TEST(Assessment, REQ_STATUS_01_FourBands) {
    EXPECT_EQ(status_for_pfd(5000.0), FuelStatus::Nominal);
    EXPECT_EQ(status_for_pfd(2400.0), FuelStatus::Caution);
    EXPECT_EQ(status_for_pfd(1500.0), FuelStatus::MinimumFuel);
    EXPECT_EQ(status_for_pfd(500.0), FuelStatus::FuelEmergency);
}

TEST(Assessment, REQ_STATUS_01_ExactlyOnEachThreshold) {
    EXPECT_EQ(status_for_pfd(2460.0), FuelStatus::Nominal);
    EXPECT_EQ(status_for_pfd(2280.0), FuelStatus::Caution);
    EXPECT_EQ(status_for_pfd(1080.0), FuelStatus::MinimumFuel);
}

TEST(Assessment, REQ_STATUS_01_OneKilogramEitherSideOfEachThreshold) {
    EXPECT_EQ(status_for_pfd(2461.0), FuelStatus::Nominal);
    EXPECT_EQ(status_for_pfd(2459.0), FuelStatus::Caution);
    EXPECT_EQ(status_for_pfd(2281.0), FuelStatus::Caution);
    EXPECT_EQ(status_for_pfd(2279.0), FuelStatus::MinimumFuel);
    EXPECT_EQ(status_for_pfd(1081.0), FuelStatus::MinimumFuel);
    EXPECT_EQ(status_for_pfd(1079.0), FuelStatus::FuelEmergency);
}

TEST(Assessment, REQ_STATUS_01_NegativeAndZeroPfdAreEmergency) {
    EXPECT_EQ(status_for_pfd(-600.0), FuelStatus::FuelEmergency);
    EXPECT_EQ(status_for_pfd(0.0), FuelStatus::FuelEmergency);
}

TEST(Assessment, REQ_STATUS_01_NaNPfdFailsSafeToEmergency) {
    EXPECT_EQ(status_for_pfd(std::numeric_limits<double>::quiet_NaN()), FuelStatus::FuelEmergency);
}

TEST(Assessment, REQ_STATUS_01_ExplanationIsGivenForEveryStatus) {
    for (double pfd : {5000.0, 2400.0, 1500.0, 500.0}) {
        EXPECT_FALSE(assess_fuel_status(make_calculation(pfd)).explanation.empty());
    }
    const FuelAssessment nominal = assess_fuel_status(make_calculation(2460.0));
    EXPECT_NE(nominal.explanation.find("2460.0"), std::string::npos);  // PFD value is stated
}

TEST(Assessment, REQ_PFD_01_StatusFromFullPipelineOnBoundaryFlights) {
    struct Case {
        double fuel_on_board_kg;
        double expected_pfd_kg;
        FuelStatus expected_status;
    };
    const Case cases[] = {
        {6060.0, 2460.0, FuelStatus::Nominal},
        {6059.0, 2459.0, FuelStatus::Caution},
        {5880.0, 2280.0, FuelStatus::Caution},
        {5879.0, 2279.0, FuelStatus::MinimumFuel},
        {4680.0, 1080.0, FuelStatus::MinimumFuel},
        {4679.0, 1079.0, FuelStatus::FuelEmergency},
        {3000.0, -600.0, FuelStatus::FuelEmergency},
    };
    for (const Case& test_case : cases) {
        SCOPED_TRACE(test_case.fuel_on_board_kg);
        FlightRecord flight = make_base_flight();
        flight.fuel_on_board_kg = test_case.fuel_on_board_kg;
        ASSERT_TRUE(validate_flight_record(flight).is_valid);
        const FuelCalculation c = calculate_fuel(flight);
        EXPECT_NEAR(c.predicted_fuel_at_destination_kg, test_case.expected_pfd_kg, kTolerance);
        EXPECT_EQ(assess_fuel_status(c).status, test_case.expected_status);
    }
}

TEST(Assessment, REQ_PFD_01_RoundingNeverMakesMarginalCaseSafer) {
    // PFD is exactly 2461. Unrounded, protected fuel would be 2460.575 (NOMINAL);
    // rounded up it is 2462, so the flight is CAUTION.
    FlightRecord flight = make_base_flight();
    flight.cruise_flow_kg_per_hour = 2401.0;
    flight.fuel_on_board_kg = 6062.5;

    const FuelCalculation c = calculate_fuel(flight);
    EXPECT_NEAR(c.predicted_fuel_at_destination_kg, 2461.0, kTolerance);
    EXPECT_DOUBLE_EQ(c.protected_fuel_kg, 2462.0);
    EXPECT_EQ(assess_fuel_status(c).status, FuelStatus::Caution);
}