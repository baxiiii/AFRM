// test_data_generator.cpp - repeatable generation, boundary records and the generator-to-loader link.
// Repeatability is checked on this machine and toolchain only.

#include <gtest/gtest.h>
#include "pch.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "Assessment.h"
#include "Calculations.h"
#include "Config.h"
#include "CsvLoader.h"
#include "DataGenerator.h"
#include "test_helpers.h"

using namespace afrm;
using namespace afrm_test;

namespace {

    constexpr std::size_t kBoundaryCount = 7;  // fixed records emitted before the random ones

    std::string csv_text(const std::vector<FlightRecord>& records) {
        std::ostringstream out;
        write_flights_csv(out, records);
        return out.str();
    }

    FuelStatus status_of(const FlightRecord& record) {
        EXPECT_TRUE(validate_flight_record(record).is_valid) << record.flight_id;
        return assess_fuel_status(calculate_fuel(record)).status;
    }

}  // namespace

TEST(DataGenerator, REQ_DATA_01_SameSeedGivesIdenticalData) {
    EXPECT_EQ(csv_text(generate_flights()), csv_text(generate_flights()));
    EXPECT_EQ(csv_text(generate_flights(42, 10)), csv_text(generate_flights(42, 10)));
}

TEST(DataGenerator, REQ_DATA_01_DifferentSeedsGiveDifferentData) {
    EXPECT_NE(csv_text(generate_flights(1, 20)), csv_text(generate_flights(2, 20)));
}

TEST(DataGenerator, REQ_DATA_01_RecordCountFollowsArgument) {
    EXPECT_EQ(generate_flights(kDefaultSeed, kDefaultRecordCount).size(),
        kBoundaryCount + static_cast<std::size_t>(kDefaultRecordCount));
    EXPECT_EQ(generate_flights(kDefaultSeed, 0).size(), kBoundaryCount);
}

TEST(DataGenerator, REQ_DATA_01_AllRecordsAreValidAndUniquelyNamed) {
    const std::vector<FlightRecord> records = generate_flights();
    std::set<std::string> ids;
    for (const FlightRecord& record : records) {
        SCOPED_TRACE(record.flight_id);
        EXPECT_TRUE(validate_flight_record(record).is_valid);
        ids.insert(record.flight_id);
    }
    EXPECT_EQ(ids.size(), records.size());
}

TEST(DataGenerator, REQ_DATA_01_GeneratedDataExercisesAllFourStatuses) {
    const std::vector<FlightRecord> records = generate_flights();
    std::array<std::size_t, 4> counts{};  // random records only
    for (std::size_t i = kBoundaryCount; i < records.size(); ++i) {
        ++counts[static_cast<std::size_t>(status_of(records[i]))];
    }
    // 20 random records cycle through the four targets, so 5 each.
    for (std::size_t count : counts) {
        EXPECT_EQ(count, static_cast<std::size_t>(kDefaultRecordCount) / 4);
    }
}

TEST(DataGenerator, REQ_STATUS_01_BoundaryRecordsHitTheThresholds) {
    const std::vector<FlightRecord> records = generate_flights();
    ASSERT_GE(records.size(), kBoundaryCount);

    struct Expected {
        double pfd_kg;
        FuelStatus status;
    };
    const Expected expected[kBoundaryCount] = {
        {2460.0, FuelStatus::Nominal},       {2459.0, FuelStatus::Caution},
        {2280.0, FuelStatus::Caution},       {2279.0, FuelStatus::MinimumFuel},
        {1080.0, FuelStatus::MinimumFuel},   {1079.0, FuelStatus::FuelEmergency},
        {-600.0, FuelStatus::FuelEmergency},
    };
    for (std::size_t i = 0; i < kBoundaryCount; ++i) {
        SCOPED_TRACE(records[i].flight_id);
        const FuelCalculation c = calculate_fuel(records[i]);
        EXPECT_NEAR(c.predicted_fuel_at_destination_kg, expected[i].pfd_kg, kTolerance);
        EXPECT_EQ(assess_fuel_status(c).status, expected[i].status);
    }
}

TEST(DataGenerator, REQ_DATA_01_CsvHasHeaderAndEightColumns) {
    const std::string csv = csv_text(generate_flights());
    std::istringstream lines(csv);
    std::string line;

    ASSERT_TRUE(std::getline(lines, line));
    EXPECT_EQ(line, std::string(kCsvHeader));
    while (std::getline(lines, line)) {
        EXPECT_EQ(std::count(line.begin(), line.end(), ','), 7) << line;
    }
}

TEST(DataGenerator, REQ_DATA_01_GeneratedCsvLoadsBackWithoutRejections) {
    const std::vector<FlightRecord> original = generate_flights();
    std::istringstream input(csv_text(original));
    const LoadResult loaded = load_flights_from_stream(input);

    ASSERT_TRUE(loaded.file_error.empty()) << loaded.file_error;
    EXPECT_TRUE(loaded.rejected.empty());
    ASSERT_EQ(loaded.records.size(), original.size());
    for (std::size_t i = 0; i < original.size(); ++i) {
        EXPECT_EQ(loaded.records[i].flight_id, original[i].flight_id);
        EXPECT_EQ(loaded.records[i].fuel_on_board_kg, original[i].fuel_on_board_kg);
    }
}