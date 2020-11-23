#include <catch2/catch.hpp>

#include "FFT.h"
#include "../Signal Analyzers.h"

#include "../../../Utilities/Random.h"
#include "../../../Utilities/DecibelMatchers.h"

constexpr auto numIterations = 1000000;

//Test that the largest bin in the fft, when given a DC input, is 0
TEST_CASE ("DC Response", "[FFT]") {
    FFTHelper<float, 10> fft;

    for(auto i = 0; i < numIterations; ++i)
        fft.perform(.5);

    const auto& fftData = fft.getFFTData();
    const auto largestBin = std::distance(fftData.begin(), std::max_element(fftData.begin(), fftData.end()));
    REQUIRE(largestBin == 0);
}

//Test that the largest bin in the fft, when given a sin input, is the sin frequency
TEST_CASE ("Sin Response", "[FFT]")
{
    static constexpr size_t FFTSize = 1024;
    FFTHelper<float, FFTSize> fft;
    const double sampleRate = 44100.0;
    const size_t currentBin = GENERATE(0, FFTSize);
    const double frequency = currentBin*(sampleRate/FFTSize);
    const double phaseIncrement = frequency/sampleRate;
    double phase = 0.0;

    for(auto i = 0; i < numIterations; ++i) {
        const auto in = std::sin(juce::MathConstants<double>::twoPi*phase);
        fft.perform(in);
        phase += phaseIncrement;
    }

    const auto& fftData = fft.getFFTData();
    const auto largestBin = std::distance(fftData.begin(), std::max_element(fftData.begin(), fftData.end()));
    //The largest bin should be the bin that contains our sin wave
    REQUIRE(largestBin == currentBin%(FFTSize/2));
    //The bins adjacent should roll off a certain amount
    const auto threshold = Decibel(-3.0f);

    const auto largestDecibelValue = Decibel<float>::convertAmplitudeToDecibel(fftData[largestBin]);

    REQUIRE_THAT(largestDecibelValue, ResidualDecibels<float>(fftData[largestBin-1], threshold));
    REQUIRE_THAT(largestDecibelValue, ResidualDecibels<float>(fftData[largestBin+1], threshold));
}


//Test that a noise input outputs a homogenous spectrum
TEST_CASE ("Noise Response", "[FFT]")
{
    //Generate the noise spectru,
    static constexpr size_t fftSize = 1024;
    BufferAverager<double, fftSize * 2> accumulator{};
    FFTHelper<float, fftSize> fft;
    for(auto i = 0; i < numIterations; ++i) {
        const auto& result = fft.perform(getBoundedRandom(-1.0, 1.0));
        if (result != std::nullopt)
            accumulator.perform(result.value());
    }

    //And test that each bin is close in value
    const auto loopSize = accumulator.getBuffer().size();
    for(size_t i = 1; i < loopSize-2; ++i) {
        const auto current = std::pow (accumulator.getBuffer()[i].getAverage(), 2);
        const auto next = std::pow (accumulator.getBuffer()[i + 1].getAverage(), 2);
        const auto total = std::abs (current - next) / loopSize;
        REQUIRE (total < .001);
    }
}