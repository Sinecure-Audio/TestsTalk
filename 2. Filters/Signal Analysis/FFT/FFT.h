#pragma once

#include <array>
#include <juce_dsp/juce_dsp.h>

//#include "FFTUtils.h"

//#include "../../Sinecure-Audio-Library/Utilities/Units/include/Units.h"

template<typename SampleType, size_t FFTSize>
class FFTHelper
{
public:
    constexpr const auto& getFFTData() const noexcept {
        return fftData;
    }

    std::optional<const std::array<SampleType, FFTSize*2>>
    pushNextSampleIntoFifo (SampleType sample) noexcept
    {
        //If we've received enough samples to output a frame:
        // reset the data buffer, window our input, normalize the level, and then transform the data
        //Then, reset the sample counter and return a reference to the frame
        if (fftInputIndex == FFTSize) {
            std::fill (fftData.begin()+FFTSize, fftData.end(), 0.0f);
            std::copy (fftInput.begin(), fftInput.end(), fftData.begin());
            applyWindowToInput(fftData);
            scaleFFTData(fftData);
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

    auto perform (const SampleType& sample) noexcept {
        return pushNextSampleIntoFifo (sample);
    }

    //Reset all fft data to 0
    void reset() {
        std::fill (fftData.begin(), fftData.end(), 0.0f);
        std::fill (fftInput.begin(), fftInput.end(), 0.0f);
    }

private:
    juce::dsp::FFT forwardFFT{ juce::roundToInt(std::log2(FFTSize)) };
    std::array<SampleType, FFTSize> fftInput;
    std::array<SampleType, FFTSize * 2> fftData;

    size_t fftInputIndex{ 0 };
    juce::dsp::WindowingFunction<SampleType> windowFunc{FFTSize, juce::dsp::WindowingFunction<SampleType>::WindowingMethod::hann};

    //Apply a Hann Window to the input data
    template<typename T>
    void applyWindowToInput(T& container) noexcept {
        const auto windowSize = container.size()*.5;
        windowFunc.multiplyWithWindowingTable(container.data(), windowSize);
    }

    //Normalize the input data
    template<typename T>
    auto scaleFFTData(T& collection) noexcept {
        for(auto&& sample : collection)
            sample *= 2.0/FFTSize;
    }
};