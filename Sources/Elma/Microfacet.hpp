#pragma once

#include "Elma.hpp"
#include "Spectrum.hpp"

namespace elma {

inline Real SchlickWeight(Real cos_theta)
{
    return std::pow(Max(1 - cos_theta, Real(0)), Real(5));
}

template<typename T> inline T SchlickFresnel(const T& F0, Real cos_theta)
{
    return F0 + (Real(1) - F0) * SchlickWeight(cos_theta);
}

inline Real FresnelDielectric(Real n_dot_i, Real n_dot_t, Real eta)
{
    assert(n_dot_i >= 0 && n_dot_t >= 0 && eta > 0);
    Real rs = (n_dot_i - eta * n_dot_t) / (n_dot_i + eta * n_dot_t);
    Real rp = (eta * n_dot_i - n_dot_t) / (eta * n_dot_i + n_dot_t);
    Real F  = (rs * rs + rp * rp) / 2;
    return F;
}

inline Real FresnelDielectric(Real n_dot_i, Real eta)
{
    assert(eta > 0);
    Real n_dot_t_sq = 1 - (1 - n_dot_i * n_dot_i) / (eta * eta);
    if (n_dot_t_sq < 0) {
        // total internal reflection
        return 1;
    }
    Real n_dot_t = std::sqrt(n_dot_t_sq);
    return FresnelDielectric(std::abs(n_dot_i), n_dot_t, eta);
}

inline Real GTR1(Real n_dot_h, Real roughness)
{
    const auto alpha = roughness * roughness;
    const auto a2    = alpha * alpha;
    return (a2 - kOne<Real>) / (kPi * std::log2(a2) * (kOne<Real> + (a2 - kOne<Real>)*n_dot_h * n_dot_h));
}

inline Real SmithGGXG1(const Vector3& v_local, Real roughness)
{
    const auto alpha    = roughness * roughness;
    const auto a2       = alpha * alpha;
    const auto absDotNV = AbsCosTheta(v_local);
    return kTwo<Real> / (kOne<Real> + std::sqrt(a2 + (kOne<Real> - a2) * absDotNV * absDotNV));
}

inline Real GTR2(Real n_dot_h, Real roughness)
{
    Real alpha = roughness * roughness;
    Real a2    = alpha * alpha;
    Real t     = 1 + (a2 - 1) * n_dot_h * n_dot_h;
    return a2 / (kPi * t * t);
}

inline Real GGX(Real n_dot_h, Real roughness)
{
    return GTR2(n_dot_h, roughness);
}

inline Real SmithMaskingGTR2(const Vector3& v_local, Real roughness)
{
    Real alpha  = roughness * roughness;
    Real a2     = alpha * alpha;
    Vector3 v2  = v_local * v_local;
    Real Lambda = (-1 + std::sqrt(1 + (v2.x * a2 + v2.y * a2) / v2.z)) / 2;
    return 1 / (1 + Lambda);
}

inline Spectrum CalculateTint(const Spectrum& baseColor)
{
    const auto lum = Luminance(baseColor);
    return (lum > 0.0f) ? baseColor * Real(1.0f / lum) : MakeConstSpectrum(Real(1));
}

inline Vector3 SampleVisibleNormals(const Vector3& local_dir_in, Real alpha, const Vector2& rnd_param)
{
    // The incoming direction is in the "ellipsodial configuration" in Heitz's paper
    if (local_dir_in.z < 0) {
        // Ensure the input is on top of the surface.
        return -SampleVisibleNormals(-local_dir_in, alpha, rnd_param);
    }

    // Transform the incoming direction to the "hemisphere configuration".
    Vector3 hemi_dir_in = Normalize(Vector3{alpha * local_dir_in.x, alpha * local_dir_in.y, local_dir_in.z});

    // Parameterization of the projected area of a hemisphere.
    // First, sample a disk.
    Real r   = std::sqrt(rnd_param.x);
    Real phi = 2 * kPi * rnd_param.y;
    Real t1  = r * std::cos(phi);
    Real t2  = r * std::sin(phi);
    // Vertically scale the position of a sample to account for the projection.
    Real s = (1 + hemi_dir_in.z) / 2;
    t2     = (1 - s) * std::sqrt(1 - t1 * t1) + s * t2;
    // Point in the disk space
    Vector3 disk_N{t1, t2, std::sqrt(Max(Real(0), 1 - t1 * t1 - t2 * t2))};

    // Reprojection onto hemisphere -- we get our sampled normal in hemisphere space.
    Frame hemi_frame(hemi_dir_in);
    Vector3 hemi_N = ToWorld(hemi_frame, disk_N);

    // Transforming the normal back to the ellipsoid configuration
    return Normalize(Vector3{alpha * hemi_N.x, alpha * hemi_N.y, Max(Real(0), hemi_N.z)});
}
} // namespace elma
