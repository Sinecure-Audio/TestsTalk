#include <catch2/catch.hpp>

#include "../Utilities/Random.h"

#include "Signal Analysis/FFT/FFT.h"

#include "SpectrumComparators.h"

//These tests are all about creating a context that has all of the info we need to test the filter
//We make the context, and then pass it to a function that actually tests the filter
//We're looking for things like the shape of the filter's output frequency response,
// the way in which it rolls off, and the level at the cutoff frequency
// There are definitely other useful things to test- these are just the ones I came up with

template<typename FilterType, FilterResponse Response>
auto getFilterContext() noexcept {
    static constexpr size_t FFTSize = 1024;

    constexpr auto sampleRate = 44100.0;

    //Generate a series of cutoffs equal to each bin's frequency
    const auto binNumber = GENERATE(range(size_t{1}, FFTSize/2));
    const auto cutoff = binNumber*sampleRate/FFTSize;

    const auto q = getQValue<Response, double>();
    const auto gain = getGainValue<Response, double>();

    //A level for determining how close the measured level has to be to the expected level
    constexpr auto tolerance = Decibel{4.0};

    //Set the type of the filter to test
//    using FilterType = juce::dsp::IIR::Filter<double>;
//    constexpr auto FilterResponse = getFilterResponseType();

    return makeFilterContext<FFTSize,
            Response,
            FilterType>
            (cutoff, q, sampleRate, gain,
             Decibel{-12.0}, tolerance);
}

//TODO: Add the capability to test gain values below 1 to the peak and shelf filters
//TODO: Add the capability to test resonant filters (i.e. variable Q)
//TEMPLATE_TEST_CASE("Test FILTERTYPE Frequency Response", "[Filter]", float, double) {
//    //Call out to the test
//    auto testContext = getFilterContext<juce::dsp::IIR::Filter<double>, getFilterResponseType()>();
//    runFilterTests<getFilterResponseType()>(testContext);
//}

TEMPLATE_TEST_CASE("Test Lowpass Filter", "[Filter]", float, double) {
    //Call out to the test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<double>, FilterResponse::Lowpass>();
    runFilterTests<FilterResponse::Lowpass>(testContext);
}

TEMPLATE_TEST_CASE("Test Highpass Filter", "[Filter]", float, double) {
    //Call out to the test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<double>, FilterResponse::Highpass>();
    runFilterTests<FilterResponse::Highpass>(testContext);
}

TEMPLATE_TEST_CASE("Test Bandpass Filter", "[Filter]", float, double) {
    //Call out to the test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<double>, FilterResponse::Bandpass>();
    runFilterTests<FilterResponse::Bandpass>(testContext);
}

TEMPLATE_TEST_CASE("Test Bandreject Filter", "[Filter]", float, double) {
    //Call out to the test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<double>, FilterResponse::BandReject>();
    runFilterTests<FilterResponse::BandReject>(testContext);
}

TEMPLATE_TEST_CASE("Test Allpass Filter", "[Filter]", float, double) {
    //Call out to the test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<double>, FilterResponse::Allpass>();
    runFilterTests<FilterResponse::Allpass>(testContext);
}

TEMPLATE_TEST_CASE("Test Peak Filter", "[Filter]", float, double) {
    //Call out to the test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<double>, FilterResponse::Peak>();
    runFilterTests<FilterResponse::Peak>(testContext);
}

TEMPLATE_TEST_CASE("Test LowShelf Filter", "[Filter]", float, double) {
    //Call out to the test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<double>, FilterResponse::LowShelf>();
    runFilterTests<FilterResponse::LowShelf>(testContext);
}

TEMPLATE_TEST_CASE("Test HighShelf Filter", "[Filter]", float, double) {
    //Call out to the test
    auto testContext = getFilterContext<juce::dsp::IIR::Filter<double>, FilterResponse::HighShelf>();
    runFilterTests<FilterResponse::HighShelf>(testContext);
}