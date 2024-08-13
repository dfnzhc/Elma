#include "../Microfacet.hpp"

Spectrum EvalOp::operator()(const DisneyGlass& bsdf) const
{
    bool reflect = Dot(vertex.normal, dirIn) * Dot(vertex.normal, dirOut) > 0;
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dirIn) * Dot(vertex.normal, dirIn) < 0) {
        frame = -frame;
    }
    // Homework 1: implement this!
    (void)reflect; // silence unuse warning, remove this when implementing hw

    return MakeZeroSpectrum();
}

Real PdfSampleBSDFOp::operator()(const DisneyGlass& bsdf) const
{
    bool reflect = Dot(vertex.normal, dir_in) * Dot(vertex.normal, dir_out) > 0;
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dir_in) * Dot(vertex.normal, dir_in) < 0) {
        frame = -frame;
    }
    // Homework 1: implement this!
    (void)reflect; // silence unuse warning, remove this when implementing hw

    return 0;
}

std::optional<BSDFSampleRecord> SampleBSDFOp::operator()(const DisneyGlass& bsdf) const
{
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dirIn) * Dot(vertex.normal, dirIn) < 0) {
        frame = -frame;
    }
    // Homework 1: implement this!

    return {};
}

TextureSpectrum get_texture_op::operator()(const DisneyGlass& bsdf) const
{
    return bsdf.baseColor;
}
