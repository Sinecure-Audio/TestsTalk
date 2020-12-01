#pragma once

#include "Signal Analysis/Signal Analyzers.h"

#include "../1. Oscillator/Oscillator.h"

// Makes and returns a container of white noise samples
// Using vector so that very large buffers don't cause a stack overflow
template<typename SampleType, size_t NumSamples>
auto makeNoiseBuffer() {
    std::vector<SampleType> vec;
    vec.resize(NumSamples);
    std::generate(vec.begin(), vec.end(), []() {return getBoundedRandom(SampleType{ -1 }, SampleType{ 1 });});
    return vec;
}

template<typename SampleType, size_t FFTSize, typename T>
auto makeNoiseSpectrum(const T& buffer) {
    BufferAverager<SampleType, FFTSize * 2> accumulator{};
    FFTHelper<FFTSize> fft{};
    for (auto &&sample : buffer) {
        const auto result = fft.perform(static_cast<float>(sample));
        if (result != std::nullopt)
            accumulator.perform(result.value());
    }
    return accumulator.getBuffer();
}

template<typename T, size_t BufferSize, size_t SpectrumSize>
auto makeNoiseBufferAndSpectrum() {
    const auto buffer = makeNoiseBuffer<T, BufferSize>();
    const auto spectrum = makeNoiseSpectrum<T, SpectrumSize>(buffer);
    return std::pair{buffer, spectrum};
}

//Hold a buffer of noise and its spectrum for use in multiple test cases
//Making these is time intensive, the tests take too long as is,
// semantically one buffer/spectrum of noise is equivalent to another,
// so this lets the filter test context cache a set based on type
template<typename SampleType>
struct NoiseContext
{
private:
    //Initialize the buffer and spectrum together in a single function to prevent static order initialization fiasco
    static const inline auto vars = makeNoiseBufferAndSpectrum<SampleType, 100000, 1024>();

public:
    //Provide references to the buffer and spectrum
    static auto& getBuffer() {
        return vars.first;
    }

    static auto& getSpectrum() {
        return vars.second;
    }
};

template<typename FFTSize, typename FilterType, typename T>
struct FilterTestContext 
{
public:
    using SampleType = T;

    static constexpr size_t SpectrumSize = FFTSize::value*2;

    FilterTestContext(SampleType newCutoff, const QCoefficient<SampleType>& newQ,
                      SampleType newSampleRate, SampleType newGain, FilterType&& newFilter,
                      const Decibel<SampleType>& newRolloffPerOctave, const Decibel<SampleType>& newTolerance)//,
            : cutoff(newCutoff), q(newQ), sampleRate(newSampleRate), filterGain(newGain), filter(std::move(newFilter)),
              rolloffPerOctave(newRolloffPerOctave), tolerance(newTolerance) {}

    SampleType cutoff{};
    QCoefficient<SampleType> q{0};
    SampleType sampleRate{};
    SampleType filterGain{};
    FilterType filter{};

    Decibel<SampleType> rolloffPerOctave{}, tolerance{};

    const static inline auto&  noiseBuffer =
            NoiseContext<SampleType>::getBuffer();
    const static inline auto& noiseSpectrum =
            NoiseContext<SampleType>::getSpectrum();
//    decltype(NoiseContext<SampleType>::buffer) noiseBuffer   =
//            NoiseContext<SampleType>::buffer;
//    decltype(NoiseContext<SampleType>::spectrum) noiseSpectrum =
//            NoiseContext<SampleType>::spectrum;

    FFTHelper<FFTSize::value> fft{};

    void reset() { fft.reset(); filter.reset(); }
};


enum FilterResponse {
    Lowpass,
    Highpass,
    Bandpass,
    BandReject,
    Allpass,
    Peak,
    LowShelf,
    HighShelf,
};


