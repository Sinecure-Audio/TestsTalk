#include "Oscillator.h"

#include <catch2/catch.hpp>

#include "../../Sinecure-Audio-Library/Utilities/Units/include/Units.h"

constexpr size_t numIterations = 50;

TEST_CASE("Perform", "[Oscillator]") {
    SECTION("Perform Sin Oscillator") {
        //Set up variables and the oscillator
        constexpr auto oscillatorFrequency = 440.0;
        constexpr auto sampleRate = 44100.0;

        Oscillator<double> oscillator{};
        oscillator.setFrequency(oscillatorFrequency);
        oscillator.setSampleRate(sampleRate);

        for(auto i = 0; i < numIterations; ++i) {
            const auto angleInRadians =  oscillatorFrequency
                                         * i
                                         * juce::MathConstants<double>::twoPi
                                         / sampleRate;
            REQUIRE_THAT(oscillator.perform(), Catch::WithinRel(std::sin(angleInRadians)));
        }
    }

    SECTION("Perform With Different Sample Rates") {
        const auto sampleRate = GENERATE(44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0);
        constexpr auto oscillatorFrequency = 440.0;

        Oscillator<double> oscillator{};
        oscillator.setFrequency(oscillatorFrequency);
        oscillator.setSampleRate(sampleRate);

        for(auto i = 0; i < numIterations; ++i) {
            const auto angleInRadians =  oscillatorFrequency
                                         * i
                                         * juce::MathConstants<double>::twoPi
                                         / sampleRate;
            REQUIRE_THAT(oscillator.perform(), Catch::WithinRel(std::sin(angleInRadians)));
        }
    }

    SECTION("Perform Sin Oscillator With Random Frequencies") {
        const auto oscillatorFrequency = GENERATE(take(100, random(0.0, 20000.0)));
        constexpr auto sampleRate = 44100.0;

        Oscillator<double> oscillator{};
        oscillator.setFrequency(oscillatorFrequency);
        oscillator.setSampleRate(sampleRate);

        for(auto i = 0; i < numIterations; ++i) {
            const auto angleInRadians =  oscillatorFrequency
                                         * i
                                         * juce::MathConstants<double>::twoPi
                                         / sampleRate;

        REQUIRE_THAT(oscillator.perform(),
                     Catch::WithinAbs(std::sin(angleInRadians), .00000001));
        }
    }
}
