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

    constexpr std::size_t kBoundaryCount = 21;
    constexpr std::size_t kMediumBoundaryCount = 7;

    std::string csv_text(const std::vector<FlightRecord>& records) {
        std::ostringstream out;
        write_flights_csv(out, records);
        return out.str();
    }

    FuelStatus status_of(const FlightRecord& record) {
        EXPECT_TRUE(validate_flight_record(record).is_valid)
            << record.flight_id;

        return assess_fuel_status(calculate_fuel(record)).status;
    }

    std::size_t count_records_with_prefix(
        const std::vector<FlightRecord>& records,
        const std::string& prefix) {
        std::size_t count = 0;

        for (const FlightRecord& record : records) {
            if (record.flight_id.rfind(prefix, 0) == 0) {
                ++count;
            }
        }

        return count;
    }

} // namespace

TEST(DataGenerator, REQ_DATA_01_SameSeedGivesIdenticalData) {
    EXPECT_EQ(
        csv_text(generate_flights()),
        csv_text(generate_flights()));

    EXPECT_EQ(
        csv_text(generate_flights(42, 10)),
        csv_text(generate_flights(42, 10)));
}

TEST(DataGenerator, REQ_DATA_01_DifferentSeedsGiveDifferentData) {
    EXPECT_NE(
        csv_text(generate_flights(1, 20)),
        csv_text(generate_flights(2, 20)));
}

TEST(DataGenerator, REQ_DATA_01_RecordCountFollowsArgument) {
    EXPECT_EQ(
        generate_flights(kDefaultSeed, kDefaultRecordCount).size(),
        kBoundaryCount +
        static_cast<std::size_t>(kDefaultRecordCount));

    EXPECT_EQ(
        generate_flights(kDefaultSeed, 0).size(),
        kBoundaryCount);
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
    std::array<std::size_t, 4> counts{};

    // Only random records count here.
    for (std::size_t i = kBoundaryCount;
        i < records.size();
        ++i) {
        ++counts[static_cast<std::size_t>(
            status_of(records[i]))];
    }

    // Twenty random records cycle through the four target statuses.
    for (std::size_t count : counts) {
        EXPECT_EQ(
            count,
            static_cast<std::size_t>(kDefaultRecordCount) / 4);
    }
}

TEST(DataGenerator, REQ_STATUS_01_BoundaryRecordsHitTheThresholds) {
    const std::vector<FlightRecord> records = generate_flights();
    ASSERT_GE(records.size(), kMediumBoundaryCount);

    struct Expected {
        double pfd_kg;
        FuelStatus status;
    };

    const Expected expected[kMediumBoundaryCount] = {
        {2460.0, FuelStatus::Nominal},
        {2459.0, FuelStatus::Caution},
        {2280.0, FuelStatus::Caution},
        {2279.0, FuelStatus::MinimumFuel},
        {1080.0, FuelStatus::MinimumFuel},
        {1079.0, FuelStatus::FuelEmergency},
        {-600.0, FuelStatus::FuelEmergency}
    };

    for (std::size_t i = 0; i < kMediumBoundaryCount; ++i) {
        SCOPED_TRACE(records[i].flight_id);

        EXPECT_EQ(
            records[i].aircraft_weight_kg,
            kMediumAircraftWeightKg);

        const FuelCalculation calculation =
            calculate_fuel(records[i]);

        EXPECT_NEAR(
            calculation.predicted_fuel_at_destination_kg,
            expected[i].pfd_kg,
            kTolerance);

        EXPECT_EQ(
            assess_fuel_status(calculation).status,
            expected[i].status);
    }
}

TEST(DataGenerator, REQ_DATA_01_CsvHasHeaderAndNineColumns) {
    const std::string csv = csv_text(generate_flights());
    std::istringstream lines(csv);
    std::string line;

    ASSERT_TRUE(std::getline(lines, line));
    EXPECT_EQ(line, std::string(kCsvHeader));

    while (std::getline(lines, line)) {
        EXPECT_EQ(
            std::count(line.begin(), line.end(), ','),
            8)
            << line;
    }
}

TEST(DataGenerator, REQ_DATA_01_GeneratedCsvLoadsBackWithoutRejections) {
    const std::vector<FlightRecord> original =
        generate_flights();

    std::istringstream input(csv_text(original));
    const LoadResult loaded =
        load_flights_from_stream(input);

    ASSERT_TRUE(loaded.file_error.empty())
        << loaded.file_error;

    EXPECT_TRUE(loaded.rejected.empty());

    ASSERT_EQ(
        loaded.records.size(),
        original.size());

    for (std::size_t i = 0;
        i < original.size();
        ++i) {
        EXPECT_EQ(
            loaded.records[i].flight_id,
            original[i].flight_id);

        EXPECT_EQ(
            loaded.records[i].fuel_on_board_kg,
            original[i].fuel_on_board_kg);

        EXPECT_EQ(
            loaded.records[i].aircraft_weight_kg,
            original[i].aircraft_weight_kg);
    }
}

TEST(DataGenerator, REQ_WEIGHT_01_BoundaryRecordsPerClass) {
    const std::vector<FlightRecord> records =
        generate_flights(kDefaultSeed, 20);

    ASSERT_EQ(records.size(), 41u);

    int medium_count = 0;
    int light_count = 0;
    int heavy_count = 0;

    for (const FlightRecord& record : records) {
        if (record.flight_id.rfind("BND-L-", 0) == 0) {
            ++light_count;
        }
        else if (record.flight_id.rfind("BND-H-", 0) == 0) {
            ++heavy_count;
        }
        else if (record.flight_id.rfind("BND-", 0) == 0) {
            ++medium_count;
        }
    }

    EXPECT_EQ(medium_count, 7);
    EXPECT_EQ(light_count, 7);
    EXPECT_EQ(heavy_count, 7);

    bool saw_light = false;
    bool saw_medium = false;
    bool saw_heavy = false;

    for (const FlightRecord& record : records) {
        saw_light |=
            record.aircraft_weight_kg ==
            kLightAircraftWeightKg;

        saw_medium |=
            record.aircraft_weight_kg ==
            kMediumAircraftWeightKg;

        saw_heavy |=
            record.aircraft_weight_kg ==
            kHeavyAircraftWeightKg;
    }

    EXPECT_TRUE(saw_light);
    EXPECT_TRUE(saw_medium);
    EXPECT_TRUE(saw_heavy);
}

TEST(DataGenerator, REQ_WEIGHT_02_AllThreeClassesGenerated) {
    const std::vector<FlightRecord> records =
        generate_flights(kDefaultSeed, 20);

    bool saw_light = false;
    bool saw_medium = false;
    bool saw_heavy = false;

    for (const FlightRecord& record : records) {
        saw_light |=
            record.aircraft_weight_kg ==
            kLightAircraftWeightKg;

        saw_medium |=
            record.aircraft_weight_kg ==
            kMediumAircraftWeightKg;

        saw_heavy |=
            record.aircraft_weight_kg ==
            kHeavyAircraftWeightKg;
    }

    EXPECT_TRUE(saw_light);
    EXPECT_TRUE(saw_medium);
    EXPECT_TRUE(saw_heavy);
}