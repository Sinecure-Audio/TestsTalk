#pragma once

//Include std::sin and juce's Pi constant
#include <cmath>
#include <juce_core/juce_core.h>

// A simple oscillator class that plays a sin wave at 440hz
// with an arbitrary sample rate
template<typename SampleType>
class Oscillator
{
public:
    void setSampleRate(SampleType newPerformRate) noexcept {
        performRate = newPerformRate;
    }


    void reset() noexcept {
        phase = SampleType{0};
    }

    void setFrequency(const SampleType& newFrequency) noexcept {
        frequency = newFrequency;
    }

    //increments the phase, returning the previous phase value
    SampleType perform() noexcept {
        const auto output = std::sin(phase*SampleType{juce::MathConstants<double>::twoPi});
        phase += frequency/performRate;
        return output;
    }

private:
    SampleType phase{}, frequency{}, performRate{};
};