#include "Oscillator.h"

#include <catch2/catch.hpp>

#include "OscillatorUtilities.h"
#include "../Utilities/DecibelMatchers.h"
#include "../Utilities/Random.h"
#include "../Utilities/Lerp.h"

constexpr size_t numIterations = 100000;
//This is turned down to incite errors. Use this to see which lines the test reposrts as failing.
// If it does so well, consider refactoring the tests to use a common run oscillator test function
const auto residualThreshold = Decibel<double>(-120.0);
const auto tolerance = Decibel<double>(.5);

using TestType = double;

TEST_CASE("Perform Phasor", "[Phasor]") {
    SECTION("Perform Phasor") {
        constexpr TestType oscillatorFrequency = 440.0;
        constexpr TestType sampleRate = 44100.0;

        Phasor<TestType> phasor{};
        phasor.setFrequency(oscillatorFrequency);
        phasor.setSampleRate(sampleRate);

        for (auto i = 0; i < numIterations; ++i) {
            const auto phasorReference = i*oscillatorFrequency/ sampleRate;
            REQUIRE_THAT(Decibel<TestType>::convertAmplitudeToDecibel(phasor.perform()),
                         ResidualDecibels<TestType>(phasorReference, residualThreshold));
        }
    }

    SECTION("Perform Phasor With Different Sample Rates") {
        const TestType sampleRate = GENERATE(44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0);
        constexpr TestType oscillatorFrequency = 440.0;

        Phasor<TestType> phasor{};
        phasor.setFrequency(oscillatorFrequency);
        phasor.setSampleRate(sampleRate);

        for (auto i = 0; i < numIterations; ++i) {
            const auto phasorReference = i*oscillatorFrequency/ sampleRate;
            REQUIRE_THAT(Decibel<TestType>::convertAmplitudeToDecibel(phasor.perform()),
                         ResidualDecibels<TestType>(phasorReference, residualThreshold));
        }
    }

    SECTION("Perform Phasor With Random Frequencies") {
        const TestType oscillatorFrequency = GENERATE(take(100, random(0.0, 20000.0)));
        constexpr TestType sampleRate = 44100.0;

        Phasor<TestType> phasor{};
        phasor.setFrequency(oscillatorFrequency);
        phasor.setSampleRate(sampleRate);

        for (auto i = 0; i < numIterations; ++i) {
            const auto phasorReference = std::fmod(i*oscillatorFrequency/ sampleRate, 1.0);
            const auto oscOutput = Decibel<TestType>::convertAmplitudeToDecibel(phasor.perform());
            REQUIRE_THAT(oscOutput,ResidualDecibels<TestType>(phasorReference, tolerance));
        }
    }

    SECTION("Phasor Sync") {
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
                const auto output = phasor.perform(rand);
                REQUIRE_THAT(output,
                             Catch::WithinRel(std::fmod(i*(oscillatorFrequency/sampleRate), TestType{1})));
            }
                //Otherwise, just perform the oscillator
            else
                phasor.perform();
        }
    }
}

TEST_CASE("Perform Oscillator", "[Oscillator]") {
    SECTION("Perform Sin Oscillator") {
        constexpr TestType oscillatorFrequency = 440.0;
        constexpr TestType sampleRate = 44100.0;

        Oscillator<TestType> oscillator{};
        oscillator.setFrequency(oscillatorFrequency);
        oscillator.setSampleRate(sampleRate);

        for (auto i = 0; i < numIterations; ++i) {
            const auto angleInRadians = oscillatorFrequency
                                        * i
                                        / sampleRate;

            const auto oscOut = oscillator.perform();
            const Decibel<TestType> oscillatorLevel = Amplitude{oscOut};
            const auto reference = std::fmod(angleInRadians, TestType{1});
            const Decibel<TestType> referenceLevel = Amplitude{reference};


            REQUIRE((ResidualDecibels<TestType>(referenceLevel, residualThreshold).match(oscillatorLevel)
                    || closeToEdge({oscOut, reference}, TestType{0}, TestType{1})));
        }
    }

    SECTION("Perform Sin Oscillator With Different Sample Rates") {
        const TestType sampleRate = GENERATE(44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0);
        constexpr TestType oscillatorFrequency = 440.0;

        Oscillator<TestType> oscillator{};
        oscillator.setFrequency(oscillatorFrequency);
        oscillator.setSampleRate(sampleRate);

        for (auto i = 0; i < numIterations; ++i) {
            const auto angleInRadians = oscillatorFrequency
                                        * i
                                        / sampleRate;
            const auto oscOutput = oscillator.perform();
            const Decibel<TestType> oscLevel = Amplitude{oscOutput};
            const auto ref = std::fmod(angleInRadians, TestType{1});
            const Decibel<TestType> refLevel = Amplitude{ref};

            REQUIRE((ResidualDecibels<TestType>(ref, residualThreshold).match(oscLevel)
                  || closeToEdge({oscOutput, ref}, TestType{0}, TestType{1})));
        }
    }

    SECTION("Perform Sin Oscillator With Random Frequencies") {
        const TestType oscillatorFrequency = GENERATE(take(100, random(0.0, 20000.0)));
        constexpr TestType sampleRate = 44100.0;

        Oscillator<TestType> oscillator{};
        oscillator.setFrequency(oscillatorFrequency);
        oscillator.setSampleRate(sampleRate);

        for (auto i = 0; i < numIterations; ++i) {
            const auto angleInRadians = oscillatorFrequency
                                        * i
                                        / sampleRate;
            const Decibel<TestType> oscOutput = Amplitude{oscillator.perform()};

            REQUIRE_THAT(oscOutput,
                         ResidualDecibels<TestType>(std::fmod(angleInRadians, TestType{1}),
                                                            residualThreshold));
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
                REQUIRE_THAT(output,
                             Catch::WithinRel(rand));
            }
                //Otherwise, just perform the oscillator
            else
                oscillator.perform();
        }
    }
}

