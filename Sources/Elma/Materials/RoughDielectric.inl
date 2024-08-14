#include "../Microfacet.hpp"

Spectrum EvalOp::operator()(const RoughDielectric& bsdf) const
{
    bool reflect = Dot(vertex.normal, dirIn) * Dot(vertex.normal, dirOut) > 0;
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dirIn) * Dot(vertex.normal, dirIn) < 0) {
        frame = -frame;
    }
    // If we are going into the surface, then we use normal eta
    // (internal/external), otherwise we use external/internal.
    Real eta = Dot(vertex.normal, dirIn) > 0 ? bsdf.eta : 1 / bsdf.eta;

    Spectrum Ks    = Eval(bsdf.specularReflectance, vertex.uv, vertex.uvScreenSize, texturePool);
    Spectrum Kt    = Eval(bsdf.specularTransmittance, vertex.uv, vertex.uvScreenSize, texturePool);
    Real roughness = Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texturePool);

    Vector3 half_vector;
    if (reflect) {
        half_vector = Normalize(dirIn + dirOut);
    }
    else {
        // "Generalized half-vector" from Walter et al.
        // See "Microfacet Models for Refraction through Rough Surfaces"
        half_vector = Normalize(dirIn + dirOut * eta);
    }

    // Flip half-vector if it's below surface
    if (Dot(half_vector, frame.n) < 0) {
        half_vector = -half_vector;
    }

    // Clamp roughness to avoid numerical issues.
    roughness = std::clamp(roughness, Real(0.01), Real(1));

    // Compute F / D / G
    // Note that we use the incoming direction
    // for evaluating the Fresnel reflection amount.
    // We can also use outgoing direction -- then we would need to
    // use 1/bsdf.eta and we will get the same result.
    // However, using the incoming direction allows
    // us to use F to decide whether to reflect or refract during sampling.
    Real h_dot_in = Dot(half_vector, dirIn);
    Real F        = elma::FresnelDielectric(h_dot_in, eta);
    Real D        = elma::GTR2(Dot(frame.n, half_vector), roughness);
    Real G        = elma::SmithMaskingGTR2(ToLocal(frame, dirIn), roughness) *
             elma::SmithMaskingGTR2(ToLocal(frame, dirOut), roughness);
    if (reflect) {
        return Ks * (F * D * G) / (4 * std::abs(Dot(frame.n, dirIn)));
    }
    else {
        // Snell-Descartes law predicts that the light will contract/expand
        // due to the different index of refraction. So the normal BSDF needs
        // to scale with 1/eta^2. However, the "adjoint" of the BSDF does not have
        // the eta term. This is due to the non-reciprocal nature of the index of refraction:
        // f(wi -> wo) / eta_o^2 = f(wo -> wi) / eta_i^2
        // thus f(wi -> wo) = f(wo -> wi) (eta_o / eta_i)^2
        // The adjoint of a BSDF is defined as swapping the parameter, and
        // this cancels out the eta term.
        // See Chapter 5 of Eric Veach's thesis "Robust Monte Carlo Methods for Light Transport Simulation"
        // for more details.
        Real eta_factor = dir == TransportDirection::TO_LIGHT ? (1 / (eta * eta)) : 1;
        Real h_dot_out  = Dot(half_vector, dirOut);
        Real sqrt_denom = h_dot_in + eta * h_dot_out;
        // Very complicated BSDF. See Walter et al.'s paper for more details.
        // "Microfacet Models for Refraction through Rough Surfaces"
        return Kt * (eta_factor * (1 - F) * D * G * eta * eta * std::abs(h_dot_out * h_dot_in)) /
               (std::abs(Dot(frame.n, dirIn)) * sqrt_denom * sqrt_denom);
    }
}

