#pragma once

#include <catch2/catch.hpp>

#include "../Utilities/DecibelMatchers.h"

#include "FilterMeasurementUtilities.h"

//Test the spectrum shape, rolloff, and level reduction at the cutoff frequency of a non-resonant lowpass filter
//TODO: Add the capability to test various q values
template<typename... Ts>
void testLowpassResponse(FilterTestContext<Ts...>& testContext) {
    constexpr auto FFTSize = FilterTestContext<Ts...>::SpectrumSize;

    const auto filterSpectrum = getFilteredSpectrum(testContext.fft,
                                                    testContext.noiseBuffer,
                                                    testContext.filter);

    SECTION("Spectrum Shape") {
        for (auto i = 0; i < FFTSize/2; ++i) {
            //From 0 to nyquist, check that:
            //The current bin's is around the same level for both the input and output
            //Or it is quieter for the input
            const Amplitude<float> noiseLevel = testContext.noiseSpectrum[i].getAverage();
            const Amplitude<float> filteredLevel = filterSpectrum[i].getAverage();
            REQUIRE((WithinDecibels<float>(filteredLevel,
                                           testContext.tolerance)
                                   .match(Decibel<float>(noiseLevel))
                    || noiseLevel > filteredLevel));
            if(i > 0) {
                //After the first bin:
                //Check that the current bin is either the same level or quieter than the previous
                //Or that the difference etween them is below the threshold of hearing
                const auto currentBinLevel  = Amplitude{filterSpectrum[i].getAverage()};
                const auto previousBinLevel = Amplitude{filterSpectrum[i-1].getAverage()};
                REQUIRE((WithinDecibels<float>(previousBinLevel,
                                              testContext.tolerance)
                                        .match(Decibel(currentBinLevel))
                     || Decibel(currentBinLevel) < Decibel(previousBinLevel)
                     || ResidualDecibels<float>(Decibel(currentBinLevel), -120.0_dB)
                                        .match(Decibel(previousBinLevel))));
            }
        }
    }

    //Warp the cutoff frequency
    const auto cutoff  = DigitalFrequency{testContext.cutoff};

    SECTION("Peak Level Amplitude At Cutoff") {
        // Measure the gain difference of sin wave with the same frequncy as the warped cutoff
        // input and output from the filter
        //Check that it's within half a dB of a 3dB reduction
        REQUIRE_THAT(calculateLevelReductionAtFrequency(testContext.filter,
                                                             cutoff.count(),
                                                             testContext.sampleRate),
                     WithinDecibels<float>(-3.0_dB, .5_dB));
    }



    SECTION("Filter Gain Rolloff") {
        // Test that the rolloff per octave happens at the expected rate over several octaves
        // This will pass if the rollof is inside the tolerance
        testRolloffCharacteristics<RolloffDirection::Up>(testContext.filter,
                                                         cutoff,
                                                         testContext.sampleRate,
                                                         testContext.rolloffPerOctave,
                                                         testContext.tolerance);
    }
}

