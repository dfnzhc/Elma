#pragma once

#include "PhaseFunction.hpp"
#include "Spectrum.hpp"
#include "Volume.hpp"
#include <variant>

namespace elma {
struct Scene;

struct MediumBase
{
    PhaseFunction phaseFunction;
};

struct HomogeneousMedium : public MediumBase
{
    Spectrum sigmaA, sigmaS;
};

struct HeterogeneousMedium : public MediumBase
{
    VolumeSpectrum albedo, density;
};

using Medium = std::variant<HomogeneousMedium, HeterogeneousMedium>;

/// the maximum of sigma_t = sigma_s + sigma_a over the whole space
Spectrum GetMajorant(const Medium& medium, const Ray& ray);
Spectrum GetSigmaS(const Medium& medium, const Vector3& p);
Spectrum GetSigmaA(const Medium& medium, const Vector3& p);

inline PhaseFunction GetPhaseFunction(const Medium& medium)
{
    return std::visit([&](const auto& m) { return m.phaseFunction; }, medium);
}

} // namespace elma