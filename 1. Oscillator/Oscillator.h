#pragma once

#include "Phasor.h"

#include <cmath>
#include <juce_core/juce_core.h>

template<typename SampleType>
class Shaper
{
public:
    virtual SampleType perform(const SampleType& in) {
        return in;
    }
};

template<typename T>
using IdentityFunction = Shaper<T>;

template<typename SampleType>
class SinShaper : public Shaper<SampleType>
{
public:
    virtual SampleType perform(const SampleType& in) override {
        return std::sin(in*juce::MathConstants<SampleType>::twoPi);
    }
};

template<typename SampleType>
class TriShaper : public Shaper<SampleType>
{
public:
    virtual SampleType perform(const SampleType& in) override {
        if(in < .25)
            return in*SampleType{4};
        else if(in < .75)
            return SampleType{1}-SampleType{4}*(in-SampleType{.25});
//            return std::lerp(SampleType{1}, SampleType{-1}, SampleType{2}*(in-SampleType{.25}));
        else
            return 4.0*in-4.0;
    }
};

template<typename SampleType>
class SquareShaper : public Shaper<SampleType>
{
public:
    virtual SampleType perform(const SampleType& in) override {
        return static_cast<SampleType>(in >= SampleType{.5});
    }
};

template<typename SampleType>
class SawShaper : public Shaper<SampleType>
{
public:
    virtual SampleType perform(const SampleType& in) override {
        return in*SampleType{2}-SampleType{1};
    }
};

//
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

    SampleType perform(const SampleType& newPhase) noexcept {
        setPhase(newPhase);
        return perform();
    }

    SampleType perform() noexcept { return shaper->perform(phasor.perform()); }

    void setWaveform(std::unique_ptr<ShaperType>&& newShaper) noexcept {
        shaper.swap(newShaper);
    }

    const auto& getWaveform() const noexcept {
        return shaper;
    }

private:
    Phasor<SampleType> phasor{};
    std::unique_ptr<ShaperType> shaper = std::make_unique<ShaperType>();
};