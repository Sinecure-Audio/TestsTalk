#pragma once

//Include std::sin and juce's Pi constant
#include <cmath>
#include <juce_core/juce_core.h>

// A simple oscillator class that plays a sin wave at 440hz
// with a sampling rate of 44.1
class Oscillator
{
public:
    //increments the phase, returning the previous phase value
    auto perform() noexcept {
        const auto output = std::sin(phase*juce::MathConstants<double>::twoPi);
        phase += 440.0/44100.0;
        return output;
    }

private:
    double phase{0};
};