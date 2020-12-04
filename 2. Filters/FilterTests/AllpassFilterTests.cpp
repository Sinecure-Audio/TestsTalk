#include <catch2/catch.hpp>

#include "../../Utilities/Random.h"
#include "../Signal Analysis/FFT/FFT.h"
#include "../../Utilities/DecibelMatchers.h"
#include "../FilterMeasurementUtilities.h"

//Test the spectrum shape and gain of an allapss filter
// TODO: check why using references in the test context for the noise stuff
//  doesn't work in just this test
TEMPLATE_TEST_CASE("Allpass Filter Spectrum Shape", "[Allpass Filter] "
                                                    "[Filter]",
                                                    float, double) {
    using SampleType = TestType;
    //Initialize the filter and noise we'll use in the test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<SampleType>,
                                        FilterResponse::Allpass, SampleType>();

    //Set the tolerance to something tighter than normal
    testContext.tolerance = Decibel{SampleType{-1.5}};

    constexpr auto FFTSize = testContext.SpectrumSize/2;

    //Get the buffer a generate its spectrum
    const auto inputNoiseSpectrum
            = makeSpectrum<SampleType, 1024>(testContext.noiseBuffer);

    //Get the spectrum of the noise run through the filter
    const auto filteredSpectrum = getFilteredSpectrum<SampleType>(testContext.fft,
                                                                  testContext.noiseBuffer,
                                                                  testContext.filter);

    // For every bin in the buffer,
    // get the difference between the input and output levels
    // And check to see if they're within the threshold
    for (size_t i = 0; i < FFTSize/2; ++i) {
        const auto noiseLevel =
                Decibel{Amplitude{inputNoiseSpectrum[i].getAverage()}};
        const auto filteredLevel =
                Decibel{Amplitude{filteredSpectrum[i].getAverage()}};

        REQUIRE_THAT(noiseLevel,
                     WithinDecibels(filteredLevel, testContext.tolerance));
    }
}