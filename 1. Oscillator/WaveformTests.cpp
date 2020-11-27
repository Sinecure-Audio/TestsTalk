#include "Oscillator.h"

#include <catch2/catch.hpp>

#include "OscillatorUtilities.h"
#include "OscillatorTestConstants.h"
#include "../Utilities/DecibelMatchers.h"
#include "../Utilities/Random.h"
#include "../Utilities/Lerp.h"

TEMPLATE_TEST_CASE("Change Oscillator Wavetables", "[Oscillator]", float, double) {
    Oscillator<TestType> osc{};
    osc.setWaveform(std::make_unique<SinShaper<TestType>>());
    CHECK(dynamic_cast<SinShaper<TestType>*>(osc.getWaveform().get()));
    osc.setWaveform(std::make_unique<Shaper<TestType>>());
    CHECK(dynamic_cast<Shaper<TestType>*>(osc.getWaveform().get()));
}

template<typename T>
auto getOscillatorAndSampleRate() {
    const T oscillatorFrequency = GENERATE(take(100, random(0.0, 20000.0)));
    const auto sampleRate = GENERATE(T{44100},
                                     T{48000},
                                     T{88200},
                                     T{96000},
                                     T{176400},
                                     T{192000});

    return std::tuple{oscillatorFrequency, sampleRate};
}

TEMPLATE_TEST_CASE("Sin Wave", "[Oscillator]", float, double) {
    const auto [oscillatorFrequency, sampleRate] = getOscillatorAndSampleRate<TestType>();

    const auto phaseIncrement = oscillatorFrequency/sampleRate;
    const auto iterationsPerCycle = TestType{1}/phaseIncrement;

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

TEMPLATE_TEST_CASE("Tri Wave", "[Oscillator]", float, double) {
    const auto [oscillatorFrequency, sampleRate] = getOscillatorAndSampleRate<TestType>();

    const auto phaseIncrement = oscillatorFrequency/sampleRate;
    const auto iterationsPerCycle = TestType{1}/phaseIncrement;

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

TEMPLATE_TEST_CASE("Square Wave", "[Oscillator]", float, double) {
    const auto [oscillatorFrequency, sampleRate] = getOscillatorAndSampleRate<TestType>();

    const auto phaseIncrement = oscillatorFrequency/sampleRate;
    const auto iterationsPerCycle = TestType{1}/phaseIncrement;

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

TEMPLATE_TEST_CASE("Saw Wave", "[Oscillator]", float, double) {
    const auto [oscillatorFrequency, sampleRate] = getOscillatorAndSampleRate<TestType>();

    const auto phaseIncrement = oscillatorFrequency/sampleRate;
    const auto iterationsPerCycle = TestType{1}/phaseIncrement;

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
//Check that the sawtooth is correctly between -1 and 1,
// or that the oscillator and reference are very close to a phase of 0
        CHECK((WithinDecibels<TestType>(ref, tolerance<TestType>).match(oscOut)
               || closeToEdge({oscOut, ref}, TestType{-1}, TestType{1})));
    }
}
