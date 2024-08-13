#include "Intersection.hpp"
#include "Material.hpp"
#include "Ray.hpp"
#include "Scene.hpp"
#include <embree4/rtcore.h>

namespace elma {

std::optional<PathVertex> Intersect(const Scene& scene, const Ray& ray, const RayDifferential& ray_diff)
{
    RTCIntersectArguments rtc_args;
    rtcInitIntersectArguments(&rtc_args);
    RTCRayHit rtc_rayhit;
    RTCRay& rtc_ray = rtc_rayhit.ray;
    RTCHit& rtc_hit = rtc_rayhit.hit;
    rtc_ray         = RTCRay{
      (float)ray.org.x,
      (float)ray.org.y,
      (float)ray.org.z,
      (float)ray.tNear,
      (float)ray.dir.x,
      (float)ray.dir.y,
      (float)ray.dir.z,
      0.f,                // time
      (float)ray.tFar,
      (unsigned int)(-1), // mask
      0,                  // ray ID
      0                   // ray flags
    };
    rtc_hit = RTCHit{
      0,
      0,
      0,                        // Ng_x, Ng_y, Ng_z
      0,
      0,                        // u, v
      RTC_INVALID_GEOMETRY_ID,  // primitive ID
      RTC_INVALID_GEOMETRY_ID,  // geometry ID
      {RTC_INVALID_GEOMETRY_ID} // instance IDs
    };
    rtcIntersect1(scene.embreeScene, &rtc_rayhit, &rtc_args);
    if (rtc_hit.geomID == RTC_INVALID_GEOMETRY_ID) {
        return {};
    };
    assert(rtc_hit.geomID < scene.shapes.size());

    PathVertex vertex;
    vertex.position =
        Vector3{ray.org.x, ray.org.y, ray.org.z} + Vector3{ray.dir.x, ray.dir.y, ray.dir.z} * Real(rtc_ray.tfar);
    vertex.normal             = Normalize(Vector3{rtc_hit.Ng_x, rtc_hit.Ng_y, rtc_hit.Ng_z});
    vertex.shapeId            = rtc_hit.geomID;
    vertex.primitiveId        = rtc_hit.primID;
    const Shape& shape        = scene.shapes[vertex.shapeId];
    vertex.materialId         = GetMaterialId(shape);
    vertex.interiorMediumId   = GetInteriorMediumId(shape);
    vertex.exteriorMediumId   = GetExteriorMediumId(shape);
    vertex.st                 = Vector2{rtc_hit.u, rtc_hit.v};

    ShadingInfo shading_info = ComputeShadingInfo(scene.shapes[vertex.shapeId], vertex);
    vertex.shadingFrame      = shading_info.shadingFrame;
    vertex.uv                = shading_info.uv;
    vertex.meanCurvature     = shading_info.meanCurvature;
    vertex.rayRadius         = Transfer(ray_diff, Distance(ray.org, vertex.position));
    // vertex.ray_radius stores approximatedly dp/dx,
    // we get uv_screen_size (du/dx) using (dp/dx)/(dp/du)
    vertex.uvScreenSize = vertex.rayRadius / shading_info.invUvSize;

    // Flip the geometry normal to the same direction as the shading normal
    if (Dot(vertex.normal, vertex.shadingFrame.n) < 0) {
        vertex.normal = -vertex.normal;
    }

    return vertex;
}

bool Occluded(const Scene& scene, const Ray& ray)
{
    RTCOccludedArguments rtc_args;
    rtcInitOccludedArguments(&rtc_args);
    RTCRay rtc_ray;
    rtc_ray.org_x = (float)ray.org[0];
    rtc_ray.org_y = (float)ray.org[1];
    rtc_ray.org_z = (float)ray.org[2];
    rtc_ray.dir_x = (float)ray.dir[0];
    rtc_ray.dir_y = (float)ray.dir[1];
    rtc_ray.dir_z = (float)ray.dir[2];
    rtc_ray.tnear = (float)ray.tNear;
    rtc_ray.tfar  = (float)ray.tFar;
    rtc_ray.mask  = (unsigned int)(-1);
    rtc_ray.time  = 0.f;
    rtc_ray.flags = 0;
    // TODO: switch to rtcOccluded16
    rtcOccluded1(scene.embreeScene, &rtc_ray, &rtc_args);
    return rtc_ray.tfar < 0;
}

Spectrum Emission(const PathVertex& v, const Vector3& view_dir, const Scene& scene)
{
    int light_id = GetAreaLightId(scene.shapes[v.shapeId]);
    assert(light_id >= 0);
    const Light& light = scene.lights[light_id];
    return Emission(light, view_dir, v.uvScreenSize, PointAndNormal{v.position, v.normal}, scene);
}

} // namespace elma