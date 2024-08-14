#include "../Microfacet.hpp"

Spectrum EvalOp::operator()(const DisneyBSDF& bsdf) const
{
    bool reflect = Dot(vertex.normal, dirIn) * Dot(vertex.normal, dirOut) > 0;
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dirIn) * Dot(vertex.normal, dirIn) < 0) {
        frame = -frame;
    }

    // clang-format off
    const auto baseColor = Eval(bsdf.baseColor, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto subsurface = Eval(bsdf.subsurface, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto specular = Eval(bsdf.specular, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto specularTrans = Eval(bsdf.specularTransmission, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto specularTint = Eval(bsdf.specularTint, vertex.uv, vertex.uvScreenSize, texturePool);

    const auto anisotropic = Eval(bsdf.anisotropic, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto roughness = Clamp(Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texturePool), Real(0.01), Real(1));
    const auto metallic = Eval(bsdf.metallic, vertex.uv, vertex.uvScreenSize, texturePool);

    const auto sheen = Eval(bsdf.sheen, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto sheenTint = Eval(bsdf.sheenTint, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto clearcoat = Eval(bsdf.clearcoat, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto gloss = Eval(bsdf.clearcoatGloss, vertex.uv, vertex.uvScreenSize, texturePool);
    // clang-format on

    auto diffuseBSDF   = DisneyDiffuse{bsdf.baseColor, bsdf.roughness, bsdf.subsurface};
    auto metalBSDF     = DisneyMetal{bsdf.baseColor, bsdf.roughness, bsdf.anisotropic};
    auto clearcoatBSDF = DisneyClearcoat{bsdf.clearcoatGloss};
    auto glassBSDF     = DisneyGlass{bsdf.baseColor, bsdf.roughness, bsdf.anisotropic, bsdf.eta};
    auto sheenBSDF     = DisneySheen{bsdf.baseColor, bsdf.sheenTint};

    auto eval = EvalOp{dirIn, dirOut, vertex, texturePool, dir};

    if (Dot(dirIn, vertex.normal) <= 0) {
        // inside
        return (1.0 - metallic) * specularTrans * eval(glassBSDF);
    }

    const auto eta = Dot(vertex.normal, dirIn) > 0 ? bsdf.eta : 1 / bsdf.eta;

    const auto f_diffuse = eval(diffuseBSDF);
    const auto f_metal   = eval(metalBSDF, specular, metallic, specularTint, eta);
    const auto f_cc      = eval(clearcoatBSDF);
    const auto f_glass   = eval(glassBSDF);
    const auto f_sheen   = eval(sheenBSDF);

    // blend things together
    return (1.0 - specularTrans) * (1.0 - metallic) * f_diffuse + // diffuse
           (1.0 - metallic) * sheen * f_sheen +                   // sheen
           (1.0 - specularTrans * (1.0 - metallic)) * f_metal +   // metal
           Real(0.25) * clearcoat * f_cc +                        // clearcoat
           (1.0 - metallic) * specularTrans * f_glass;
}

Real PdfSampleBSDFOp::operator()(const DisneyBSDF& bsdf) const
{
    bool reflect = Dot(vertex.normal, dirIn) * Dot(vertex.normal, dirOut) > 0;
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dirIn) * Dot(vertex.normal, dirIn) < 0) {
        frame = -frame;
    }

    // clang-format off
    const auto baseColor = Eval(bsdf.baseColor, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto subsurface = Eval(bsdf.subsurface, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto specular = Eval(bsdf.specular, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto specularTrans = Eval(bsdf.specularTransmission, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto specularTint = Eval(bsdf.specularTint, vertex.uv, vertex.uvScreenSize, texturePool);

    const auto anisotropic = Eval(bsdf.anisotropic, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto roughness = Clamp(Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texturePool), Real(0.01), Real(1));
    const auto metallic = Eval(bsdf.metallic, vertex.uv, vertex.uvScreenSize, texturePool);

    const auto sheen = Eval(bsdf.sheen, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto sheenTint = Eval(bsdf.sheenTint, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto clearcoat = Eval(bsdf.clearcoat, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto gloss = Eval(bsdf.clearcoatGloss, vertex.uv, vertex.uvScreenSize, texturePool);
    // clang-format on

    auto pdf = PdfSampleBSDFOp{dirIn, dirOut, vertex, texturePool, dir};

    auto diffuseBSDF   = DisneyDiffuse{bsdf.baseColor, bsdf.roughness, bsdf.subsurface};
    auto metalBSDF     = DisneyMetal{bsdf.baseColor, bsdf.roughness, bsdf.anisotropic};
    auto clearcoatBSDF = DisneyClearcoat{bsdf.clearcoatGloss};
    auto glassBSDF     = DisneyGlass{bsdf.baseColor, bsdf.roughness, bsdf.anisotropic, bsdf.eta};
    //    auto sheenBSDF     = DisneySheen{bsdf.baseColor, bsdf.sheenTint};

    // 4 weights
    auto diffuseW   = (1.0 - metallic) * (1.0 - specularTrans);
    auto metalW     = 1.0 - specularTrans * (1.0 - metallic);
    auto glassW     = (1.0 - metallic) * specularTrans;
    auto clearcoatW = 0.25 * clearcoat;

    // normalize to [0,1]
    const auto totalW  = diffuseW + metalW + glassW + clearcoatW;
    diffuseW          /= totalW;
    metalW            /= totalW;
    glassW            /= totalW;
    clearcoatW        /= totalW;

    if (Dot(dirIn, vertex.normal) <= 0) {
        return pdf(glassBSDF);
    }

    return pdf(diffuseBSDF) * diffuseW + pdf(metalBSDF) * metalW + pdf(glassBSDF) * glassW +
           pdf(clearcoatBSDF) * clearcoatW;
}

std::optional<BSDFSampleRecord> SampleBSDFOp::operator()(const DisneyBSDF& bsdf) const
{
    // Flip the shading frame if it is inconsistent with the geometry normal
    Frame frame = vertex.shadingFrame;
    if (Dot(frame.n, dirIn) * Dot(vertex.normal, dirIn) < 0) {
        frame = -frame;
    }

    // clang-format off
    const auto baseColor = Eval(bsdf.baseColor, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto subsurface = Eval(bsdf.subsurface, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto specular = Eval(bsdf.specular, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto specularTrans = Eval(bsdf.specularTransmission, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto specularTint = Eval(bsdf.specularTint, vertex.uv, vertex.uvScreenSize, texturePool);

    const auto anisotropic = Eval(bsdf.anisotropic, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto roughness = Clamp(Eval(bsdf.roughness, vertex.uv, vertex.uvScreenSize, texturePool), Real(0.01), Real(1));
    const auto metallic = Eval(bsdf.metallic, vertex.uv, vertex.uvScreenSize, texturePool);

    const auto sheen = Eval(bsdf.sheen, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto sheenTint = Eval(bsdf.sheenTint, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto clearcoat = Eval(bsdf.clearcoat, vertex.uv, vertex.uvScreenSize, texturePool);
    const auto gloss = Eval(bsdf.clearcoatGloss, vertex.uv, vertex.uvScreenSize, texturePool);
    // clang-format on

    auto sample = SampleBSDFOp{dirIn, vertex, texturePool, rndParamUV, rndParamW, dir};

    auto diffuseBSDF   = DisneyDiffuse{bsdf.baseColor, bsdf.roughness, bsdf.subsurface};
    auto metalBSDF     = DisneyMetal{bsdf.baseColor, bsdf.roughness, bsdf.anisotropic};
    auto clearcoatBSDF = DisneyClearcoat{bsdf.clearcoatGloss};
    auto glassBSDF     = DisneyGlass{bsdf.baseColor, bsdf.roughness, bsdf.anisotropic, bsdf.eta};
    //    auto sheenBSDF     = DisneySheen{bsdf.baseColor, bsdf.sheenTint};

    if (Dot(dirIn, vertex.normal) <= 0) {
        return sample(glassBSDF);
    }

    // 4 weights
    auto diffuseW   = (1.0 - metallic) * (1.0 - specularTrans);
    auto metalW     = 1.0 - specularTrans * (1.0 - metallic);
    auto glassW     = (1.0 - metallic) * specularTrans;
    auto clearcoatW = 0.25 * clearcoat;

    // normalize to [0,1]
    const auto totalW  = diffuseW + metalW + glassW + clearcoatW;
    diffuseW          /= totalW;
    metalW            /= totalW;
    glassW            /= totalW;
    clearcoatW        /= totalW;

    if (rndParamW < diffuseW) {
        return sample(diffuseBSDF);
    }
    else if (rndParamW < diffuseW + metalW) {
        return sample(metalBSDF);
    }
    else if (rndParamW < diffuseW + metalW + glassW) {
        return sample(glassBSDF);
    }
    else {
        return sample(clearcoatBSDF);
    }
}

TextureSpectrum get_texture_op::operator()(const DisneyBSDF& bsdf) const
{
    return bsdf.baseColor;
}
