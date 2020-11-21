#pragma once

#include "Signal Analysis/Signal Analyzers.h"

#include "../1. Oscillator/Oscillator.h"

// Makes and returns a container of white noise samples
// Using vector so that very large buffers don't cause a stack overflow
template<typename SampleType, size_t NumSamples>
auto makeNoiseBuffer() {
    std::vector<SampleType> vec;
    vec.resize(NumSamples);
    std::generate(vec.begin(), vec.end(), [](){return getBoundedRandom(-1.0, 1.0);});
    return vec;
}

//Makes a buffer of noise and takes its spectrum, returning both
template<size_t FFTSize>
const auto makeNoiseBufferAndSpectrum() {
    static const auto buffer = makeNoiseBuffer<float, 100000>();
    static const auto spectrum = [&]() {
        BufferAverager<float, FFTSize * 2> accumulator{};
        FFTHelper<float, FFTSize> fft{};
        for (auto &&sample : buffer) {
            const auto result = fft.perform(sample);
            if (result != std::nullopt)
                accumulator.perform(result.value());
        }
        return accumulator.getBuffer();
    }();

    return std::tuple(buffer, spectrum);
}


template<typename FilterType, typename NoiseBuffer, typename NoiseSpectrum>
struct FilterTestContext {
    FilterTestContext(double newCutoff, const QCoefficient<double>& newQ,
                      double newSampleRate, FilterType&& newFilter,
                      const Decibel<double>& newRolloffPerOctave, const Decibel<double>& newTolerance,
                      const NoiseBuffer& newNoiseBuffer, const NoiseSpectrum& newNoiseSpectrum)
                      : cutoff(newCutoff), q(newQ), sampleRate(newSampleRate), filter(std::move(newFilter)),
                      rolloffPerOctave(newRolloffPerOctave), tolerance(newTolerance),
                      noiseBuffer(newNoiseBuffer), noiseSpectrum(newNoiseSpectrum)
    {}

    FilterTestContext(double newCutoff, const QCoefficient<double>& newQ,
                      double newSampleRate, FilterType&& newFilter, double newGain,
                      const Decibel<double>& newRolloffPerOctave, const Decibel<double>& newTolerance,
                      const NoiseBuffer& newNoiseBuffer, const NoiseSpectrum& newNoiseSpectrum)
            : cutoff(newCutoff), q(newQ), sampleRate(newSampleRate), filter(std::move(newFilter)), filterGain(newGain),
              rolloffPerOctave(newRolloffPerOctave), tolerance(newTolerance),
              noiseBuffer(newNoiseBuffer), noiseSpectrum(newNoiseSpectrum)
    {}

    double cutoff;
    QCoefficient<double> q;
    double sampleRate;
    double filterGain = 1.0;
    FilterType filter;

    Decibel<double> rolloffPerOctave, tolerance;

    const NoiseBuffer noiseBuffer;
    const NoiseSpectrum noiseSpectrum;

    static constexpr size_t SpectrumSize = std::tuple_size_v<NoiseSpectrum>;

    FFTHelper<float, SpectrumSize/2> fft{};

    void reset() { fft.reset(); filter.reset(); }
};

template<typename FilterType, typename V, typename W>
FilterTestContext(double, QCoefficient<double>, double, FilterType,
                  Decibel<double>, Decibel<double>,
                  V, W) ->
FilterTestContext<FilterType, V, W>;

template<typename FilterType, typename V, typename W>
FilterTestContext(double, QCoefficient<double>, double, FilterType, double,
                  Decibel<double>, Decibel<double>,
                  V, W) ->
FilterTestContext<FilterType, V, W>;


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

enum RolloffDirection {
    Up, Down
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

template<size_t FFTSize, FilterResponse Response, typename Filter, typename T>
auto makeFilterContext(T cutoff, QCoefficient<T> q, T sampleRate,
                       Decibel<T> rolloff, Decibel<T> tolerance, T gain = 1.0)
{
    const auto& [noiseBuffer, noiseSpectrum] = makeNoiseBufferAndSpectrum<FFTSize>();
// TODO: check why using references in the test context for the noise stuff doesn't work
    return FilterTestContext{cutoff, q, sampleRate,
                             std::move(setupFilter<Filter, Response>(cutoff,
                                                                     q,
                                                                     sampleRate,
                                                                     gain)),
                             gain,
                             rolloff, tolerance,
                             noiseBuffer, noiseSpectrum};
}
