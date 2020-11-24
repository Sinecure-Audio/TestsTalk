#include "Oscillator.h"

#include <catch2/catch.hpp>

#include "OscillatorUtilities.h"
#include "../Utilities/DecibelMatchers.h"
#include "../Utilities/Random.h"
#include "../Utilities/Lerp.h"

//Set the number of times each test is run
constexpr size_t numIterations = 100000;

//A value we use to determine whether the difference between two signals is small enough to be acceptable
template<typename T>
const auto residualThreshold = Decibel<T>{T{-120}};
template<typename T>
const auto tolerance = Decibel<T>{T{.5}};

//TODO: Add tests that change freq and SR on the fly,
// checking to make sure behavior remains correct-
// the changes to the perform function will cause incorrect results in these cases

TEMPLATE_TEST_CASE("Perform Phasor", "[Phasor]", float, double) {
    //TODO: add edge check to all phasor tests
    SECTION("Perform Phasor") {
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

    SECTION("Perform Phasor With Different Sample Rates") {
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

    SECTION("Perform Phasor With Random Frequencies") {
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
}

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

TEMPLATE_TEST_CASE("Oscillator Waveforms", "[Oscillator]", float, double) {

    SECTION("Change Wavetables") {
        Oscillator<TestType> osc{};
        osc.setWaveform(std::make_unique<SinShaper<TestType>>());
        CHECK(dynamic_cast<SinShaper<TestType>*>(osc.getWaveform().get()));
        osc.setWaveform(std::make_unique<Shaper<TestType>>());
        CHECK(dynamic_cast<Shaper<TestType>*>(osc.getWaveform().get()));
    }

    SECTION("Test Basic Waveforms") {
        const TestType oscillatorFrequency = GENERATE(take(100, random(0.0, 20000.0)));
        const auto sampleRate = GENERATE(TestType{44100},
                                   TestType{48000},
                                   TestType{88200},
                                   TestType{96000},
                                   TestType{176400},
                                   TestType{192000});

        const auto phaseIncrement = oscillatorFrequency/sampleRate;
        const auto iterationsPerCycle = TestType{1}/phaseIncrement;

        SECTION("Perform Sin Wave") {
            Oscillator<TestType> osc{};
            osc.setWaveform(std::make_unique<SinShaper<TestType>>());
            osc.setFrequency(oscillatorFrequency);
            osc.setSampleRate(sampleRate);


            for (auto i = 0; i < numIterations; ++i) {
                const TestType phaseReference = std::fmod(i, iterationsPerCycle)*phaseIncrement;
                const auto angleInRadians = phaseReference * juce::MathConstants<TestType>::twoPi;
                const auto oscOutput = osc.perform();
                const Decibel<TestType> oscLevel = Amplitude{oscOutput};

                CHECK_THAT(oscLevel,
                             ResidualDecibels<TestType>(std::sin(angleInRadians), residualThreshold<TestType>));
            }
        }

        SECTION("Perform Triangle Wave") {
            Oscillator<TestType> osc{};
            osc.setWaveform(std::make_unique<TriShaper<TestType>>());
            osc.setFrequency(oscillatorFrequency);
            osc.setSampleRate(sampleRate);

            for (auto i = 0; i < numIterations; ++i) {
                const TestType phaseReference = std::fmod(i, iterationsPerCycle)*phaseIncrement;
                if (phaseReference < TestType{.25}) {
                    CHECK_THAT(osc.perform(),
                                 Catch::WithinAbs(lerp(TestType{0},
                                                       TestType{1},
                                                       phaseReference * TestType{4}),
                                                  TestType{.000001}));
                } else if (phaseReference < TestType{.75}) {
                    CHECK_THAT(osc.perform(),
                                 Catch::WithinAbs(lerp(TestType{1},
                                                       TestType{-1},
                                                       (phaseReference - TestType{.25}) * TestType{2}),
                                                  TestType{.000001}));
                } else {
                    CHECK_THAT(osc.perform(),
                                 Catch::WithinAbs(lerp(TestType{-1},
                                                       TestType{0},
                                                       (phaseReference - TestType{.75}) * TestType{4}),
                                                  TestType{.000001}));
                }
            }
        }

        SECTION("Perform Square Wave") {
            Oscillator<TestType> osc{};
            osc.setWaveform(std::make_unique<SquareShaper<TestType>>());
            osc.setFrequency(oscillatorFrequency);
            osc.setSampleRate(sampleRate);

            for (auto i = 0; i < numIterations; ++i) {
                const TestType phaseReference = std::fmod(i, iterationsPerCycle)*phaseIncrement;
                const auto isHigh = phaseReference >= TestType{.5};
                const auto reference = isHigh ? TestType{1} : TestType{-1};
                const auto oscOut = osc.perform();

                //Test if our reference matches the output of the oscillator, or if they are close
                CHECK((Catch::WithinRel(reference).match(oscOut)
                         || closeToEdge({oscOut, reference}, TestType{-1}, TestType{1})));
            }
        }

        SECTION("Perform Sawtooth Wave") {
            Oscillator<TestType> osc{};
            osc.setWaveform(std::make_unique<SawShaper<TestType>>());
            osc.setFrequency(oscillatorFrequency);
            osc.setSampleRate(sampleRate);

            for (auto i = 0; i < numIterations; ++i) {
                const TestType phaseReference = std::fmod(i, iterationsPerCycle)*phaseIncrement;
                const auto oscOut = Amplitude{osc.perform()};
                const auto ref = lerp(TestType{-1}, TestType{1}, phaseReference);

                const auto a = WithinDecibels<TestType>(ref, tolerance<TestType>).match(oscOut);
                const auto b = closeToEdge({oscOut, ref}, TestType{-1}, TestType{1});
                if(!(a||b))
                    auto x = 2;
                //Check that the sawtooth is correctly between -1 and 1,
                // or that the oscillator and reference are very close to a phase of 0
                CHECK((WithinDecibels<TestType>(ref, tolerance<TestType>).match(oscOut)
                    || closeToEdge({oscOut, ref}, TestType{-1}, TestType{1})));
            }
        }
    }
}