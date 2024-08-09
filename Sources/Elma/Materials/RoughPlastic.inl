#include "../Microfacet.hpp"

Spectrum EvalOp::operator()(const RoughPlastic& bsdf) const
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

    // The half-vector is a crucial component of the microfacet models.
    // Since microfacet assumes that the surface is made of many small mirrors/glasses,
    // The "average" between input and output direction determines the orientation
    // of the mirror our ray hits (since applying reflection of dir_in over half_vector
    // gives us dir_out). Microfacet models build all sorts of quantities based on the
    // half vector. It's also called the "micro normal".
    Vector3 half_vector = normalize(dirIn + dirOut);
    Real n_dot_h        = Dot(frame.n, half_vector);
    Real n_dot_in       = Dot(frame.n, dirIn);
    Real n_dot_out      = Dot(frame.n, dirOut);
    if (n_dot_out <= 0 || n_dot_h <= 0) {
        return MakeZeroSpectrum();
    }

    Spectrum Kd    = Eval(bsdf.diffuseReflectance, vertex.uv, vertex.uvScreenSize, texture_pool);
    Spectrum Ks    = Eval(bsdf.specularReflectance, vertex.uv, vertex.uvScreenSize, texture_pool);
    Real roughness = Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texture_pool);
    // Clamp roughness to avoid numerical issues.
    roughness = std::clamp(roughness, Real(0.01), Real(1));

    // We first account for the dielectric layer.

    // Fresnel equation determines how much light goes through,
    // and how much light is reflected for each wavelength.
    // Fresnel equation is determined by the angle between the (micro) normal and
    // both incoming and outgoing directions (dir_out & dir_in).
    // However, since they are related through the Snell-Descartes law,
    // we only need one of them.
    Real F_o = elma::FresnelDielectric(Dot(half_vector, dirOut), bsdf.eta); // F_o is the reflection percentage.
    Real D   = elma::GTR2(n_dot_h, roughness); // "Generalized Trowbridge Reitz", GTR2 is equivalent to GGX.
    Real G   = elma::SmithMaskingGTR2(ToLocal(frame, dirIn), roughness) *
             elma::SmithMaskingGTR2(ToLocal(frame, dirOut), roughness);

    Spectrum spec_contrib = Ks * (G * F_o * D) / (4 * n_dot_in * n_dot_out);

    // Next we account for the diffuse layer.
    // In order to reflect from the diffuse layer,
    // the photon needs to bounce through the dielectric layers twice.
    // The transmittance is computed by 1 - fresnel.
    Real F_i = elma::FresnelDielectric(Dot(half_vector, dirIn), bsdf.eta);
    // Multiplying with Fresnels leads to an overly dark appearance at the
    // object boundaries. Disney BRDF proposes a fix to this -- we will implement this in problem set 1.
    Spectrum diffuse_contrib = Kd * (Real(1) - F_o) * (Real(1) - F_i) / kPi;

    return (spec_contrib + diffuse_contrib) * n_dot_out;
}

Real pdf_sample_bsdf_op::operator()(const RoughPlastic& bsdf) const
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

    Vector3 half_vector = normalize(dir_in + dir_out);
    Real n_dot_in       = Dot(frame.n, dir_in);
    Real n_dot_out      = Dot(frame.n, dir_out);
    Real n_dot_h        = Dot(frame.n, half_vector);
    if (n_dot_out <= 0 || n_dot_h <= 0) {
        return 0;
    }

    Spectrum S = Eval(bsdf.specularReflectance, vertex.uv, vertex.uvScreenSize, texture_pool);
    Spectrum R = Eval(bsdf.diffuseReflectance, vertex.uv, vertex.uvScreenSize, texture_pool);
    Real lS = Luminance(S), lR = Luminance(R);
    if (lS + lR <= 0) {
        return 0;
    }
    Real roughness = Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texture_pool);
    // Clamp roughness to avoid numerical issues.
    roughness = std::clamp(roughness, Real(0.01), Real(1));
    // We use the reflectance to determine whether to choose specular sampling lobe or diffuse.
    Real spec_prob = lS / (lS + lR);
    Real diff_prob = 1 - spec_prob;
    // For the specular lobe, we use the ellipsoidal sampling from Heitz 2018
    // "Sampling the GGX Distribution of Visible Normals"
    // https://jcgt.org/published/0007/04/01/
    // this importance samples smith_masking(cos_theta_in) * GTR2(cos_theta_h, roughness) * cos_theta_out
    Real G = elma::SmithMaskingGTR2(ToLocal(frame, dir_in), roughness);
    Real D = elma::GTR2(n_dot_h, roughness);
    // (4 * cos_theta_v) is the Jacobian of the reflectiokn
    spec_prob *= (G * D) / (4 * n_dot_in);
    // For the diffuse lobe, we importance sample cos_theta_out
    diff_prob *= n_dot_out / kPi;
    return spec_prob + diff_prob;
}

std::optional<BSDFSampleRecord> sample_bsdf_op::operator()(const RoughPlastic& bsdf) const
{
    if (Dot(vertex.normal, dir_in) < 0) {
        // No light below the surface
        return {};
    }
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dir_in) < 0) {
        frame = -frame;
    }

    // We use the reflectance to choose between sampling the dielectric or diffuse layer.
    Spectrum Ks = Eval(bsdf.specularReflectance, vertex.uv, vertex.uvScreenSize, texture_pool);
    Spectrum Kd = Eval(bsdf.diffuseReflectance, vertex.uv, vertex.uvScreenSize, texture_pool);
    Real lS = Luminance(Ks), lR = Luminance(Kd);
    if (lS + lR <= 0) {
        return {};
    }
    Real spec_prob = lS / (lS + lR);
    if (rnd_param_w < spec_prob) {
        // Sample from the specular lobe.

        // Convert the incoming direction to local coordinates
        Vector3 local_dir_in = ToLocal(frame, dir_in);
        Real roughness       = Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texture_pool);
        // Clamp roughness to avoid numerical issues.
        roughness                  = std::clamp(roughness, Real(0.01), Real(1));
        Real alpha                 = roughness * roughness;
        Vector3 local_micro_normal = elma::SampleVisibleNormals(local_dir_in, alpha, rnd_param_uv);

        // Transform the micro normal to world space
        Vector3 half_vector = ToWorld(frame, local_micro_normal);
        // Reflect over the world space normal
        Vector3 reflected = normalize(-dir_in + 2 * Dot(dir_in, half_vector) * half_vector);
        return BSDFSampleRecord{
          reflected, Real(0) /* eta */, roughness /* roughness */
        };
    }
    else {
        // Lambertian sampling
        return BSDFSampleRecord{
          ToWorld(frame, sample_cos_hemisphere(rnd_param_uv)), Real(0) /* eta */, Real(1) /* roughness */};
    }
}

TextureSpectrum get_texture_op::operator()(const RoughPlastic& bsdf) const
{
    return bsdf.diffuseReflectance;
}
