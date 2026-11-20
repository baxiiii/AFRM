# AFRM Test Plan

## 1. Purpose

This document defines the verification strategy for AFRM, an educational C++20
prototype for calculating indicative fuel status from fictional flight records.

AFRM is not an operational flight-planning system and is not approved aviation
guidance. All data, thresholds and aircraft-weight multipliers are fictional
teaching assumptions.

The plan covers the original functionality and CR-001, which adds aircraft-weight
classes, weight-dependent fuel burn and a ninth CSV column.

---

## 2. Scope

Testing covers:

- fuel-component calculations;
- remaining-trip calculations;
- contingency and final-reserve calculations;
- predicted fuel at destination;
- all four fuel statuses;
- exact threshold boundaries;
- input validation;
- CSV loading and rejection behaviour;
- deterministic data generation;
- aircraft-weight classes and burn multipliers;
- MEDIUM-weight regression compatibility;
- generated-data CSV round trips;
- build quality and performance.

The following are outside the scope of this prototype:

- wind;
- climb or descent;
- taxi fuel;
- aircraft-specific performance tables;
- delays;
- route changes;
- operational reserve rules;
- real-world dispatch or pilot decision support.

---

## 3. Requirements under test

| ID | Requirement |
| --- | --- |
| R1 | Fuel components and remaining-trip calculations are calculated correctly. |
| R2 | Contingency fuel is the greater of 5% of trip fuel to go and five minutes of holding fuel. |
| R3 | Final reserve fuel equals thirty minutes of holding fuel. |
| R4 | Predicted fuel at destination equals fuel on board minus trip fuel to go and is used for assessment. |
| R5 | All four statuses and their exact threshold boundaries behave correctly. |
| R6 | Invalid records are rejected safely and generated data is repeatable. |
| R7 | Aircraft weight class changes fuel burn using the LIGHT, MEDIUM and HEAVY multipliers. Unsupported weights are rejected safely. |

---

## 4. Units and formulas

The implementation uses:

- fuel: kilograms (`kg`);
- distance: nautical miles (`nm`);
- time: hours (`h`);
- speed: knots (`nm/h`);
- fuel flow: kilograms per hour (`kg/h`).

The supported aircraft weights and fictional burn multipliers are:

| Class | Aircraft weight | Burn multiplier |
| --- | ---: | ---: |
| LIGHT | `5000 kg` | `0.25` |
| MEDIUM | `20000 kg` | `1.0` |
| HEAVY | `100000 kg` | `5.0` |

The multiplier is applied once to both cruise and holding fuel flow:

```text
effective_cruise_flow_kg_per_hour =
    cruise_flow_kg_per_hour * weight_multiplier

effective_holding_flow_kg_per_hour =
    holding_flow_kg_per_hour * weight_multiplier
```

The calculations are:

```text
remaining_distance_nm =
    trip_distance_nm - distance_flown_nm

trip_fuel_to_go_kg =
    remaining_distance_nm / ground_speed_knots
    * effective_cruise_flow_kg_per_hour

alternate_fuel_kg =
    alternate_distance_nm / ground_speed_knots
    * effective_cruise_flow_kg_per_hour

contingency_fuel_kg =
    max(
        0.05 * trip_fuel_to_go_kg,
        effective_holding_flow_kg_per_hour * 5 / 60
    )

final_reserve_fuel_kg =
    effective_holding_flow_kg_per_hour * 30 / 60

predicted_fuel_at_destination_kg =
    fuel_on_board_kg - trip_fuel_to_go_kg

protected_fuel_kg =
    contingency_fuel_kg
    + alternate_fuel_kg
    + final_reserve_fuel_kg

diversion_protected_fuel_kg =
    alternate_fuel_kg
    + final_reserve_fuel_kg
```

Alternate fuel, contingency fuel and final reserve fuel are rounded up to whole
kilograms before protection totals are calculated.

Trip fuel to go and predicted fuel at destination are not rounded.

Statuses are evaluated in this order:

| Condition | Status |
| --- | --- |
| `PFD >= protected_fuel_kg` | `NOMINAL` |
| `PFD >= diversion_protected_fuel_kg` | `CAUTION` |
| `PFD >= final_reserve_fuel_kg` | `MINIMUM FUEL` |
| Otherwise | `FUEL EMERGENCY` |

All threshold comparisons use `>=`.

---

## 5. Test strategy

Testing uses:

