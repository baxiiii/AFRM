// Config.h - central assumptions, thresholds and constants.

#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace afrm {

    // Fuel criteria

    struct FuelCriteria {
        // Contingency = max(fraction * trip fuel to go, N minutes of holding fuel).
        double contingency_fraction = 0.05;

        double contingency_minimum_holding_minutes = 5.0;

        double final_reserve_holding_minutes = 30.0;

        // When true, contingency, alternate and final reserve are rounded UP to the
        // next whole kg before being stored and compared (see FuelCalculation in
        // Models.h). Rounding up only makes the requirement stricter.
        // Tests of the raw formulas set this to false to compare exact values.
        bool round_required_fuel_up = true;
    };

    inline constexpr double kMinutesPerHour = 60.0;

    // Guards against floating-point noise (e.g. 150.00000000000003) pushing a value
    // that is mathematically a whole number up by a full kilogram when rounding.
    // 1e-9 kg is far below any meaningful fuel quantity.
    inline constexpr double kRoundingNoiseKg = 1e-9;

    // CSV format

    inline constexpr std::string_view kCsvHeader =
        "flight_id,trip_distance_nm,alternate_distance_nm,distance_flown_nm,"
        "fuel_on_board_kg,ground_speed_knots,cruise_flow_kg_per_hour,holding_flow_kg_per_hour";

    inline constexpr std::size_t kCsvColumnCount = 8;

    // Relative to the executable's directory (the build copies the dataset there).
    inline constexpr std::string_view kDefaultDatasetRelativePath = "data/flights.csv";

    // Data generation

    struct GeneratorSettings {
        std::uint32_t seed = 20260601;   // fixed default seed; arbitrary but documented
        int records_per_status = 5;      // random flights generated for each of the four statuses
    };

}  // namespace afrm