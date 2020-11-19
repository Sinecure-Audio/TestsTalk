#include <catch2/catch.hpp>

#include "DecibelMatchers.h"

#include "Random.h"

static constexpr size_t numIterations = 1000000;

TEST_CASE("Within Decibels", "[Decibel Matchers]") {
    SECTION("Known Inputs") {
        REQUIRE_THAT(0.0_dB, WithinDecibels(0.0_dB, 0.001_dB));
    REQUIRE_THAT(12.0_dB, WithinDecibels(12.4_dB, 0.5_dB));
        REQUIRE_THAT(-0.01_dB, WithinDecibels(0.01_dB, 0.025_dB));
    }

    SECTION("Random Numbers") {
        for (auto i = 0; i < numIterations; ++i) {
            const auto db1 = Decibel{getBoundedRandom(-120.0, 120.0)};
            const auto db2 = Decibel{getBoundedRandom(-120.0, 120.0)};
            const auto dbDiff = Decibel{std::abs(db1.count()-db2.count())+.001};
            REQUIRE_THAT(db1, WithinDecibels(db1, Decibel{0.001}));
            REQUIRE_THAT(db2, WithinDecibels(db2, Decibel{0.001}));
            REQUIRE_THAT(db1, WithinDecibels(db2, dbDiff));
        }
    }
}


TEST_CASE("Residual Decibels", "[Decibel Matchers]") {
    SECTION("Known Inputs") {
        REQUIRE_THAT(0.0_dB, ResidualDecibels(0.0_dB, -120.0_dB));
        REQUIRE_THAT(-12.0_dB, ResidualDecibels(-12.4_dB, (-12.0_dB - -12.5_dB)+.01_dB));
        REQUIRE_THAT(-12.5_dB, ResidualDecibels(-12.0_dB, (-12.5_dB - -12.0_dB)+.01_dB));
    }

    SECTION("Random Numbers") {
        for (auto i = 0; i < numIterations; ++i) {
            const auto db1 = Decibel{getBoundedRandom(-120.0, 120.0)};
            const auto db2 = Decibel{getBoundedRandom(-120.0, 120.0)};
            const auto dbDiff = db1-db2;
            REQUIRE_THAT(db1, ResidualDecibels(db2, dbDiff+.01_dB));
        }
    }
}