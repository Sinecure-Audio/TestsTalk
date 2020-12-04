#include <catch2/catch.hpp>

#include "../../Utilities/Random.h"
#include "../Signal Analysis/FFT/FFT.h"
#include "../../Utilities/DecibelMatchers.h"
#include "../FilterMeasurementUtilities.h"

//TODO: Add the capability to test various q values i.e. filters with resonance
TEMPLATE_TEST_CASE("Lowpass Filter Shape", "[Lowpass Filter] [Filter]",
                   float, double) {
    using SampleType = TestType;

    //Initialize the filter and noise we'll test with
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<SampleType>, FilterResponse::Lowpass, SampleType>();

    constexpr auto FFTSize = testContext.SpectrumSize/2;

    const auto filterSpectrum = getFilteredSpectrum<SampleType>(testContext.fft,
                                                                testContext.noiseBuffer,
                                                                testContext.filter);

    // Get the warped frequency
    // this represents the actual cutoff frequency of our filter
    const auto warpedCutoff = DigitalFrequency{testContext.cutoff};

    //From 0 to nyquist, check that:
    //The current bin's is around the same level for both the input and output
    //Or it is quieter for the input
    for (size_t i = 0; i < FFTSize/2; ++i) {
        const Decibel<SampleType> noiseLevel    = Amplitude{testContext.noiseSpectrum[i].getAverage()};
        const Decibel<SampleType> filteredLevel = Amplitude{filterSpectrum[i].getAverage()};

        const auto outputSameOrQuieter = isSameOr<GainChange::Quieter>(noiseLevel,
                                                                       filteredLevel,
                                                                       testContext.tolerance);
        const auto differenceVeryQuiet = ResidualDecibels<SampleType>(noiseLevel,
                                                                      -120.0_dB)
                                         .match(filteredLevel);

        REQUIRE((outputSameOrQuieter || differenceVeryQuiet));

        //After the first bin:
        //Check that the current bin is either the same level or quieter than the previous
        //Or that the difference between them is below the threshold of hearing
        if(i > 0) {
            const Decibel<SampleType> currentBinLevel  = Amplitude{filterSpectrum[i].getAverage()};
            const Decibel<SampleType> previousBinLevel = Amplitude{filterSpectrum[i-1].getAverage()};

            const auto currentBinSameOrQuieter = isSameOr<GainChange::Quieter>(currentBinLevel,
                                                                               previousBinLevel,
                                                                               testContext.tolerance);
            const auto binDifferenceVeryQuiet = ResidualDecibels<SampleType>(currentBinLevel,
                                                                             -120.0_dB)
                    .match(previousBinLevel);

            REQUIRE((currentBinSameOrQuieter || binDifferenceVeryQuiet));
        }
    }
}

TEMPLATE_TEST_CASE("Lowpass Filter Cutoff Gain", "[Lowpass Filter] [Filter]",
                   float, double) {
    using SampleType = TestType;

    //Initialize the filter and noise we'll test with
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<SampleType>, FilterResponse::Lowpass, SampleType>();

    // Get the warped frequency
    // this represents the actual cutoff frequency of our filter
    const auto warpedCutoff = DigitalFrequency{testContext.cutoff};

    // Measure the gain difference of sin wave with the same frequency as the warped cutoff
    // input and output from the filter
    // Check that it's within half a dB of a 3dB reduction
    REQUIRE_THAT(calculateLevelReductionAtFrequency<SampleType>(testContext.filter,
                                                                warpedCutoff.count(),
                                                                testContext.sampleRate),
                 WithinDecibels(Decibel{SampleType{-3}}, Decibel{SampleType{.5}}));
}

TEMPLATE_TEST_CASE("Lowpass Filter Rolloff", "[Lowpass Filter] [Filter]",
                                                               float, double) {
    using SampleType = TestType;

    //Initialize the filter and noise we'll test with
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<SampleType>,
                                        FilterResponse::Lowpass, SampleType>();

    // Get the warped frequency
    // this represents the actual cutoff frequency of our filter
    const auto warpedCutoff = DigitalFrequency{testContext.cutoff};

    // Test that the rolloff per octave happens at the expected rate over several octaves
    // This will pass if the rolloff is inside the tolerance
    testRolloffCharacteristics<RolloffDirection::Up>(testContext.filter,
                                                     warpedCutoff,
                                                     testContext.sampleRate,
                                                     testContext.rolloffPerOctave,
                                                     testContext.tolerance);
}
