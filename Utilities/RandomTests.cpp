#include <catch2/catch.hpp>

#include "Random.h"

//
TEST_CASE("Noise Generator Perform", "[Noise]") {
    SECTION("Unit Amplitude Random Generation") {
        constexpr auto min = -1.0;
        constexpr auto max = 1.0;

        for (auto i = 0; i < 1000000; ++i) {
            const auto rand = getBoundedRandom(min, max);
            CHECK(rand <= max);
            CHECK(rand >= min);
        }
    }

    SECTION("Random Bounds Random Generation") {
        //Use half the numeric limit because catch's rng only outputs infs if you let it go the whole range
        const auto bound1 = GENERATE(take(100, random(std::numeric_limits<double>::lowest()*.5, std::numeric_limits<double>::max()*.5)));
        const auto bound2 = GENERATE(take(100, random(std::numeric_limits<double>::lowest()*.5, std::numeric_limits<double>::max()*.5)));

        for (auto i = 0; i < 100; ++i) {
            const auto rand = getBoundedRandom(bound1, bound2);
            const auto max = std::max(bound1, bound2);
            const auto min = std::min(bound1, bound2);

            const auto closeToMax = Catch::WithinRel(max).match(rand);
            const auto closeToMin = Catch::WithinRel(min).match(rand);
            CHECK((closeToMax || rand < max));
            CHECK((closeToMin || rand > min));
        }
    }
}