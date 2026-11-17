// test_csv_loader.cpp - R6, loader side: invalid records are rejected safely.

#include <gtest/gtest.h>
#include "pch.h"

#include <sstream>
#include <string>
#include <vector>

#include "Config.h"
#include "CsvLoader.h"
#include "test_helpers.h"

using namespace afrm;
using namespace afrm_test;

namespace {

    const std::string kValidRow =
        "AF1001,1000,200,400,6060,400,2400,2160,20000";

    std::string header_line() { return std::string(kCsvHeader) + "\n"; }

    LoadResult load_text(const std::string& text) {
        std::istringstream input(text);
        return load_flights_from_stream(input);
    }

}  // namespace

TEST(CsvLoader, REQ_DATA_01_ValidRowsLoaded) {
    const LoadResult result =
        load_text(header_line() + kValidRow + "\n" + "AF1002,800,100,300,5000,420,2520,2100,20000\n");
    ASSERT_TRUE(result.file_error.empty()) << result.file_error;
    EXPECT_EQ(result.records.size(), 2u);
    EXPECT_TRUE(result.rejected.empty());

    const FlightRecord& r = result.records[0];
    EXPECT_EQ(r.flight_id, "AF1001");
    EXPECT_NEAR(r.trip_distance_nm, 1000.0, kTolerance);
    EXPECT_NEAR(r.alternate_distance_nm, 200.0, kTolerance);
    EXPECT_NEAR(r.distance_flown_nm, 400.0, kTolerance);
    EXPECT_NEAR(r.fuel_on_board_kg, 6060.0, kTolerance);
    EXPECT_NEAR(r.ground_speed_knots, 400.0, kTolerance);
    EXPECT_NEAR(r.cruise_flow_kg_per_hour, 2400.0, kTolerance);
    EXPECT_NEAR(r.holding_flow_kg_per_hour, 2160.0, kTolerance);
}

TEST(CsvLoader, REQ_DATA_01_DecimalsSpacesAndCrLfAccepted) {
    const LoadResult spaced = load_text(header_line() + "AF7, 1000.5 ,200,400,6060.25,400,2400,2160,20000\n");
    ASSERT_EQ(spaced.records.size(), 1u);
    EXPECT_NEAR(spaced.records[0].trip_distance_nm, 1000.5, kTolerance);

    const LoadResult crlf = load_text(std::string(kCsvHeader) + "\r\n" + kValidRow + "\r\n");
    ASSERT_TRUE(crlf.file_error.empty());
    EXPECT_EQ(crlf.records.size(), 1u);
}

TEST(CsvLoader, REQ_DATA_01_EachInvalidRowIsRejectedWithLineAndReason) {
    struct BadRow {
        const char* name;
        const char* row;
    };
    const std::vector<BadRow> rows = {
        {"too few fields", "AF1,1000,200,400,6060,400,2400,20000"},
        {"too many fields", "AF1,1000,200,400,6060,400,2400,2160,5,20000"},
        {"empty numeric field", "AF1,1000,200,400,,400,2400,2160,20000"},
        {"letters instead of number", "AF1,abc,200,400,6060,400,2400,2160,20000"},
        {"trailing characters", "AF1,1000x,200,400,6060,400,2400,2160,20000"},
        {"empty flight id", ",1000,200,400,6060,400,2400,2160,20000"},
        {"negative distance", "AF1,-1000,200,400,6060,400,2400,2160,20000"},
        {"distance flown exceeds trip", "AF1,1000,200,1500,6060,400,2400,2160,20000"},
        {"negative fuel", "AF1,1000,200,400,-5,400,2400,2160,20000"},
        {"zero speed", "AF1,1000,200,400,6060,0,2400,2160,20000"},
        {"zero cruise flow", "AF1,1000,200,400,6060,400,0,2160,20000"},
        {"zero holding flow", "AF1,1000,200,400,6060,400,2400,0,20000"},
        {"NaN fuel", "AF1,1000,200,400,nan,400,2400,2160,20000"},
        {"infinite distance", "AF1,inf,200,400,6060,400,2400,2160,20000"},
    };

    for (const BadRow& bad : rows) {
        SCOPED_TRACE(bad.name);
        const LoadResult result = load_text(header_line() + bad.row + "\n");
        EXPECT_TRUE(result.file_error.empty());
        EXPECT_TRUE(result.records.empty());
        ASSERT_EQ(result.rejected.size(), 1u);
        EXPECT_EQ(result.rejected[0].line_number, 2u);  // the header is line 1
        EXPECT_FALSE(result.rejected[0].reason.empty());
    }
}

