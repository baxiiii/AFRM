# Changelog

## V2.0.0 - 2026-11-18

CR-001: aircraft weight classes.

| | |
|---|---|
| Tag | `V2.0.0` |
| Tagged commit | `2c113805933a6b9f7cbd261a936ca2e193589213` |
| Merge commit | `41482508d41c12be54f59b38742166e51c81e704` |
| Pull request | [#3](https://github.com/baxiiii/AFRM/pull/3) |
| CI | Failing acceptance-test run: `44182559456`. Passing merge run: `44191020560`, 44 tests |
| Change record | `docs/CR-001.md` |
| Review | `docs/REVIEW_CHECKLIST.md`, Accept with limitations |

### Breaking change

The CSV format changed from eight to nine columns. Data files now require
`aircraft_weight_kg`, with a value of `5000`, `20000` or `100000`.

Eight-column files are rejected. To preserve the calculation behaviour of
`v1.0.1`, add `20000` (`MEDIUM`) to every existing row.

### Added

- Three aircraft weight classes:
  - `LIGHT`: 5000 kg, multiplier `0.25`
  - `MEDIUM`: 20000 kg, multiplier `1.0`
  - `HEAVY`: 100000 kg, multiplier `5.0`
- Cruise and holding flow scaling before fuel calculation.
- The `aircraft_weight_kg` CSV column.
- Validation and row-level rejection of unsupported weights.
- Generator support for all three classes.
- Twenty-one generated boundary records: seven per class.
- Requirements R6 and R7.
- Weight-class and CSV-format tests.
- MEDIUM regression tests confirming preservation of baseline calculations.

The repository history records a failed acceptance-test run before the completed
implementation. It does not represent a clean pre-implementation TDD sequence;
the acceptance tests were added during the CR-001 implementation history.

### Changed

- `DataGenerator.REQ_DATA_01_CsvHasHeaderAndEightColumns` now verifies the
  nine-column format.
- Sample CSV files in `data/` now contain `aircraft_weight_kg`.
- `docs/README.md` documents the new CSV format and migration procedure.
- The test suite increased from 35 to 44 tests.
- The `MEDIUM` class preserves the original multiplier and calculation behaviour.

### Removed

- `Calculations.REQ_WEIGHT_00_AircraftWeightDoesNotAffectFuelCalculation`
- `Assessment.REQ_WEIGHT_00_AircraftWeightDoesNotAffectFuelAssessment`

These tests characterised the limitation removed by CR-001. They were replaced
by weight-dependent calculation and assessment tests.

### Unchanged

- `Assessment.cpp` and `Assessment.h`; status labels, thresholds and comparison
  rules remain unchanged.
- Rounding rules.
- The baseline calculation behaviour for MEDIUM-class records.

### Known limitations

- The multipliers are fictional and proportional weight scaling is not a
  realistic aircraft-performance model.
- Units are represented using plain `double` values and are not type-safe.
- Quoted CSV fields are unsupported.
- `weight_burn_multiplier()` has a MEDIUM fallback for unsupported direct API
  inputs; the loader and record validation reject unsupported weights before
  calculation.
- The change introduces an incompatible CSV format and requires data migration.
- Review and approval were performed by the same developer; no independent
  verification was available.

## v1.0.1 - 2026-11-15

Controlled baseline for testing and subsequent change.

| | |
|---|---|
| Tag | `v1.0.1` |
| Commit | `5e94657c7ef007095bbe03e57c7b673b246b289f` |
| CI | `43748060562` |

### Contents

- Modular C++20 solution containing the `AFRM.Core` library, console
  application and GoogleTest project.
- Fuel calculations for trip fuel, alternate fuel, contingency, final reserve
  and predicted fuel at destination.
- Four-status assessment using `>=` threshold comparisons.
- CSV loading with exact-header validation and row-level rejection.
- Seeded fictional data generation.
- Thirty-five GoogleTests across four suites.
- Two aircraft-weight characterisation tests demonstrating that the original
  model did not use aircraft weight.
- GitHub Actions Debug and Release build/test verification.

### Known limitation

Aircraft weight was not modelled. This limitation was addressed by
`docs/CR-001.md` in `V2.0.0`.