Real PdfSampleBSDFOp::operator()(const RoughDielectric& bsdf) const
{
    bool reflect = Dot(vertex.normal, dirIn) * Dot(vertex.normal, dirOut) > 0;
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dirIn) * Dot(vertex.normal, dirIn) < 0) {
        frame = -frame;
    }
    // If we are going into the surface, then we use normal eta
    // (internal/external), otherwise we use external/internal.
    Real eta = Dot(vertex.normal, dirIn) > 0 ? bsdf.eta : 1 / bsdf.eta;
    assert(eta > 0);

    Vector3 half_vector;
    if (reflect) {
        half_vector = Normalize(dirIn + dirOut);
    }
    else {
        // "Generalized half-vector" from Walter et al.
        // See "Microfacet Models for Refraction through Rough Surfaces"
        half_vector = Normalize(dirIn + dirOut * eta);
    }

    // Flip half-vector if it's below surface
    if (Dot(half_vector, frame.n) < 0) {
        half_vector = -half_vector;
    }

    Real roughness = Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texturePool);
    // Clamp roughness to avoid numerical issues.
    roughness = std::clamp(roughness, Real(0.01), Real(1));

    // We sample the visible normals, also we use F to determine
    // whether to sample reflection or refraction
    // so PDF ~ F * D * G_in for reflection, PDF ~ (1 - F) * D * G_in for refraction.
    Real h_dot_in = Dot(half_vector, dirIn);
    Real F        = elma::FresnelDielectric(h_dot_in, eta);
    Real D        = elma::GTR2(Dot(half_vector, frame.n), roughness);
    Real G_in     = elma::SmithMaskingGTR2(ToLocal(frame, dirIn), roughness);
    if (reflect) {
        return (F * D * G_in) / (4 * std::fabs(Dot(frame.n, dirIn)));
    }
    else {
        Real h_dot_out  = Dot(half_vector, dirOut);
        Real sqrt_denom = h_dot_in + eta * h_dot_out;
        Real dh_dout    = eta * eta * h_dot_out / (sqrt_denom * sqrt_denom);
        return (1 - F) * D * G_in * fabs(dh_dout * h_dot_in / Dot(frame.n, dirIn));
    }
}

std::optional<BSDFSampleRecord> SampleBSDFOp::operator()(const RoughDielectric& bsdf) const
{
    // If we are going into the surface, then we use normal eta
    // (internal/external), otherwise we use external/internal.
    Real eta = Dot(vertex.normal, dirIn) > 0 ? bsdf.eta : 1 / bsdf.eta;
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dirIn) * Dot(vertex.normal, dirIn) < 0) {
        frame = -frame;
    }
    Real roughness = Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texturePool);
    // Clamp roughness to avoid numerical issues.
    roughness = std::clamp(roughness, Real(0.01), Real(1));
    // Sample a micro normal and transform it to world space -- this is our half-vector.
    Real alpha                 = roughness * roughness;
    Vector3 local_dir_in       = ToLocal(frame, dirIn);
    Vector3 local_micro_normal = elma::SampleVisibleNormals(local_dir_in, alpha, rndParamUV);

    Vector3 half_vector = ToWorld(frame, local_micro_normal);
    // Flip half-vector if it's below surface
    if (Dot(half_vector, frame.n) < 0) {
        half_vector = -half_vector;
    }

    // Now we need to decide whether to reflect or refract.
    // We do this using the Fresnel term.
    Real h_dot_in = Dot(half_vector, dirIn);
    Real F        = elma::FresnelDielectric(h_dot_in, eta);

    if (rndParamW <= F) {
        // Reflection
        Vector3 reflected = Normalize(-dirIn + 2 * Dot(dirIn, half_vector) * half_vector);
        // set eta to 0 since we are not transmitting
        return BSDFSampleRecord{reflected, Real(0) /* eta */, roughness};
    }
    else {
        // Refraction
        // https://en.wikipedia.org/wiki/Snell%27s_law#Vector_form
        // (note that our eta is eta2 / eta1, and l = -dir_in)
        Real h_dot_out_sq = 1 - (1 - h_dot_in * h_dot_in) / (eta * eta);
        if (h_dot_out_sq <= 0) {
            // Total internal reflection
            // This shouldn't really happen, as F will be 1 in this case.
            return {};
        }
        // flip half_vector if needed
        if (h_dot_in < 0) {
            half_vector = -half_vector;
        }
        Real h_dot_out    = std::sqrt(h_dot_out_sq);
        Vector3 refracted = -dirIn / eta + (fabs(h_dot_in) / eta - h_dot_out) * half_vector;
        return BSDFSampleRecord{refracted, eta, roughness};
    }
}

TextureSpectrum get_texture_op::operator()(const RoughDielectric& bsdf) const
{
    return bsdf.specularReflectance;
}
