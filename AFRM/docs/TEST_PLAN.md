# AFRM Test Plan

**Applies to:** controlled baseline `v1.0.1`, and the change released as `v2.0.0` (CR-001).
**Author:** Ibby (sole developer)
**Status:** written before coverage, debugger and profiler results were collected, so the pass criteria were fixed before results were seen. [SUBJECT TO UPDATES]

AFRM is an educational prototype that uses fictional data. The priorities and targets below are my own engineering judgement. They are not certified requirements, and AFRM must not be used operationally.

## 1. Failure impact and criticality

AFRM tells a user whether a flight's predicted fuel at destination (PFD) is NOMINAL, CAUTION, MINIMUM FUEL or FUEL EMERGENCY. If it were a real advisory tool, a status that looks safer than the true position could contribute to fuel exhaustion.

For this exercise I treat the status logic as **notional DO-178C Level B** (a hazardous failure condition). A crew would normally cross-check fuel, so a single wrong status is unlikely to be catastrophic, but it could seriously reduce safety margins. AFRM is not certified and this is an exercise in applying the approach.

Level B drives these choices:

- Requirements-based testing of every requirement (R1 to R5, and R6 to R7 after CR-001).
- Boundary value analysis and equivalence partitioning on every threshold and input limit.
- **Statement and decision** coverage of the assessment and calculation code. MC/DC (Level A) is not applied, because it is disproportionate for a prototype.
- Regression testing of the whole suite at every version.

## 2. Test priorities

A result that looks safe when it is not matters more than one that is too cautious, because the cautious result would at least be checked.

| Failure | Why it matters | Priority | How it is tested |
|---|---|---|---|
| False NOMINAL or CAUTION (status safer than true PFD) | Low fuel gets no warning | High | Threshold tests on and one kilogram either side (`Assessment.REQ_STATUS_01_*`), boundary records |
| Rounding makes a marginal case look safer | Same as above, hidden in arithmetic | High | `REQ_CALC_01_RequiredFuelRoundedUpTripFuelAndPfdNot`, `REQ_PFD_01_RoundingNeverMakesMarginalCaseSafer` |
| Invalid input accepted (negative, NaN, infinite, distance flown beyond trip distance) | Meaningless result that looks real | High | `REQ_CALC_01_InvalidInputsRejected`, CSV rejection tests |
| NaN or non-finite PFD treated as safe | Silent failure | High | `REQ_STATUS_01_NaNPfdFailsSafeToEmergency` |
| Aircraft size not represented, so burn cannot be tied to weight | Same fuel inputs give the same result for any aircraft | High | Characterisation tests `REQ_WEIGHT_00_*` |
| Wrong flow used for the aircraft (after CR-001) | Reserves and status wrong for that aircraft | High | `REQ_WEIGHT_01_*`, `REQ_WEIGHT_02_*` |
| Wrong CSV column order or header | Values read into the wrong fields | High | Header and row tests, `REQ_DATA_01_BadOrMissingHeaderRejectsWholeFile` |
| Bad row hides later rows | Records silently lost | Medium | `REQ_DATA_01_InvalidRowsDoNotLoseValidRows` |
| Wrong units in the CSV | Numbers look fine but are wrong | High | Cannot be tested from values; see limitations |
| Generated data not repeatable | Evidence cannot be reproduced | Medium | `REQ_DATA_01_SameSeedGivesIdenticalData` |
| False FUEL EMERGENCY or MINIMUM FUEL | Over-cautious, but it would be checked | Medium | Same threshold tests |
| Slow processing of large files | Inconvenient, results still right | Low | Timing measurement (section 6) |

## 3. Test levels and techniques

| Level | Where | Technique |
|---|---|---|
| Unit | `test_calculations.cpp`, `test_assessment.cpp` | Known answers worked by hand, boundary values, invalid inputs |
| Integration | `test_assessment.cpp` (full pipeline), `test_csv_loader.cpp`, `test_data_generator.cpp` | CSV to record to calculation to status |
| System | Running `AFRM.exe` on a data file and checking the output | Black-box, compared to hand calculations |
| Characterisation | `REQ_WEIGHT_00_*` | Documents current behaviour where a capability is missing |
| Regression | Whole suite at every version, plus baseline data through `v2.0.0` | Output comparison with `v1.0.1` |

