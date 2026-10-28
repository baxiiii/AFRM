// CsvLoader.cpp
#include "CsvLoader.h"

#include <charconv>
#include <cstddef>
#include <fstream>
#include <iterator>
#include <string>
#include <system_error>
#include <utility>
#include <vector>
#include "pch.h"

#include "Calculations.h"
#include "Config.h"

namespace afrm {

    namespace {

        std::string_view trim(std::string_view text) {
            constexpr std::string_view kWhitespace = " \t\r\n";
            const auto first = text.find_first_not_of(kWhitespace);
            if (first == std::string_view::npos) return {};
            const auto last = text.find_last_not_of(kWhitespace);
            return text.substr(first, last - first + 1);
        }

        std::vector<std::string_view> split_on_commas(std::string_view line) {
            std::vector<std::string_view> fields;
            std::size_t start = 0;
            while (true) {
                const auto comma = line.find(',', start);
                if (comma == std::string_view::npos) {
                    fields.push_back(line.substr(start));
                    return fields;
                }
                fields.push_back(line.substr(start, comma - start));
                start = comma + 1;
            }
        }

        // from_chars always uses '.' as the decimal point, whatever the system locale.
        bool parse_double(std::string_view text, double& value) {
            const char* const end = text.data() + text.size();
            const auto result = std::from_chars(text.data(), end, value);
            return !text.empty() && result.ec == std::errc{} && result.ptr == end;
        }

    }  // namespace

    ValidationResult parse_flight_row(std::string_view line, FlightRecord& record) {
        const auto fields = split_on_commas(line);
        if (fields.size() != kCsvColumnCount) {
            return ValidationResult::fail("expected " + std::to_string(kCsvColumnCount) +
                " fields but found " + std::to_string(fields.size()));
        }

        static constexpr const char* names[] = {
            "trip_distance_nm", "alternate_distance_nm", "distance_flown_nm", "fuel_on_board_kg",
            "ground_speed_knots", "cruise_flow_kg_per_hour", "holding_flow_kg_per_hour" };
        double v[7] = {};
        for (std::size_t i = 0; i < 7; ++i) {
            if (!parse_double(trim(fields[i + 1]), v[i])) {
                return ValidationResult::fail(std::string(names[i]) + " is missing or not a number");
            }
        }

        record = FlightRecord{ std::string(trim(fields[0])), v[0], v[1], v[2], v[3], v[4], v[5], v[6] };
        return ValidationResult::ok();
    }

    LoadResult load_flights_from_stream(std::istream& input) {
        LoadResult result;
        std::string line;

        if (!std::getline(input, line) || trim(line) != kCsvHeader) {
            result.file_error = "file is empty or the header row is not the expected one";
            return result;
        }

        std::size_t line_number = 1;  // the header
        while (std::getline(input, line)) {
            ++line_number;
            if (trim(line).empty()) continue;  // skip blank lines

            FlightRecord record;
            ValidationResult status = parse_flight_row(line, record);
            if (status.is_valid) status = validate_flight_record(record);

            if (status.is_valid) {
                result.records.push_back(std::move(record));
            }
            else {
                result.rejected.push_back({ line_number, status.message });
            }
        }
        return result;
    }

    LoadResult load_flights_from_file(const std::filesystem::path& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            LoadResult result;
            result.file_error = "cannot open file: " + path.string();
            return result;
        }
        return load_flights_from_stream(file);
    }

}  // namespace afrm