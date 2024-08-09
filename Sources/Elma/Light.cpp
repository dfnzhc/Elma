#include "Light.hpp"
#include "Scene.hpp"
#include "Spectrum.hpp"
#include "Transform.hpp"

namespace elma {

struct light_power_op
{
    Real operator()(const DiffuseAreaLight& light) const;
    Real operator()(const Envmap& light) const;

    const Scene& scene;
};

struct sample_point_on_light_op
{
    PointAndNormal operator()(const DiffuseAreaLight& light) const;
    PointAndNormal operator()(const Envmap& light) const;

    const Vector3& ref_point;
    const Vector2& rnd_param_uv;
    const Real& rnd_param_w;
    const Scene& scene;
};

struct pdf_point_on_light_op
{
    Real operator()(const DiffuseAreaLight& light) const;
    Real operator()(const Envmap& light) const;

    const PointAndNormal& point_on_light;
    const Vector3& ref_point;
    const Scene& scene;
};

struct emission_op
{
    Spectrum operator()(const DiffuseAreaLight& light) const;
    Spectrum operator()(const Envmap& light) const;

    const Vector3& view_dir;
    const PointAndNormal& point_on_light;
    Real view_footprint;
    const Scene& scene;
};

struct InitSamplingDistOp
{
    void operator()(DiffuseAreaLight& light) const;
    void operator()(Envmap& light) const;

    const Scene& scene;
};

#include "Lights/DiffuseAreaLight.inl"
#include "Lights/Envmap.inl"

Real LightPower(const Light& light, const Scene& scene)
{
    return std::visit(light_power_op{scene}, light);
}

PointAndNormal SamplePointOnLight(
    const Light& light, const Vector3& ref_point, const Vector2& rnd_param_uv, Real rnd_param_w, const Scene& scene)
{
    return std::visit(sample_point_on_light_op{ref_point, rnd_param_uv, rnd_param_w, scene}, light);
}

Real PdfPointOnLight(const Light& light,
                     const PointAndNormal& point_on_light,
                     const Vector3& ref_point,
                     const Scene& scene)
{
    return std::visit(pdf_point_on_light_op{point_on_light, ref_point, scene}, light);
}

Spectrum Emission(const Light& light,
                  const Vector3& view_dir,
                  Real view_footprint,
                  const PointAndNormal& point_on_light,
                  const Scene& scene)
{
    return std::visit(emission_op{view_dir, point_on_light, view_footprint, scene}, light);
}

void InitSamplingDist(Light& light, const Scene& scene)
{
    return std::visit(InitSamplingDistOp{scene}, light);
}

} // namespace elma