#include "Oscillator.h"

#include <catch2/catch.hpp>

TEST_CASE("Perform", "[Oscillator]") {
    //Make an oscillator and check to see that when performed it outputs 0.
    Oscillator osc{};
    REQUIRE_THAT(osc.perform(), Catch::WithinRel(0.0));
};