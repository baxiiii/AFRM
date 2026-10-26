// Models.h - plain data types shared by every AFRM component.
// Units: kilograms (kg), nautical miles (nm), hours (h); 1 knot = 1 nm/h.

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace afrm {

    struct FlightRecord {
        std::string flight_id;
        double trip_distance_nm = 0.0;
        double alternate_distance_nm = 0.0;
        double distance_flown_nm = 0.0;
        double fuel_on_board_kg = 0.0;
        double ground_speed_knots = 0.0;
        double cruise_flow_kg_per_hour = 0.0;
        double holding_flow_kg_per_hour = 0.0;
    };

    struct ValidationResult {
        bool is_valid = true;
        std::string message;  // empty when valid

        static ValidationResult ok() { return { true, {} }; }
        static ValidationResult fail(std::string reason) { return { false, std::move(reason) }; }
    };

    // Only the values the assessment and report need.
    // Contingency, alternate and final reserve are rounded UP to whole kg and the
    // two totals are sums of those rounded values, so rounding can only make a
    // marginal case stricter. Trip fuel and PFD are not rounded.
    struct FuelCalculation {
        double trip_fuel_to_go_kg = 0.0;
        double alternate_fuel_kg = 0.0;
        double contingency_fuel_kg = 0.0;
        double final_reserve_fuel_kg = 0.0;
        double predicted_fuel_at_destination_kg = 0.0;  // fuel on board - trip fuel to go
        double protected_fuel_kg = 0.0;                 // contingency + alternate + final reserve
        double diversion_protected_fuel_kg = 0.0;       // alternate + final reserve
    };

    enum class FuelStatus { Nominal, Caution, MinimumFuel, FuelEmergency };

    inline std::string_view to_string(FuelStatus status) {
        switch (status) {
        case FuelStatus::Nominal:       return "NOMINAL";
        case FuelStatus::Caution:       return "CAUTION";
        case FuelStatus::MinimumFuel:   return "MINIMUM FUEL";
        case FuelStatus::FuelEmergency: return "FUEL EMERGENCY";
        }
        return "UNKNOWN";
    }

    struct FuelAssessment {
        FuelStatus status = FuelStatus::FuelEmergency;  // fails safe
        std::string explanation;
    };

    struct RejectedRow {
        std::size_t line_number = 0;  // 1-based; header is line 1
        std::string reason;
    };

    struct LoadResult {
        std::string file_error;  // empty if the file opened and the header was right
        std::vector<FlightRecord> records;  // valid rows only
        std::vector<RejectedRow> rejected;
    };

}  // namespace afrm