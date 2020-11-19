#pragma once

#include <cstdlib>

//Generates a random number inside of the supplied bounds, inclusive
template<typename T>
T getBoundedRandom(const T& bound1, const T& bound2) {
    return std::abs(bound1-bound2)*(static_cast<T>(std::rand())/static_cast<T>(RAND_MAX))+std::min(bound1, bound2);
}