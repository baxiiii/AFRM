// Assessment.h - the single status-assessment function used by the
// application, the data generator and the tests.
//
// Educational prototype only. The four statuses are simplified indicative
// flags, NOT formal pilot declarations, and are not approved aviation guidance.

#pragma once

#include "C:\Users\ibrah\OneDrive\Documents\University\REPOs\AFRM\AFRM\src\AFRM.Core\Models.h"

namespace afrm {

	// Compares predicted fuel at destination (PFD) with the stored protection
	// quantities, in this order (comparisons are >=, so equality counts as met):
	//
	//   PFD >= protected_fuel_kg             -> NOMINAL
	//   PFD >= diversion_protected_fuel_kg   -> CAUTION
	//   PFD >= final_reserve_fuel_kg         -> MINIMUM FUEL
	//   otherwise                            -> FUEL EMERGENCY
	//
	// A NaN PFD fails every comparison and so is reported as FUEL EMERGENCY.
	// The explanation states why the status was assigned, with the key quantities.
	FuelAssessment assess_fuel_status(const FuelCalculation& calculation);

}  // namespace afrm#
