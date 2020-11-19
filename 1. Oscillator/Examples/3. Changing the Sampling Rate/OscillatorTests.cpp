#include "Oscillator.h"

#include <catch2/catch.hpp>

constexpr size_t numIterations = 50;

TEST_CASE("Perform", "[Oscillator]") {
    SECTION("Perform Sin Oscillator") {
        //Set up variables and the oscillator
        constexpr auto oscillatorFrequency = 440.0;
        constexpr auto sampleRate = 44100.0;

        Oscillator<double> oscillator{};
        oscillator.setSampleRate(sampleRate);

        //Check to see that the sin oscillator is close to our reference when performed many times
        for(auto i = 0; i < numIterations; ++i) {
            const auto angleInRadians =  oscillatorFrequency
                                         * i
                                         * juce::MathConstants<double>::twoPi
                                         / sampleRate;
            REQUIRE_THAT(oscillator.perform(), Catch::WithinRel(std::sin(angleInRadians)));
        }
    }

    SECTION("Change Sample Rate") {
//        Set up the oscillator
        constexpr auto oscillatorFrequency = 440.0;
        Oscillator<double> oscillator{};

        //Make a lambda that captures the oscillator and the frequency
        //And that performs the oscillator several times with a variable sample rate
        const auto runTest = [&](const auto &performRate) {
            oscillator.setSampleRate(performRate);
            oscillator.reset();

            for (auto i = 0; i < numIterations; ++i) {
                const auto angleInRadians = oscillatorFrequency
                                            * i
                                            * juce::MathConstants<double>::twoPi
                                            / performRate;
                const auto reference = std::sin(angleInRadians);
                REQUIRE_THAT(oscillator.perform(), Catch::WithinRel(reference));
            }
        };

        //Run the perform test with several common sample rates
        runTest(44100.0);
        runTest(48000.0);
        runTest(88200.0);
        runTest(96000.0);
        runTest(176400.0);
        runTest(192000.0);
    }
    
}
