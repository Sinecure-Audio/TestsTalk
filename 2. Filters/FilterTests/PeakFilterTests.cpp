#include <catch2/catch.hpp>

#include "../../Utilities/Random.h"
#include "../Signal Analysis/FFT/FFT.h"
#include "../../Utilities/DecibelMatchers.h"
#include "../FilterMeasurementUtilities.h"

//Test to verify the shape of the spectrum of the output of the peak filter
TEMPLATE_TEST_CASE("Peak Filter Spectrum Shape", "[Peak Filter] [Filter]",
                   float, double) {
    using SampleType = TestType;

    // Initialize the noise buffer and spectrum, the filter,
    // and other variables we'll use in this test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<SampleType>,
                                        FilterResponse::Peak, SampleType>();

    // Get the number of bins in our fft output.
    // This is half the size of our total spectrum,
    // as we only care about the real part of our frequency
    // This covers the range from 0 to half of nyquist?
    constexpr auto FFTSize = testContext.SpectrumSize/2;

    // Get the warped frequency
    // this represents the actual cutoff frequency of our filter
    const auto warpedCutoff = DigitalFrequency{testContext.cutoff};

    // Get the bin number that the cutoff is in
    // We use this so we can re-use ascending and descending gain checks
    // for each side of the filter
    const auto warpedCutoffBinIndex = (warpedCutoff.count() / testContext.sampleRate) * FFTSize;

    // Get the spectrum of the filtered output
    const auto filterSpectrum = getFilteredSpectrum<SampleType>(testContext.fft,
                                                                testContext.noiseBuffer,
                                                                testContext.filter);

    //For every bin between 0hz and nyquist, check if the
    for (size_t i = 0; i < FFTSize/2; ++i) {
        const Decibel<SampleType> noiseLevel    = Amplitude{testContext.noiseSpectrum[i].getAverage()};
        const Decibel<SampleType> filteredLevel = Amplitude{filterSpectrum[i].getAverage()};

        // Check if the input is the same or quieter than the output
        const auto outputSameOrQuieter = isSameOr<GainChange::Quieter>(noiseLevel,
                                                                       filteredLevel,
                                                                       testContext.tolerance);
        // Check if the difference between the input and the output is unhearable
        // We want to know this because if the input is louder than the output
        // But not so much louder we can hear it, the test should pass.
        // This is useful because for much of the spectrum,
        // the two levels will be similar.
        // So, it makes sense to allow the input to be louder than the output
        // Just not noticeably so
        const auto differenceVeryQuiet = ResidualDecibels<SampleType>(noiseLevel,
                                                                      Decibel{SampleType{-120}})
                                         .match(filteredLevel);

        // Pass the test if the input is less than or unhearably louder than the output
        REQUIRE((outputSameOrQuieter || differenceVeryQuiet));


        if (i > 0) {
            const Decibel<SampleType> currentBinLevel  = Amplitude{filterSpectrum[i].getAverage()};
            const Decibel<SampleType> previousBinLevel = Amplitude{filterSpectrum[i - 1].getAverage()};

            const auto binDifferenceVeryQuiet = ResidualDecibels<SampleType>(currentBinLevel,
                                                                             Decibel{SampleType{-120}})
                    .match(previousBinLevel);

            const auto level1 = testContext.filterGain >= SampleType{1}
                                ? currentBinLevel
                                : previousBinLevel;
            const auto level2 = testContext.filterGain >= SampleType{1}
                                ? previousBinLevel
                                : currentBinLevel;

            if (i < warpedCutoffBinIndex) {
                const auto currentBinSameOrLouder = isSameOr<GainChange::Louder>(level1,
                                                                                 level2,
                                                                                 testContext.tolerance);
                REQUIRE((currentBinSameOrLouder || binDifferenceVeryQuiet));
            }
            else if (i-1 >= warpedCutoffBinIndex) {
                const auto currentBinSameOrQuieter = isSameOr<GainChange::Quieter>(level1,
                                                                                   level2,
                                                                                   testContext.tolerance);
                REQUIRE((currentBinSameOrQuieter || binDifferenceVeryQuiet));
            }
        }
    }
}


//Test the level difference between the input and output of the filter
TEMPLATE_TEST_CASE("Peak Filter Cutoff Gain", "[Peak Filter] [Filter]",
                   float, double) {
    using SampleType = TestType;
    //Initialize the noise buffer and spectrum, the filter,
    // and other variables we'll use in this test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<SampleType>,
                                        FilterResponse::Peak, SampleType>();

    //Get the warped frequency of the filter cutoff
    const auto warpedCutoff  = DigitalFrequency{testContext.cutoff};

    //Get the difference in level between the input and the output
    const auto levelDifference
        = calculateLevelReductionAtFrequency<SampleType>(testContext.filter,
                                                         warpedCutoff.count(),
                                                         testContext.sampleRate);
    //Check that the difference in levels is within half a dB
    REQUIRE_THAT(levelDifference,
                 WithinDecibels(Decibel{Amplitude{testContext.filterGain}},
                                        Decibel{ SampleType{.5}}));
}
