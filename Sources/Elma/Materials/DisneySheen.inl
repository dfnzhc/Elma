#include "../Microfacet.hpp"

Spectrum EvalOp::operator()(const DisneySheen& bsdf) const
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
    
    // sheen 材质的颜色根据 sheenTint 参数在白色和自身颜色之间变化。
    // 它的目的是模拟光在表面上的掠角，因此它将主要用于布料或粗糙表面上的反向反射，以补充由于只模拟单散射的几何术语而损失的能量。

    const auto baseColor = Eval(bsdf.baseColor, vertex.uv, vertex.uvScreenSize, texture_pool);
    const auto sheenTint = Eval(bsdf.sheenTint, vertex.uv, vertex.uvScreenSize, texture_pool);

    const auto h         = Normalize(dirIn + dirOut);
    const auto h_dot_out = AbsDot(h, dirOut);
    const auto n_dot_out = AbsDot(frame.n, dirOut);

    const auto tint  = elma::CalculateTint(baseColor);
    const auto color = Lerp(MakeConstSpectrum(Real(1)), tint, sheenTint);

    return color * elma::SchlickWeight(h_dot_out) * n_dot_out;
}

Real PdfSampleBSDFOp::operator()(const DisneySheen& bsdf) const
{
    if (Dot(vertex.normal, dir_in) < 0 || Dot(vertex.normal, dir_out) < 0) {
        // No light below the surface
        return 0;
    }
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dir_in) < 0) {
        frame = -frame;
    }

    return AbsDot(frame.n, dir_out) / kPi;
}

std::optional<BSDFSampleRecord> SampleBSDFOp::operator()(const DisneySheen& bsdf) const
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

    return BSDFSampleRecord{
      ToWorld(frame, SampleCosHemisphere(rndParamUV)), Real(0) /* eta */, Real(1) /* roughness */};
}

TextureSpectrum get_texture_op::operator()(const DisneySheen& bsdf) const
{
    return bsdf.baseColor;
}