- focused GoogleTest unit tests;
- integration tests for CSV loading and generation;
- deterministic generated data;
- boundary-value analysis;
- equivalence testing for MEDIUM aircraft;
- negative testing for invalid input;
- Debug coverage analysis;
- Release performance measurements;
- Windows CI builds and test execution.

Tests follow this naming convention:

```cpp
TEST(<Suite>, REQ_<AREA>_<NUMBER>_<Description>)
```

The requirement prefixes are:

| Prefix | Requirement |
| --- | --- |
| `REQ_CALC_01` | R1 |
| `REQ_CONT_01` | R2 |
| `REQ_RESERVE_01` | R3 |
| `REQ_PFD_01` | R4 |
| `REQ_STATUS_01` | R5 |
| `REQ_DATA_01` | R6 |
| `REQ_WEIGHT_01` | R7 |

---

## 6. Test conditions

### 6.1 Normal test environment

- Windows;
- Visual Studio with Desktop development with C++;
- MSVC C++20 support;
- x64 platform;
- GoogleTest installed through vcpkg manifest mode;
- Debug and Release configurations;
- the same source revision and dataset for repeatable comparisons.

### 6.2 Build configurations

Debug is used for:

- normal development testing;
- coverage collection;
- diagnostic investigation.

Release x64 is used for:

- performance measurements;
- final runtime comparison.

Both Debug and Release must be built and tested in CI.

---

## 7. Functional test cases

### 7.1 R1 – Fuel calculations and validation

| Test case | Expected result |
| --- | --- |
| Base valid flight | All fuel components match hand-worked expected values. |
| Different speed and flow values | Results change according to the formulas. |
| No distance remaining | Trip fuel to go is zero. |
| Invalid numeric input | Record is rejected safely. |
| Negative distance | Record is rejected. |
| Distance flown greater than trip distance | Record is rejected. |
| Zero or negative ground speed | Record is rejected. |
| Zero or negative fuel flow | Record is rejected. |
| Boundary but valid inputs | Valid boundary values are accepted. |
| Rounding values | Alternate, contingency and reserve values round upward correctly. |
| Required-fuel calculation | Trip fuel and PFD remain unrounded. |

### 7.2 R2 – Contingency fuel

Test both branches of the maximum operation:

| Test case | Expected result |
| --- | --- |
| 5% of trip fuel is larger | Contingency equals 5% of trip fuel, rounded upward. |
| Five minutes of holding fuel is larger | Contingency equals five minutes of holding fuel, rounded upward. |
| Values are equal | Either branch produces the same result. |
| Weight classes | Effective holding and cruise flows are used. |

### 7.3 R3 – Final reserve

| Test case | Expected result |
| --- | --- |
| MEDIUM aircraft | Reserve equals 30 minutes of holding fuel. |
| LIGHT aircraft | Reserve uses the `0.25` holding-flow multiplier. |
| HEAVY aircraft | Reserve uses the `5.0` holding-flow multiplier. |
| Fractional result | Reserve is rounded upward. |

### 7.4 R4 – Predicted fuel at destination

| Test case | Expected result |
| --- | --- |
| Normal flight | PFD equals fuel on board minus trip fuel to go. |
| PFD on protected threshold | Status is `NOMINAL`. |
| PFD below protected threshold | Status is assessed using the next band. |
| Weight classes | PFD changes because effective cruise flow changes. |
| Rounding boundary | Rounding does not make a marginal case safer. |

### 7.5 R5 – Status bands

Each status must be tested both at and around its threshold:

| Test case | Expected result |
| --- | --- |
| PFD exactly at protected fuel | `NOMINAL` |
| PFD one kilogram below protected fuel | `CAUTION` |
| PFD exactly at diversion-protected fuel | `CAUTION` |
| PFD one kilogram below diversion-protected fuel | `MINIMUM FUEL` |
| PFD exactly at final reserve | `MINIMUM FUEL` |
| PFD one kilogram below final reserve | `FUEL EMERGENCY` |
| Zero or negative PFD | `FUEL EMERGENCY` |
| NaN PFD | Safe emergency result |
| Status labels | Exact required labels are returned. |
| Status explanations | Every status has an explanation. |

### 7.6 R6 – Validation, loading and generation

A record is invalid when it contains:

- an empty flight identifier;
- NaN or infinity;
- a negative distance;
- negative fuel on board;
- distance flown greater than trip distance;
- zero or negative ground speed;
- zero or negative cruise flow;
- zero or negative holding flow.

The CSV loader must also reject:

- a missing header;
- an incorrect header;
- the old eight-column header;
- too few fields;
- too many fields;
- missing numeric values;
- malformed numeric values.

Invalid rows must contain:

- a one-based line number;
- a clear rejection reason.

Valid rows after invalid rows must still load.

The generator must verify:

- same seed gives identical output;
- different seeds give different output;
- the requested random record count is honoured;
- generated identifiers are unique;
- all generated records are valid;
- all four statuses are represented;
- generated CSV can be loaded without rejection.

### 7.7 R7 – Aircraft weight and multipliers

| Test case | Expected result |
| --- | --- |
| LIGHT weight (`5000 kg`) | Cruise and holding flows use multiplier `0.25`. |
| MEDIUM weight (`20000 kg`) | Multiplier is `1.0`; baseline results are preserved. |
| HEAVY weight (`100000 kg`) | Cruise and holding flows use multiplier `5.0`. |
| Multiplier applied to cruise flow | Trip fuel and alternate fuel change correctly. |
| Multiplier applied to holding flow | Contingency and final reserve change correctly. |
| Multiplier applied once | No double scaling occurs. |
| Unsupported weight | Row is rejected without a silent MEDIUM fallback. |
| Zero weight | Row is rejected. |
| Negative weight | Row is rejected. |
| NaN or infinity weight | Row is rejected. |
| Nine-column CSV row | Weight is parsed and preserved. |
| CSV round trip | Weight remains unchanged after writing and loading. |
| Generated weight classes | LIGHT, MEDIUM and HEAVY records are all present. |
| Generated boundary records | Seven records are generated for each class. |

---

## 8. Generator test data

The default generated dataset contains:

- seven MEDIUM boundary records;
- seven LIGHT boundary records;
- seven HEAVY boundary records;
- twenty random records.

The default total is therefore:

```text
21 boundary records + 20 random records = 41 records
```

The generator must produce:

- exact threshold boundary values;
- unique boundary identifiers;
- all three supported weights;
- random records with valid weights;
- random records cycling through the four target statuses;
- deterministic output for a fixed seed.

---

## 9. CSV integration tests

The CSV header must contain exactly nine columns:

```text
flight_id,trip_distance_nm,alternate_distance_nm,distance_flown_nm,fuel_on_board_kg,ground_speed_knots,cruise_flow_kg_per_hour,holding_flow_kg_per_hour,aircraft_weight_kg
```

CSV tests must verify:

- valid nine-column files load successfully;
- the old eight-column format is rejected as a whole-file error;
- missing headers are rejected;
- extra fields are rejected;
- missing fields are rejected;
- malformed numbers are rejected;
- unsupported weights reject only the affected row;
- later valid rows still load;
- spaces and decimal values are handled;
- CRLF line endings are handled;
- generated data writes nine fields;
- generated data loads back without rejections;
- aircraft weight survives the round trip.

Quoted CSV fields are not supported.

---

## 10. Regression testing

MEDIUM weight (`20000 kg`) is the compatibility class because its multiplier is
`1.0`.

Regression testing must compare v1.0.1 and CR-001 using equivalent MEDIUM records.

The comparison should verify:

- trip fuel;
- alternate fuel;
- contingency fuel;
- final reserve fuel;
- predicted fuel at destination;
- protected-fuel totals;
- diversion-protected-fuel totals;
- final status;
- status explanation where applicable.

The complete CSV files cannot be byte-identical because CR-001 changes the format
from eight columns to nine columns. The valid comparison is therefore either:

- numerical and status equivalence for equivalent MEDIUM records; or
- a normalised comparison that removes the new weight column.

---

## 11. Test execution order

The recommended order is:

1. Build the solution in Debug x64.
2. Run the complete Debug test suite.
3. Build the solution in Release x64.
4. Run the complete Release test suite.
5. Run the MEDIUM regression comparison.
6. Collect Debug coverage.
7. Run Release performance measurements.
8. Review warnings and generated evidence.
9. Record the results in the review checklist.

The expected final GoogleTest result is:

```text
[==========] 44 tests from 4 test suites ran.
[  PASSED  ] 44 tests.
```

The same test count and passing result are required in Debug and Release.

---

## 12. Coverage plan

Coverage is collected using the Debug x64 build.

The coverage report must record:

- statement coverage;
- decision or branch coverage;
- uncovered lines;
- uncovered branches;
- comparison with the v1.0.1 coverage result.

Coverage must include the new CR-001 paths:

- supported weight classification;
- LIGHT multiplier;
- MEDIUM multiplier;
- HEAVY multiplier;
- unsupported-weight rejection;
- invalid-weight rejection;
- nine-column CSV parsing;
- generated records for all three classes;
- all generator boundary records.

Coverage demonstrates that code paths were executed. It does not independently
prove that the implementation is correct, so the focused assertions and regression
tests remain necessary.

---
## Decision coverage table

OpenCppCoverage reports line coverage rather than decision or branch coverage.
The decisions below were therefore reviewed manually. Both true and false outcomes
must be exercised for each decision.

| # | Decision | True outcome | False outcome | Evidence |
|---|---|---|---|---|
| D1 | Flight identifier is empty | Reject `"flight_id is empty"` | Continue validation | `REQ_DATA_01_AllRecordsAreValidAndUniquelyNamed`; invalid-row loader tests |
| D2 | A numeric field is non-finite | Reject `"numeric field is not finite"` | Continue validation | `Calculations.*InvalidInputsRejected`; loader malformed and NaN tests |
| D3 | Trip distance is negative | Reject negative trip-distance record | Continue validation | `Calculations.REQ_CALC_01_InvalidInputsRejected` |
| D4 | Alternate distance is negative | Reject negative alternate-distance record | Continue validation | Loader invalid-row tests |
| D5 | Distance flown is negative | Reject negative distance-flown record | Continue validation | Loader invalid-row tests |
| D6 | Distance flown exceeds trip distance | Reject record | Continue validation | `Calculations.REQ_CALC_01_InvalidInputsRejected` |
| D7 | Fuel on board is negative | Reject negative-fuel record | Continue validation | `Calculations.REQ_CALC_01_InvalidInputsRejected` |
| D8 | Ground speed is zero or negative | Reject record | Continue validation | `Calculations.REQ_CALC_01_InvalidInputsRejected` |
| D9 | Cruise fuel flow is zero or negative | Reject record | Continue validation | `Calculations.REQ_CALC_01_InvalidInputsRejected` |
| D10 | Holding fuel flow is zero or negative | Reject record | Continue validation | `Calculations.REQ_CALC_01_InvalidInputsRejected` |
| D11 | Aircraft weight is supported | Continue with the weight calculation | Reject unsupported weight | `REQ_WEIGHT_02` loader and unsupported-weight tests |
| D12 | Weight equals 5000 kg | Return LIGHT | Continue weight classification | `REQ_WEIGHT_01` LIGHT calculation and boundary tests |
| D13 | Weight equals 100000 kg | Return HEAVY | Continue weight classification | `REQ_WEIGHT_01` HEAVY calculation and boundary tests |
| D14 | No LIGHT or HEAVY weight match | Return MEDIUM | Not applicable | MEDIUM baseline and regression tests |
| D15 | Effective cruise flow produces rounded alternate fuel | Round alternate fuel upward | Zero result or no rounding difference | `REQ_CALC_01_RoundUpToWholeKg` |
| D16 | 5% of trip fuel is greater than five-minute holding fuel | Return contingency from 5% of trip fuel | Evaluate five-minute holding branch | `REQ_CONT_01_BothBranchesAndCrossover` |
| D17 | PFD is greater than or equal to protected fuel | Return `NOMINAL` | Evaluate the CAUTION branch | `REQ_STATUS_01_ExactlyOnEachThreshold`; `REQ_STATUS_01_OneKilogramEitherSideOfEachThreshold` |
| D18 | PFD is greater than or equal to diversion-protected fuel | Return `CAUTION` | Evaluate the MINIMUM FUEL branch | `REQ_STATUS_01_ExactlyOnEachThreshold`; `REQ_STATUS_01_OneKilogramEitherSideOfEachThreshold` |
| D19 | PFD is greater than or equal to final reserve | Return `MINIMUM FUEL` | Return `FUEL EMERGENCY` | `REQ_STATUS_01_ExactlyOnEachThreshold`; `REQ_STATUS_01_OneKilogramEitherSideOfEachThreshold` |
| D20 | PFD is NaN | Return safe emergency assessment | Continue normal status comparison | `REQ_STATUS_01_NaNPfdFailsSafeToEmergency` |
| D21 | PFD is negative or zero | Return `FUEL EMERGENCY` | Continue normal status comparison | `REQ_STATUS_01_NegativeAndZeroPfdAreEmergency` |
| D22 | CSV field count is correct | Parse the row | Reject with an incorrect-field-count reason | Valid-row and too-few/too-many-field loader tests |
| D23 | CSV header exactly matches nine-column header | Continue loading | Reject the file as a header error | `REQ_WEIGHT_02_NineColumnHeaderRequired`; `REQ_WEIGHT_02_EightColumnFileRejected` |
| D24 | A CSV row has invalid weight | Reject that row with line and reason | Load the row | `REQ_WEIGHT_02_UnsupportedWeightRejectedWithLineAndReason` |
| D25 | Generated record count follows argument | Emit boundary plus requested random records | Emit only boundary records when count is zero | `REQ_DATA_01_RecordCountFollowsArgument` |
| D26 | Random fuel-target iteration has a fourth status target | Cycle to the next status | Not applicable | `REQ_DATA_01_GeneratedDataExercisesAllFourStatuses` |
| D27 | Two generated datasets use the same seed | Produce identical CSV data | Produce different data for a different seed | Same-seed and different-seed generator tests |

