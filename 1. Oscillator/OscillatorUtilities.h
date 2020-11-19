#pragma once

#include <catch2/catch.hpp>

// checks whether a pair of inputs are relatively close to a set of bounds
// This is useful for testing discontinuous signals-
// if the pair of ins are on either side of the discontinuity
// (specified by the two bounds values, which should be at each end of the discontinuity)
// Then this will return true
template<typename T>
auto closeToEdge(const std::pair<T, T>& ins, const T& lowerBound, const T& upperBound) noexcept {
    const auto nextToLower = Catch::WithinAbs(lowerBound, .000001).match(ins.first);
    const auto nextToUpper = Catch::WithinAbs(upperBound, .000001).match(ins.second);

    const auto nextToLower1 = Catch::WithinAbs(lowerBound, .000001).match(ins.second);
    const auto nextToUpper1 = Catch::WithinAbs(upperBound, .000001).match(ins.first);
    return (nextToLower && nextToUpper) || (nextToLower1 && nextToUpper1);
}