// CsvLoader.h
#pragma once

#include <filesystem>
#include <istream>
#include <string_view>

#include "C:\Users\ibrah\OneDrive\Documents\University\REPOs\AFRM\AFRM\src\AFRM.Core\Models.h"

namespace afrm {

	// Checks field count and that each number parses fully. Range rules are left to
	// validate_flight_record(). `record` is only changed on success.
	// Limitation: fields are split on every comma (no quoted fields).
	ValidationResult parse_flight_row(std::string_view line, FlightRecord& record);

	// Valid rows go to `records`; bad rows go to `rejected` and loading continues.
	// A missing or wrong header sets file_error.
	LoadResult load_flights_from_stream(std::istream& input);
	LoadResult load_flights_from_file(const std::filesystem::path& path);

}  // namespace afrm#pragma once
