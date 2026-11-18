# CR-001 Review Checklist

This checklist was written **before** CR-001 was implemented, so the review is judged against points fixed in advance. The Result, Finding and Action columns are completed against the final diff before merging.

This is a **self-review**. I am the only developer, so it is less independent than a peer review. To reduce author bias I use a fixed checklist, run the tests and CI, and record every finding, including ones I decide not to fix.

| | |
|---|---|
| Change request | `CR-001.md` |
| Review technique | Checklist-based self-review of the diff |
| Diff reviewed | `git diff v2.0.0 <merge-candidate commit>` |
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
| 6 | Unsupported weights cannot silently fall back to MEDIUM | Fail | Validation rejects unsupported weights, but `weight_burn_multiplier()` still has a MEDIUM fallback return. | Replace the fallback with an explicit safe design, or make the function require validated input and document the invariant. Add a direct helper test. |
| 7 | `Assessment.cpp` and `Assessment.h` are unchanged | Pass | No changes to either production assessment file were found against `v1.0.1`. | None |

## B. Loader and generator

| # | Check | Result | Finding | Action |
| --- | --- | --- | --- | --- |
| 8 | The exact nine-column header is required and eight-column files are rejected | Pass | `kCsvHeader`, `kCsvColumnCount` and the loader tests cover this. | None |
| 9 | Unsupported weights reject only their row with line and reason | Pass | Loader validation records the row number and reason, while later rows continue. | None |
| 10 | Generation remains seeded and deterministic | Pass | Existing same-seed tests pass in CI. | None |
| 11 | All three classes and 21 boundary records are generated | Pass | Generator tests pass and CI confirms the final suite passes. | None |

## C. Regression and tests

| # | Check | Result | Finding | Action |
| --- | --- | --- | --- | --- |
| 12 | MEDIUM gives results identical to `v1.0.1` for every baseline record | Pending | The MEDIUM calculation tests pass, but a complete record-by-record regression comparison has not yet been evidenced. | Run both versions against equivalent MEDIUM data and save the comparison output. |
| 13 | Adding 20000 to each baseline row gives byte-identical output | N/A | The old file has eight columns and the new file has nine, so the files cannot be byte-identical. | Replace this check with semantic comparison after removing the new weight column, or compare calculated results and statuses. |
| 14 | All 11 acceptance tests were committed before implementation and failed first | Fail | The acceptance-test commit `285086e226` appears after the implementation commits `3c37cc2bfa` and `151b3c3f9f`. | Do not claim genuine red-then-green ordering. Record the actual history accurately. |
| 15 | Test changes are limited to the CR-001 changes and baseline tests pass | Pass with limitations | The planned weight, loader and generator tests were changed. The final CI suite passes, but the history contains several corrective commits. | List the changed tests explicitly in the pull request/review record. |
| 16 | The suite contains 44 passing tests | Pass | CI run `44190613541` passed 44 tests in both Debug and Release. | None |
| 17 | Coverage and decision-coverage targets are met | Pending | The v1.0.1 coverage report is present, but a final v2.0.0 coverage result and completed decision table are not yet recorded. | Run final coverage and complete the decision table. |
| 18 | The performance target is met | Pending | A 100,000-record dataset is present, but final profiler results and repeated timings are not recorded here. | Run the Release profiler and save the five timing results. |

## D. Build, quality and documentation

| # | Check | Result | Finding | Action |
| --- | --- | --- | --- | --- |
| 19 | Clean `/W4` Debug and Release builds have no new warnings | Pending | CI builds pass, but the warning output has not been separately reviewed as evidence. | Save the Debug and Release build logs and confirm warnings. |
| 20 | New code is readable with no dead code or unused includes | Fail | `DataGenerator.cpp` contains redundant boundary-generation logic and an unused `prefix` variable/temporary loop from the earlier implementation. | Remove the dead code and rerun the tests. |
| 21 | Required documentation and breaking change are updated | Fail | `CR-001.md`, `TEST_PLAN.md` and `REVIEW_CHECKLIST.md` exist, but `CHANGELOG.md` and `CONFIGURATION.md` are not present in the current branch. | Add the missing documents or amend the checklist to state that they were deliberately omitted. Update the README with the nine-column format. |
| 22 | CI passes on the branch and after merge to `master` | Pending | Branch CI passes in run `44190613541`; the change has not yet been verified after merging to `master`. | Merge only after the remaining findings are resolved, then record the master CI run. |

## Other findings

| # | Finding | Action |
| --- | --- | --- |
| 1 | Coverage HTML output, `.vs/slnx.sqlite` and `data/100kflights.csv` are included in the branch diff. | Remove generated IDE state and large temporary evidence files from the source diff, or move them to an evidence artefact/archive location. Update `.gitignore` if required. |
| 2 | The change is spread across several corrective commits rather than one clean implementation sequence. | Keep the history; describe it honestly as iterative correction rather than claiming a strict red-then-green sequence. |
| 3 | The new CSV format is breaking because the loader requires nine columns. | Keep the major-version justification and document migration from eight to nine columns. |

## Final decision

|  |  |
| --- | --- |
| Decision | Accept with limitations, subject to resolving Findings 6, 20 and 21 and recording items 12, 17, 18, 19 and 22 |
| Remaining risks | Fictional multipliers, self-review, no independent peer review, incompatible CSV format, unverified final coverage/performance, and unsupported-weight fallback design |
| Reviewed by | Ibby (self-review) |
| Date | 2026-11-18 |

[STILL TO DO]
