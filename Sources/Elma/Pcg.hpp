#pragma once

#include "Elma.hpp"

namespace elma {
// Lajolla uses a random number generator called PCG https://www.pcg-random.org/
// which is a very lightweight random number generator based on a simple
// postprocessing of a standard linear congruent generators.
// PCG has generally good statistical properties and is much cheaper to compute compared
// to more advanced RNG e.g., Merserner Twister.
// Highly recommend Melissa O'Neill's talk about PCG https://www.youtube.com/watch?v=45Oet5qjlms

// A crucial feature of PCG is that it allows multiple "streams":
// given a seed, we can initialize many different streams of RNGs
// that have different independent random numbers.

struct Pcg32State
{
    uint64_t state;
    uint64_t inc;
};

// http://www.pcg-random.org/download.html
inline uint32_t NextPcg32(Pcg32State& rng)
{
    uint64_t oldstate = rng.state;
    // Advance internal state
    rng.state = oldstate * 6'364'136'223'846'793'005ULL + (rng.inc | 1);
    // Calculate output function (XSH RR), uses old state for max ILP
    uint32_t xorshifted = uint32_t(((oldstate >> 18u) ^ oldstate) >> 27u);
    uint32_t rot        = uint32_t(oldstate >> 59u);
    return uint32_t((xorshifted >> rot) | (xorshifted << ((-rot) & 31)));
}

// https://github.com/wjakob/pcg32/blob/master/pcg32.h#L47
inline Pcg32State InitPcg32(uint64_t stream_id = 1, uint64_t seed = 0x31e241f862a1fb5eULL)
{
    Pcg32State s;
    s.state = 0U;
    s.inc   = (stream_id << 1u) | 1u;
    NextPcg32(s);
    s.state += seed;
    NextPcg32(s);
    return s;
}

template<typename T> T NextPcg32Real(Pcg32State& rng)
{
    return T(0);
}

// https://github.com/wjakob/pcg32/blob/master/pcg32.h
template<> float NextPcg32Real(Pcg32State& rng)
{
    union
    {
        uint32_t u;
        float f;
    } x;

    x.u = (NextPcg32(rng) >> 9) | 0x3f800000u;
    return x.f - 1.0f;
}

// https://github.com/wjakob/pcg32/blob/master/pcg32.h
template<> double NextPcg32Real(Pcg32State& rng)
{
    union
    {
        uint64_t u;
        double d;
    } x{};

    x.u = ((uint64_t)NextPcg32(rng) << 20) | 0x3ff0000000000000ULL;
    return x.d - 1.0;
}

static inline uint64_t wyhash64Stateless(uint64_t* seed)
{
    *seed += UINT64_C(0x60bee2bee120fc15);
    __uint128_t tmp;
    tmp         = (__uint128_t)*seed * UINT64_C(0xa3b195354a39b70d);
    uint64_t m1 = (tmp >> 64) ^ tmp;
    tmp         = (__uint128_t)m1 * UINT64_C(0x1b03738712fad5c9);
    uint64_t m2 = (tmp >> 64) ^ tmp;
    return m2;
}

static inline uint64_t wyhash64(uint64_t val)
{
    return wyhash64Stateless(&val);
}

static inline uint32_t wyhash64Cast32(uint32_t val)
{
    return (uint32_t)wyhash64(uint64_t(val));
}

} // namespace elma