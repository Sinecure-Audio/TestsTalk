#pragma once

#include <array>

//A simple class for taking the running average of a stream of numbers
//Useful when you don't know what the size of the data set will be
template <typename Type>
struct CumulativeAverage
{
    constexpr CumulativeAverage() = default;

    auto updateAverage(const Type& newValue) noexcept {
        return mean += (newValue-mean)/++counter;
    }

    constexpr const auto& getAverage() const noexcept {return mean;}

    auto reset() noexcept {
        counter = 0;
        mean = 0;
    }

private:
    size_t counter{0};
    Type mean{0};
};

//A struct that measures the peak level of an incoming stream over time
template <typename Type>
struct PeakDetector {
    void reset() noexcept {
        peakValue = 0;
    }

    void updatePeak(Type input) noexcept {
        peakValue = std::max(input, peakValue);
    }

    auto getPeak() const noexcept {
        return peakValue;
    }

private:
    Type peakValue{0};
};

//A struct that measures the rms level of an incoming stream over time
template <typename Type>
struct RMSAverage
{
    constexpr RMSAverage() = default;

    void setRMSLength(double sampleRate) noexcept {
        buffer.resize(std::ceil(sampleRate*.3));
    }

    auto getRMS(Type input) noexcept {
        const auto newValue = input*input;
        const auto nextIndex = wrapIndex(++index);
        runningTotal -= buffer[nextIndex];
        runningTotal += newValue;

        index = nextIndex;
        buffer[index] = newValue;
        return std::sqrt (runningTotal);
    }

    auto wrapIndex(size_t input) noexcept {
        return input%(buffer.size()-1);
    }

    auto reset() noexcept {
        std::fill(buffer.begin(), buffer.end(), Type{0});
        index = 0;
    }

private:
    std::vector<Type> buffer{};
    size_t index{0};
    Type runningTotal{};
};

//A struct that keeps a running average of each element in a collection
//Useful to average the level of an fft's bins over multiple frames, for example
template<typename FloatType, size_t Size>
class BufferAverager
{
public:
    template<typename Collection>
    auto perform(const Collection& collection) noexcept {
        for(auto i = 0; i < collection.size(); ++i)
            buffer[i].updateAverage(collection[i]);
    }

    template<typename T>
    auto perform(const std::array<T, Size>& collection) noexcept {
        for(auto i = 0; i < collection.size(); ++i)
            buffer[i].updateAverage(collection[i]);
    }

    void reset() noexcept {
        for(auto&& average : buffer)
            average.reset();
    }

    constexpr const auto& getBuffer() const noexcept { return buffer; }

private:
    std::array<CumulativeAverage<FloatType>, Size> buffer{};
};