# AFRM Requirements and Traceability

AFRM is an educational prototype. It is not an operational flight-planning
system and is not approved aviation guidance. All data is fictional and all
thresholds are simplified teaching assumptions.

## Requirements

| ID | Requirement |
|---|---|
| R1 | Fuel components and remaining-trip calculations are calculated correctly. |
| R2 | Contingency fuel is the greater of 5% of trip fuel to go and five minutes of holding fuel. |
| R3 | Final reserve fuel equals thirty minutes of holding fuel. |
| R4 | Predicted fuel at destination is calculated from fuel on board minus trip fuel to go and is used for assessment. |
| R5 | All four statuses and their exact threshold boundaries behave correctly. |
| R6 | Invalid records are rejected safely and generated data is repeatable. |

## Units

- Fuel: kilograms (`kg`)
- Distance: nautical miles (`nm`)
- Time: hours (`h`)
- Speed: knots (`nm/h`)

## Formulas

```text
remaining_distance_nm =
    trip_distance_nm - distance_flown_nm

trip_fuel_to_go_kg =
    remaining_distance_nm / ground_speed_knots
    * cruise_flow_kg_per_hour

alternate_fuel_kg =
    alternate_distance_nm / ground_speed_knots
    * cruise_flow_kg_per_hour

contingency_fuel_kg =
    max(
        0.05 * trip_fuel_to_go_kg,
        holding_flow_kg_per_hour * 5 / 60
    )

final_reserve_fuel_kg =
    holding_flow_kg_per_hour * 30 / 60

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
kilograms before the protection totals are calculated. Trip fuel to go and
predicted fuel at destination are not rounded.

## Status bands

Statuses are evaluated in this order:

| Order | Condition | Status |
|---:|---|---|
| 1 | `PFD >= protected_fuel_kg` | `NOMINAL` |
| 2 | `PFD >= diversion_protected_fuel_kg` | `CAUTION` |
| 3 | `PFD >= final_reserve_fuel_kg` | `MINIMUM FUEL` |
| 4 | Otherwise | `FUEL EMERGENCY` |

The comparisons use `>=`, so equality satisfies a threshold.

## Validation and rejection rules

A flight record is rejected when:

- `flight_id` is empty;
- a numeric value is NaN or infinity;
- a distance is negative;
- fuel on board is negative;
- distance flown is greater than trip distance;
- ground speed is zero or negative;
- cruise fuel flow is zero or negative;
- holding fuel flow is zero or negative.

The CSV loader also rejects rows with:

- too few fields;
- too many fields;
- missing numeric values;
- malformed numeric values;
- an invalid or missing header.

Invalid rows are recorded with their one-based line number and a reason.
Loading continues for later rows.

## Test naming convention

Tests use the following format:

```text
TEST(<Suite>, REQ_<AREA>_<NUMBER>_<Description>)
```

The current requirement prefixes are:

| Prefix | Requirement |
|---|---|
| `REQ_CALC_01` | R1 |
| `REQ_CONT_01` | R2 |
| `REQ_RESERVE_01` | R3 |
| `REQ_PFD_01` | R4 |
| `REQ_STATUS_01` | R5 |
| `REQ_DATA_01` | R6 |

## Requirement-to-implementation-to-test mapping

| Requirement | Implementation | Current tests |
|---|---|---|
| R1 | `src/AFRM.Core/Calculations.h`, `Calculations.cpp`, `Models.h`, `Config.h` | `Calculations.REQ_CALC_01_BaseFlightValues`; `DifferentSpeedAndFlow`; `NothingLeftToFly`; `InvalidInputsRejected`; `BoundaryButValidInputsAccepted`; `RoundUpToWholeKg`; `RequiredFuelRoundedUpTripFuelAndPfdNot` |
| R2 | `src/AFRM.Core/Calculations.cpp`, `Config.h` | `Calculations.REQ_CONT_01_BothBranchesAndCrossover` |
| R3 | `src/AFRM.Core/Calculations.cpp`, `Config.h` | `Calculations.REQ_RESERVE_01_ThirtyMinutesOfHolding` |
| R4 | `src/AFRM.Core/Calculations.cpp`, `Assessment.cpp` | `Calculations.REQ_PFD_01_FuelOnBoardMinusTripFuelToGo`; `Assessment.REQ_PFD_01_StatusFromFullPipelineOnBoundaryFlights`; `Assessment.REQ_PFD_01_RoundingNeverMakesMarginalCaseSafer` |
| R5 | `src/AFRM.Core/Assessment.h`, `Assessment.cpp`, `Models.h` | `Assessment.REQ_STATUS_01_StatusLabelsAreExact`; `FourBands`; `ExactlyOnEachThreshold`; `OneKilogramEitherSideOfEachThreshold`; `NegativeAndZeroPfdAreEmergency`; `NaNPfdFailsSafeToEmergency`; `ExplanationIsGivenForEveryStatus`; `DataGenerator.REQ_STATUS_01_BoundaryRecordsHitTheThresholds` |
| R6 | `src/AFRM.Core/CsvLoader.h`, `CsvLoader.cpp`, `DataGenerator.h`, `DataGenerator.cpp`, `Calculations.cpp` | `CsvLoader.REQ_DATA_01_ValidRowsLoaded`; `DecimalsSpacesAndCrLfAccepted`; `EachInvalidRowIsRejectedWithLineAndReason`; `InvalidRowsDoNotLoseValidRows`; `BadOrMissingHeaderRejectsWholeFile`; `MissingFileReportsError`; `DataGenerator.REQ_DATA_01_SameSeedGivesIdenticalData`; `DifferentSeedsGiveDifferentData`; `RecordCountFollowsArgument`; `AllRecordsAreValidAndUniquelyNamed`; `GeneratedDataExercisesAllFourStatuses`; `CsvHasHeaderAndEightColumns`; `GeneratedCsvLoadsBackWithoutRejections` |

## Implementation status

The requirements above describe the current simplified implementation.

The documentation does not claim that the project has been built or that all
tests have passed. Verification should be performed by building the solution
and running `AFRM.Tests.exe`.

## Scope limitations

The current model does not include:

- wind;
- climb or descent;
- taxi fuel;
- aircraft weight;
- aircraft-specific performance tables;
- delays;
- route changes;
- operational reserve rules;
- real-world dispatch or pilot decision support.

## Provenance

This document describes the current implementation and its tests. It does not
claim that every requirement was written before implementation. Git history is
the appropriate evidence for development chronology.