TEST_CASE("Oscillator Waveforms" "[Oscillator]") {
    SECTION("Change Wavetables") {
        Oscillator<TestType> osc{};
        osc.setWaveform(std::make_unique<SinShaper<TestType>>());
        REQUIRE(dynamic_cast<SinShaper<TestType>*>(osc.getWaveform().get()));
        osc.setWaveform(std::make_unique<Shaper<TestType>>());
        REQUIRE(dynamic_cast<Shaper<TestType>*>(osc.getWaveform().get()));
    }

    SECTION("Perform Sin Wave") {
        constexpr TestType oscillatorFrequency = 440.0;
        constexpr TestType sampleRate = 44100.0;

        Oscillator<TestType> osc{};
        osc.setWaveform(std::make_unique<SinShaper<TestType>>());
        osc.setFrequency(440.0);
        osc.setSampleRate(44100.0);


        for (auto i = 0; i < numIterations; ++i) {
            const auto angleInRadians = oscillatorFrequency
                                        * i
                                        * juce::MathConstants<TestType>::twoPi
                                        / sampleRate;
            const auto oscOutput = Decibel<TestType>::convertAmplitudeToDecibel(osc.perform());

            REQUIRE_THAT(oscOutput,
                         ResidualDecibels<TestType>(std::sin(angleInRadians), residualThreshold));
        }
    }

    SECTION("Perform Triangle Wave") {
        Oscillator<TestType> osc{};
        osc.setWaveform(std::make_unique<TriShaper<TestType>>());
        osc.setFrequency(440.0);
        osc.setSampleRate(44100.0);

        for(auto i = 0; i < numIterations; ++i) {
            const TestType referencePhase = std::fmod(i*440.0/44100.0, 1.0);
            if(referencePhase < TestType{.25}) {
                    REQUIRE_THAT(osc.perform(),
                                 Catch::WithinAbs(lerp(TestType{0},
                                                               TestType{1},
                                                               referencePhase * TestType{4}),
                                                  TestType{.000001}));
            }
            else if(referencePhase < TestType{.75}) {
                    REQUIRE_THAT(osc.perform(),
                                 Catch::WithinAbs(lerp(TestType{1},
                                                               TestType{-1},
                                                               (referencePhase - TestType{.25}) * TestType{2}),
                                         TestType{.000001}));
            }
            else {
                REQUIRE_THAT(osc.perform(),
                             Catch::WithinAbs(lerp(TestType{-1},
                                                           TestType{0},
                                                           (referencePhase - TestType{.75}) * TestType{4}),
                                              TestType{.000001}));
            }
        }
    }

    SECTION("Perform Square Wave") {
        Oscillator<TestType> osc{};
        osc.setWaveform(std::make_unique<SquareShaper<TestType>>());
        osc.setFrequency(440.0);
        osc.setSampleRate(44100.0);

        for(auto i = 0; i < numIterations; ++i) {
            const auto reference = double(i*440.0/44100.0 >= .5);
            const auto oscOut = osc.perform();

            REQUIRE((Catch::WithinRel(reference).match(oscOut)
            || (Catch::WithinRel(reference).match(1.0) && Catch::WithinRel(oscOut).match(0.0))
            || (Catch::WithinRel(reference).match(0.0) && Catch::WithinRel(oscOut).match(1.0))));
        }
    }

    SECTION("Perform Sawtooth Wave") {
        Oscillator<TestType> osc{};
        osc.setWaveform(std::make_unique<SawShaper<TestType>>());
        osc.setFrequency(440.0);
        osc.setSampleRate(44100.0);

        for(auto i = 0; i < numIterations; ++i) {
            const TestType lerpIndex = std::fmod(i*440.0/44100.0, 1.0);
            const auto oscOut = Amplitude{osc.perform()};
            const auto ref = lerp(TestType{-1}, TestType{1}, lerpIndex);

            //Check that the sawtooth is correcty between -1 and 1,
            // or that the oscillator and reference are very close to a phase of 0
            REQUIRE((WithinDecibels<TestType>(ref, tolerance).match(oscOut)
                  || closeToEdge({oscOut.count(), ref}, TestType{-1}, TestType{1})));
        }
    }

}