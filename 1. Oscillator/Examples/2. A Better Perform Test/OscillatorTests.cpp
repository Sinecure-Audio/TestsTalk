#include "Oscillator.h"

#include <catch2/catch.hpp>

TEST_CASE("Perform", "[Oscillator]") {
    //Set up variables and the oscillator
    constexpr auto sampleRate = 44100.0;
    constexpr auto oscillatorFrequency = 440.0;

    Oscillator oscillator{};

    SECTION("Perform Sin Oscillator") {
        //Check to see that the sin oscillator is close to our reference when performed many times
        for(auto i = 0; i < 50; ++i) {
            const auto angleInRadians = oscillatorFrequency
                                        * i
                                        * juce::MathConstants<double>::twoPi
                                        / sampleRate;
            const auto reference = std::sin(angleInRadians);
            REQUIRE_THAT(oscillator.perform(), Catch::WithinRel(reference));
        }
    }
}
