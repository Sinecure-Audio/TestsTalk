#pragma once

//Include fmod and size_t
#include <cmath>

// A phasor class for driving an oscillator.
// This outputs a signal from 0-1 periodically according to a frequency and sample rate.
template<typename SampleType>
class Phasor
{
public:
    void setSampleRate(SampleType newPerformRate) noexcept {
        sampleRate = newPerformRate;
        phaseVelocity = frequency/sampleRate;
        iterationsPerCycle = SampleType{1}/phaseVelocity;
        internalPhase = std::fmod(internalPhase+getPhaseFromIndex(counter), SampleType{1});
    }

    void reset() noexcept {
        counter = 0;
        internalPhase = phase = SampleType{0};
    }

    void setFrequency(const SampleType& newFrequency) noexcept {
        frequency = newFrequency;
        phaseVelocity = frequency/sampleRate;
        iterationsPerCycle = SampleType{1}/phaseVelocity;
        internalPhase = std::fmod(internalPhase+getPhaseFromIndex(counter), SampleType{1});
    }

    void setPhase(const SampleType& newPhase) noexcept {
        phase = std::fmod(newPhase, SampleType{1});
        counter = 0;
        internalPhase = SampleType{0};
    }

    //Set the phase, and then increment the phase value, and then wrap it between 0 and 1
    SampleType perform(const SampleType& newPhase) noexcept {
        setPhase(std::forward<const SampleType>(newPhase));
        return perform();
    }

    //Increment the phase value, and then wrap it between 0 and 1
    SampleType perform() noexcept {
        return std::fmod(phase+getPhaseFromIndex(counter++), SampleType{1});
    }

private:
    SampleType phase{0}, frequency{0}, sampleRate{44100},
            phaseVelocity{0}, internalPhase{0};

    size_t counter {0};
    SampleType iterationsPerCycle{0};

    constexpr auto getPhaseFromIndex(size_t index) const noexcept {
        return std::fmod(index, iterationsPerCycle)*phaseVelocity;
    }


};