#include "Phasor.h"

#include <catch2/catch.hpp>

#include "OscillatorUtilities.h"
#include "OscillatorTestConstants.h"
#include "../Utilities/DecibelMatchers.h"
#include "../Utilities/Random.h"

//TODO: add edge check to all phasor tests
TEMPLATE_TEST_CASE("Perform Phasor", "[Phasor]", float, double) {
    constexpr TestType oscillatorFrequency = 440.0;
    constexpr TestType sampleRate = 44100.0;

    Phasor<TestType> phasor{};
    phasor.setFrequency(oscillatorFrequency);
    phasor.setSampleRate(sampleRate);

    const auto phaseIncrement = oscillatorFrequency/sampleRate;

    for (auto i = 0; i < numIterations; ++i) {
        const auto phasorReference = std::fmod(i*phaseIncrement, 1.0);
        const auto phasorOutput = phasor.perform();

        const auto levelsClose = ResidualDecibels<TestType>(phasorReference, residualThreshold<TestType>).match(Decibel<TestType>{Amplitude{phasorOutput}});
        const auto atTransition = closeToEdge({phasorOutput, phasorReference}, TestType{0}, TestType{1});
        const auto testPasses = levelsClose || atTransition;
//            CHECK_THAT(Decibel<TestType>{Amplitude{phasorOutput}},
//                         ResidualDecibels<TestType>(phasorReference, residualThreshold<TestType>));
        CHECK((testPasses));
    }
}

TEMPLATE_TEST_CASE("Perform Phasor With Different Sample Rates", "[Phasor]", float, double) {
    const TestType sampleRate = GENERATE(44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0);
    constexpr TestType oscillatorFrequency = 440.0;

    Phasor<TestType> phasor{};
    phasor.setFrequency(oscillatorFrequency);
    phasor.setSampleRate(sampleRate);

    const auto phaseIncrement = oscillatorFrequency/sampleRate;

    for (auto i = 0; i < numIterations; ++i) {
        const auto phasorReference = std::fmod(i*phaseIncrement, 1.0);
        const auto phasorOutput = phasor.perform();

        const auto levelsClose = ResidualDecibels<TestType>(phasorReference, residualThreshold<TestType>).match(Decibel<TestType>{Amplitude{phasorOutput}});
        const auto atTransition = closeToEdge({phasorOutput, phasorReference}, TestType{0}, TestType{1});
        const auto testPasses = levelsClose || atTransition;

        CHECK(testPasses);
//            CHECK_THAT(Decibel{Amplitude{phasorOutput}},
//                         ResidualDecibels<TestType>(phasorReference, residualThreshold<TestType>));
    }
}

TEMPLATE_TEST_CASE("Perform Phasor With Random Frequencies", "[Phasor]", float, double) {
    const TestType oscillatorFrequency = GENERATE(take(100, random(0.0, 20000.0)));
    constexpr TestType sampleRate = 44100.0;

    Phasor<TestType> phasor{};
    phasor.setFrequency(oscillatorFrequency);
    phasor.setSampleRate(sampleRate);

    const auto phaseIncrement = oscillatorFrequency/sampleRate;

    for (auto i = 0; i < numIterations; ++i) {
        const auto phasorReference = std::fmod(i*phaseIncrement, 1.0);
        const auto phasorOutput = phasor.perform();

        const auto levelsClose = ResidualDecibels<TestType>(phasorReference, residualThreshold<TestType>).match(Decibel<TestType>{Amplitude{phasorOutput}});
        const auto atTransition = closeToEdge({phasorOutput, phasorReference}, TestType{0}, TestType{1});
        const auto testPasses = levelsClose || atTransition;

        CHECK(testPasses);
//            CHECK_THAT(Decibel{Amplitude{phasorOutput}},ResidualDecibels<TestType>(phasorReference, tolerance<TestType>));
    }
}

TEMPLATE_TEST_CASE("Phasor Sync", "[Phasor]", float, double) {
    const TestType oscillatorFrequency = GENERATE(take(100, random(5.0, 20000.0)));
    constexpr TestType sampleRate = 44100.0;

    Phasor<TestType> phasor{};
    phasor.setFrequency(oscillatorFrequency);
    phasor.setSampleRate(sampleRate);

//Pick an amount of samples after which the oscillator is sync'd
    const int mod = getBoundedRandom(1, 100);

    for(int i = 0; i < numIterations; ++i) {
//If we should sync the oscillator, let's sync it and test the output
        if(i%mod) {
            const auto rand = getBoundedRandom(0.0, 1.0);
            const auto phasorOutput = phasor.perform(rand);
            CHECK_THAT(Decibel{Amplitude{phasorOutput}},ResidualDecibels<TestType>(rand, tolerance<TestType>));
//                CHECK_THAT(phasorOutput,
//                             Catch::WithinRel(std::fmod(i*(oscillatorFrequency/sampleRate), TestType{1})));
        }
//Otherwise, just perform the oscillator
        else
            phasor.perform();
    }
}