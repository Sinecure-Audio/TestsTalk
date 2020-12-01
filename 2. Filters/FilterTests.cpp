#include <catch2/catch.hpp>

#include "../Utilities/Random.h"

#include "Signal Analysis/FFT/FFT.h"

#include "SpectrumComparators.h"

//These tests are all about creating a context that has all of the info we need to test the filter
//We make the context, and then pass it to a function that actually tests the filter
//We're looking for things like the shape of the filter's output frequency response,
// the way in which it rolls off, and the level at the cutoff frequency
// There are definitely other useful things to test- these are just the ones I came up with

template<typename FilterType, FilterResponse Response, typename T>
auto getFilterContext() noexcept {
    static constexpr size_t FFTSize = 1024;

    constexpr auto sampleRate = T{ 44100 };

    //Generate a series of cutoffs equal to each bin's frequency
    const auto binNumber = GENERATE(range(size_t{1}, FFTSize/2));
    const auto cutoff = binNumber*sampleRate/FFTSize;

    const auto q = getQValue<Response, T>();
    const auto gain = getGainValue<Response, T>();

    //A level for determining how close the measured level has to be to the expected level
    constexpr auto tolerance = Decibel{ T{-4} };

    return FilterTestContext<std::integral_constant<size_t, FFTSize>, FilterType, T>
            {cutoff, q, sampleRate, gain,
             setupFilter<FilterType, Response, T>(cutoff, q, sampleRate, gain),
             Decibel{T{-12}}, tolerance};//,
//             noiseBuffer, noiseSpectrum};
}

//TODO: Add the capability to test gain values below 1 to the peak and shelf filters
//TODO: Add the capability to test resonant filters (i.e. variable Q)
TEMPLATE_TEST_CASE("Test Lowpass Filter", "[Filter]", float, double) {
    //Call out to the test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<TestType>, FilterResponse::Lowpass, TestType>();
    runFilterTests<FilterResponse::Lowpass>(testContext);
}

TEMPLATE_TEST_CASE("Test Highpass Filter", "[Filter]", float, double) {
    //Call out to the test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<TestType>, FilterResponse::Highpass, TestType>();
    runFilterTests<FilterResponse::Highpass>(testContext);
}

TEMPLATE_TEST_CASE("Test Bandpass Filter", "[Filter]", float, double) {
    //Call out to the test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<TestType>, FilterResponse::Bandpass, TestType>();
    runFilterTests<FilterResponse::Bandpass>(testContext);
}

TEMPLATE_TEST_CASE("Test Bandreject Filter", "[Filter]", float, double) {
    //Call out to the test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<TestType>, FilterResponse::BandReject, TestType>();
    runFilterTests<FilterResponse::BandReject>(testContext);
}

TEMPLATE_TEST_CASE("Test Allpass Filter", "[Filter]", float, double) {
    //Call out to the test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<TestType>, FilterResponse::Allpass, TestType>();
    testContext.tolerance = Decibel{TestType{-1.5}};
    runFilterTests<FilterResponse::Allpass>(testContext);
}

TEMPLATE_TEST_CASE("Test Peak Filter", "[Filter]", float, double) {
    //Call out to the test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<TestType>, FilterResponse::Peak, TestType>();
    runFilterTests<FilterResponse::Peak>(testContext);
}

TEMPLATE_TEST_CASE("Test LowShelf Filter", "[Filter]", float, double) {
    //Call out to the test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<TestType>, FilterResponse::LowShelf, TestType>();
    runFilterTests<FilterResponse::LowShelf>(testContext);
}

TEMPLATE_TEST_CASE("Test HighShelf Filter", "[Filter]", float, double) {
    //Call out to the test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<TestType>, FilterResponse::HighShelf, TestType>();
    runFilterTests<FilterResponse::HighShelf>(testContext);
}