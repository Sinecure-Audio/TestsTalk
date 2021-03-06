#include <catch2/catch.hpp>

#include "FFT.h"
#include "../Signal Analyzers.h"

#include "../../../Utilities/Random.h"
#include "../../../Utilities/DecibelMatchers.h"

constexpr auto numIterations = 1000000;

//Test that the largest bin in the fft, when given a DC input, is 0
TEST_CASE ("FFT DC Response", "[FFT]") {
    FFTHelper<1024> fft;

    for(auto i = 0; i < numIterations; ++i)
        fft.perform(.5);

    const auto& fftData = fft.getFFTData();
    const auto largestBin = std::distance(fftData.begin(), std::max_element(fftData.begin(), fftData.end()));
    REQUIRE(largestBin == 0);
}

//Test that the largest bin in the fft, when given a sin input, is the sin frequency
TEST_CASE ("FFT Sin Response", "[FFT]")
{
    static constexpr size_t FFTSize = 1024;
    FFTHelper<FFTSize> fft;
    const double sampleRate = 44100.0;
    const size_t currentBin = GENERATE(0, FFTSize);
    const double frequency = currentBin*(sampleRate/FFTSize);
    const double phaseIncrement = frequency/sampleRate;
    double phase = 0.0;

    for(auto i = 0; i < numIterations; ++i) {
        const auto in = std::sin(juce::MathConstants<double>::twoPi*phase);
        fft.perform(static_cast<float>(in));
        phase += phaseIncrement;
    }

    const auto& fftData = fft.getFFTData();
    const size_t largestBin = std::distance(fftData.begin(), std::max_element(fftData.begin(), fftData.end()));
    //The largest bin should be the bin that contains our sin wave
    REQUIRE(largestBin == currentBin%(FFTSize/2));
    //The bins adjacent should roll off a certain amount
    const auto threshold = Decibel(-3.0f);

    const auto largestDecibelValue = Decibel<float>::convertAmplitudeToDecibel(fftData[largestBin]);

    if(largestBin > 0)
        REQUIRE_THAT(largestDecibelValue, ResidualDecibels<float>(fftData[largestBin-1], threshold));
    if (largestBin < fftData.size()-1)
        REQUIRE_THAT(largestDecibelValue, ResidualDecibels<float>(fftData[largestBin+1], threshold));
}


//Test that a noise input outputs a homogenous spectrum
TEST_CASE ("FFT Noise Response", "[FFT]")
{
    //Make an fft and accumulator, and run noise through it for the set number of iterations
    static constexpr size_t FFTSize = 1024;
    BufferAverager<double, FFTSize * 2> accumulator{};
    FFTHelper<FFTSize> fft;
    for(auto i = 0; i < numIterations; ++i) {
        const auto& result = fft.perform(getBoundedRandom(-1.0f, 1.0f));
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