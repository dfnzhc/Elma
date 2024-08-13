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
    // Homework 1: implement this!

    return MakeZeroSpectrum();
}

Real PdfSampleBSDFOp::operator()(const DisneyMetal& bsdf) const
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
    // Homework 1: implement this!

    return 0;
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
    // Homework 1: implement this!

    return {};
}

TextureSpectrum get_texture_op::operator()(const DisneyMetal& bsdf) const
{
    return bsdf.baseColor;
}