TEST(CsvLoader, REQ_DATA_01_InvalidRowsDoNotLoseValidRows) {
    const std::string text = header_line() +
        kValidRow + "\n" +                         // line 2: valid
        "AF2,abc,200,400,6060,400,2400,2160,20000\n" +   // line 3: bad number
        "\n" +                                     // line 4: blank, skipped
        "AF3,1000,200,400,6000,400,2400,2160,20000\n" +  // line 5: valid
        "AF4,1000,200,400,6000,0,2400,2160,20000\n";     // line 6: zero speed
    const LoadResult result = load_text(text);

    ASSERT_TRUE(result.file_error.empty());
    ASSERT_EQ(result.records.size(), 2u);
    EXPECT_EQ(result.records[0].flight_id, "AF1001");
    EXPECT_EQ(result.records[1].flight_id, "AF3");
    ASSERT_EQ(result.rejected.size(), 2u);
    EXPECT_EQ(result.rejected[0].line_number, 3u);
    EXPECT_EQ(result.rejected[1].line_number, 6u);
}

TEST(CsvLoader, REQ_DATA_01_BadOrMissingHeaderRejectsWholeFile) {
    const LoadResult wrong_header = load_text("flight,distance\n" + kValidRow + "\n");
    EXPECT_FALSE(wrong_header.file_error.empty());
    EXPECT_TRUE(wrong_header.records.empty());

    EXPECT_FALSE(load_text("").file_error.empty());
}

TEST(CsvLoader, REQ_DATA_01_MissingFileReportsError) {
    const LoadResult result = load_flights_from_file("afrm_tests_no_such_file.csv");
    EXPECT_FALSE(result.file_error.empty());
    EXPECT_TRUE(result.records.empty());
}

TEST(CsvLoader, REQ_WEIGHT_02_ValidWeightsLoaded) {
    const std::string text =
        header_line() +
        "LIGHT,1000,200,400,6060,400,2400,2160,5000\n"
        "MEDIUM,1000,200,400,6060,400,2400,2160,20000\n"
        "HEAVY,1000,200,400,6060,400,2400,2160,100000\n";

    const LoadResult result = load_text(text);

    ASSERT_TRUE(result.file_error.empty());
    ASSERT_EQ(result.records.size(), 3u);
    EXPECT_TRUE(result.rejected.empty());

    EXPECT_DOUBLE_EQ(
        result.records[0].aircraft_weight_kg,
        kLightAircraftWeightKg);

    EXPECT_DOUBLE_EQ(
        result.records[1].aircraft_weight_kg,
        kMediumAircraftWeightKg);

    EXPECT_DOUBLE_EQ(
        result.records[2].aircraft_weight_kg,
        kHeavyAircraftWeightKg);
}

TEST(CsvLoader,
    REQ_WEIGHT_02_UnsupportedWeightRejectedWithLineAndReason) {
    const char* unsupported_rows[] = {
        "AF1,1000,200,400,6060,400,2400,2160,15000",
        "AF1,1000,200,400,6060,400,2400,2160,",
        "AF1,1000,200,400,6060,400,2400,2160,heavy"
    };

    for (const char* row : unsupported_rows) {
        SCOPED_TRACE(row);

        const LoadResult result = load_text(header_line() + row + "\n");

        ASSERT_TRUE(result.file_error.empty());
        EXPECT_TRUE(result.records.empty());
        ASSERT_EQ(result.rejected.size(), 1u);
        EXPECT_EQ(result.rejected[0].line_number, 2u);
        EXPECT_FALSE(result.rejected[0].reason.empty());
        EXPECT_NE(
            result.rejected[0].reason.find("aircraft_weight_kg"),
            std::string::npos);
    }
}

TEST(CsvLoader, REQ_WEIGHT_02_NineColumnHeaderRequired) {
    const LoadResult result = load_text(
        header_line() + kValidRow + "\n");

    ASSERT_TRUE(result.file_error.empty());
    ASSERT_EQ(result.records.size(), 1u);
    EXPECT_DOUBLE_EQ(
        result.records[0].aircraft_weight_kg,
        kMediumAircraftWeightKg);
}

TEST(CsvLoader, REQ_WEIGHT_02_EightColumnFileRejected) {
    const std::string old_header =
        "flight_id,trip_distance_nm,alternate_distance_nm,"
        "distance_flown_nm,fuel_on_board_kg,ground_speed_knots,"
        "cruise_flow_kg_per_hour,holding_flow_kg_per_hour";

    const LoadResult result = load_text(
        old_header + "\n" +
        "AF1,1000,200,400,6060,400,2400,2160\n");

    EXPECT_FALSE(result.file_error.empty());
    EXPECT_TRUE(result.records.empty());
    EXPECT_TRUE(result.rejected.empty());
}