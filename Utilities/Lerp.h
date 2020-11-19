#pragma once

template<typename T>
T lerp(const T& a, const T& b, const T& x) noexcept {
    return a+(b-a)*x;
}