#include "../Microfacet.hpp"

Spectrum EvalOp::operator()(const DisneyMetal& bsdf) const
{
    if (Dot(vertex.normal, dirIn) < 0 || Dot(vertex.normal, dirOut) < 0) {
        // No light below the surface
        return MakeZeroSpectrum();
    }
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dirIn) < 0) {
        frame = -frame;
    }

    // clang-format off
    const auto baseColor = Eval(bsdf.baseColor, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto anisotropic = Eval(bsdf.anisotropic, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto roughness = Clamp(Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texturePool), Real(0.01), Real(1));
    // clang-format on

    const auto aspect = std::sqrt(kOne<Real> - Real(0.9) * anisotropic);
    const auto ax     = std::max(Real(0.0001), roughness * roughness / aspect);
    const auto ay     = std::max(Real(0.0001), roughness * roughness * aspect);

    const auto h = Normalize(dirIn + dirOut);

    const auto F = elma::SchlickFresnel(baseColor, AbsDot(h, dirOut));
    const auto D = elma::GGXAnisotropic(ToLocal(frame, h), ax, ay);
    const auto G = elma::SmithMaskingGTR2Anisotropic(ToLocal(frame, dirIn), ax, ay) *
                   elma::SmithMaskingGTR2Anisotropic(ToLocal(frame, dirOut), ax, ay);

    return MakeConstSpectrum(0.25) * F * D * G / AbsDot(frame.n, dirIn);
}

Spectrum EvalOp::operator()(const DisneyMetal& bsdf, Real specular, Real metallic, Real specularTint, Real eta) const
{
    if (Dot(vertex.normal, dirIn) < 0 || Dot(vertex.normal, dirOut) < 0) {
        // No light below the surface
        return MakeZeroSpectrum();
    }
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dirIn) < 0) {
        frame = -frame;
    }

    // clang-format off
    const auto baseColor = Eval(bsdf.baseColor, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto anisotropic = Eval(bsdf.anisotropic, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto roughness = Clamp(Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texturePool), Real(0.01), Real(1));
    // clang-format on

    const auto aspect = std::sqrt(kOne<Real> - Real(0.9) * anisotropic);
    const auto ax     = std::max(Real(0.0001), roughness * roughness / aspect);
    const auto ay     = std::max(Real(0.0001), roughness * roughness * aspect);

    const auto h = Normalize(dirIn + dirOut);

    const auto Ks     = Lerp(MakeConstSpectrum(1), elma::CalculateTint(baseColor), specularTint);
    const auto R0_eta = std::pow((eta - 1.0) / (eta + 1.0), 2);
    const auto C      = specular * R0_eta * (1.0 - metallic) * Ks + metallic * baseColor;
    const auto F      = elma::SchlickFresnel(C, Dot(h, dirOut));

    const auto D = elma::GGXAnisotropic(ToLocal(frame, h), ax, ay);
    const auto G = elma::SmithMaskingGTR2Anisotropic(ToLocal(frame, dirIn), ax, ay) *
                   elma::SmithMaskingGTR2Anisotropic(ToLocal(frame, dirOut), ax, ay);

    return MakeConstSpectrum(0.25) * F * D * G / AbsDot(frame.n, dirIn);
}

Real PdfSampleBSDFOp::operator()(const DisneyMetal& bsdf) const
{
    if (Dot(vertex.normal, dirIn) < 0 || Dot(vertex.normal, dirOut) < 0) {
        // No light below the surface
        return 0;
    }
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dirIn) < 0) {
        frame = -frame;
    }

    // clang-format off
    const auto anisotropic = Eval(bsdf.anisotropic, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto roughness = Clamp(Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texturePool), Real(0.01), Real(1));
    // clang-format on

    const auto aspect = std::sqrt(kOne<Real> - Real(0.9) * anisotropic);
    const auto ax     = std::max(Real(0.0001), roughness * roughness / aspect);
    const auto ay     = std::max(Real(0.0001), roughness * roughness * aspect);

    const auto h = Normalize(dirIn + dirOut);

    const auto D = elma::GGXAnisotropic(ToLocal(frame, h), ax, ay);
    const auto G = elma::SmithMaskingGTR2Anisotropic(ToLocal(frame, dirIn), ax, ay) *
                   elma::SmithMaskingGTR2Anisotropic(ToLocal(frame, dirOut), ax, ay);

    return Real(0.25) * D * G / AbsDot(frame.n, dirIn);
}

std::optional<BSDFSampleRecord> SampleBSDFOp::operator()(const DisneyMetal& bsdf) const
{
    if (Dot(vertex.normal, dirIn) < 0) {
        // No light below the surface
        return {};
    }
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dirIn) < 0) {
        frame = -frame;
    }

    // clang-format off
    const auto anisotropic = Eval(bsdf.anisotropic, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto roughness = Clamp(Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texturePool), Real(0.01), Real(1));
    // clang-format on

    const auto aspect = std::sqrt(kOne<Real> - Real(0.9) * anisotropic);
    const auto ax     = std::max(Real(0.0001), roughness * roughness / aspect);
    const auto ay     = std::max(Real(0.0001), roughness * roughness * aspect);

    const auto localNormal = elma::SampleVisibleNormalsAnisotropic(ToLocal(frame, dirIn), ax, ay, rndParamUV);

    const auto h         = ToWorld(frame.n, localNormal);
    const auto reflected = Reflect(dirIn, h);

    return BSDFSampleRecord{reflected, Real(1) /* eta */, roughness /* roughness */};
}

TextureSpectrum get_texture_op::operator()(const DisneyMetal& bsdf) const
{
    return bsdf.baseColor;
}
