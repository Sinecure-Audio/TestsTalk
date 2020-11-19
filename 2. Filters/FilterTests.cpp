#include <catch2/catch.hpp>

#include "../Utilities/Random.h"

#include "Signal Analysis/FFT/FFT.h"

#include "SpectrumComparators.h"

//These tests are all about creating a context that has all of the info we need to test the filter
//We make the context, and then pass it to a function that actually tests the filter
//We're looking for things like the shape of the filter's output frequency response,
// the way in which it rolls off, and the level at the cutoff frequency
// There are definitely other useful things to test- these are just the ones I came up with



TEST_CASE("Test Lowpass Frequency Response", "[Filter]") {
    static constexpr size_t FFTSize = 1024;

    constexpr auto sampleRate = 44100.0;

    //Generate a series of cutoffs equal to each bin's frequency
    const auto binNumber = GENERATE(range(size_t{1}, FFTSize/2));
    const auto cutoff = binNumber*sampleRate/FFTSize;

    const auto q = QCoefficient{1.0/sqrt(2.0)};

    //A level for determining how close the measured level has to be to the expected level
    constexpr auto tolerance = Decibel{4.0};

    //Set the type of the filter to test
    using FilterType = juce::dsp::IIR::Filter<double>;

    auto testContext = makeFilterContext<FFTSize,
                                         FilterResponse::Lowpass,
                                         FilterType>
                                         (cutoff, q, sampleRate,
                                          Decibel{-12.0}, tolerance);

    //Call out to the test
    testLowpassResponse(testContext);
};

TEST_CASE("Test Highpass Frequency Response", "[Filter]") {
    static constexpr size_t FFTSize = 1024;

    constexpr auto sampleRate = 44100.0;
    //Generate a series of cutoffs equal to each bin's frequency
    const auto binNumber = GENERATE(range(size_t{1}, FFTSize/2));
    const auto cutoff = binNumber*sampleRate/FFTSize;

    const auto q = QCoefficient{.707};

    //A level for determining how close the measured level has to be to the expected level
    constexpr auto tolerance = Decibel{4.0};

    //Set the type of the filter to test
    using FilterType = juce::dsp::IIR::Filter<double>;

    auto testContext = makeFilterContext<FFTSize,
            FilterResponse::Highpass,
            FilterType>
            (cutoff, q, sampleRate,
             Decibel{-12.0}, tolerance);

    testHighpassResponse(testContext);
};

TEST_CASE("Test Bandpass Frequency Response", "[Filter]") {
    static constexpr size_t FFTSize = 1024;

    constexpr auto sampleRate = 44100.0;
    //Generate a series of cutoffs equal to each bin's frequency
    const auto binNumber = GENERATE(range(size_t{1}, FFTSize/2));
    const auto cutoff = binNumber*sampleRate/FFTSize;

    constexpr auto q = QCoefficient{.707};

    //A level for determining how close the measured level has to be to the expected level
    constexpr auto tolerance = Decibel{4.0};

    //Set the type of the filter to test
    using FilterType = juce::dsp::IIR::Filter<double>;

    auto testContext = makeFilterContext<FFTSize,
                                         FilterResponse::Bandpass,
                                         FilterType>
                                         (cutoff, q, sampleRate,
                                          Decibel{-12.0}, tolerance);

    testBandpassResponse(testContext);
}

TEST_CASE("Test Notch Frequency Response", "[Filter]") {
    static constexpr size_t FFTSize = 1024;

    constexpr auto sampleRate = 44100.0;
    //Generate a series of cutoffs equal to each bin's frequency
    const auto binNumber = GENERATE(range(size_t{1}, FFTSize/2));
    const auto cutoff = binNumber*sampleRate/FFTSize;

    constexpr auto q = QCoefficient{.707};

    //A level for determining how close the measured level has to be to the expected level
    constexpr auto tolerance = Decibel{4.0};

    //Set the type of the filter to test
    using FilterType = juce::dsp::IIR::Filter<double>;

    auto testContext = makeFilterContext<FFTSize,
                                         FilterResponse::BandReject,
                                         FilterType>
                                         (cutoff, q, sampleRate,
                                          Decibel{-12.0}, tolerance);

    testBandrejectResponse(testContext);
}


//TODO: Find out why the allpass filter sometimes resets the coefficients to 0
TEST_CASE("Test Allpass Frequency Response", "[Filter]") {
    static constexpr size_t FFTSize = 1024;

    constexpr auto sampleRate = 44100.0;
    //Generate a series of cutoffs equal to each bin's frequency
    const auto binNumber = GENERATE(range(size_t{1}, FFTSize/2));
    const auto cutoff = binNumber*sampleRate/FFTSize;

    constexpr auto q = QCoefficient{.707};

    //A level for determining how close the measured level has to be to the expected level
    constexpr auto tolerance = Decibel{4.0};

    //Set the type of the filter to test
    using FilterType = juce::dsp::IIR::Filter<double>;

    auto testContext = makeFilterContext<FFTSize,
                                         FilterResponse::Allpass,
                                         FilterType>
                                         (cutoff, q, sampleRate,
                                          Decibel{0.0}, tolerance);

    testAllpassResponse(testContext);
}