// Calculations.h
#pragma once

#include "C:\Users\ibrah\OneDrive\Documents\University\REPOs\AFRM\AFRM\src\AFRM.Core\Models.h"

namespace afrm {

	// Rejects: empty id; NaN/infinity; negative distances or fuel; distance flown
	// greater than trip distance; speed, cruise flow or holding flow <= 0.
	ValidationResult validate_flight_record(const FlightRecord& record);

	// Rounds UP to the next whole kg (tiny floating-point noise is ignored).
	double round_up_to_whole_kg(double quantity_kg);

	// Call only with a record that passed validate_flight_record().
	FuelCalculation calculate_fuel(const FlightRecord& record);

}  // namespace afrm