#include "Oscillator.h"

#include <catch2/catch.hpp>

#include "OscillatorUtilities.h"
#include "OscillatorTestConstants.h"
#include "../Utilities/DecibelMatchers.h"
#include "../Utilities/Random.h"


//TODO: Add tests that change freq and SR on the fly,
// checking to make sure behavior remains correct-
// the changes to the perform function will cause incorrect results in these cases

TEMPLATE_TEST_CASE("Perform Oscillator", "[Oscillator]", float, double) {
    SECTION("Perform Sin Oscillator") {
        constexpr TestType oscillatorFrequency = 440.0;
        constexpr TestType sampleRate = 44100.0;

        Oscillator<TestType> oscillator{};
        oscillator.setFrequency(oscillatorFrequency);
        oscillator.setSampleRate(sampleRate);

        const auto phaseIncrement = oscillatorFrequency/sampleRate;

        for (auto i = 0; i < numIterations; ++i) {
            const auto phaseReference = std::fmod(i, TestType{1}/phaseIncrement)*phaseIncrement;

            const auto oscOut = oscillator.perform();
            const Decibel<TestType> oscillatorLevel = Amplitude{oscOut};
            const auto reference = std::fmod(phaseReference, TestType{1});
            const Decibel<TestType> referenceLevel = Amplitude{reference};


            CHECK((ResidualDecibels<TestType>(referenceLevel, residualThreshold<TestType>).match(oscillatorLevel)
                    || closeToEdge({oscOut, reference}, TestType{0}, TestType{1})));
        }
    }

    SECTION("Perform Sin Oscillator With Different Sample Rates") {
        const TestType sampleRate = GENERATE(44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0);
        constexpr TestType oscillatorFrequency = 440.0;

        Oscillator<TestType> oscillator{};
        oscillator.setFrequency(oscillatorFrequency);
        oscillator.setSampleRate(sampleRate);

        const auto phaseIncrement = oscillatorFrequency/sampleRate;

        for (auto i = 0; i < numIterations; ++i) {
            const auto phaseReference = std::fmod(i, TestType{1}/phaseIncrement)*phaseIncrement;
            const auto oscOutput = oscillator.perform();
            const Decibel<TestType> oscLevel = Amplitude{oscOutput};
            const auto ref = std::fmod(phaseReference, TestType{1});

            CHECK((ResidualDecibels<TestType>(ref, residualThreshold<TestType>).match(oscLevel)
                  || closeToEdge({oscOutput, ref}, TestType{0}, TestType{1})));
        }
    }

    SECTION("Perform Sin Oscillator With Random Frequencies") {
        const TestType oscillatorFrequency = GENERATE(take(100, random(0.0, 20000.0)));
        constexpr TestType sampleRate = 44100.0;

        Oscillator<TestType> oscillator{};
        oscillator.setFrequency(oscillatorFrequency);
        oscillator.setSampleRate(sampleRate);

        const auto phaseIncrement = oscillatorFrequency/sampleRate;

        for (auto i = 0; i < numIterations; ++i) {
            const auto phaseReference = std::fmod(i, TestType{1}/phaseIncrement)*phaseIncrement;
            const Decibel<TestType> oscOutput = Amplitude{oscillator.perform()};

            CHECK_THAT(oscOutput,
                         ResidualDecibels<TestType>(std::fmod(phaseReference, TestType{1}),
                                                            residualThreshold<TestType>));
        }
    }

    SECTION("Oscillator Sync") {
        const TestType oscillatorFrequency = GENERATE(take(100, random(5.0, 20000.0)));
        constexpr TestType sampleRate = 44100.0;

        Oscillator<TestType> oscillator{};
        oscillator.setFrequency(oscillatorFrequency);
        oscillator.setSampleRate(sampleRate);

        //Pick an amount of samples after which the oscillator is sync'd
        const int mod = getBoundedRandom(1, 100);

        for(int i = 0; i < numIterations; ++i) {
            //If we should sync the oscillator, let's sync it and test the output
            if(i%mod) {
                const auto rand = getBoundedRandom(TestType{0}, TestType{1});
                const auto output = oscillator.perform(rand);
                CHECK_THAT(output,
                             Catch::WithinRel(rand));
            }
                //Otherwise, just perform the oscillator
            else
                oscillator.perform();
        }
    }
}