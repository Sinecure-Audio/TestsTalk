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
        updatePhase();
    }

    void reset() noexcept {
        counter = 0;
        phase = SampleType{0};
    }

    void setFrequency(const SampleType& newFrequency) noexcept {
        frequency = newFrequency;
        updatePhase();
    }

    void setPhase(const SampleType& newPhase) noexcept {
        phase = std::fmod(newPhase, SampleType{1});
        counter = 0;
    }

    //Set the phase, and then increment the phase value, and then wrap it between 0 and 1
    SampleType perform(const SampleType& newPhase) noexcept {
        setPhase(std::forward<const SampleType>(newPhase));
        return perform();
    }

    //Gets the current value of the phasor, plus any phase offsets
    SampleType perform() noexcept {
        return std::fmod(phase+getPhaseFromIndex(counter++), SampleType{1});
    }

private:
    SampleType phase{0}, frequency{0}, sampleRate{44100},
            phaseVelocity{0};

    size_t counter {0};
    SampleType iterationsPerCycle{0};

    //Get the phase increment value by multiplying the number of iterations by the phase velocity
    // The phase increment is bounded by the number of iterations per cycle
    // For example, if it takes 10 increment to go one waveform, and the index is 99
    // The result will be 9*phaseVelocity
    //This is done to minimize the magnitude of any floating point error
    constexpr auto getPhaseFromIndex(size_t index) const noexcept {
        return std::fmod(static_cast<SampleType>(index), iterationsPerCycle)*phaseVelocity;
    }

    void updatePhase() {
        phaseVelocity = frequency/sampleRate;
        iterationsPerCycle = SampleType{1}/phaseVelocity;
        phase = std::fmod(static_cast<SampleType>(phase+getPhaseFromIndex(counter)), SampleType{1});
    }


};