#pragma once

#include <catch2/catch.hpp>

#include "../Utilities/DecibelMatchers.h"

#include "FilterMeasurementUtilities.h"

//Test the spectrum shape, rolloff, and level reduction at the cutoff frequency of a non-resonant lowpass filter
//TODO: Add the capability to test various q values i.e. filters with resonance

template<typename... Ts>
void testLowpassResponse(FilterTestContext<Ts...>& testContext) 
{
    using SampleType = typename FilterTestContext<Ts...>::SampleType;
    constexpr auto FFTSize = FilterTestContext<Ts...>::SpectrumSize/2;

    const auto filterSpectrum = getFilteredSpectrum<SampleType>(testContext.fft,
                                                                testContext.noiseBuffer,
                                                                testContext.filter);

    SECTION("Spectrum Shape") {
        for (size_t i = 0; i < FFTSize/2; ++i) {
            //From 0 to nyquist, check that:
            //The current bin's is around the same level for both the input and output
            //Or it is quieter for the input
            const Decibel<SampleType> noiseLevel    = Amplitude{testContext.noiseSpectrum[i].getAverage()};
            const Decibel<SampleType> filteredLevel = Amplitude{filterSpectrum[i].getAverage()};

            const auto outputSameOrQuieter = isSameOr<GainChange::Quieter>(noiseLevel,
                                                                           filteredLevel,
                                                                           testContext.tolerance);
            const auto differenceVeryQuiet = ResidualDecibels<SampleType>(noiseLevel,
                                                                          -120.0_dB)
                                             .match(filteredLevel);

            REQUIRE((outputSameOrQuieter || differenceVeryQuiet));

            if(i > 0) {
                //After the first bin:
                //Check that the current bin is either the same level or quieter than the previous
                //Or that the difference between them is below the threshold of hearing
                const Decibel<SampleType> currentBinLevel  = Amplitude{filterSpectrum[i].getAverage()};
                const Decibel<SampleType> previousBinLevel = Amplitude{filterSpectrum[i-1].getAverage()};

                const auto currentBinSameOrQuieter = isSameOr<GainChange::Quieter>(currentBinLevel,
                                                                                   previousBinLevel,
                                                                                   testContext.tolerance);
                const auto binDifferenceVeryQuiet = ResidualDecibels<SampleType>(currentBinLevel,
                                                                                 -120.0_dB)
                                                    .match(previousBinLevel);

                REQUIRE((currentBinSameOrQuieter || binDifferenceVeryQuiet));
            }
        }
    }

    //Warp the cutoff frequency
    const auto cutoff  = DigitalFrequency{testContext.cutoff};

    SECTION("Peak Level Amplitude At Cutoff") {
        // Measure the gain difference of sin wave with the same frequency as the warped cutoff
        // input and output from the filter
        //Check that it's within half a dB of a 3dB reduction
        REQUIRE_THAT(calculateLevelReductionAtFrequency<SampleType>(testContext.filter,
                                                                        cutoff.count(),
                                                                        testContext.sampleRate),
                     WithinDecibels(Decibel{SampleType{-3}}, Decibel{SampleType{.5}}));
    }



    SECTION("Filter Gain Rolloff") {
        // Test that the rolloff per octave happens at the expected rate over several octaves
        // This will pass if the rolloff is inside the tolerance
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
void testHighpassResponse(FilterTestContext<Ts...>& testContext) 
{
    using SampleType = typename FilterTestContext<Ts...>::SampleType;
    constexpr auto FFTSize = FilterTestContext<Ts...>::SpectrumSize/2;

    const auto filterSpectrum = getFilteredSpectrum<SampleType>(testContext.fft,
                                                    testContext.noiseBuffer,
                                                    testContext.filter);

    SECTION("Spectrum Shape") {
        for (size_t i = 0; i < FFTSize/2; ++i) {
            const Decibel<SampleType> noiseLevel    = Amplitude{testContext.noiseSpectrum[i].getAverage()};
            const Decibel<SampleType> filteredLevel = Amplitude{filterSpectrum[i].getAverage()};

            const auto outputSameOrQuieter = isSameOr<GainChange::Quieter>(noiseLevel,
                                                                           filteredLevel,
                                                                           testContext.tolerance);
            const auto differenceVeryQuiet = ResidualDecibels<SampleType>(noiseLevel,
                                                                          Decibel{SampleType{-120}})
                                             .match(filteredLevel);

            REQUIRE((outputSameOrQuieter || differenceVeryQuiet));

            if(i > 0) {
                const Decibel<SampleType> currentBinLevel  = Amplitude{filterSpectrum[i].getAverage()};
                const Decibel<SampleType> previousBinLevel = Amplitude{filterSpectrum[i-1].getAverage()};

                const auto currentBinSameOrLouder = isSameOr<GainChange::Louder>(currentBinLevel,
                                                                                 previousBinLevel,
                                                                               testContext.tolerance);
                const auto binDifferenceVeryQuiet = ResidualDecibels<SampleType>(currentBinLevel,
                                                                                 Decibel{SampleType{-120}})
                                                    .match(previousBinLevel);

                REQUIRE((currentBinSameOrLouder || binDifferenceVeryQuiet));
            }
        }
    }

    const auto cutoff = DigitalFrequency{testContext.cutoff};

    // Measure the gain difference of sin wave with the same frequency as the warped cutoff
    // input and output from the filter
    //Check that it's within half a dB of a 3dB reduction
    SECTION("Peak Level Amplitude At Cutoff") {
        REQUIRE_THAT(calculateLevelReductionAtFrequency<SampleType>(testContext.filter,
                                                                    cutoff.count(),
                                                                    testContext.sampleRate),
                     WithinDecibels(Decibel{SampleType{-3}}, Decibel{SampleType{.5}}));
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
void testBandpassResponse(FilterTestContext<Ts...>& testContext) 
{
    using SampleType = typename FilterTestContext<Ts...>::SampleType;
    constexpr auto FFTSize = FilterTestContext<Ts...>::SpectrumSize/2;

    const auto cutoff = DigitalFrequency{testContext.cutoff};

    const auto centerBinIndex = (cutoff.count() / testContext.sampleRate) * FFTSize;

    const auto filterSpectrum = getFilteredSpectrum<SampleType>(testContext.fft,
                                                    testContext.noiseBuffer,
                                                    testContext.filter);

    SECTION("Spectrum Shape") {
        for (size_t i = 0; i < FFTSize/2; ++i) {
            const Decibel<SampleType> noiseLevel    = Amplitude{testContext.noiseSpectrum[i].getAverage()};
            const Decibel<SampleType> filteredLevel = Amplitude{filterSpectrum[i].getAverage()};

            const auto outputSameOrQuieter = isSameOr<GainChange::Quieter>(noiseLevel,
                                                                           filteredLevel,
                                                                           testContext.tolerance);
            const auto differenceVeryQuiet = ResidualDecibels<SampleType>(noiseLevel,
                                                                          Decibel{SampleType{-120}})
                                             .match(filteredLevel);

            REQUIRE((outputSameOrQuieter || differenceVeryQuiet));

            if (i > 0) {
                const Decibel<SampleType> currentBinLevel  = Amplitude{filterSpectrum[i].getAverage()};
                const Decibel<SampleType> previousBinLevel = Amplitude{filterSpectrum[i - 1].getAverage()};

                const auto binDifferenceVeryQuiet = ResidualDecibels<SampleType>(currentBinLevel,
                                                                                 Decibel{SampleType{-120}})
                                                    .match(previousBinLevel);

                if (i < centerBinIndex) {
                    const auto currentBinSameOrLouder = isSameOr<GainChange::Louder>(currentBinLevel,
                                                                                     previousBinLevel,
                                                                                     testContext.tolerance);
                    REQUIRE((currentBinSameOrLouder || binDifferenceVeryQuiet));
                }
                else if (i-1 >= centerBinIndex) {
                    const auto currentBinSameOrQuieter = isSameOr<GainChange::Quieter>(currentBinLevel,
                                                                                       previousBinLevel,
                                                                                       testContext.tolerance);
                    REQUIRE((currentBinSameOrQuieter || binDifferenceVeryQuiet));
                }
            }
        }
    }

    // Measure the gain difference of sin wave with the same frequency as the warped cutoff
    // input and output from the filter
    //Check that the output is within half a dB of the input
    SECTION("Peak Level At Center Frequency") {
        REQUIRE_THAT(calculateLevelReductionAtFrequency<SampleType>(testContext.filter,
                                                                        cutoff.count(),
                                                                        testContext.sampleRate),
                     WithinDecibels(Decibel{SampleType{0}}, Decibel{SampleType{.5}}));
    }
}

//Test the spectrum shape, and level reduction at the center frequency of a bandreject filter
//TODO: Add the capability to test various q values and verify the rolloff rate
template<typename... Ts>
void testBandrejectResponse(FilterTestContext<Ts...>& testContext) 
{
    using SampleType = typename FilterTestContext<Ts...>::SampleType;
    constexpr auto FFTSize = FilterTestContext<Ts...>::SpectrumSize/2;

    const auto cutoff  = DigitalFrequency{testContext.cutoff};

    const auto centerBinIndex = (cutoff.count() / testContext.sampleRate) * FFTSize;

    const auto filterSpectrum = getFilteredSpectrum<SampleType>(testContext.fft,
                                                    testContext.noiseBuffer,
                                                    testContext.filter);

    SECTION("Spectrum Shape") {
        for (size_t i = 0; i < FFTSize/2; ++i) {
            const Decibel<SampleType> noiseLevel    = Amplitude{testContext.noiseSpectrum[i].getAverage()};
            const Decibel<SampleType> filteredLevel = Amplitude{filterSpectrum[i].getAverage()};

            const auto outputSameOrQuieter = isSameOr<GainChange::Quieter>(noiseLevel,
                                                                           filteredLevel,
                                                                           testContext.tolerance);
            const auto differenceVeryQuiet = ResidualDecibels<SampleType>(noiseLevel,
                                                                          Decibel{SampleType{-120}})
                                             .match(filteredLevel);

            REQUIRE((outputSameOrQuieter || differenceVeryQuiet));

            if (i > 0) {
                const Decibel<SampleType> currentBinLevel  = Amplitude{filterSpectrum[i].getAverage()};
                const Decibel<SampleType> previousBinLevel = Amplitude{filterSpectrum[i - 1].getAverage()};

                const auto binDifferenceVeryQuiet = ResidualDecibels<SampleType>(currentBinLevel,
                                                                                 Decibel{SampleType{-120}})
                                                    .match(previousBinLevel);

                if (i <= centerBinIndex) {
                    const auto currentBinSameOrQuieter = isSameOr<GainChange::Quieter>(currentBinLevel,
                                                                                       previousBinLevel,
                                                                                       testContext.tolerance);
                    REQUIRE((currentBinSameOrQuieter || binDifferenceVeryQuiet));
                }
                //Make sure last two bins are on the same side of the notch
                else if (i-1 >= centerBinIndex) {
                    const auto currentBinSameOrLouder = isSameOr<GainChange::Louder>(currentBinLevel,
                                                                                     previousBinLevel,
                                                                                     testContext.tolerance);
                    CHECK((currentBinSameOrLouder || binDifferenceVeryQuiet));
                }
            }
        }
    }

    // Measure the gain difference of sin wave with the same frequency as the warped cutoff
    // input and output from the filter
    // Check that the difference is lower than -48dB
    // This number can go lower if you run more iterations on the aver
    SECTION("Peak Level At Center Frequency") {
        REQUIRE(calculateLevelReductionAtFrequency<SampleType>(testContext.filter,
                                                               cutoff.count(),
                                                               testContext.sampleRate)
                < -48.0_dB);
    }
}

//Test the spectrum shape and gain of a peak filter
// TODO: check why using references in the test context for the noise stuff
//  doesn't work in just this test
template<typename... Ts>
void testAllpassResponse(FilterTestContext<Ts...>& testContext)
{
    using SampleType = typename FilterTestContext<Ts...>::SampleType;
    constexpr auto FFTSize = FilterTestContext<Ts...>::SpectrumSize/2;

    const auto nb = NoiseContext<SampleType>::getBuffer();
    const auto ns = makeNoiseSpectrum<SampleType, 1024>(nb);
//            NoiseContext<SampleType>::getSpectrum();

    const auto filteredSpectrum = getFilteredSpectrum<SampleType>(testContext.fft,
                                                                  nb,
                                                                  testContext
                                                                 .filter);

    SECTION("Spectrum Shape") {
        for (size_t i = 0; i < FFTSize/2; ++i) {
            const auto noiseLevel =
                Decibel{Amplitude{ns[i]
                                  .getAverage()}};
            const auto filteredLevel =
                Decibel{Amplitude{filteredSpectrum[i].getAverage()}};

            REQUIRE_THAT(noiseLevel,
                         WithinDecibels(filteredLevel, testContext.tolerance));
        }
    }
}

//Test the spectrum shape and gain of a peak filter
template<typename... Ts>
void testPeakResponse(FilterTestContext<Ts...>& testContext)
{
    using SampleType = typename FilterTestContext<Ts...>::SampleType;
    constexpr auto FFTSize = FilterTestContext<Ts...>::SpectrumSize/2;

    const auto cutoff  = DigitalFrequency{testContext.cutoff};

    const auto centerBinIndex = (cutoff.count() / testContext.sampleRate) * FFTSize;

    const auto filterSpectrum = getFilteredSpectrum<SampleType>(testContext.fft,
                                                    testContext.noiseBuffer,
                                                    testContext.filter);

    SECTION("Spectrum Shape") {
        for (size_t i = 0; i < FFTSize/2; ++i) {
            const Decibel<SampleType> noiseLevel    = Amplitude{testContext.noiseSpectrum[i].getAverage()};
            const Decibel<SampleType> filteredLevel = Amplitude{filterSpectrum[i].getAverage()};

            const auto outputSameOrQuieter = isSameOr<GainChange::Quieter>(noiseLevel,
                                                                           filteredLevel,
                                                                           testContext.tolerance);
            const auto differenceVeryQuiet = ResidualDecibels<SampleType>(noiseLevel,
                                                                          Decibel{SampleType{-120}})
                                             .match(filteredLevel);

            REQUIRE((outputSameOrQuieter || differenceVeryQuiet));

            if (i > 0) {
                const Decibel<SampleType> currentBinLevel  = Amplitude{filterSpectrum[i].getAverage()};
                const Decibel<SampleType> previousBinLevel = Amplitude{filterSpectrum[i - 1].getAverage()};

                const auto binDifferenceVeryQuiet = ResidualDecibels<SampleType>(currentBinLevel,
                                                                                 Decibel{SampleType{-120}})
                                                    .match(previousBinLevel);

                //TODO: rewrite this section so that negative gains pass
                const auto level1 = testContext.filterGain >= SampleType{1} ? currentBinLevel : previousBinLevel;
                const auto level2 = testContext.filterGain >= SampleType{1} ? previousBinLevel : currentBinLevel;
                if (i < centerBinIndex) {
                    const auto currentBinSameOrLouder = isSameOr<GainChange::Louder>(level1,
                                                                                     level2,
                                                                                     testContext.tolerance);
                    REQUIRE((currentBinSameOrLouder || binDifferenceVeryQuiet));
                }
                else if (i-1 >= centerBinIndex) {
                    const auto currentBinSameOrQuieter = isSameOr<GainChange::Quieter>(level1,
                                                                                       level2,
                                                                                       testContext.tolerance);
                    REQUIRE((currentBinSameOrQuieter || binDifferenceVeryQuiet));
                }
            }
        }
    }

    SECTION("Peak Level At Center Frequency") {
        const auto levelDifference = calculateLevelReductionAtFrequency<SampleType>(testContext.filter,
                                                                                    cutoff.count(),
                                                                                    testContext.sampleRate);
        REQUIRE_THAT(levelDifference, WithinDecibels(Decibel{Amplitude{testContext.filterGain}}, Decibel{ SampleType{.5}}));
    }
}

//Shelf level increase should be the sqrt of the gain

template<typename... Ts>
void testLowShelfResponse(FilterTestContext<Ts...>& testContext)
{
    using SampleType = typename FilterTestContext<Ts...>::SampleType;
    constexpr auto FFTSize = FilterTestContext<Ts...>::SpectrumSize/2;

    const auto cutoff  = DigitalFrequency{testContext.cutoff};

    const auto filterSpectrum = getFilteredSpectrum<SampleType>(testContext.fft,
                                                    testContext.noiseBuffer,
                                                    testContext.filter);

    SECTION("Spectrum Shape") {
        for (size_t i = 1; i < FFTSize; ++i) {
                //After the first bin:
                //Check that the current bin is either the same level or quieter than the previous
                //Or that the difference between them is below the threshold of hearing
                const Decibel<SampleType> currentBinLevel  = Amplitude{filterSpectrum[i].getAverage()};
                const Decibel<SampleType> previousBinLevel = Amplitude{filterSpectrum[i-1].getAverage()};

            const auto outputSameOrQuieter = isSameOr<GainChange::Quieter>(currentBinLevel,
                                                                           previousBinLevel,
                                                                           testContext.tolerance);
            const auto differenceVeryQuiet = ResidualDecibels<SampleType>(currentBinLevel,
                                                                          Decibel{SampleType{-120}})
                                             .match(previousBinLevel);

            REQUIRE((outputSameOrQuieter || differenceVeryQuiet));
        }
    }


    SECTION("Peak Level At Center Frequency") {
        if(testContext.cutoff < testContext.sampleRate/4.01) {
            const auto levelDifference = calculateLevelReductionAtFrequency<SampleType>(testContext.filter,
                                                                                        cutoff.count(),
                                                                                        testContext.sampleRate);
            REQUIRE_THAT(levelDifference,
                WithinDecibels(Decibel{ Amplitude{std::sqrt(testContext.filterGain)} }, Decibel{ SampleType{.75} }));
        }
    }
}

template<typename... Ts>
void testHighShelfResponse(FilterTestContext<Ts...>& testContext)
{
    using SampleType = typename FilterTestContext<Ts...>::SampleType;
    constexpr auto FFTSize = FilterTestContext<Ts...>::SpectrumSize/2;

    const auto cutoff  = DigitalFrequency{testContext.cutoff};

    const auto filterSpectrum = getFilteredSpectrum<SampleType>(testContext.fft,
                                                    testContext.noiseBuffer,
                                                    testContext.filter);

    SECTION("Spectrum Shape") {
        //Gain seems to become inaccurate around a quarter of the sampling rate, so only measure up to that pout
        for (size_t i = 1; i < FFTSize/2; ++i) {
                //After the first bin:
                //Check that the current bin is either the same level or quieter than the previous
                //Or that the difference between them is below the threshold of hearing
                const Decibel<SampleType> currentBinLevel  = Amplitude{filterSpectrum[i].getAverage()};
                const Decibel<SampleType> previousBinLevel = Amplitude{filterSpectrum[i-1].getAverage()};

                const auto outputSameOrLouder = isSameOr<GainChange::Louder>(currentBinLevel,
                                                                             previousBinLevel,
                                                                             testContext.tolerance);
                const auto differenceVeryQuiet = ResidualDecibels<SampleType>(currentBinLevel,
                                                                              Decibel{SampleType{-120}})
                                                 .match(previousBinLevel);

                REQUIRE((outputSameOrLouder || differenceVeryQuiet));
        }
    }


    SECTION("Peak Level At Center Frequency") {
        //Shelf gain becomes unstable at about 1/4th the sampling rate-
        // let's only run the test inside the expected bounds
        if(testContext.cutoff < testContext.sampleRate/4.0) {
            const auto levelDifference = calculateLevelReductionAtFrequency<SampleType>(testContext.filter,
                                                                                        cutoff.count(),
                                                                                        testContext.sampleRate);
            const Decibel<SampleType> referenceGain = Amplitude{std::sqrt(testContext.filterGain)};
            REQUIRE_THAT(levelDifference, WithinDecibels(referenceGain, Decibel{SampleType{.75}}));
        }
    }
}
