#include <catch2/catch.hpp>

#include "../../Utilities/Random.h"
#include "../Signal Analysis/FFT/FFT.h"
#include "../../Utilities/DecibelMatchers.h"
#include "../FilterMeasurementUtilities.h"


TEMPLATE_TEST_CASE("HighShelf Filter Shape", "[LowShelf Filter] [Filter]", float,
                   double) {
    using SampleType = TestType;

    //Initialize the filter and noise we'll test with
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<SampleType>, FilterResponse::HighShelf, SampleType>();

    constexpr auto FFTSize = testContext.SpectrumSize/2;

    const auto filterSpectrum = getFilteredSpectrum<SampleType>(testContext.fft,
                                                                testContext.noiseBuffer,
                                                                testContext.filter);

    for (size_t i = 1; i < FFTSize/2; ++i) {
        //After the first bin:
        //Check that the current bin is either the same level or quieter than the previous
        //Or that the difference between them is below the threshold of hearing
        const Decibel<SampleType> currentBinLevel  = Amplitude{filterSpectrum[i].getAverage()};
        const Decibel<SampleType> previousBinLevel = Amplitude{filterSpectrum[i-1].getAverage()};

        const auto outputSameOrQuieter = isSameOr<GainChange::Louder>(currentBinLevel,
                                                                      previousBinLevel,
                                                                      testContext.tolerance);
        const auto differenceVeryQuiet = ResidualDecibels<SampleType>(currentBinLevel,
                                                                      Decibel{SampleType{-120}})
                                         .match(previousBinLevel);

        REQUIRE((outputSameOrQuieter || differenceVeryQuiet));
    }
}

TEMPLATE_TEST_CASE("HighShelf Filter Gain", "[LowShelf Filter] [Filter]", float,
                   double) {
    using SampleType = TestType;

    //Initialize the filter and noise we'll test with
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<SampleType>, FilterResponse::HighShelf, SampleType>();

    // Get the warped frequency
    // this represents the actual cutoff frequency of our filter
    const auto warpedCutoff = DigitalFrequency{testContext.cutoff};

    if(testContext.cutoff < testContext.sampleRate/4.0) {
        const auto levelDifference = calculateLevelReductionAtFrequency<SampleType>(testContext.filter,
                                                                                    warpedCutoff.count(),
                                                                                    testContext.sampleRate);
        REQUIRE_THAT(levelDifference,
                     WithinDecibels(Decibel{ Amplitude{std::sqrt(testContext.filterGain)} }, Decibel{ SampleType{.75} }));
    }
}