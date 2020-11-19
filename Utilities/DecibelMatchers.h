#pragma once

#include <sstream>

#include <catch2/catch.hpp>

#include "../submodules/Units/include/Units.h"


// A catch style matcher
// This checks that the difference between two levels is less than or equal to a give threshold
// i.e. you might check the level between two signals, seeing if the difference is less than -120dB (meaning it can't be heard)
template<typename T>
struct ResidualDecibels : public Catch::MatcherBase<Decibel<T>> {
    explicit ResidualDecibels(const Amplitude<T>& target, const Decibel<T>& threshold)
            : target{ Decibel<T>{target} }, threshold{threshold} {}

//subtract the decibel value of the target from the decibel value of the margin, and convert the result to an amplitude
    bool match(const Decibel<T>& other) const override {
        return other-target <= threshold;
    }

    std::string describe() const override {
        std::ostringstream ss;
        ss << "minus " << target << " is less than or equal to " << threshold;
        return ss.str();
    }
private:
    const Decibel<T> threshold, target, result{};
};

// A catch style matcher
// This checks that two levels are within a certain threshold of each other
// i.e. you might check the level of two signals, seeing if they're within 2dB of each other
template<typename T>
struct WithinDecibels : public Catch::MatcherBase<Decibel<T>> {
    explicit WithinDecibels(const Amplitude<T>& target, const Decibel<T>& threshold)
            : target{ Decibel<T>{target} }, threshold{threshold} {}

//subtract the decibel value of the target from the decibel value of the margin, and convert the result to an amplitude
    bool match(const Decibel<T>& other) const override {
        return std::abs(other.count()-target.count()) < std::abs(threshold.count());
    }

    std::string describe() const override {
        std::ostringstream ss;
        ss << "is within " << threshold << " of " << target;
        return ss.str();
    }
private:
    const Decibel<T> threshold, target;
};


//CTAD guides for the matchers so you don't need to explicitly specialize them
template<typename T>
WithinDecibels(T, Decibel<T>) -> WithinDecibels<T>;
template<typename T>
WithinDecibels(Amplitude<T>, Decibel<T>) -> WithinDecibels<T>;
template<typename T>
WithinDecibels(Decibel<T>, Decibel<T>) -> WithinDecibels<T>;

template<typename T>
ResidualDecibels(T, Decibel<T>) -> ResidualDecibels<T>;
template<typename T>
ResidualDecibels(Amplitude<T>, Decibel<T>) -> ResidualDecibels<T>;
template<typename T>
ResidualDecibels(Decibel<T>, Decibel<T>) -> ResidualDecibels<T>;