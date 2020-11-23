#pragma once

#include "FilterTestUtilities.h"

//Measure the level of a sin wave run through a filter at a certain frquency
template<typename T>
auto measureFilteredSinLevelAtFrequency(T& filter, double testFrequency, double sampleRate) {
    CumulativeAverage<float> outputAverage{};

    Oscillator<double> sinWave;
    sinWave.setSampleRate(sampleRate);
    sinWave.setFrequency(testFrequency);
    sinWave.setWaveform(std::make_unique<SinShaper<double>>());

    filter.reset();


    for (auto i = 0; i < 100000; ++i) {
        const auto filteredVal = filter.processSample(sinWave.perform());
        outputAverage.updateAverage(std::abs(filteredVal));
    }
    return outputAverage;
}

//Get the spectrum of white noise through a filter
template<size_t FFTSize, typename NoiseBuffer, typename Filter>
auto getFilteredSpectrum(FFTHelper<float, FFTSize>& fft, const NoiseBuffer& noiseBuffer, Filter& filter)
{
    BufferAverager<float, FFTSize * 2> accumulator{};

    fft.reset();
    filter.reset();

    for(auto&& sample : noiseBuffer) {
        const auto result = fft.perform(filter.processSample(sample));
        if (result != std::nullopt)
            accumulator.perform(result.value());
    }
    return accumulator.getBuffer();
}

//Calculates the difference in level of a sin wave before and after filtering
template<typename Filter>
auto calculateLevelReductionAtFrequency(Filter& filter,
                                        const Frequency<double>& frequency,
                                        double sampleRate)
{
    CumulativeAverage<float> sinAverage{};
    CumulativeAverage<float> filterAverage{};

    Oscillator<double> sinWave;
    sinWave.setSampleRate(sampleRate);
    sinWave.setFrequency(frequency);
    sinWave.setWaveform(std::make_unique<SinShaper<double>>());

    filter.reset();

    for (auto i = 0; i < 100000; ++i) {
        const auto sinVal = sinWave.perform();
        sinAverage.updateAverage(std::abs(sinVal));
        filterAverage.updateAverage(std::abs(filter.processSample(sinVal)));
    }

    const Decibel<float> peakSinLevel    = Amplitude(sinAverage.getAverage());
    const Decibel<float> peakFilterLevel = Amplitude(filterAverage.getAverage());
    const auto decibelValue = peakSinLevel.count()-peakFilterLevel.count();
    const auto sign = std::signbit(decibelValue) ? 1.0 : -1.0;
    return Decibel{sign*std::abs(decibelValue)};
}

//Get the average absolute amplitude of a sin wave at a given frequency
auto getSinAverage(double frequency, double sampleRate) {
    //Make a sine wave oscillator
    Oscillator<double> sinWave;
    sinWave.setSampleRate(sampleRate);
    sinWave.setFrequency(frequency);
    sinWave.setWaveform(std::make_unique<SinShaper<double>>());

    //Make an averager to measure the value
    CumulativeAverage<float> sinAverage{};

    for (auto i = 0; i < 100000; ++i) {
        const auto sinVal = sinWave.perform();
        sinAverage.updateAverage(std::abs(sinVal));
    }

    return sinAverage;
}

enum RolloffDirection {
    Up, Down
};

//Tests the level of a sin wave through a filter at different octaves
//The test passes if the difference in level between octaves matches rolloff, within the tolerance
template<RolloffDirection Direction, typename Filter, typename T>
void testRolloffCharacteristics(Filter& filter,
                                const DigitalFrequency<T>& cutoff,
                                double sampleRate,
                                const Decibel<T>& rolloffAmount,
                                const Decibel<T>& tolerance)
{
    //If the spectrum rolloffs as the frequency gets higher, then we want to measure increasing doubles of a frequency
    //If the rolloff is in the down direction, then we want successive halves
    constexpr auto octaveScalar = Direction == RolloffDirection::Up ? 2 : .5;

    filter.reset();

    //Over 8 octaves...
    for(auto i = 0; i < 8; ++i) {
        //Get the desired frequency value of the first octave, clamping it if it gets too high
        const auto boundedFrequency = AnalogFrequency<double>{DigitalFrequency<double>{std::min(sampleRate/8.0,
                                                                                                std::pow(octaveScalar, i)*cutoff.count()), sampleRate}, sampleRate};

        //Get the desired frequency value of the first octave, clamping it if it gets too high
        const auto boundedFrequency1 = DigitalFrequency{boundedFrequency*.5, sampleRate};

        //Get the average level of the first ocatve
        const Decibel<double> currentCutoffAverage
                = Amplitude<double>(measureFilteredSinLevelAtFrequency(filter, boundedFrequency.count(), sampleRate).getAverage());
        //Get the average level of the second ocatve
        const Decibel<double> nextCutoffAverage
                = Amplitude<double>(measureFilteredSinLevelAtFrequency(filter, boundedFrequency1.count(), sampleRate).getAverage());
        //Check that the second octave plus the rolloff and threshold is higher than the current octave
        //Meaning that, when correcting for rolloff, the two octaves are withen the tolerance level of each other
        REQUIRE(nextCutoffAverage.count()
                - rolloffAmount.count()
                + std::abs(tolerance.count())
                >= currentCutoffAverage.count());
    }
}

enum GainChange {
    Louder, Quieter
};

template<GainChange Direction, typename T>
auto isSameOr(const Decibel<T>& level1,
              const Decibel<T>& level2,
              const Decibel<T>& tolerance)
{
    const auto withinTolerance = std::abs(level1.count()-level2.count())
                               < std::abs(tolerance.count());

    if constexpr(Direction == GainChange::Louder)
        return withinTolerance || (level1 > level2);
    else
        return withinTolerance || (level1 < level2);
}

template<GainChange Direction, typename T>
auto isSameOr(const Amplitude<T>& level1,
              const Amplitude<T>& level2,
              const Decibel<T>& tolerance)
{
    return isSameOr<Direction>(Decibel{level1}, Decibel{level2}, tolerance);
}
