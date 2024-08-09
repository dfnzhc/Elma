#pragma once

namespace elma {
// The simplest volumetric renderer:
// single absorption only homogeneous volume
// only handle directly visible light sources
Spectrum VolPathTracing1(const Scene& scene,
                         int x,
                         int y, /* pixel coordinates */
                         Pcg32State& rng)
{
    // Homework 2: implememt this!
    return MakeZeroSpectrum();
}

// The second simplest volumetric renderer:
// single monochromatic homogeneous volume with single scattering,
// no need to handle surface lighting, only directly visible light source
Spectrum VolPathTracing2(const Scene& scene,
                         int x,
                         int y, /* pixel coordinates */
                         Pcg32State& rng)
{
    // Homework 2: implememt this!
    return MakeZeroSpectrum();
}

// The third volumetric renderer (not so simple anymore):
// multiple monochromatic homogeneous volumes with multiple scattering
// no need to handle surface lighting, only directly visible light source
Spectrum VolPathTracing3(const Scene& scene,
                         int x,
                         int y, /* pixel coordinates */
                         Pcg32State& rng)
{
    // Homework 2: implememt this!
    return MakeZeroSpectrum();
}

// The fourth volumetric renderer:
// multiple monochromatic homogeneous volumes with multiple scattering
// with MIS between next event estimation and phase function sampling
// still no surface lighting
Spectrum VolPathTracing4(const Scene& scene,
                         int x,
                         int y, /* pixel coordinates */
                         Pcg32State& rng)
{
    // Homework 2: implememt this!
    return MakeZeroSpectrum();
}

// The fifth volumetric renderer:
// multiple monochromatic homogeneous volumes with multiple scattering
// with MIS between next event estimation and phase function sampling
// with surface lighting
Spectrum VolPathTracing5(const Scene& scene,
                         int x,
                         int y, /* pixel coordinates */
                         Pcg32State& rng)
{
    // Homework 2: implememt this!
    return MakeZeroSpectrum();
}

// The final volumetric renderer:
// multiple chromatic heterogeneous volumes with multiple scattering
// with MIS between next event estimation and phase function sampling
// with surface lighting
Spectrum VolPathTracing(const Scene& scene,
                        int x,
                        int y, /* pixel coordinates */
                        Pcg32State& rng)
{
    // Homework 2: implememt this!
    return MakeZeroSpectrum();
}

} // namespace elma