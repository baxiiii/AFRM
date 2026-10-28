// DataGenerator.cpp
#include "DataGenerator.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>  // C++20; std::format ignores the system locale
#include <fstream>
#include <random>
#include <utility>

#include "pch.h"
#include "Calculations.h"

namespace afrm {

    namespace {

        int pick(std::mt19937& rng, int low, int high) {
            return std::uniform_int_distribution<int>(low, high)(rng);
        }

        // Shared base flight with exact values: trip fuel 3600, alternate 1200,
        // contingency 180 (5% of trip fuel equals 5 min holding, so this is also the
        // crossover), final reserve 1080. Protected 2460, diversion protected 2280.
        FlightRecord boundary(const char* id, double fuel_on_board_kg) {
            return FlightRecord{ id, 1000.0, 200.0, 400.0, fuel_on_board_kg, 400.0, 2400.0, 2160.0 };
        }

        std::vector<FlightRecord> boundary_records() {
            return {
                boundary("BND-01", 6060.0),  // PFD 2460 = protected (NOMINAL threshold)
                boundary("BND-02", 6059.0),  // 1 kg below
                boundary("BND-03", 5880.0),  // PFD 2280 = diversion protected (CAUTION threshold)
                boundary("BND-04", 5879.0),
                boundary("BND-05", 4680.0),  // PFD 1080 = final reserve (MINIMUM FUEL threshold)
                boundary("BND-06", 4679.0),
                boundary("BND-07", 3000.0),  // negative PFD
            };
        }

        // Picks whole-kg fuel on board so the PFD lands inside the target status band,
        // at least 1 kg clear of each threshold.
        double choose_fuel_kg(std::mt19937& rng, const FlightRecord& base, FuelStatus target) {
            const FuelCalculation c = calculate_fuel(base);  // base has zero fuel, which is valid
            const int protected_kg = static_cast<int>(c.protected_fuel_kg);
            const int diversion_kg = static_cast<int>(c.diversion_protected_fuel_kg);
            const int reserve_kg = static_cast<int>(c.final_reserve_fuel_kg);
            const int trip_fuel_kg = static_cast<int>(std::ceil(c.trip_fuel_to_go_kg));

            int pfd_kg = 0;
            switch (target) {
            case FuelStatus::Nominal:       pfd_kg = protected_kg + pick(rng, 0, 1500); break;
            case FuelStatus::Caution:       pfd_kg = diversion_kg + pick(rng, 0, protected_kg - diversion_kg - 2); break;
            case FuelStatus::MinimumFuel:   pfd_kg = reserve_kg + pick(rng, 0, diversion_kg - reserve_kg - 2); break;
            case FuelStatus::FuelEmergency: pfd_kg = reserve_kg - 2 - pick(rng, 0, reserve_kg + 600); break;
            }
            return static_cast<double>(std::max(0, trip_fuel_kg + pfd_kg));  // fuel can't be negative
        }

    }  // namespace

    std::vector<FlightRecord> generate_flights(std::uint32_t seed, int record_count) {
        std::vector<FlightRecord> records = boundary_records();

        std::mt19937 rng(seed);
        const FuelStatus targets[] = { FuelStatus::Nominal, FuelStatus::Caution,
                                      FuelStatus::MinimumFuel, FuelStatus::FuelEmergency };

        for (int i = 0; i < record_count; ++i) {
            const int trip = pick(rng, 300, 2500);
            const int percent_flown = pick(rng, 10, 70);

            FlightRecord r;
            r.flight_id = std::format("AF{:04d}", 1001 + i);
            r.trip_distance_nm = trip;
            r.distance_flown_nm = trip * percent_flown / 100;  // whole nm
            r.alternate_distance_nm = pick(rng, 50, 250);
            r.ground_speed_knots = pick(rng, 380, 480);
            r.cruise_flow_kg_per_hour = pick(rng, 2200, 2800);
            r.holding_flow_kg_per_hour = pick(rng, 2000, 2400);
            r.fuel_on_board_kg = choose_fuel_kg(rng, r, targets[i % 4]);
            records.push_back(std::move(r));
        }
        return records;
    }

    void write_flights_csv(std::ostream& out, const std::vector<FlightRecord>& records) {
        out << kCsvHeader << '\n';
        for (const FlightRecord& r : records) {
            out << std::format("{},{},{},{},{},{},{},{}\n", r.flight_id, r.trip_distance_nm,
                r.alternate_distance_nm, r.distance_flown_nm, r.fuel_on_board_kg,
                r.ground_speed_knots, r.cruise_flow_kg_per_hour,
                r.holding_flow_kg_per_hour);
        }
    }

}  // namespace afrm