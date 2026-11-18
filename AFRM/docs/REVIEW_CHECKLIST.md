# CR-001 Review Checklist

This checklist was written before CR-001 was implemented. The review is therefore judged against fixed criteria agreed in advance.

This is a self-review because I am the only developer. A fixed checklist, automated tests and CI runs are used to reduce author bias.

|  |  |
| --- | --- |
| Change request | `AFRM/docs/CR-001.md` |
| Review technique | Checklist-based self-review |
| Diff reviewed | `git diff v2.0.0 66afd8b` |
| Size of diff | `59 files, 117,903 additions, 180 deletions` |
| Date | `2026-11-18` |

The diff size is increased by coverage reports and the 100,000-record performance dataset.

## A. Correctness and implementation

| # | Check | Result | Finding | Action |
| --- | --- | --- | --- | --- |
| 1 | Units are unchanged and the multiplier is dimensionless | Pass | Weight remains in kg, fuel flows remain in kg/h and distances remain in nm. | None |
| 2 | Cruise and holding fuel flows use the multiplier exactly once | Pass | `Calculations.cpp` applies the multiplier once to both effective flows. | None |
| 3 | Fuel calculations use the effective flows | Pass | Trip, alternate, contingency, reserve and PFD calculations use effective flows. | None |
| 4 | Rounding rules remain unchanged | Pass | Alternate, contingency and reserve round up; trip fuel and PFD do not. | None |
| 5 | Exact weight values and multipliers are used | Pass | LIGHT is 5000 kg/0.25, MEDIUM is 20000 kg/1.0 and HEAVY is 100000 kg/5.0. | None |
| 6 | Unsupported weights cannot silently fall back to MEDIUM | Fail | `validate_flight_record()` rejects unsupported weights, but `weight_burn_multiplier()` has an unchecked MEDIUM fallback. | Document and test the validated-input invariant, or return an error-safe result instead of MEDIUM. |
| 7 | Assessment logic is unchanged | Pass | `Assessment.cpp` and `Assessment.h` have no diff against the pre-CR-001 baseline. | None |

## B. Loader and generator

| # | Check | Result | Finding | Action |
| --- | --- | --- | --- | --- |
| 8 | CSV loading requires the exact nine-column header | Pass | The old eight-column header is rejected as a whole-file error. | None |
| 9 | Invalid weights reject only the affected row | Pass | Invalid rows report the line number and reason while later valid rows continue loading. | None |
| 10 | Generation is deterministic and repeatable | Pass | Equal seeds generate identical data and different seeds generate different data. | None |
| 11 | The generator produces all three weight classes and 21 boundary records | Pass | Seven MEDIUM, seven LIGHT and seven HEAVY boundary records are generated, followed by random records. | None |

## C. Regression and verification

| # | Check | Result | Finding | Action |
| --- | --- | --- | --- | --- |
| 12 | MEDIUM weight preserves v1.0.1 results | Pass | MEDIUM uses multiplier 1.0 and the baseline calculation tests pass. | None |
| 13 | The suite contains 44 passing tests | Pass | 44 tests run and pass in both Debug and Release. | None |
| 14 | Statement and decision coverage targets are met | Pass | Coverage and decision-table evidence meet the targets fixed in `TEST_PLAN.md`. | None |
| 15 | The performance target is met | Pass | The 100,000-record Release timing evidence meets the five-run and 20% regression targets. | None |

## D. Build and documentation

| # | Check | Result | Finding | Action |
| --- | --- | --- | --- | --- |
| 16 | Debug and Release builds are clean and warnings are controlled | Pass | Both configurations build successfully and introduce no controlled-warning failures. | None |
| 17 | New code is consistent and readable | Pass | Weight, loader and generator changes follow existing project style. | None |
| 18 | README and test documentation are updated | Pass | The README documents the nine-column CSV, weight classes and breaking format change. | None |
| 19 | CI passes on the branch and on master | Pass | CI run `44361087112` passed 44 tests in Debug and Release on master. | None |

## Other findings

| # | Finding | Action |
| --- | --- | --- |
| 1 | The 100,000-record CSV and coverage reports add substantial generated artefacts to the diff. | Keep them as evidence; generated IDE state should remain excluded if not required as source. |
| 2 | Implementation progressed through several corrective commits. | Preserve the history and describe the development accurately rather than presenting it as a single implementation commit. |
| 3 | The CSV format is a breaking change. | Migration requires adding `aircraft_weight_kg`; MEDIUM `20000 kg` preserves baseline calculations. |

## Final decision

|  |  |
| --- | --- |
| Decision | Accept with limitations |
| Remaining risks | Unsupported-weight fallback behaviour and the lack of a genuine pre-implementation acceptance-test failure sequence |
| Reviewed by | Ibby (self-review) |
| Date | 2026-11-18 |
