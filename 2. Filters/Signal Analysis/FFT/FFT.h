#pragma once

#include <array>
#include <optional>
#include <juce_dsp/juce_dsp.h>

//#include "FFTUtils.h"

//#include "../../Sinecure-Audio-Library/Utilities/Units/include/Units.h"

template<size_t FFTSize>
class FFTHelper
{
public:
//    FFT
    constexpr const auto& getFFTData() const noexcept {
        return fftData;
    }

    std::optional<std::reference_wrapper<const std::array<float, FFTSize*2>>>
    pushNextSampleIntoFifo (float sample) noexcept
    {
        //If we've received enough samples to output a frame:
        // reset the data buffer, window our input, normalize the level, and then transform the data
        //Then, reset the sample counter and return a reference to the frame
        if (fftInputIndex == FFTSize) {
            std::fill (fftData.begin()+FFTSize, fftData.end(), 0.0f);
            std::copy (fftInput.begin(), fftInput.end(), fftData.begin());
            windowFFTData();
            scaleFFTData();
            forwardFFT.performFrequencyOnlyForwardTransform (fftData.data());

            fftInputIndex = 0;
            fftInput[fftInputIndex++] = sample;
            return fftData;
        }
        //If we're not ready to return a frame, return a nullopt
        else {
            fftInput[fftInputIndex++] = sample;
            return {};
        }
    }

    auto perform (float sample) noexcept {
        return pushNextSampleIntoFifo (sample);
    }

    //Reset all fft data to 0
    void reset() {
        std::fill (fftData.begin(), fftData.end(), 0.0f);
        std::fill (fftInput.begin(), fftInput.end(), 0.0f);
    }

private:
    juce::dsp::FFT forwardFFT{ juce::roundToInt(std::log2(FFTSize)) };
    std::array<float, FFTSize> fftInput{};
    std::array<float, FFTSize * 2> fftData{};
//    std::vector<float> fftInput{FFTSize};
//    std::vector<float> fftData{FFTSize*2};

    size_t fftInputIndex{ 0 };
    juce::dsp::WindowingFunction<float> windowFunc{FFTSize, juce::dsp::WindowingFunction<float>::WindowingMethod::hann};

    //Apply a Hann Window to the FFT data
    void windowFFTData() noexcept {
        const auto windowSize = fftData.size()/2;
        windowFunc.multiplyWithWindowingTable(fftData.data(), windowSize);
    }

    //Normalize the FFT data
    auto scaleFFTData() noexcept {
        for(auto&& sample : fftData)
            sample *= 2.0f / FFTSize;
    }
};