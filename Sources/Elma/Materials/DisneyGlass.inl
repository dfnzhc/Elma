#include "../Microfacet.hpp"

Spectrum EvalOp::operator()(const DisneyGlass& bsdf) const
{
    bool reflect = Dot(vertex.normal, dirIn) * Dot(vertex.normal, dirOut) > 0;
    Frame frame  = vertex.shadingFrame;
    if (Dot(frame.n, dirIn) * Dot(vertex.normal, dirIn) < 0) {
        frame = -frame;
    }

    const auto eta = Dot(vertex.normal, dirIn) > 0 ? bsdf.eta : 1 / bsdf.eta;

    // clang-format off
    const auto baseColor  = Eval(bsdf.baseColor, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto anisotropic = Eval(bsdf.anisotropic, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto roughness = Clamp(Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texturePool), Real(0.01), Real(1));
    // clang-format on

    const auto aspect = std::sqrt(kOne<Real> - Real(0.9) * anisotropic);
    const auto ax     = std::max(Real(0.0001), roughness * roughness / aspect);
    const auto ay     = std::max(Real(0.0001), roughness * roughness * aspect);

    auto h = Normalize(dirIn + dirOut);
    if (!reflect) {
        h = Normalize(dirIn + dirOut * eta);
    }

    if (Dot(h, frame.n) < 0) {
        h = -h;
    }

    const auto h_dot_in  = Dot(h, dirIn);
    const auto h_dot_out = Dot(h, dirOut);
    const auto n_dot_in  = AbsDot(frame.n, dirIn);

    const auto F = elma::FresnelDielectric(h_dot_in, eta);
    const auto D = elma::GGXAnisotropic(ToLocal(frame.n, h), ax, ay);
    const auto G = elma::SmithMaskingGTR2Anisotropic(ToLocal(frame, dirIn), ax, ay) *
                   elma::SmithMaskingGTR2Anisotropic(ToLocal(frame, dirOut), ax, ay);

    if (reflect) {
        return Real(0.25) * baseColor * (F * D * G) / n_dot_in;
    }

    const auto etaFactor = dir == TransportDirection::TO_LIGHT ? (1 / (eta * eta)) : 1;
    const auto sqrtDenom = h_dot_in + eta * h_dot_out;

    return Sqrt(baseColor) * (etaFactor * (1 - F) * D * G * eta * eta * std::abs(h_dot_out * h_dot_in)) /
           (n_dot_in * sqrtDenom * sqrtDenom);
}

Real PdfSampleBSDFOp::operator()(const DisneyGlass& bsdf) const
{
    bool reflect = Dot(vertex.normal, dirIn) * Dot(vertex.normal, dirOut) > 0;
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dirIn) * Dot(vertex.normal, dirIn) < 0) {
        frame = -frame;
    }

    const auto eta = Dot(vertex.normal, dirIn) > 0 ? bsdf.eta : 1 / bsdf.eta;

    // clang-format off
    const auto anisotropic = Eval(bsdf.anisotropic, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto roughness = Clamp(Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texturePool), Real(0.01), Real(1));
    // clang-format on

    const auto aspect = std::sqrt(kOne<Real> - Real(0.9) * anisotropic);
    const auto ax     = std::max(Real(0.0001), roughness * roughness / aspect);
    const auto ay     = std::max(Real(0.0001), roughness * roughness * aspect);

    auto h = Normalize(dirIn + dirOut);
    if (!reflect) {
        h = Normalize(dirIn + dirOut * eta);
    }

    if (Dot(h, frame.n) < 0) {
        h = -h;
    }

    const auto h_dot_in  = Dot(h, dirIn);
    const auto h_dot_out = Dot(h, dirOut);
    const auto n_dot_in  = AbsDot(frame.n, dirIn);

    const auto F = elma::FresnelDielectric(h_dot_in, eta);
    const auto D = elma::GGXAnisotropic(ToLocal(frame.n, h), ax, ay);
    const auto G = elma::SmithMaskingGTR2Anisotropic(ToLocal(frame, dirOut), ax, ay);

    if (reflect) {
        return Real(0.25) * (F * D * G) / n_dot_in;
    }

    const auto sqrtDenom = h_dot_in + eta * h_dot_out;

    return (1 - F) * D * G * std::abs(eta * eta * h_dot_out / (sqrtDenom * sqrtDenom) * h_dot_in / Dot(frame.n, dirIn));
}

std::optional<BSDFSampleRecord> SampleBSDFOp::operator()(const DisneyGlass& bsdf) const
{
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dirIn) * Dot(vertex.normal, dirIn) < 0) {
        frame = -frame;
    }

    const auto eta = Dot(vertex.normal, dirIn) > 0 ? bsdf.eta : 1 / bsdf.eta;

    // clang-format off
    const auto anisotropic = Eval(bsdf.anisotropic, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto roughness = Clamp(Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texturePool), Real(0.01), Real(1));
    // clang-format on

    const auto aspect = std::sqrt(kOne<Real> - Real(0.9) * anisotropic);
    const auto ax     = std::max(Real(0.0001), roughness * roughness / aspect);
    const auto ay     = std::max(Real(0.0001), roughness * roughness * aspect);

    const auto localNormal = elma::SampleVisibleNormalsAnisotropic(ToLocal(frame, dirIn), ax, ay, rndParamUV);

    Vector3 h = ToWorld(frame, localNormal);
    if (Dot(h, frame.n) < 0) {
        h = -h;
    }

    const auto h_dot_in = Dot(h, dirIn);
    const auto F        = elma::FresnelDielectric(h_dot_in, eta);

    if (rndParamW <= F) {
        // Reflection
        return BSDFSampleRecord{Reflect(dirIn, h), Real(0) /* eta */, roughness /* roughness */};
    }

    // Refraction
    const auto h_dot_out2 = 1 - (1 - h_dot_in * h_dot_in) / (eta * eta);
    if (h_dot_out2 <= 0) {
        // Total internal reflection
        return {};
    }
    // flip half_vector if needed
    if (h_dot_in < 0) {
        h = -h;
    }

    const auto h_dot_out = std::sqrt(h_dot_out2);
    const auto refracted = -dirIn / eta + (std::abs(h_dot_in) / eta - h_dot_out) * h;
    return BSDFSampleRecord{refracted, eta /* eta */, roughness /* roughness */};
}

TextureSpectrum get_texture_op::operator()(const DisneyGlass& bsdf) const
{
    return bsdf.baseColor;
}
