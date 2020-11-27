#pragma once

#include "../submodules/Units/include/Units.h"

//Set the number of times each test is run
constexpr size_t numIterations = 100000;

//A value we use to determine whether the difference between two signals is small enough to be acceptable
template<typename T>
constexpr auto residualThreshold = Decibel<T>{T{-120}};
template<typename T>
constexpr auto tolerance = Decibel<T>{T{.5}};