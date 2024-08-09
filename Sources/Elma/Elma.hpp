#pragma once

// CMake insert NDEBUG when building with RelWithDebInfo
// This is an ugly hack to undo that...
#undef NDEBUG

#include <cassert>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <algorithm>

namespace elma {

// We use double for most of our computation.
// Rendering is usually done in single precision Reals.
// However, lajolla is a educational renderer with does not
// put emphasis on the absolute performance.
// We choose double so that we do not need to worry about
// numerical accuracy as much when we render.
// Switching to floating point computation is easy --
// just set Real = float.
using Real = double;

// Lots of PIs!
const Real kPi         = Real(3.14159265358979323846);
const Real kInvPi      = Real(1.0) / kPi;
const Real kTwoPi      = Real(2.0) * kPi;
const Real kInvTwoPi   = Real(1.0) / kTwoPi;
const Real kFourPi     = Real(4.0) * kPi;
const Real kInvFourPi  = Real(1.0) / kFourPi;
const Real kPiOverTwo  = Real(0.5) * kPi;
const Real kPiOverFour = Real(0.25) * kPi;

template<typename T> inline T Infinity()
{
    return std::numeric_limits<T>::infinity();
}

namespace fs = std::filesystem;

inline std::string ToLowercase(const std::string& s)
{
    std::string out = s;
    std::transform(s.begin(), s.end(), out.begin(), ::tolower);
    return out;
}

inline int Modulo(int a, int b)
{
    auto r = a % b;
    return (r < 0) ? r + b : r;
}

inline float Modulo(float a, float b)
{
    float r = std::fmodf(a, b);
    return (r < 0.0f) ? r + b : r;
}

inline double Modulo(double a, double b)
{
    double r = std::fmod(a, b);
    return (r < 0.0) ? r + b : r;
}

template<typename T> inline T Max(const T& a, const T& b)
{
    return a > b ? a : b;
}

template<typename T> inline T Min(const T& a, const T& b)
{
    return a < b ? a : b;
}

inline Real Radians(const Real deg)
{
    return (kPi / Real(180)) * deg;
}

inline Real Degrees(const Real rad)
{
    return (Real(180) / kPi) * rad;
}

} // namespace elma