//Test the spectrum shape, rolloff, and level reduction at the cutoff frequency of a non-resonant highpass filter
//TODO: Add the capability to test various q values
template<typename... Ts>
void testHighpassResponse(FilterTestContext<Ts...>& testContext) {
    constexpr auto FFTSize = FilterTestContext<Ts...>::SpectrumSize;
    BufferAverager<float, FFTSize> accumulator{};

    accumulator.reset();

    const auto filterSpectrum = accumulator.getBuffer();

    SECTION("Spectrum Shape") {
        for (auto i = 0; i < FFTSize/2; ++i) {
            const Amplitude<float> noiseLevel = testContext.noiseSpectrum[i].getAverage();
            const Amplitude<float> filteredLevel = filterSpectrum[i].getAverage();
            REQUIRE((WithinDecibels<float>(filteredLevel,
                                           testContext.tolerance)
                                    .match(Decibel<float>(noiseLevel))
                    || noiseLevel > filteredLevel));

            if(i > 0) {
                const Amplitude<float> currentBinLevel = filterSpectrum[i].getAverage();
                const Amplitude<float> previousBinLevel = filterSpectrum[i-1].getAverage();

                REQUIRE((WithinDecibels<float>(previousBinLevel,
                                               testContext.tolerance)
                                        .match(Decibel(currentBinLevel))
                         || ResidualDecibels<float>(Decibel(currentBinLevel), -120.0_dB)
                                        .match(Decibel(previousBinLevel))
                         || Decibel(currentBinLevel) > Decibel(previousBinLevel)));
            }
        }
    }

    const auto cutoff = DigitalFrequency{testContext.cutoff};

    // Measure the gain difference of sin wave with the same frequncy as the warped cutoff
    // input and output from the filter
    //Check that it's within half a dB of a 3dB reduction
    SECTION("Peak Level Amplitude At Cutoff") {
        REQUIRE_THAT(calculateLevelReductionAtFrequency(testContext.filter,
                                                             cutoff.count(),
                                                             testContext.sampleRate),
                     WithinDecibels<float>(-3.0_dB, .5_dB));
    }


    SECTION("Filter Gain Rolloff") {
        // Test that the rolloff per octave happens at the expected rate over several octaves
        // This will pass if the rollof is inside the tolerance
        testRolloffCharacteristics<RolloffDirection::Down>(testContext.filter,
                                                           cutoff,
                                                           testContext.sampleRate,
                                                           testContext.rolloffPerOctave,
                                                           testContext.tolerance);
    }
}

//Test the spectrum shape, and level reduction at the center frequency of a bandpass filter
//TODO: Add the capability to test various q values and verify the rolloff rate
template<typename... Ts>
void testBandpassResponse(FilterTestContext<Ts...>& testContext) {
    constexpr auto FFTSize = FilterTestContext<Ts...>::SpectrumSize;

    const auto cutoff = DigitalFrequency{testContext.cutoff};

    const auto filterSpectrum = getFilteredSpectrum(testContext.fft,
                                                    testContext.noiseBuffer,
                                                    testContext.filter);

    SECTION("Spectrum Shape") {
        for (auto i = 0; i < (cutoff.count()/testContext.sampleRate)*FFTSize/2; ++i) {
            const Amplitude<float> noiseLevel = testContext.noiseSpectrum[i].getAverage();
            const Amplitude<float> filteredLevel = filterSpectrum[i].getAverage();
            REQUIRE((WithinDecibels<float>(filteredLevel,
                                           testContext.tolerance)
                             .match(Decibel<float>(noiseLevel))
                     || noiseLevel > filteredLevel));
            if(i > 0) {
                const auto currentBinLevel  = Amplitude{filterSpectrum[i].getAverage()};
                const auto previousBinLevel = Amplitude{filterSpectrum[i-1].getAverage()};
                REQUIRE((WithinDecibels<float>(previousBinLevel,
                                               testContext.tolerance)
                                 .match(Decibel(currentBinLevel))
                         || ResidualDecibels<float>(Decibel(currentBinLevel), -120.0_dB)
                                 .match(Decibel(previousBinLevel))
                         || Decibel(currentBinLevel) > Decibel(previousBinLevel)));
            }
        }

        for (auto i = (cutoff.count()/testContext.sampleRate); i < FFTSize/2; ++i) {
            const Amplitude<float> noiseLevel = testContext.noiseSpectrum[i].getAverage();
            const Amplitude<float> filteredLevel = filterSpectrum[i].getAverage();
            REQUIRE((WithinDecibels<float>(filteredLevel,
                                           testContext.tolerance)
                             .match(Decibel<float>(noiseLevel))
                     || noiseLevel > filteredLevel));

            const auto currentBinLevel  = Amplitude{filterSpectrum[i].getAverage()};
            const auto previousBinLevel = Amplitude{filterSpectrum[i-1].getAverage()};
            REQUIRE((WithinDecibels<float>(previousBinLevel,
                                           testContext.tolerance)
                             .match(Decibel(currentBinLevel))
                     || ResidualDecibels<float>(Decibel(currentBinLevel), -120.0_dB)
                             .match(Decibel(previousBinLevel))
                     || Decibel(currentBinLevel) < Decibel(previousBinLevel)));
        }
    }

    // Measure the gain difference of sin wave with the same frequncy as the warped cutoff
    // input and output from the filter
    //Check that the output is within half a dB of the input
    SECTION("Peak Level At Center Frequency") {
        REQUIRE_THAT(calculateLevelReductionAtFrequency(testContext.filter,
                                                        cutoff.count(),
                                                        testContext.sampleRate),
                     WithinDecibels<float>(0.0_dB, .5_dB));
    }
}

