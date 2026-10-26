// Assessment.cpp - see Assessment.h. No I/O in this file.

#include "C:\Users\ibrah\OneDrive\Documents\University\REPOs\AFRM\AFRM\src\AFRM.Core\Assessment.h"

#include <format>  // C++20; std::format is locale-independent by default

namespace afrm {

    FuelAssessment assess_fuel_status(const FuelCalculation& calculation) {
        const double pfd_kg = calculation.predicted_fuel_at_destination_kg;
        FuelAssessment assessment;

        if (pfd_kg >= calculation.protected_fuel_kg) {
            assessment.status = FuelStatus::Nominal;
            assessment.explanation = std::format(
                "PFD {:.1f} kg covers contingency, alternate and final reserve ({:.0f} kg required).",
                pfd_kg, calculation.protected_fuel_kg);
        }
        else if (pfd_kg >= calculation.diversion_protected_fuel_kg) {
            assessment.status = FuelStatus::Caution;
            assessment.explanation = std::format(
                "PFD {:.1f} kg is below the {:.0f} kg needed for contingency, alternate and final reserve; "
                "alternate and final reserve ({:.0f} kg) remain protected.",
                pfd_kg, calculation.protected_fuel_kg, calculation.diversion_protected_fuel_kg);
        }
        else if (pfd_kg >= calculation.final_reserve_fuel_kg) {
            assessment.status = FuelStatus::MinimumFuel;
            assessment.explanation = std::format(
                "PFD {:.1f} kg is below the {:.0f} kg needed for alternate and final reserve; "
                "final reserve ({:.0f} kg) remains protected.",
                pfd_kg, calculation.diversion_protected_fuel_kg, calculation.final_reserve_fuel_kg);
        }
        else {
            assessment.status = FuelStatus::FuelEmergency;
            assessment.explanation = std::format(
                "PFD {:.1f} kg is below the final reserve of {:.0f} kg.",
                pfd_kg, calculation.final_reserve_fuel_kg);
        }
        return assessment;
    }

}  // namespace afrm