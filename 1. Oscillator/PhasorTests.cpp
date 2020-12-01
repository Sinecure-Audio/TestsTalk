#include "Phasor.h"

#include <catch2/catch.hpp>

#include "OscillatorUtilities.h"
#include "OscillatorTestConstants.h"
#include "../Utilities/DecibelMatchers.h"
#include "../Utilities/Random.h"

//TODO: add edge check to all phasor tests
TEMPLATE_TEST_CASE("Perform Phasor", "[Phasor]", float, double) {
    constexpr auto oscillatorFrequency = TestType{ 440 };
    constexpr auto sampleRate = TestType{ 44100 };

    Phasor<TestType> phasor{};
    phasor.setFrequency(oscillatorFrequency);
    phasor.setSampleRate(sampleRate);

    const auto phaseIncrement = oscillatorFrequency/sampleRate;

    for (auto i = 0; i < numIterations; ++i) {
        const auto phasorReference = std::fmod(i * phaseIncrement, TestType{ 1 });
        const auto phasorOutput = phasor.perform();

        const auto levelsClose = ResidualDecibels<TestType>(phasorReference, residualThreshold<TestType>).match(Decibel<TestType>{Amplitude{phasorOutput}});
        const auto atTransition = closeToEdge({phasorOutput, phasorReference}, TestType{0}, TestType{1});
        const auto testPasses = levelsClose || atTransition;
        CHECK((testPasses));
    }
}

TEMPLATE_TEST_CASE("Perform Phasor With Different Sample Rates", "[Phasor]", float, double) {
    const auto sampleRate = GENERATE(TestType{ 44100.0 }, TestType{ 48000.0 }, TestType{ 88200.0 }, TestType{ 96000.0 }, TestType{ 176400.0 }, TestType{ 192000.0 });
    constexpr auto oscillatorFrequency = TestType{ 440 };

    Phasor<TestType> phasor{};
    phasor.setFrequency(oscillatorFrequency);
    phasor.setSampleRate(sampleRate);

    const auto phaseIncrement = oscillatorFrequency/sampleRate;

    for (auto i = 0; i < numIterations; ++i) {
        const auto phasorReference = std::fmod(i * phaseIncrement, TestType{ 1 });
        const auto phasorOutput = phasor.perform();

        const auto levelsClose = ResidualDecibels<TestType>(phasorReference, residualThreshold<TestType>).match(Decibel<TestType>{Amplitude{phasorOutput}});
        const auto atTransition = closeToEdge({phasorOutput, phasorReference}, TestType{0}, TestType{1});
        const auto testPasses = levelsClose || atTransition;

        CHECK(testPasses);
    }
}

TEMPLATE_TEST_CASE("Perform Phasor With Random Frequencies", "[Phasor]", float, double) {
    const auto oscillatorFrequency = GENERATE(take(100, random(TestType{ 0 }, TestType{ 20000 })));
    constexpr auto sampleRate = TestType{ 44100 };

    Phasor<TestType> phasor{};
    phasor.setFrequency(oscillatorFrequency);
    phasor.setSampleRate(sampleRate);

    const auto phaseIncrement = oscillatorFrequency/sampleRate;

    for (auto i = 0; i < numIterations; ++i) {
        const auto phasorReference = std::fmod(i * phaseIncrement, TestType{ 1 });
        const auto phasorOutput = phasor.perform();

        const auto levelsClose = ResidualDecibels<TestType>(phasorReference, residualThreshold<TestType>).match(Decibel<TestType>{Amplitude{phasorOutput}});
        const auto atTransition = closeToEdge({phasorOutput, phasorReference}, TestType{0}, TestType{1});
        const auto testPasses = levelsClose || atTransition;

        CHECK(testPasses);
    }
}

TEMPLATE_TEST_CASE("Phasor Sync", "[Phasor]", float, double) {
    const auto oscillatorFrequency = GENERATE(take(100, random(TestType{ 5 }, TestType{ 20000 })));
    constexpr auto sampleRate = TestType{ 44100 };

    Phasor<TestType> phasor{};
    phasor.setFrequency(oscillatorFrequency);
    phasor.setSampleRate(sampleRate);

//Pick an amount of samples after which the oscillator is sync'd
    const int mod = getBoundedRandom(1, 100);

    for(int i = 0; i < numIterations; ++i) {
//If we should sync the oscillator, let's sync it and test the output
        if(i%mod) {
            const auto rand = getBoundedRandom(TestType{ 0 }, TestType{ 1 });
            const auto phasorOutput = phasor.perform(rand);
            CHECK_THAT(Decibel{Amplitude{phasorOutput}},ResidualDecibels<TestType>(rand, tolerance<TestType>));
        }
//Otherwise, just perform the oscillator
        else
            phasor.perform();
    }
}