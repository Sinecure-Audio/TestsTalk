#pragma once

#include "FilterTestUtilities.h"

//A variable that hold the number of iterations for the loops where we're
// measuring things
constexpr size_t numMeasurementIterations = 100000;

//Measure the level of a sin wave run through a filter at a certain frequency
template<typename SampleType, typename Filter>
auto measureFilteredSinLevelAtFrequency(Filter& filter,
                                        SampleType testFrequency,
                                        SampleType sampleRate) {
    CumulativeAverage<SampleType> outputAverage{};

    Oscillator<SampleType> sinWave;
    sinWave.setSampleRate(sampleRate);
    sinWave.setFrequency(testFrequency);
    sinWave.setWaveform(std::make_unique<SinShaper<SampleType>>());

    filter.reset();


    for (auto i = 0; i < numMeasurementIterations; ++i) {
        const auto filteredVal = filter.processSample(sinWave.perform());
        outputAverage.updateAverage(std::abs(filteredVal));
    }
    return outputAverage;
}

//Get the spectrum of white noise through a filter
template<typename SampleType,
         size_t FFTSize,
         typename NoiseBuffer,
         typename Filter>
auto getFilteredSpectrum(FFTHelper<FFTSize>& fft,
                         const NoiseBuffer& noiseBuffer,
                         Filter& filter)
{
    BufferAverager<SampleType, FFTSize*2> accumulator{};

    fft.reset();
    filter.reset();

    for(auto&& sample : noiseBuffer) {
        const auto result = fft.perform(static_cast<float>(filter.processSample(sample)));
        if (result != std::nullopt)
            accumulator.perform(result.value());
    }
    return accumulator.getBuffer();
}

//Calculates the difference in level of a sin wave before and after filtering
template<typename SampleType, typename Filter>
auto calculateLevelReductionAtFrequency(Filter& filter,
                                        const Frequency<SampleType>& frequency,
                                        SampleType sampleRate)
{
    CumulativeAverage<SampleType> sinAverage{};
    CumulativeAverage<SampleType> filterAverage{};

    Oscillator<SampleType> sinWave;
    sinWave.setSampleRate(sampleRate);
    sinWave.setFrequency(frequency);
    sinWave.setWaveform(std::make_unique<SinShaper<SampleType>>());

    filter.reset();

    for (auto i = 0; i < numMeasurementIterations; ++i) {
        const auto sinVal = sinWave.perform();
        sinAverage.updateAverage(std::abs(sinVal));
        filterAverage.updateAverage(std::abs(filter.processSample(sinVal)));
    }

    const Decibel<SampleType> peakSinLevel    = Amplitude(sinAverage.getAverage());
    const Decibel<SampleType> peakFilterLevel = Amplitude(filterAverage.getAverage());
    const auto decibelValue = peakSinLevel.count()-peakFilterLevel.count();
    const auto sign = std::signbit(decibelValue) ? 1.0 : -1.0;
    return Decibel{sign*std::abs(decibelValue)};
}

//Get the average absolute amplitude of a sin wave
// at a given frequency and samplerate
template<typename T>
auto getSinAverage(T frequency, T sampleRate) {
    //Make a sine wave oscillator
    Oscillator<T> sinWave;
    sinWave.setSampleRate(sampleRate);
    sinWave.setFrequency(frequency);
    sinWave.setWaveform(std::make_unique<SinShaper<T>>());

    //Make an averager to measure the value
    CumulativeAverage<T> sinAverage{};

    for (auto i = 0; i < numMeasurementIterations; ++i) {
        const auto sinVal = sinWave.perform();
        sinAverage.updateAverage(std::abs(sinVal));
    }

    return sinAverage;
}

// Tagged Value for setting the behavior of testRolloffCharacteristics
// Up means the filter starts flat and rolls off as the frequency increases
// Down means the filter ends flat and rolls off as the frequency decreases
// i.e. testRolloffCharacteristics<Up> testRolloffCharacteristics<Down>
enum RolloffDirection {
    Up, Down
};

//Tests the level of a sin wave through a filter at different octaves
//The test passes if the difference in level between octaves matches rolloff, within the tolerance
template<RolloffDirection Direction, typename Filter, typename T>
void testRolloffCharacteristics(Filter& filter,
                                const DigitalFrequency<T>& cutoff,
                                T sampleRate,
                                const Decibel<T>& rolloffAmount,
                                const Decibel<T>& tolerance)
{
    // If the spectrum rolloffs as the frequency gets higher,
    // then we want to measure successive doublings of a frequency
    // If the rolloff is in the down direction, then we want successive halves
    constexpr auto octaveScalar = Direction == RolloffDirection::Up
                                  ? T{ 2 }
                                  : T{ .5 };

    // Find how many octaves we can measure over before we starting hitting
    // nyquist/2
    const auto numOctaves = static_cast<size_t>(std::ceil(std::log(sampleRate/T{2})/std::log(cutoff.count())/std::log(2.0)));

    for(auto i = 0; i < numOctaves; ++i) {
        //Get the desired frequency value of the first octave, clamping it if it gets too high
        const auto boundedFrequency = AnalogFrequency<T>{DigitalFrequency<T>{std::min(sampleRate / T{8},
                                                                                       std::pow(octaveScalar,
                                                                                                T(i) * cutoff.count())
                                                                                       ),
                                                                              sampleRate},
                                                         sampleRate};

        //Get the desired frequency value of the first octave
        const auto lowFrequency = DigitalFrequency{ boundedFrequency / T{2},
                                           sampleRate };

        //Get the average level of the first octave
        const Decibel<T> currentCutoffAverage
                = Amplitude<T>(measureFilteredSinLevelAtFrequency(filter, boundedFrequency.count(), sampleRate).getAverage());
        //Get the average level of the second octave
        const Decibel<T> nextCutoffAverage
                = Amplitude<T>(measureFilteredSinLevelAtFrequency(filter, lowFrequency.count(), sampleRate).getAverage());
        //Check that the second octave plus the rolloff and threshold is higher than the current octave
        //Meaning that, when correcting for rolloff, the two octaves are within the tolerance level of each other
        REQUIRE(nextCutoffAverage.count()
                - rolloffAmount.count()
                + std::abs(tolerance.count())
                >= currentCutoffAverage.count());
    }
}

// Tagged Value for setting the behavior of isSameOr
// i.e. isSameOr<Louder> isSameOr<Quieter>
enum GainChange {
    Louder, Quieter
};

// Checks if two decibels are within a certain tolerance of each other
// Or if the first is louder or quieter than second, depending on the mode
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
