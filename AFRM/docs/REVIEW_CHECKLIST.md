# CR-001 Review Checklist

This checklist was written **before** CR-001 was implemented, so the review is judged against points fixed in advance. The Result, Finding and Action columns are completed against the final diff before merging.

This is a **self-review**. I am the only developer, so it is less independent than a peer review. To reduce author bias I use a fixed checklist, run the tests and CI, and record every finding, including ones I decide not to fix.

| | |
|---|---|
| Change request | `CR-001.md` |
| Review technique | Checklist-based self-review of the diff |
| Diff reviewed | `git diff v1.0.1 <merge-candidate commit>` |
| Size of diff | `<files changed, lines added, lines removed>` |
| Date | `YYYY-MM-DD` |

**Result** is Pass, Fail or N/A. **Action** is what I did about the finding.

## A. Correctness of the change

| # | Check | Result | Finding | Action |
|---|---|---|---|---|
| 1 | Units are unchanged (kg, nm, knots, kg/h); the multiplier is dimensionless | | | |
| 2 | The multiplier is applied to **both** cruise and holding flow, and only once | | | |
| 3 | Trip fuel, alternate fuel, contingency, final reserve and PFD all use the effective flows, with the formulas in `CR-001.md` section 2 | | | |
| 4 | Rounding rules are unchanged: alternate, contingency and final reserve round up, trip fuel and PFD do not | | | |
| 5 | The weight table in `Config.h` holds exactly 5000/0.25, 20000/1.0 and 100000/5.0, with no magic numbers elsewhere | | | |
| 6 | An unsupported weight can never fall back to MEDIUM silently (no unchecked default in the mapping or `switch`) | | | |
| 7 | `Assessment.cpp` and `Assessment.h` have no diff since `v1.0.1` (`git diff v1.0.1 -- src/AFRM.Core/Assessment.cpp src/AFRM.Core/Assessment.h` is empty) | | | |

## B. Loader and generator

| # | Check | Result | Finding | Action |
|---|---|---|---|---|
| 8 | The loader requires the exact nine-column header and rejects the eight-column file as a whole with a clear error | | | |
| 9 | A bad or unsupported weight rejects only that row, with line number and reason, and later rows still load | | | |
| 10 | The generator is still seeded and deterministic, so the same seed gives identical data | | | |
| 11 | The generator produces all three classes, scales fuel on board per class, and creates 21 boundary records from hand-worked values | | | |

## C. Regression and tests

| # | Check | Result | Finding | Action |
|---|---|---|---|---|
| 12 | MEDIUM gives results identical to `v1.0.1` for every baseline record | | | |
| 13 | The baseline data with `20000` added to each row gives byte-identical output to `v1.0.1` (hashes saved in `evidence/`) | | | |
| 14 | All 11 acceptance tests exist, were committed before the implementation, and failed on assertions in CI first | | | |
| 15 | Test changes are limited to those in `CR-001.md` section 6 (two `REQ_WEIGHT_00` tests retired, header test updated, loader fixtures if needed), each recorded in the pull request, and every other baseline test passes unchanged | | | |
| 16 | The suite contains 44 tests (35 − 2 + 11) and all pass | | | |
| 17 | Statement and decision coverage targets in `TEST_PLAN.md` are met, and the decision table is complete | | | |
| 18 | The performance target in `TEST_PLAN.md` is met | | | |

## D. Build, quality and documentation

| # | Check | Result | Finding | Action |
|---|---|---|---|---|
| 19 | Clean build at `/W4` with no new warnings in Debug and Release, including no unhandled-enum warnings | | | |
| 20 | New code is readable and consistent with the existing style, with no dead code or unused includes | | | |
| 21 | `README.md`, `CHANGELOG.md`, `CONFIGURATION.md`, `TEST_PLAN.md` and the sample CSV files in `data/` are updated, and the breaking change is stated | | | |
| 22 | CI passes on the branch and again on `master` after the merge | | | |

## Other findings

Findings that were not on the checklist but were noticed during the review.

| # | Finding | Action |
|---|---|---|
| | | |

## Final decision

| | |
|---|---|
| Decision (accept / accept with limitations / reject) | |
| Remaining risks | |
| Reviewed by | Ibby (self-review) |
| Date | |

[STILL TO DO]