Expected values come from the requirements or from hand calculation, not from running the code under test.

## 4. Test environment

| Item | Value |
|---|---|
| OS | Windows 11 |
| IDE and compiler | Visual Studio Community 2026, MSVC 19.51.36260.0 (toolset 14.51.36231) |
| Language and flags | ISO C++20 (`/std:c++20`), `/W4`, `/fp:precise`, x64 |
| Configurations | Debug (`/Od`) and Release (`/O2`) |
| Test framework | GoogleTest, pinned through the `vcpkg.json` baseline |
| Coverage tool | OpenCppCoverage (version to be recorded in `evidence/`) |
| Debugging and profiling | Visual Studio debugger and Performance Profiler |
| CI | GitHub Actions, Windows runner, Debug and Release build and test on every push |
| Test data | Seeded generator (seed 20260601), plus hand-made boundary records |

The CI toolchain version may differ from my local one, so CI shows the committed files build elsewhere but not an identical toolchain.

## 5. Tests

### 5.1 Baseline tests (35, at `v1.0.1`)

| Suite | Count | Requirement IDs |
|---|---|---|
| Calculations | 11 | `REQ_CALC_01`, `REQ_CONT_01`, `REQ_RESERVE_01`, `REQ_PFD_01`, `REQ_WEIGHT_00` |
| Assessment | 10 | `REQ_STATUS_01`, `REQ_PFD_01`, `REQ_WEIGHT_00` |
| CsvLoader | 6 | `REQ_DATA_01` |
| DataGenerator | 8 | `REQ_DATA_01`, `REQ_STATUS_01` |
| **Total** | **35** | |

### 5.2 Characterisation tests in the baseline

Two of the 35 tests look at what the model does when aircraft weight changes. They were designed from the failure analysis in section 2, by asking what AFRM does for a very light and a very heavy aircraft.

| Test | Method | Pass if |
|---|---|---|
| `Calculations.REQ_WEIGHT_00_AircraftWeightDoesNotAffectFuelCalculation` | Characterisation | Two records built for hypothetical 5,000 kg and 100,000 kg aircraft give identical trip fuel, alternate fuel, contingency, final reserve, PFD and protected fuel values |
| `Assessment.REQ_WEIGHT_00_AircraftWeightDoesNotAffectFuelAssessment` | Characterisation | The same two records give the same status and explanation |

Both pass at the baseline, and that result is the finding: the model has no weight input, so aircraft size cannot influence the fuel calculation or the status. A pass here documents a missing capability. It does not show a fault in the existing logic. This finding is the basis of CR-001.

### 5.3 Tests for CR-001 (`v2.0.0`)

These 11 tests are written and committed before any implementation, so they fail first. The hand-worked values are in `CR-001.md`.

| Test | Method | Pass if |
|---|---|---|
| `Calculations.REQ_WEIGHT_01_MediumMatchesBaseline` | Regression | MEDIUM gives results identical to `v1.0.1` |
| `Calculations.REQ_WEIGHT_01_LightAndHeavyScaleBothFlows` | Known answers | Cruise and holding flow scale by 0.25 and 5.0 |
| `Calculations.REQ_WEIGHT_01_ReservesFollowClass` | Known answers | Contingency, alternate and final reserve match hand values per class |
| `Assessment.REQ_WEIGHT_01_ThresholdsExactAtEachClass` | Boundary | Status changes exactly at the hand-worked fuel values and one kilogram below |
| `Assessment.REQ_WEIGHT_01_SameFlightDifferentStatusByClass` | Equivalence | Same record and fuel gives different statuses by class |
| `CsvLoader.REQ_WEIGHT_02_ValidWeightsLoaded` | Equivalence | 5000, 20000 and 100000 load |
| `CsvLoader.REQ_WEIGHT_02_UnsupportedWeightRejectedWithLineAndReason` | Invalid input | Any other value, blank or non-numeric rejects that row only |
| `CsvLoader.REQ_WEIGHT_02_NineColumnHeaderRequired` | Invalid input | Exact nine-column header accepted |
| `CsvLoader.REQ_WEIGHT_02_EightColumnFileRejected` | Invalid input | The `v1.0.1` format is rejected with a clear error |
| `DataGenerator.REQ_WEIGHT_01_BoundaryRecordsPerClass` | Boundary | 21 boundary records, each on or one kilogram below a threshold |
| `DataGenerator.REQ_WEIGHT_02_AllThreeClassesGenerated` | Equivalence | All three classes appear in generated data |

