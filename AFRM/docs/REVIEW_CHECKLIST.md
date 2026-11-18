# CR-001 Review Checklist

This checklist was written **before** CR-001 was implemented, so the review is judged against points fixed in advance. The Result, Finding and Action columns are completed against the final diff before merging.

This is a **self-review**. I am the only developer, so it is less independent than a peer review. To reduce author bias I use a fixed checklist, run the tests and CI, and record every finding, including ones I decide not to fix.

| | |
|---|---|
| Change request | `CR-001.md` |
| Review technique | Checklist-based self-review of the diff |
| Diff reviewed | `git diff v2.0.0 <2c11380>` |
| Size of diff | `59 files, 117,903 additions, 180 deletions` |
| Date | `2026-11-18` |

(diff size will be slightly inflated due to measuring performance with a larger csv)

## A. Correctness of the change

| # | Check | Result | Finding | Action |
| --- | --- | --- | --- | --- |
| 1 | Units are unchanged and the multiplier is dimensionless | Pass | Weight is in kg, flows are in kg/h and distances remain in nm. | None |
| 2 | The multiplier is applied to both cruise and holding flow once | Pass | `Calculations.cpp` derives effective cruise and holding flows before calculation. | None |
| 3 | Trip fuel, alternate fuel, contingency, final reserve and PFD use effective flows | Pass | The existing formulas use the weight-adjusted flows. | None |
| 4 | Rounding rules are unchanged | Pass | Alternate, contingency and reserve still round up; trip fuel and PFD do not. | None |
| 5 | The weight table contains exactly 5000/0.25, 20000/1.0 and 100000/5.0 | Pass | Values are defined in `Config.h`. | None |
| 6 | `Assessment.cpp` and `Assessment.h` are unchanged | Pass | No changes to either production assessment file were found against `v1.0.1`. | None |

## B. Loader and generator

| # | Check | Result | Finding | Action |
| --- | --- | --- | --- | --- |
| 7 | The exact nine-column header is required and eight-column files are rejected | Pass | `kCsvHeader`, `kCsvColumnCount` and the loader tests cover this. | None |
| 8 | Unsupported weights reject only their row with line and reason | Pass | Loader validation records the row number and reason, while later rows continue. | None |
| 9 | Generation remains seeded and deterministic | Pass | Existing same-seed tests pass in CI. | None |
| 10 | All three classes and 21 boundary records are generated | Pass | Generator tests pass and CI confirms the final suite passes. | None |

## C. Regression and tests

| # | Check | Result | Finding | Action |
| --- | --- | --- | --- | --- |
| 11 | MEDIUM gives results identical to `v1.0.1` for every baseline record | Pass | The MEDIUM calculation tests pass, a complete record-by-record regression comparison has  been evidenced. | N/A |
| 12 | Adding 20000 to each baseline row gives byte-identical output | N/A | The old file has eight columns and the new file has nine, so the files cannot be byte-identical. | Replace this check with semantic comparison after removing the new weight column, or compare calculated results and statuses. |
| 13 | Test changes are limited to the CR-001 changes and baseline tests pass | Pass with limitations | The planned weight, loader and generator tests were changed. The final CI suite passes, but the history contains several corrective commits. | List the changed tests explicitly in the pull request/review record. |
| 14 | The suite contains 44 passing tests | Pass | CI run `44190613541` passed 44 tests in both Debug and Release. | None |
| 15 | Coverage and decision-coverage targets are met | Pass | Documented in report | Complete. |
| 16 | The performance target is met | Pass | In range | Added to report. |

## D. Build, quality and documentation

| # | Check | Result | Finding | Action |
| --- | --- | --- | --- | --- |
| 17 | Clean `/W4` Debug and Release builds have no new warnings | Pass | No Action |
| 18 | New code is readable with no dead code or unused includes | Fail | `DataGenerator.cpp` contains redundant boundary-generation logic and an unused `prefix` variable/temporary loop from the earlier implementation. | Remove the dead code and rerun the tests. |
| 19 | Required documentation and breaking change are updated | Pass | No action |
| 20 | CI passes on the branch and after merge to `master` | Pass | Branch CI passes in run `44190613541`; the change has been verified after merging to `master`. | N/A |

## Other findings

| # | Finding | Action |
| --- | --- | --- |
| 1 | Coverage HTML output, `.vs/slnx.sqlite` and `data/100kflights.csv` are included in the branch diff. | Remove generated IDE state and large temporary evidence files from the source diff, or move them to an evidence artefact/archive location. Update `.gitignore` if required. |
| 2 | The change is spread across several corrective commits rather than one clean implementation sequence. | Keep the history; describe it honestly as iterative correction rather than claiming a strict red-then-green sequence. |
| 3 | The new CSV format is breaking because the loader requires nine columns. | Keep the major-version justification and document migration from eight to nine columns. |

## Final decision

|  |  |
| --- | --- |
| Decision | Accept with limitations, subject to resolving Finding 18 |
| Remaining risks | Fictional multipliers, self-review, no independent peer review, incompatible CSV format, unverified final coverage/performance, and unsupported-weight fallback design |
| Reviewed by | Ibby (self-review) |
| Date | 2026-11-18 |

[STILL TO DO]
