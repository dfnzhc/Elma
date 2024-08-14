Spectrum EvalOp::operator()(const DisneyDiffuse& bsdf) const
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
    const auto subsurface = Eval(bsdf.subsurface, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto roughness = Clamp(Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texturePool), Real(0.01), Real(1));
    // clang-format on

    const auto h         = Normalize(dirIn + dirOut);
    const auto n_dot_out = AbsDot(frame.n, dirOut);
    const auto n_dot_in  = AbsDot(frame.n, dirIn);
    const auto h_dot_out = AbsDot(h, dirOut);

    const auto Schlick_i = elma::SchlickWeight(AbsDot(frame.n, dirIn));
    const auto Schlick_o = elma::SchlickWeight(AbsDot(frame.n, dirOut));

    const auto Fd_90 = kHalf<Real> + kTwo<Real> * roughness * h_dot_out;
    const auto Fd_i  = kOne<Real> + (Fd_90 - kOne<Real>)*Schlick_i;
    const auto Fd_o  = kOne<Real> + (Fd_90 - kOne<Real>)*Schlick_o;

    const auto dd = baseColor * kInvPi * Fd_i * Fd_o * n_dot_out;

    const auto Fss_90 = roughness * h_dot_out * h_dot_out;
    const auto Fss_i  = kOne<Real> + (Fss_90 - kOne<Real>)*Schlick_i;
    const auto Fss_o  = kOne<Real> + (Fss_90 - kOne<Real>)*Schlick_o;

    const auto ss = Real(1.25) * baseColor * kInvPi *
                    (Fss_i * Fss_o * (kOne<Real> / (n_dot_out + n_dot_in) - kHalf<Real>)+kHalf<Real>)*n_dot_out;

    return Lerp(dd, ss, subsurface);
}

Real PdfSampleBSDFOp::operator()(const DisneyDiffuse& bsdf) const
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

    return AbsDot(frame.n, dirOut) * kInvPi;
}

std::optional<BSDFSampleRecord> SampleBSDFOp::operator()(const DisneyDiffuse& bsdf) const
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
    
    const auto roughness =
        Clamp(Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texturePool), Real(0.01), Real(1));

    return BSDFSampleRecord{SampleCosHemisphere(rndParamUV), Real(1) /* eta */, roughness /* roughness */};
}

TextureSpectrum get_texture_op::operator()(const DisneyDiffuse& bsdf) const
{
    return bsdf.baseColor;
}