//Test the spectrum shape, and level reduction at the center frequency of a bandreject filter
//TODO: Add the capability to test various q values and verify the rolloff rate
template<typename... Ts>
void testBandrejectResponse(FilterTestContext<Ts...>& testContext) {
    constexpr auto FFTSize = FilterTestContext<Ts...>::SpectrumSize;

    const auto cutoff  = DigitalFrequency{testContext.cutoff};

    const auto filterSpectrum = getFilteredSpectrum(testContext.fft,
                                                    testContext.noiseBuffer,
                                                    testContext.filter);

    SECTION("Spectrum Shape") {
        //Get the bin that holds the cutoff frequency
        const auto centerBinIndex = (cutoff.count()/testContext.sampleRate)*FFTSize/2;
        //From the bottom of the spectrum to the cutoff frequency, check to make sure:
        // That the output is quieter than the input
        // That the output spectrum increases as frequency increases
        for (int i = 0; i < centerBinIndex; ++i) {
            const Amplitude<float> noiseLevel = testContext.noiseSpectrum[i].getAverage();
            const Amplitude<float> filteredLevel = filterSpectrum[i].getAverage();
            REQUIRE((WithinDecibels<float>(filteredLevel,
                                           testContext.tolerance)
                             .match(Decibel<float>(noiseLevel))
                     || noiseLevel > filteredLevel));
            if(i > 0) {
                const auto currentBinLevel  = Amplitude{filterSpectrum[i].getAverage()};
                const auto previousBinLevel = Amplitude{filterSpectrum[i-1].getAverage()};
                REQUIRE((WithinDecibels<float>(previousBinLevel,
                                               testContext.tolerance)
                                 .match(Decibel(currentBinLevel))
                         || ResidualDecibels<float>(Decibel(currentBinLevel), -120.0_dB)
                                 .match(Decibel(previousBinLevel))
                         || Decibel(currentBinLevel) < Decibel(previousBinLevel)));
            }
        }

        //From the cutoff frequency to the top of the spectrum, check to make sure:
        // That the output is quieter than the input
        // That the output spectrum decreases as frequency increases
        for (int i = centerBinIndex; i < (20000.0/testContext.sampleRate)*FFTSize/2; ++i) {
            const Amplitude<float> noiseLevel = testContext.noiseSpectrum[i].getAverage();
            const Amplitude<float> filteredLevel = filterSpectrum[i].getAverage();
            REQUIRE((WithinDecibels<float>(filteredLevel,
                                           testContext.tolerance)
                             .match(Decibel<float>(noiseLevel))
                     || noiseLevel > filteredLevel));

            if(i-1 > centerBinIndex) {
                const auto currentBinLevel = Amplitude{filterSpectrum[i].getAverage()};
                const auto previousBinLevel = Amplitude{filterSpectrum[i - 1].getAverage()};

                REQUIRE((WithinDecibels<float>(previousBinLevel,
                                               testContext.tolerance)
                                 .match(Decibel(currentBinLevel))
                         || ResidualDecibels<float>(Decibel(currentBinLevel), -120.0_dB)
                                 .match(Decibel(previousBinLevel))
                         || Decibel(currentBinLevel) > Decibel(previousBinLevel)));
            }
        }
    }

    // Measure the gain difference of sin wave with the same frequncy as the warped cutoff
    // input and output from the filter
    // Check that the difference is lower than -48dB
    // This number can go lower if you run more iterations on the aver
    SECTION("Peak Level At Center Frequency") {
        REQUIRE(calculateLevelReductionAtFrequency(testContext.filter,
                                                        cutoff.count(),
                                                        testContext.sampleRate)
                < -48.0_dB);
    }
}

