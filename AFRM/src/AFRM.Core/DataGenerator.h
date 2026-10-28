// DataGenerator.h
#pragma once

#include <cstdint>
#include <ostream>
#include <vector>

#include "Config.h"
#include "Models.h"

namespace afrm {

    // 7 fixed boundary records (exactly on, and 1 kg either side of, each threshold),
    // then `record_count` random flights whose target status cycles
    // NOMINAL, CAUTION, MINIMUM FUEL, FUEL EMERGENCY. Same seed gives the same data
    // with the same compiler and standard library. No status is stored; it is always
    // calculated afterwards by assess_fuel_status().
    std::vector<FlightRecord> generate_flights(std::uint32_t seed = kDefaultSeed,
        int record_count = kDefaultRecordCount);

    // Writes the header and one row per record, each line ending in '\n'.
    void write_flights_csv(std::ostream& out, const std::vector<FlightRecord>& records);

}  // namespace afrm