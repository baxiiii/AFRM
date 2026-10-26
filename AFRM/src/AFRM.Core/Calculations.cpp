// Calculations.cpp
#include "C:\Users\ibrah\OneDrive\Documents\University\REPOs\AFRM\AFRM\src\AFRM.Core\Calculations.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "C:\Users\ibrah\OneDrive\Documents\University\REPOs\AFRM\AFRM\src\AFRM.Core\Config.h"

namespace afrm {

    ValidationResult validate_flight_record(const FlightRecord& r) {
        if (r.flight_id.empty()) return ValidationResult::fail("flight_id is empty");

        // NaN passes every < and <= test, so check for finite values first.
        const double numbers[] = { r.trip_distance_nm, r.alternate_distance_nm, r.distance_flown_nm,
                                  r.fuel_on_board_kg, r.ground_speed_knots,
                                  r.cruise_flow_kg_per_hour, r.holding_flow_kg_per_hour };
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
        return ValidationResult::ok();
    }

    double round_up_to_whole_kg(double quantity_kg) {
        const double rounded = std::ceil(quantity_kg - kRoundingNoiseKg);
        return rounded == 0.0 ? 0.0 : rounded;  // avoid printing -0
    }

    FuelCalculation calculate_fuel(const FlightRecord& r) {
        FuelCalculation c;

        const double remaining_distance_nm = r.trip_distance_nm - r.distance_flown_nm;
        c.trip_fuel_to_go_kg = remaining_distance_nm / r.ground_speed_knots * r.cruise_flow_kg_per_hour;

        c.alternate_fuel_kg = round_up_to_whole_kg(
            r.alternate_distance_nm / r.ground_speed_knots * r.cruise_flow_kg_per_hour);

        // Contingency: the larger of 5% of trip fuel and 5 minutes of holding fuel.
        const double five_percent_kg = kContingencyFraction * c.trip_fuel_to_go_kg;
        const double five_minutes_kg =
            r.holding_flow_kg_per_hour * (kContingencyMinHoldingMinutes / kMinutesPerHour);
        c.contingency_fuel_kg = round_up_to_whole_kg(std::max(five_percent_kg, five_minutes_kg));

        c.final_reserve_fuel_kg = round_up_to_whole_kg(
            r.holding_flow_kg_per_hour * (kFinalReserveHoldingMinutes / kMinutesPerHour));

        c.predicted_fuel_at_destination_kg = r.fuel_on_board_kg - c.trip_fuel_to_go_kg;
        c.diversion_protected_fuel_kg = c.alternate_fuel_kg + c.final_reserve_fuel_kg;
        c.protected_fuel_kg = c.contingency_fuel_kg + c.diversion_protected_fuel_kg;
        return c;
    }

}  // namespace afrm