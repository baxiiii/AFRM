// Calculations.cpp
#include "Calculations.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "Config.h"

namespace afrm {

    ValidationResult validate_flight_record(const FlightRecord& r) {
        if (r.flight_id.empty()) return ValidationResult::fail("flight_id is empty");

        // NaN passes every < and <= test, so check for finite values first.
        const double numbers[] = {
            r.trip_distance_nm,
            r.alternate_distance_nm,
            r.distance_flown_nm,
            r.fuel_on_board_kg,
            r.ground_speed_knots,
            r.cruise_flow_kg_per_hour,
            r.holding_flow_kg_per_hour,
            r.aircraft_weight_kg
        };
        for (double n : numbers) {
            if (!std::isfinite(n)) return ValidationResult::fail("a numeric field is not a finite number");
        }

        if (r.trip_distance_nm < 0.0)      return ValidationResult::fail("trip_distance_nm must not be negative");
        if (r.alternate_distance_nm < 0.0) return ValidationResult::fail("alternate_distance_nm must not be negative");
        if (r.distance_flown_nm < 0.0)     return ValidationResult::fail("distance_flown_nm must not be negative");
        if (r.distance_flown_nm > r.trip_distance_nm)
            return ValidationResult::fail("distance_flown_nm must not exceed trip_distance_nm");
        if (r.fuel_on_board_kg < 0.0)      return ValidationResult::fail("fuel_on_board_kg must not be negative");
        if (r.ground_speed_knots <= 0.0)   return ValidationResult::fail("ground_speed_knots must be greater than zero");
        if (r.cruise_flow_kg_per_hour <= 0.0)  return ValidationResult::fail("cruise_flow_kg_per_hour must be greater than zero");
        if (r.holding_flow_kg_per_hour <= 0.0) return ValidationResult::fail("holding_flow_kg_per_hour must be greater than zero");

        if (!is_supported_aircraft_weight(r.aircraft_weight_kg)) {
            return ValidationResult::fail(
                "aircraft_weight_kg must be 5000, 20000 or 100000");
        }

        return ValidationResult::ok();
    }

    //

    bool is_supported_aircraft_weight(double aircraft_weight_kg) {
        return aircraft_weight_kg == kLightAircraftWeightKg ||
            aircraft_weight_kg == kMediumAircraftWeightKg ||
            aircraft_weight_kg == kHeavyAircraftWeightKg;
    }

    WeightClass classify_aircraft_weight(double aircraft_weight_kg) {
        if (aircraft_weight_kg == kLightAircraftWeightKg) {
            return WeightClass::Light;
        }

        if (aircraft_weight_kg == kHeavyAircraftWeightKg) {
            return WeightClass::Heavy;
        }

        return WeightClass::Medium;
    }

    double weight_burn_multiplier(double aircraft_weight_kg) {
        switch (classify_aircraft_weight(aircraft_weight_kg)) {
        case WeightClass::Light:
            return kLightBurnMultiplier;
        case WeightClass::Medium:
            return kMediumBurnMultiplier;
        case WeightClass::Heavy:
            return kHeavyBurnMultiplier;
        }
        return kMediumBurnMultiplier;
    }

    double round_up_to_whole_kg(double quantity_kg) {
        const double rounded = std::ceil(quantity_kg - kRoundingNoiseKg);
        return rounded == 0.0 ? 0.0 : rounded;  // avoid printing -0
    }

    FuelCalculation calculate_fuel(const FlightRecord& r) {
        FuelCalculation c;

        const double multiplier =
            weight_burn_multiplier(r.aircraft_weight_kg);

        const double effective_cruise_flow =
            r.cruise_flow_kg_per_hour * multiplier;

        const double effective_holding_flow =
            r.holding_flow_kg_per_hour * multiplier;

        const double remaining_distance_nm =
            r.trip_distance_nm - r.distance_flown_nm;

        c.trip_fuel_to_go_kg =
            remaining_distance_nm / r.ground_speed_knots *
            effective_cruise_flow;

        c.alternate_fuel_kg = round_up_to_whole_kg(
            r.alternate_distance_nm / r.ground_speed_knots *
            effective_cruise_flow);

        const double five_percent_kg =
            kContingencyFraction * c.trip_fuel_to_go_kg;

        const double five_minutes_kg =
            effective_holding_flow *
            (kContingencyMinHoldingMinutes / kMinutesPerHour);

        c.contingency_fuel_kg =
            round_up_to_whole_kg(
                std::max(five_percent_kg, five_minutes_kg));

        c.final_reserve_fuel_kg = round_up_to_whole_kg(
            effective_holding_flow *
            (kFinalReserveHoldingMinutes / kMinutesPerHour));

        c.predicted_fuel_at_destination_kg =
            r.fuel_on_board_kg - c.trip_fuel_to_go_kg;

        c.diversion_protected_fuel_kg =
            c.alternate_fuel_kg + c.final_reserve_fuel_kg;

        c.protected_fuel_kg =
            c.contingency_fuel_kg +
            c.diversion_protected_fuel_kg;

        return c;
    }

}  // namespace afrm