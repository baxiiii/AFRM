// Models.h - plain data types shared by every AFRM component.
//
// Units used throughout: kilograms (kg), nautical miles (nm), hours (h).
// One knot = one nautical mile per hour. Units appear in member names.

#pragma once
#include <utility>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace afrm {

    // Input data

    // These are the eight CSV columns.
    struct FlightRecord {
        std::string flight_id;
        double trip_distance_nm = 0.0;           // total planned trip distance
        double alternate_distance_nm = 0.0;      // destination to alternate
        double distance_flown_nm = 0.0;          // distance flown at the checkpoint
        double fuel_on_board_kg = 0.0;           // fuel on board at the checkpoint
        double ground_speed_knots = 0.0;         // constant cruise ground speed
        double cruise_flow_kg_per_hour = 0.0;    // constant cruise fuel flow
        double holding_flow_kg_per_hour = 0.0;   // constant holding fuel flow
    };

    // Validation

    // Outcome of validating a record or a file. Used instead of exceptions for
    // ordinary control flow (std::expected is C++23 and is not used).
    struct ValidationResult {
        bool is_valid = true;
        std::string message;  // empty when valid, otherwise a clear reason

        static ValidationResult ok() { return { true, {} }; }
        static ValidationResult fail(std::string reason) { return { false, std::move(reason) }; }
    };

    // ---------------------------------------------------------------------------
    // Calculation results
    // ---------------------------------------------------------------------------

    // Results for one valid flight. See docs/requirements.md for the formulas.
    //
    // Rounding rule (applied in Calculations.cpp, controlled by FuelCriteria):
    //   * The REQUIRED quantities (contingency, alternate, final reserve) are
    //     rounded UP to the next whole kg before they are stored, and the two
    //     protection totals are the sums of those stored values.
    //   * Trip fuel to go and PFD are NOT rounded.
    //   * The status comparison uses these stored values, so rounding can only
    //     make a marginal case look less safe, never safer.
    struct FuelCalculation {
        double remaining_distance_nm = 0.0;
        double remaining_trip_time_hours = 0.0;
        double trip_fuel_to_go_kg = 0.0;
        double alternate_fuel_kg = 0.0;
        double contingency_fuel_kg = 0.0;
        double final_reserve_fuel_kg = 0.0;
        double predicted_fuel_at_destination_kg = 0.0;  // fuel_on_board - trip_fuel_to_go
        double protected_fuel_kg = 0.0;                 // contingency + alternate + final reserve
        double diversion_protected_fuel_kg = 0.0;       // alternate + final reserve
    };

    // Calculation outcome: if validation.is_valid is false, values must not be used.
    struct FuelCalculationResult {
        ValidationResult validation;
        FuelCalculation values;
    };

    // ---------------------------------------------------------------------------
    // Assessment
    // ---------------------------------------------------------------------------

    enum class FuelStatus {
        Nominal,
        Caution,
        MinimumFuel,
        FuelEmergency
    };

    // Exact display labels required by the specification.
    inline std::string_view to_string(FuelStatus status) {
        switch (status) {
        case FuelStatus::Nominal:       return "NOMINAL";
        case FuelStatus::Caution:       return "CAUTION";
        case FuelStatus::MinimumFuel:   return "MINIMUM FUEL";
        case FuelStatus::FuelEmergency: return "FUEL EMERGENCY";
        }
        return "UNKNOWN";  // unreachable for valid enum values
    }

    struct FuelAssessment {
        FuelStatus status = FuelStatus::FuelEmergency;  // most severe by default
        std::string explanation;
    };

    // CSV loading results

    struct RejectedRecord {
        std::size_t line_number = 0;  // 1-based line in the file (header is line 1)
        std::string raw_line;
        std::string reason;
    };

    struct LoadResult {
        ValidationResult file_status;           // invalid if the file cannot be opened or the header is wrong
        std::vector<FlightRecord> records;      // valid records only
        std::vector<RejectedRecord> rejected;
        std::size_t data_rows_read = 0;         // non-empty data rows (valid + rejected)
    };

}  // namespace afrm