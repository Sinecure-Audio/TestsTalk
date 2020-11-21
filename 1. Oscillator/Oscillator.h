#pragma once

#include "Phasor.h"

#include <cmath>
#include <juce_core/juce_core.h>

//Base class for our waveforms- this simply returns the input
template<typename SampleType>
class Shaper
{
public:
    virtual SampleType perform(const SampleType& in) {
        return in;
    }
};

//Semantically, our Shaper is an identity function, so let's create an alias for it
template<typename... Ts>
using IdentityFunction = Shaper<Ts...>;

//Sin wave shaper
template<typename SampleType>
class SinShaper : public Shaper<SampleType>
{
public:
    virtual SampleType perform(const SampleType& in) override {
        return std::sin(in*juce::MathConstants<SampleType>::twoPi);
    }
};

//Triangle wave shaper
template<typename SampleType>
class TriShaper : public Shaper<SampleType>
{
public:
    virtual SampleType perform(const SampleType& in) override {
        //Return a triangle wave that has a value of:
        // 0 at phase 0
        // 1 at phase .25,
        // -1 at phase .75
        if(in < .25)
            return in*SampleType{4};
        else if(in < .75)
            return SampleType{1}-SampleType{4}*(in-SampleType{.25});
        else
            return 4.0*in-4.0;
    }
};

//Square wave shaper
template<typename SampleType>
class SquareShaper : public Shaper<SampleType>
{
public:
    virtual SampleType perform(const SampleType& in) override {
        //If the waveform is in the high position, return 1, else return -1
        //Branchless, for style points:
        // this works by converting the bool check to the type of our sample
        // Then, it adds that result (1, or 0), to an expression that is -1
        // if the result is 0
        const auto isHigh = static_cast<SampleType>(in < SampleType{.5});
        return isHigh
               +(SampleType{1}-isHigh)*SampleType{-1};
    }
};

//Sawtooth wave shaper
template<typename SampleType>
class SawShaper : public Shaper<SampleType>
{
public:
    virtual SampleType perform(const SampleType& in) override {
        return in*SampleType{2}-SampleType{1};
    }
};

//An oscillator class that can change frequency and sample rate, and be synced
// By default it uses a shaper that simply returns the value of the phaser
// By calling setWaveform, it is possible to a class derived from Shaper<T> to change the waveform
template<typename SampleType>
class Oscillator
{
    using ShaperType = Shaper<SampleType>;
public:
    void setSampleRate(const SampleType& newPerformRate) noexcept {
        phasor.setSampleRate(newPerformRate);
    }

    void reset() noexcept {
        phasor.reset();
    }

    void setFrequency(const SampleType& newFrequency) noexcept {
        phasor.setFrequency(newFrequency);
    }

    void setPhase(const SampleType& newPhase) noexcept {
        phasor.setPhase(newPhase);
    }

    void setWaveform(std::unique_ptr<ShaperType>&& newShaper) noexcept {
        shaper.swap(newShaper);
    }

    const auto& getWaveform() const noexcept {
        return shaper;
    }

    SampleType perform(const SampleType& newPhase) noexcept {
        setPhase(newPhase);
        return perform();
    }

    SampleType perform() noexcept { return shaper->perform(phasor.perform()); }

private:
    Phasor<SampleType> phasor{};
    std::unique_ptr<ShaperType> shaper = std::make_unique<ShaperType>();
};