### Result

All listed decisions were exercised through automated tests or the hand-made boundary
tests. Statement coverage was measured with OpenCppCoverage, while true/false
decision coverage was confirmed manually using this table.

No additional test change was required because the existing 44-test suite exercises
both outcomes of the listed decisions, including the CR-001 weight paths and the
status threshold boundaries.
## 13. Performance plan

Performance is measured using Release x64.

The same machine, build type, dataset and measurement method must be used for both
v1.0.1 and CR-001.

The main dataset should contain 100,000 generated records.

Record:

- source revision;
- dataset size;
- machine and operating-system details;
- build configuration;
- number of repetitions;
- elapsed time;
- peak memory, where available;
- v1.0.1 result;
- CR-001 result;
- percentage difference.

The performance target is the target stated in the assignment or change request.
It must be evaluated using measured results rather than assumed from a successful
build.

Coverage and performance should not be measured in the same run because coverage
instrumentation can distort runtime results.

---

## 14. CI verification

GitHub Actions must:

- restore dependencies;
- build Debug x64;
- run the Debug tests;
- build Release x64;
- run the Release tests;
- fail if compilation warnings or tests fail according to the workflow policy.

The final branch CI run must be recorded.

After merging to `master`, CI must be run again and its result recorded separately.

---

## 15. Evidence to retain

The following evidence should be retained for the report and review:

- Debug build output;
- Release build output;
- Debug test output;
- Release test output;
- successful CI run URL;
- final coverage report;
- v1.0.1 versus CR-001 regression comparison;
- Release performance timings;
- profiler screenshot or exported profiler report;
- generated 100,000-record dataset details;
- final decision-table results;
- completed review checklist.

---

## 16. Traceability matrix

| Requirement | Implementation | Tests and evidence |
| --- | --- | --- |
| R1 | `Calculations.h`, `Calculations.cpp`, `Models.h`, `Config.h` | Base calculations, alternative inputs, remaining-distance, invalid-input and rounding tests |
| R2 | `Calculations.cpp`, `Config.h` | Both contingency branches, crossover and weight-dependent holding-flow tests |
| R3 | `Calculations.cpp`, `Config.h` | Thirty-minute reserve and weight-dependent reserve tests |
| R4 | `Calculations.cpp`, `Assessment.cpp` | PFD calculation, full-pipeline and rounding-boundary tests |
| R5 | `Assessment.h`, `Assessment.cpp`, `Models.h` | Status labels, four bands, exact thresholds, one-kilogram boundary and explanation tests |
| R6 | `CsvLoader.h`, `CsvLoader.cpp`, `DataGenerator.h`, `DataGenerator.cpp`, `Calculations.cpp` | Invalid-row, header, continuation, deterministic-generation, uniqueness and round-trip tests |
| R7 | `Models.h`, `Config.h`, `Calculations.cpp`, `CsvLoader.cpp`, `DataGenerator.cpp` | Weight multipliers, MEDIUM regression, unsupported-weight, nine-column and three-class generator tests |

---

## 17. Pass criteria

CR-001 passes this test plan when:

- all required functional tests pass;
- all 44 tests pass in Debug and Release;
- MEDIUM regression results match v1.0.1;
- all three weight classes are tested;
- invalid weights are rejected safely;
- generated CSV round trips successfully;
- the required coverage target is met;
- the performance target is met;
- CI passes;
- the documentation and review checklist are completed honestly.

Any uncompleted evidence item must be marked as pending or not applicable rather
than claimed as complete.

Assisted with GitHub Copilot
