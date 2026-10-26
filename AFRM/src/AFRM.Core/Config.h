// Config.h - fixed assumptions and constants.
//
// Model limitations: no wind, constant ground speed, constant cruise and holding
// fuel flow, no climb/descent, no taxi fuel, no weight-dependent burn.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace afrm {

    // Contingency = max(5% of trip fuel to go, 5 minutes of holding fuel).
    inline constexpr double kContingencyFraction = 0.05;
    inline constexpr double kContingencyMinHoldingMinutes = 5.0;
    inline constexpr double kFinalReserveHoldingMinutes = 30.0;
    inline constexpr double kMinutesPerHour = 60.0;

    // Stops floating-point noise (e.g. 150.00000000000003) rounding up an extra kg.
    inline constexpr double kRoundingNoiseKg = 1e-9;

    inline constexpr std::string_view kCsvHeader =
        "flight_id,trip_distance_nm,alternate_distance_nm,distance_flown_nm,"
        "fuel_on_board_kg,ground_speed_knots,cruise_flow_kg_per_hour,holding_flow_kg_per_hour";
    inline constexpr std::size_t kCsvColumnCount = 8;
    inline constexpr std::string_view kDefaultDatasetRelativePath = "data/flights.csv";

    inline constexpr std::uint32_t kDefaultSeed = 20260601;
    inline constexpr int kDefaultRecordCount = 20;  // random flights, cycling through the 4 statuses

}  // namespace afrm