#pragma once

#include <cmath>

#include <juce_core/juce_core.h>

// A phasor class for driving an oscillator.
// This outputs a signal from 0-1 periodically according to a frequency and sample rate.
template<typename SampleType>
class Phasor
{
public:
    void setSampleRate(SampleType newPerformRate) noexcept {
        sampleRate = newPerformRate;
        phaseVelocity = frequency/sampleRate;
    }

    void reset() noexcept { correctionTerm = phase = SampleType{0};}

    void setFrequency(const SampleType& newFrequency) noexcept {
        frequency = newFrequency;
        phaseVelocity = frequency/sampleRate;
    }

    void setPhase(const SampleType& newPhase) noexcept {
        phase = std::fmod(newPhase, SampleType{1});
    }

    //Set the phase, and then increment the phase value, and then wrap it between 0 and 1
    SampleType perform(const SampleType& newPhase) noexcept {
        setPhase(std::forward<const SampleType>(newPhase));
        return perform();
    }

    //Increment the phase value, and then wrap it between 0 and 1
    SampleType perform() noexcept {
        const auto output = phase;
        phase = std::fmod(kahanSummation(), SampleType{1});
        return output;
    }

private:
    SampleType phase{0}, frequency{0}, sampleRate{0},
            phaseVelocity{0}, correctionTerm{0};

    // Performs kahan summation on the phase value
    // this keeps the error of the phase value lower than by just using +=
    // Needed to pass the triangle waveform test,
    // but all of the oscillator tests will require it if the number of iterations are increased
    auto kahanSummation() noexcept {
        const auto y = phaseVelocity-correctionTerm;
        const auto t = phase+y;
        correctionTerm = (t-phase)-y;
        return t;
    }
};