**Controlled test changes at `v2.0.0`:**

- The two `REQ_WEIGHT_00` tests describe a limitation that CR-001 removes, so they are retired. They are superseded by `Calculations.REQ_WEIGHT_01_LightAndHeavyScaleBothFlows` and `Assessment.REQ_WEIGHT_01_SameFlightDifferentStatusByClass`, which assert the opposite behaviour.
- `DataGenerator.REQ_DATA_01_CsvHasHeaderAndEightColumns` encodes the old header and is updated to nine columns.
- Loader tests that build CSV text may need the new column.

Every other baseline test must pass without change. Each edit above is recorded in the pull request. The suite size at `v2.0.0` is expected to be 35 − 2 + 11 = 44.

## 6. Targets

| Target | Measure |
|---|---|
| Requirements | Every requirement R1 to R7 has at least one automated test |
| Pass rate | All tests pass in Debug and Release, locally and in CI |
| Statement coverage | 100% of `Assessment.cpp` and `Calculations.cpp`; at least 90% of `CsvLoader.cpp` and `DataGenerator.cpp` (line coverage, OpenCppCoverage) |
| Decision coverage | Every decision in `assess_fuel_status()` and the calculation functions exercised both ways, shown by a hand-built decision table, because the tool reports line coverage only |
| Regression | `v2.0.0` gives byte-identical output to `v1.0.1` for the baseline data with `20000` added to each row |
| Performance | Loading and assessing 100,000 generated records in Release has a median under 1 s over 5 repeats at `v1.0.1`, and `v2.0.0` is no more than 20% slower |

These targets are set before the measurements are taken. If one is missed, I will record it and explain why instead of changing it afterwards.

Coverage is measured on the baseline. A gap in a high-priority area is recorded in the results. It is then closed by the CR-001 tests where it falls within the change, or listed as a recommendation.

## 7. Start and finish criteria

**Start:** `v1.0.1` is tagged, the build is clean at `/W4`, dependencies restore from `vcpkg.json`, and the expected results are written down.

**Finish:** all tests pass in both configurations locally and in CI, targets are met or the misses are recorded, and results are saved in `evidence/`.

## 8. Evidence collected

Saved in `evidence/`: test output, coverage reports for each version, debugger and profiler captures, timing results, output hashes for the regression comparison, and the CI run IDs.

## 9. Limitations

- Passing tests verify the code against my stated requirements. They do not validate that the requirements match real aircraft operations.
- The `REQ_WEIGHT_00` tests call the same code with identical inputs, so they pass by determinism. They document a missing capability and do not prove a defect.
- Line coverage shows which code ran, not that the answer was correct.
- Decision coverage is shown by manual analysis, and MC/DC is not attempted.
- A value in the wrong unit is still a valid number, so units cannot be tested from the data.
- The weight multipliers are fictional.
- I am the only developer, so test design and review are not independent.
- CI runs on every push but does not block a merge, so I check each result myself.

## 10. How to run

Paths and executable names may need adjusting to the real build output folders.

```powershell
# Build
msbuild AFRM.slnx /p:Configuration=Release /p:Platform=x64

# Run tests
.\x64\Release\AFRM.Tests.exe

# Coverage (use the Debug build, because optimisation distorts line coverage)
OpenCppCoverage.exe --sources src\AFRM.Core --export_type html:evidence\coverage -- .\x64\Debug\AFRM.Tests.exe

# Timing: generate a large file, then time the application on it, 5 repeats

(Assisted with GitHub Copilot, but I wrote the plan and tests myself.)
```
