# AFRM - Aircraft Fuel and Range Management

AFRM is an educational C++20 console prototype that calculates indicative fuel
status for fictional aircraft flights.

It is **not** an operational flight-planning system and is **not** approved
aviation guidance. All data is fictional and the calculations use simplified
teaching assumptions.

## What the application does

AFRM can:

- Load fictional flight data from a CSV file.
- Reject invalid rows while continuing to process valid rows.
- Calculate:
  - trip fuel to go;
  - alternate fuel;
  - contingency fuel;
  - final reserve fuel;
  - predicted fuel at destination.
- Assign one of four indicative statuses:
  - `NOMINAL`;
  - `CAUTION`;
  - `MINIMUM FUEL`;
  - `FUEL EMERGENCY`.
- Print a console report with explanations and summary counts.
- Generate deterministic fictional CSV data for testing.

## Project structure

```text
AFRM/
├── AFRM.slnx
├── vcpkg.json
├── data/
│   └── flights.csv
├── src/
│   ├── AFRM.Core/
│   │   ├── Assessment.cpp
│   │   ├── Assessment.h
│   │   ├── Calculations.cpp
│   │   ├── Calculations.h
│   │   ├── Config.h
│   │   ├── CsvLoader.cpp
│   │   ├── CsvLoader.h
│   │   ├── DataGenerator.cpp
│   │   ├── DataGenerator.h
│   │   └── Models.h
│   └── AFRM/
│       └── AFRM.cpp
└── tests/
    └── AFRM.Tests/
        ├── test_assessment.cpp
        ├── test_calculations.cpp
        ├── test_csv_loader.cpp
        ├── test_data_generator.cpp
        └── test_main.cpp
```

`AFRM.Core` is a static library shared by the console application and the
tests. The application entry point is `src/AFRM/AFRM.cpp`.

## Requirements

- Windows
- Visual Studio with Desktop development with C++
- MSVC C++20 support
- x64 build tools
- Git
- vcpkg for GoogleTest

The project uses the C++20 language standard and GoogleTest for the test
project.

## GoogleTest dependency

The test project uses vcpkg manifest mode. The dependency is declared in
`vcpkg.json`.

The first test build may download and build GoogleTest.

## Building

Open `AFRM.slnx` in Visual Studio and select:

```text
Configuration: Debug or Release
Platform: x64
```

Alternatively, run these commands from the directory containing `AFRM.slnx`:

```powershell
msbuild AFRM.slnx /m /p:Configuration=Debug /p:Platform=x64
msbuild AFRM.slnx /m /p:Configuration=Release /p:Platform=x64
```

Build output is placed in:

```text
build\x64\Debug\
build\x64\Release\
```

The application build copies `data\flights.csv` into the output directory so
that the default report command can find it beside the executable.

## Running the application

Run the default report:

```powershell
.\build\x64\Debug\AFRM.exe
```

The application loads:

```text
data\flights.csv
```

relative to the executable location.

Generate a dataset using the default output path:

```powershell
.\build\x64\Debug\AFRM.exe --generate-data
```

Generate a dataset at an explicit path:

```powershell
.\build\x64\Debug\AFRM.exe --generate-data path\to\flights.generated.csv
```

The parent directory of an explicit output path must already exist.

The application currently supports only these command forms:

```text
AFRM
AFRM --generate-data
AFRM --generate-data [path]
```

There is currently no `--file` or `--help` option.

## Exit codes

| Code | Meaning |
|---:|---|
| `0` | Operation completed successfully |
| `1` | File, writing or unexpected runtime error |
| `2` | Invalid command-line arguments |

## Running the tests

After building the test project:

```powershell
.\build\x64\Debug\AFRM.Tests.exe
```

A non-zero exit code indicates that at least one test failed.

Run only the assessment tests:

```powershell
.\build\x64\Debug\AFRM.Tests.exe --gtest_filter=Assessment.*
```

The test suites cover:

- fuel calculations and validation;
- rounding behaviour;
- contingency and reserve calculations;
- predicted fuel at destination;
- status thresholds;
- CSV loading and rejected rows;
- deterministic data generation;
- generated CSV round trips.

## CSV format

The CSV header must exactly match:

```text
flight_id,trip_distance_nm,alternate_distance_nm,distance_flown_nm,fuel_on_board_kg,ground_speed_knots,cruise_flow_kg_per_hour,holding_flow_kg_per_hour
```

Each data row must contain eight fields:

```text
flight_id
trip_distance_nm
alternate_distance_nm
distance_flown_nm
fuel_on_board_kg
ground_speed_knots
cruise_flow_kg_per_hour
holding_flow_kg_per_hour
```

Units are:

- kilograms for fuel;
- nautical miles for distances;
- hours for fuel flow;
- knots for ground speed.

The loader accepts:

- spaces around fields;
- decimal values using `.`;
- CRLF line endings;
- blank lines.

The loader rejects:

- a missing or incorrect header;
- missing fields;
- extra fields;
- malformed numbers;
- empty flight identifiers;
- negative distances or fuel;
- distance flown greater than trip distance;
- zero or negative speed;
- zero or negative fuel flows;
- non-finite numeric values.

Invalid rows are rejected with a line number and reason. Other valid rows
continue to load.

Quoted CSV fields are not supported.

## Fuel model

The model assumes:

- constant ground speed;
- constant cruise fuel flow;
- constant holding fuel flow;
- no wind;
- no climb or descent calculation;
- no taxi fuel;
- no weight-dependent fuel-burn adjustment;
- fictional and simplified operating conditions.

The formulas are:

```text
remaining_distance_nm = trip_distance_nm - distance_flown_nm

trip_fuel_to_go_kg =
    remaining_distance_nm / ground_speed_knots
    * cruise_flow_kg_per_hour

alternate_fuel_kg =
    alternate_distance_nm / ground_speed_knots
    * cruise_flow_kg_per_hour

contingency_fuel_kg =
    max(
        5% of trip fuel to go,
        5 minutes of holding fuel
    )

final_reserve_fuel_kg =
    30 minutes of holding fuel

predicted_fuel_at_destination_kg =
    fuel_on_board_kg - trip_fuel_to_go_kg
```

Alternate fuel, contingency fuel and final reserve fuel are rounded up to the
next whole kilogram. Trip fuel to go and predicted fuel at destination are not
rounded.

## Status bands

| Condition | Status |
|---|---|
| PFD is at least protected fuel | `NOMINAL` |
| PFD is below protected fuel but at least diversion-protected fuel | `CAUTION` |
| PFD is below diversion-protected fuel but at least final reserve | `MINIMUM FUEL` |
| PFD is below final reserve | `FUEL EMERGENCY` |

Threshold equality counts as meeting the threshold.

## Dataset generation

The generator uses a fixed default seed:

```text
20260601
```

It creates:

- seven fixed boundary records;
- twenty random records by default;
- random records cycling through the four target status bands.

Generated records do not store a status. The status is calculated after loading
or generation using the same calculation and assessment functions as the main
application.

## Disclaimer

This software is an educational prototype only. It must not be used for
real-world flight planning, fuel decisions, dispatch, navigation or aviation
operations.