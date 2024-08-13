Spectrum EvalOp::operator()(const Lambertian& bsdf) const
{
    if (Dot(vertex.normal, dirIn) < 0 || Dot(vertex.normal, dirOut) < 0) {
        // No light below the surface
        return MakeZeroSpectrum();
    }
    // Sometimes the shading normal can be inconsistent with
    // the geometry normal. We flip the shading frame in that
    // case so that we don't produces "black fringes".
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dirIn) < 0) {
        frame = -frame;
    }

    return fmax(Dot(frame.n, dirOut), Real(0)) * Eval(bsdf.reflectance, vertex.uv, vertex.uvScreenSize, texture_pool) / kPi;
}

Real PdfSampleBSDFOp::operator()(const Lambertian& bsdf) const
{
    if (Dot(vertex.normal, dir_in) < 0 || Dot(vertex.normal, dir_out) < 0) {
        // No light below the surface
        return Real(0);
    }
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dir_in) < 0) {
        frame = -frame;
    }

    // For Lambertian, we importance sample the cosine hemisphere domain.
    return fmax(Dot(frame.n, dir_out), Real(0)) / kPi;
}

std::optional<BSDFSampleRecord> SampleBSDFOp::operator()(const Lambertian& bsdf) const
{
    // For Lambertian, we importance sample the cosine hemisphere domain.
    if (Dot(vertex.normal, dirIn) < 0) {
        // Incoming direction is below the surface.
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

TextureSpectrum get_texture_op::operator()(const Lambertian& bsdf) const
{
    return bsdf.reflectance;
}
