#pragma once

#include <catch2/catch.hpp>

template<typename T>
const auto getSmallestAccurateValue() {
    if constexpr (std::is_integral_v<T>)
        return 1;
    else
        return std::nextafter(std::pow(T{1}, -std::numeric_limits<T>::digits10), T{1});
}

// checks whether a pair of inputs are relatively close to a set of bounds
// This is useful for testing discontinuous signals-
// if the pair of ins are on either side of the discontinuity
// (specified by the two bounds values, which should be at each end of the discontinuity)
// Then this will return true
template<typename T>
auto closeToEdge(const std::pair<T, T>& ins, const T& lowerBound, const T& upperBound, const T& tolerance = getSmallestAccurateValue<T>()) noexcept {
    const auto distance1 = std::min(std::abs(ins.first-lowerBound), std::abs(ins.first-upperBound));
    const auto distance2 = std::min(std::abs(ins.first-lowerBound), std::abs(ins.first-upperBound));

    return distance1+distance2 < tolerance;

//    const auto nextToLower = Catch::WithinAbs(lowerBound, .000001).match(ins.first);
//    const auto nextToUpper = Catch::WithinAbs(upperBound, .000001).match(ins.second);

//    const auto nextToLower1 = Catch::WithinAbs(lowerBound, .000001).match(ins.second);
//    const auto nextToUpper1 = Catch::WithinAbs(upperBound, .000001).match(ins.first);
//    return (nextToLower && nextToUpper) || (nextToLower1 && nextToUpper1);
}