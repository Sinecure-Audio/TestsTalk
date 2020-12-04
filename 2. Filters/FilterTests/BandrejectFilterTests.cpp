#include <catch2/catch.hpp>

#include "../../Utilities/Random.h"
#include "../Signal Analysis/FFT/FFT.h"
#include "../../Utilities/DecibelMatchers.h"
#include "../FilterMeasurementUtilities.h"

//TODO: Add the capability to test various q values i.e. filters with resonance
TEMPLATE_TEST_CASE("Bandreject Filter Spectrum Shape", "[Bandreject Filter] "
                                                       "[Filter]",
                                                       float, double) {
    using SampleType = TestType;

    //Initialize the filter and noise we'll test with
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<SampleType>,
                                        FilterResponse::BandReject, SampleType>();

    constexpr auto FFTSize = testContext.SpectrumSize/2;

    // Get the warped frequency
    // this represents the actual cutoff frequency of our filter
    const auto warpedCutoff = DigitalFrequency{testContext.cutoff};

    const auto warpedCutoffBinIndex = (warpedCutoff.count() / testContext.sampleRate) * FFTSize;

    const auto filterSpectrum = getFilteredSpectrum<SampleType>(testContext.fft,
                                                                testContext.noiseBuffer,
                                                                testContext.filter);

    for (size_t i = 0; i < FFTSize/2; ++i) {
        const Decibel<SampleType> noiseLevel    = Amplitude{testContext.noiseSpectrum[i].getAverage()};
        const Decibel<SampleType> filteredLevel = Amplitude{filterSpectrum[i].getAverage()};

        const auto outputSameOrQuieter = isSameOr<GainChange::Quieter>(noiseLevel,
                                                                       filteredLevel,
                                                                       testContext.tolerance);
        const auto differenceVeryQuiet = ResidualDecibels<SampleType>(noiseLevel,
                                                                      Decibel{SampleType{-120}})
                                         .match(filteredLevel);

        REQUIRE((outputSameOrQuieter || differenceVeryQuiet));

        if (i > 0) {
            const Decibel<SampleType> currentBinLevel  = Amplitude{filterSpectrum[i].getAverage()};
            const Decibel<SampleType> previousBinLevel = Amplitude{filterSpectrum[i - 1].getAverage()};

            const auto binDifferenceVeryQuiet = ResidualDecibels<SampleType>(currentBinLevel,
                                                                             Decibel{SampleType{-120}})
                    .match(previousBinLevel);

            if (i <= warpedCutoffBinIndex) {
                const auto currentBinSameOrQuieter = isSameOr<GainChange::Quieter>(currentBinLevel,
                                                                                   previousBinLevel,
                                                                                   testContext.tolerance);
                REQUIRE((currentBinSameOrQuieter || binDifferenceVeryQuiet));
            }
                //Make sure last two bins are on the same side of the notch
            else if (i-1 >= warpedCutoffBinIndex) {
                const auto currentBinSameOrLouder = isSameOr<GainChange::Louder>(currentBinLevel,
                                                                                 previousBinLevel,
                                                                                 testContext.tolerance);
                REQUIRE((currentBinSameOrLouder || binDifferenceVeryQuiet));
            }
        }
    }
}

TEMPLATE_TEST_CASE("Bandreject Filter Cutoff Gain", "[Bandreject Filter]"
                                                    "[Filter]",
                   float, double) {
    using SampleType = TestType;

    //Initialize the filter and noise we'll test with
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<SampleType>,
                                        FilterResponse::BandReject, SampleType>();

    // Get the warped frequency
    // this represents the actual cutoff frequency of our filter
    const auto warpedCutoff = DigitalFrequency{testContext.cutoff};

    // Measure the gain difference of sin wave with the same frequency as the warped cutoff
    // input and output from the filter
    //Check that it's within half a dB of a 3dB reduction
    REQUIRE(calculateLevelReductionAtFrequency<SampleType>(testContext.filter,
                                                           warpedCutoff.count(),
                                                           testContext.sampleRate)
            < -48.0_dB);
}
