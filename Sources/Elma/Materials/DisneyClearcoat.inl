#include "../Microfacet.hpp"

Spectrum EvalOp::operator()(const DisneyClearcoat& bsdf) const
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

    const auto gloss = Eval(bsdf.clearcoatGloss, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto h     = Normalize(dirIn + dirOut);

    const auto F0 = Real(0.04);
    const auto F  = elma::SchlickFresnel(F0, AbsDot(h, dirOut));

    const auto D = elma::GTR1(AbsDot(frame.n, h), Lerp(Real(0.1), Real(0.001), gloss));
    const auto G = elma::SmithMaskingGTR2(ToLocal(frame, dirIn), Real(0.25)) *
                   elma::SmithMaskingGTR2(ToLocal(frame, dirOut), Real(0.25));

    return MakeConstSpectrum(0.25) * F * D * G / AbsDot(frame.n, dirIn);
}

Real PdfSampleBSDFOp::operator()(const DisneyClearcoat& bsdf) const
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

    const auto gloss = Eval(bsdf.clearcoatGloss, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto h     = Normalize(dirIn + dirOut);

    const auto D = elma::GTR1(AbsDot(frame.n, h), Lerp(Real(0.1), Real(0.001), gloss));

    return Real(0.25) * D * AbsDot(frame.n, h) / AbsDot(h, dirOut);
}

std::optional<BSDFSampleRecord> SampleBSDFOp::operator()(const DisneyClearcoat& bsdf) const
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

    const auto gloss  = Eval(bsdf.clearcoatGloss, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto alpha  = Lerp(Real(0.1), Real(0.001), gloss);
    const auto alpha2 = alpha * alpha;

    const auto cosTheta = std::sqrt((kOne<Real> - std::pow(alpha2, Real(1) - rndParamUV.x)) / (kOne<Real> - alpha2));
    const auto sinTheta = std::sqrt(kOne<Real> - cosTheta * cosTheta);
    const auto phi      = kTwoPi * rndParamUV.y;

    const auto localNormal = Vector3{sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta};

    const auto h         = ToWorld(frame.n, localNormal);
    const auto reflected = Reflect(dirIn, h);

    return BSDFSampleRecord{reflected, Real(1.5) /* eta */, Real(0.25) /* roughness */};
}

TextureSpectrum get_texture_op::operator()(const DisneyClearcoat& bsdf) const
{
    return MakeConstantSpectrumTexture(MakeZeroSpectrum());
}