//A utility class for initializing JUCE's dsp::IIR filter
template<typename Filter, FilterResponse Response, typename T>
auto makeJuceDspIir(const T& cutoff, const QCoefficient<T>& q, const T& sampleRate, const T& gain) {
    if constexpr(Response == FilterResponse::Lowpass) {
        return Filter {Filter::CoefficientsPtr::ReferencedType::makeLowPass(sampleRate, cutoff, q.count())};
    }
    else if constexpr(Response == FilterResponse::Highpass) {
        return Filter{Filter::CoefficientsPtr::ReferencedType::makeHighPass(sampleRate, cutoff, q.count())};
    }
    else if constexpr(Response == FilterResponse::Bandpass) {
        return Filter{Filter::CoefficientsPtr::ReferencedType::makeBandPass(sampleRate, cutoff, q.count())};
    }
    else if constexpr(Response == FilterResponse::BandReject) {
        return Filter{Filter::CoefficientsPtr::ReferencedType::makeNotch(sampleRate, cutoff, q.count())};
    }
    else if constexpr(Response == FilterResponse::Allpass) {
        return Filter{Filter::CoefficientsPtr::ReferencedType::makeAllPass(sampleRate, cutoff, q.count())};
    }
    else if constexpr(Response == FilterResponse::Peak) {
        return Filter{Filter::CoefficientsPtr::ReferencedType::makePeakFilter(sampleRate, cutoff, q.count(), gain)};
    }
    else if constexpr(Response == FilterResponse::LowShelf) {
        return Filter{Filter::CoefficientsPtr::ReferencedType::makeLowShelf(sampleRate, cutoff, q.count(), gain)};
    }
    else if constexpr(Response == FilterResponse::HighShelf) {
        return Filter{Filter::CoefficientsPtr::ReferencedType::makeHighShelf(sampleRate, cutoff, q.count(), gain)};
    }
}

template<typename Filter, FilterResponse Response, typename T>
auto setupFilter(const T& cutoff, const QCoefficient<T>& q, const T& sampleRate, const T& gain = T{1}) {
    if constexpr(std::is_same_v<Filter, juce::dsp::IIR::Filter<float>>
                 || std::is_same_v<Filter, juce::dsp::IIR::Filter<double>>)
        return makeJuceDspIir<Filter, Response>(cutoff, q, sampleRate, gain);
}

template<FilterResponse Response, typename... Ts>
auto runFilterTests(FilterTestContext<Ts...>& testContext) noexcept {
if constexpr (Response == FilterResponse::Lowpass)
    testLowpassResponse(testContext);
else if constexpr (Response == FilterResponse::Highpass)
    testHighpassResponse(testContext);
else if constexpr (Response == FilterResponse::Bandpass)
    testBandpassResponse(testContext);
else if constexpr (Response == FilterResponse::BandReject)
    testBandrejectResponse(testContext);
else if constexpr (Response == FilterResponse::Allpass)
    testAllpassResponse(testContext);
else if constexpr (Response == FilterResponse::Peak)
    testPeakResponse(testContext);
else if constexpr (Response == FilterResponse::LowShelf)
    testLowShelfResponse(testContext);
else if constexpr (Response == FilterResponse::HighShelf)
    testHighShelfResponse(testContext);
}

//TODO: Add variable Q
//Eventually, add random Q's for low/high/bandpass and bandreject
//For now, just use .707 i.e. flat
template<FilterResponse Response, typename T>
constexpr auto getQValue() {
//    if constexpr(FILTERTYPE < 4)
    return QCoefficient<T>(std::sqrt(T{2})/T{2});
//    else
//        return QCoefficient<T>{T{1}};
}

//If we're testing a shelf or peak filter, generate a random gain
//Otherwise, just set the gain as 1
template<FilterResponse Response, typename T>
constexpr auto getGainValue() {
    if constexpr (Response == FilterResponse::Peak
               || Response == FilterResponse::LowShelf
               || Response == FilterResponse::HighShelf)
        return GENERATE(take(10, random(T{.1}, T{100})));
    else
        return T{1};
}