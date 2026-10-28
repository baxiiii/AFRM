// main.cpp - console front end for the AFRM prototype.
//
// Educational prototype only. NOT an operational flight-planning system and
// NOT approved aviation guidance. All data are fictional and simulated.
//
// Usage:
//   AFRM                         report on data/flights.csv next to the executable
//   AFRM --generate-data [path]  write generated data (default: data/flights.generated.csv
//                                next to the executable)

#include <array>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

#include "C:\Users\ibrah\OneDrive\Documents\University\REPOs\AFRM\AFRM\src\AFRM.Core\Assessment.h"
#include "C:\Users\ibrah\OneDrive\Documents\University\REPOs\AFRM\AFRM\src\AFRM.Core\Calculations.h"
#include "C:\Users\ibrah\OneDrive\Documents\University\REPOs\AFRM\AFRM\src\AFRM.Core\Config.h"
#include "C:\Users\ibrah\OneDrive\Documents\University\REPOs\AFRM\AFRM\src\AFRM.Core\CsvLoader.h"
#include "C:\Users\ibrah\OneDrive\Documents\University\REPOs\AFRM\AFRM\src\AFRM.Core\DataGenerator.h"
#include "C:\Users\ibrah\OneDrive\Documents\University\REPOs\AFRM\AFRM\src\AFRM.Core\Models.h"

namespace {

    namespace fs = std::filesystem;

    void print_flight(const afrm::FlightRecord& record,
        const afrm::FuelCalculation& c,
        const afrm::FuelAssessment& assessment) {
        const double remaining_nm = record.trip_distance_nm - record.distance_flown_nm;
        std::cout << std::format(
            "{} - {}\n"
            "  Fuel on board:          {:>9.1f} kg\n"
            "  Remaining distance:     {:>9.1f} nm\n"
            "  Trip fuel to go:        {:>9.1f} kg\n"
            "  Contingency fuel:       {:>9.0f} kg\n"
            "  Alternate fuel:         {:>9.0f} kg\n"
            "  Final reserve fuel:     {:>9.0f} kg\n"
            "  Predicted fuel at dest: {:>9.1f} kg\n"
            "  Reason: {}\n\n",
            record.flight_id, afrm::to_string(assessment.status), record.fuel_on_board_kg,
            remaining_nm, c.trip_fuel_to_go_kg, c.contingency_fuel_kg, c.alternate_fuel_kg,
            c.final_reserve_fuel_kg, c.predicted_fuel_at_destination_kg, assessment.explanation);
    }

    int run_report(const fs::path& csv_path) {
        std::cout << "AFRM - Aircraft Fuel and Range Management\n"
            "EDUCATIONAL PROTOTYPE ONLY: simulated data, simplified assumptions. Not an\n"
            "operational flight-planning system and not approved aviation guidance.\n"
            "Statuses are indicative flags, not formal pilot declarations.\n\n"
            << "Dataset: " << csv_path.string() << "\n\n";

        const afrm::LoadResult loaded = afrm::load_flights_from_file(csv_path);
        if (!loaded.file_error.empty()) {
            std::cerr << "Error: " << loaded.file_error << "\n";
            return 1;
        }

        std::array<std::size_t, 4> status_counts{};  // indexed by FuelStatus

        std::cout << "--- Flights ---\n\n";
        for (const afrm::FlightRecord& record : loaded.records) {
            const afrm::FuelCalculation calc = afrm::calculate_fuel(record);
            const afrm::FuelAssessment assessment = afrm::assess_fuel_status(calc);
            ++status_counts[static_cast<std::size_t>(assessment.status)];
            print_flight(record, calc, assessment);
        }

        if (!loaded.rejected.empty()) {
            std::cout << "--- Rejected rows ---\n\n";
            for (const afrm::RejectedRow& row : loaded.rejected) {
                std::cout << std::format("  line {}: {}\n", row.line_number, row.reason);
            }
            std::cout << "\n";
        }

        std::cout << "--- Summary ---\n"
            << std::format("  Records read:    {}\n", loaded.records.size() + loaded.rejected.size())
            << std::format("  Valid processed: {}\n", loaded.records.size())
            << std::format("  Rejected:        {}\n", loaded.rejected.size())
            << "  Status counts:\n";
        for (const afrm::FuelStatus status : {afrm::FuelStatus::Nominal, afrm::FuelStatus::Caution,
            afrm::FuelStatus::MinimumFuel, afrm::FuelStatus::FuelEmergency}) {
            std::cout << std::format("    {:<15}{}\n", afrm::to_string(status),
                status_counts[static_cast<std::size_t>(status)]);
        }
        return 0;
    }

    int run_generate(const fs::path& output_path) {
        std::ofstream file(output_path, std::ios::binary | std::ios::trunc);  // binary: '\n' line endings
        if (!file) {
            std::cerr << "Error: cannot open file for writing: " << output_path.string() << "\n";
            return 1;
        }

        const auto records = afrm::generate_flights();  // fixed default seed and count
        afrm::write_flights_csv(file, records);
        file.close();
        if (!file) {
            std::cerr << "Error: failed while writing: " << output_path.string() << "\n";
            return 1;
        }

        std::cout << std::format("Wrote {} generated records (seed {}) to:\n  {}\n",
            records.size(), afrm::kDefaultSeed, output_path.string());
        return 0;
    }

}  // namespace

int main(int argc, char* argv[]) {
    try {
        // Folder containing the executable, so the default paths work from any working directory.
        const fs::path exe_dir = fs::absolute(argv[0]).parent_path();

        if (argc == 1) {
            return run_report(exe_dir / std::string(afrm::kDefaultDatasetRelativePath));
        }

        if (std::string(argv[1]) == "--generate-data" && argc <= 3) {
            if (argc == 3) {
                return run_generate(argv[2]);  // parent folder must already exist
            }
            const fs::path default_path = exe_dir / "data" / "flights.generated.csv";
            fs::create_directories(default_path.parent_path());
            return run_generate(default_path);
        }

        std::cerr << "Usage:\n"
            "  AFRM                         report on data/flights.csv\n"
            "  AFRM --generate-data [path]  write generated data\n";
        return 2;
    }
    catch (const std::exception& error) {
        std::cerr << "Unexpected error: " << error.what() << "\n";
        return 1;
    }
}