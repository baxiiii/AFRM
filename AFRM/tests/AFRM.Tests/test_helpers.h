// test_helpers.h - shared helpers for the AFRM tests.
#pragma once

#include "C:\Users\ibrah\OneDrive\Documents\University\REPOs\AFRM\AFRM\src\AFRM.Core\Models.h"

namespace afrm_test {

    inline constexpr double kTolerance = 1e-6;

    // Base flight with exact values:
    //   remaining 600 nm at 400 kt = 1.5 h; trip fuel 3600; alternate 1200;
    //   contingency 180 (5% of trip fuel = 5 min holding, the crossover); final reserve 1080;
    //   protected 2460, diversion protected 2280, PFD 2460.
    inline afrm::FlightRecord make_base_flight() {
        return afrm::FlightRecord{ "TEST-01", 1000.0, 200.0, 400.0, 6060.0, 400.0, 2400.0, 2160.0 };
    }

}  // namespace afrm_test