//Test the spectrum shape and gain of a peak filter
template<typename... Ts>
void testAllpassResponse(FilterTestContext<Ts...>& testContext)
{
    //Get the spectrum of white noise run through the filter
    const auto filteredSpectrum = getFilteredSpectrum(testContext.fft, testContext.noiseBuffer, testContext.filter);

    //Check all of the bins in the filtered spectrum against the noise spectrum
    //Require that the difference between the two be inside the tolerance
    for(auto i = 1; i < testContext.noiseSpectrum.size(); ++i) {
        const Decibel<float> noiseLevel = Amplitude{testContext.noiseSpectrum[i].getAverage()};
        const Decibel<float> filteredLevel = Amplitude{filteredSpectrum[i].getAverage()};

        REQUIRE_THAT(noiseLevel, WithinDecibels<float>(filteredLevel, testContext.tolerance));
    }
}

//Test the spectrum shape and gain of a peak filter
template<typename... Ts>
void testPeakResponse(FilterTestContext<Ts...>& testContext)
{
    constexpr auto FFTSize = FilterTestContext<Ts...>::SpectrumSize;

    const auto cutoff  = DigitalFrequency{testContext.cutoff};

    const auto filterSpectrum = getFilteredSpectrum(testContext.fft,
                                                    testContext.noiseBuffer,
                                                    testContext.filter);

    SECTION("Spectrum Shape") {
        for (auto i = 0; i < (cutoff.count()/testContext.sampleRate)*FFTSize/2; ++i) {
            const Amplitude<float> noiseLevel = testContext.noiseSpectrum[i].getAverage();
            const Amplitude<float> filteredLevel = filterSpectrum[i].getAverage();
            REQUIRE((WithinDecibels<float>(filteredLevel,
                                           testContext.tolerance)
                             .match(Decibel<float>(noiseLevel))
                     || noiseLevel < filteredLevel));
            if(i > 0) {
                const auto currentBinLevel  = Amplitude{filterSpectrum[i].getAverage()};
                const auto previousBinLevel = Amplitude{filterSpectrum[i-1].getAverage()};
                REQUIRE((WithinDecibels<float>(previousBinLevel,
                                               testContext.tolerance)
                                 .match(Decibel(currentBinLevel))
                         || ResidualDecibels<float>(Decibel(currentBinLevel), -120.0_dB)
                                 .match(Decibel(previousBinLevel))
                         || Decibel(currentBinLevel) > Decibel(previousBinLevel)));
            }
        }

        for (auto i = (cutoff.count()/testContext.sampleRate); i < FFTSize/2; ++i) {
            const Amplitude<float> noiseLevel = testContext.noiseSpectrum[i].getAverage();
            const Amplitude<float> filteredLevel = filterSpectrum[i].getAverage();
            REQUIRE((WithinDecibels<float>(filteredLevel,
                                           testContext.tolerance)
                             .match(Decibel<float>(noiseLevel))
                     || noiseLevel < filteredLevel));

            const auto currentBinLevel  = Amplitude{filterSpectrum[i].getAverage()};
            const auto previousBinLevel = Amplitude{filterSpectrum[i-1].getAverage()};
            REQUIRE((WithinDecibels<float>(previousBinLevel,
                                           testContext.tolerance)
                             .match(Decibel(currentBinLevel))
                     || ResidualDecibels<float>(Decibel(currentBinLevel), -120.0_dB)
                             .match(Decibel(previousBinLevel))
                     || Decibel(currentBinLevel) < Decibel(previousBinLevel)));
        }
    }


    SECTION("Peak Level At Center Frequency") {
        const auto levelDifference = calculateLevelReductionAtFrequency(testContext.filter,
                                                                        cutoff.count(),
                                                                        testContext.sampleRate);
        REQUIRE_THAT(levelDifference, WithinDecibels(Decibel{Amplitude{testContext.filterGain}}, Decibel{.1}));
    }
}

//TODO: Add Shelving response tests
