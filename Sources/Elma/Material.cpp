#include "Material.hpp"
#include "Intersection.hpp"

namespace elma {

inline Vector3 SampleCosHemisphere(const Vector2& rnd_param)
{
    const auto phi = kTwoPi * rnd_param[0];
    const auto tmp = std::sqrt(std::clamp(1 - rnd_param[1], Real(0), Real(1)));
    return Vector3{std::cos(phi) * tmp, std::sin(phi) * tmp, std::sqrt(std::clamp(rnd_param[1], Real(0), Real(1)))};
}

struct EvalOp
{
    Spectrum operator()(const Lambertian& bsdf) const;
    Spectrum operator()(const RoughPlastic& bsdf) const;
    Spectrum operator()(const RoughDielectric& bsdf) const;
    Spectrum operator()(const DisneyDiffuse& bsdf) const;
    Spectrum operator()(const DisneyMetal& bsdf) const;
    Spectrum operator()(const DisneyGlass& bsdf) const;
    Spectrum operator()(const DisneyClearcoat& bsdf) const;
    Spectrum operator()(const DisneySheen& bsdf) const;
    Spectrum operator()(const DisneyBSDF& bsdf) const;

    const Vector3& dirIn;
    const Vector3& dirOut;
    const PathVertex& vertex;
    const TexturePool& texture_pool;
    const TransportDirection& dir;
};

struct PdfSampleBSDFOp
{
    Real operator()(const Lambertian& bsdf) const;
    Real operator()(const RoughPlastic& bsdf) const;
    Real operator()(const RoughDielectric& bsdf) const;
    Real operator()(const DisneyDiffuse& bsdf) const;
    Real operator()(const DisneyMetal& bsdf) const;
    Real operator()(const DisneyGlass& bsdf) const;
    Real operator()(const DisneyClearcoat& bsdf) const;
    Real operator()(const DisneySheen& bsdf) const;
    Real operator()(const DisneyBSDF& bsdf) const;

    const Vector3& dir_in;
    const Vector3& dir_out;
    const PathVertex& vertex;
    const TexturePool& texture_pool;
    const TransportDirection& dir;
};

struct SampleBSDFOp
{
    std::optional<BSDFSampleRecord> operator()(const Lambertian& bsdf) const;
    std::optional<BSDFSampleRecord> operator()(const RoughPlastic& bsdf) const;
    std::optional<BSDFSampleRecord> operator()(const RoughDielectric& bsdf) const;
    std::optional<BSDFSampleRecord> operator()(const DisneyDiffuse& bsdf) const;
    std::optional<BSDFSampleRecord> operator()(const DisneyMetal& bsdf) const;
    std::optional<BSDFSampleRecord> operator()(const DisneyGlass& bsdf) const;
    std::optional<BSDFSampleRecord> operator()(const DisneyClearcoat& bsdf) const;
    std::optional<BSDFSampleRecord> operator()(const DisneySheen& bsdf) const;
    std::optional<BSDFSampleRecord> operator()(const DisneyBSDF& bsdf) const;

    const Vector3& dirIn;
    const PathVertex& vertex;
    const TexturePool& texturePool;
    const Vector2& rndParamUV;
    const Real& rndParamW;
    const TransportDirection& dir;
};

struct get_texture_op
{
    TextureSpectrum operator()(const Lambertian& bsdf) const;
    TextureSpectrum operator()(const RoughPlastic& bsdf) const;
    TextureSpectrum operator()(const RoughDielectric& bsdf) const;
    TextureSpectrum operator()(const DisneyDiffuse& bsdf) const;
    TextureSpectrum operator()(const DisneyMetal& bsdf) const;
    TextureSpectrum operator()(const DisneyGlass& bsdf) const;
    TextureSpectrum operator()(const DisneyClearcoat& bsdf) const;
    TextureSpectrum operator()(const DisneySheen& bsdf) const;
    TextureSpectrum operator()(const DisneyBSDF& bsdf) const;
};

#include "Materials/Lambertian.inl"
#include "Materials/RoughPlastic.inl"
#include "Materials/RoughDielectric.inl"
#include "Materials/DisneyDiffuse.inl"
#include "Materials/DisneyMetal.inl"
#include "Materials/DisneyGlass.inl"
#include "Materials/DisneyClearcoat.inl"
#include "Materials/DisneySheen.inl"
#include "Materials/DisneyBSDF.inl"

Spectrum Eval(const Material& material,
              const Vector3& dir_in,
              const Vector3& dir_out,
              const PathVertex& vertex,
              const TexturePool& texture_pool,
              TransportDirection dir)
{
    return std::visit(EvalOp{dir_in, dir_out, vertex, texture_pool, dir}, material);
}

std::optional<BSDFSampleRecord> SampleBSDF(const Material& material,
                                           const Vector3& dir_in,
                                           const PathVertex& vertex,
                                           const TexturePool& texture_pool,
                                           const Vector2& rnd_param_uv,
                                           const Real& rnd_param_w,
                                           TransportDirection dir)
{
    return std::visit(SampleBSDFOp{dir_in, vertex, texture_pool, rnd_param_uv, rnd_param_w, dir}, material);
}

Real PdfSampleBSDF(const Material& material,
                   const Vector3& dir_in,
                   const Vector3& dir_out,
                   const PathVertex& vertex,
                   const TexturePool& texture_pool,
                   TransportDirection dir)
{
    return std::visit(PdfSampleBSDFOp{dir_in, dir_out, vertex, texture_pool, dir}, material);
}

TextureSpectrum GetTexture(const Material& material)
{
    return std::visit(get_texture_op{}, material);
}

} // namespace elma