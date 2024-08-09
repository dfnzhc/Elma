#pragma once

#include "Elma.hpp"
#include "Frame.hpp"
#include "TableDist.hpp"
#include "Vector.hpp"
#include <embree4/rtcore.h>
#include <variant>
#include <vector>

namespace elma {
struct PointAndNormal;
struct PathVertex;

struct ShadingInfo
{
    Vector2 uv;           // UV coordinates for texture mapping
    Frame shadingFrame;   // the coordinate basis for shading
    Real meanCurvature{}; // 0.5 * (dN/du + dN/dv)
    // Stores min(length(dp/du), length(dp/dv)), for ray differentials.
    Real invUvSize{};
};

/// A Shape is a geometric entity that describes a surface. E.g., a sphere, a triangle mesh, a NURBS, etc.
/// For each shape, we also store an integer "material ID" that points to a material, and an integer
/// "area light ID" that points to a light source if the shape is an area light. area_lightID is set to -1
/// if the shape is not an area light.
struct ShapeBase
{
    int materialId  = -1;
    int areaLightId = -1;

    int interiorMediumId = -1;
    int exteriorMediumId = -1;
};

struct Sphere : public ShapeBase
{
    Vector3 position;
    Real radius;
};

struct TriangleMesh : public ShapeBase
{
    /// TODO: make these portable to GPUs
    std::vector<Vector3> positions;
    std::vector<Vector3i> indices;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    /// Below are used only when the mesh is associated with an area light
    Real totalArea;
    /// For sampling a triangle based on its area
    TableDist1D triangleSampler;
};

// To add more shapes, first create a struct for the shape, add it to the variant below,
// then implement all the relevant functions below.
using Shape = std::variant<Sphere, TriangleMesh>;

/// Add the shape to an Embree scene.
uint32_t RegisterEmbree(const Shape& shape, const RTCDevice& device, const RTCScene& scene);

/// Sample a point on the surface given a reference point.
/// uv & w are uniform random numbers.
PointAndNormal SamplePointOnShape(const Shape& shape, const Vector3& ref_point, const Vector2& uv, Real w);

/// Probability density of the operation above
Real PdfPointOnShape(const Shape& shape, const PointAndNormal& point_on_shape, const Vector3& ref_point);

/// Useful for sampling.
Real SurfaceArea(const Shape& shape);

/// Some shapes require storing sampling data structures inside. This function initialize them.
void InitSamplingDist(Shape& shape);

/// Embree doesn't calculate some shading information for us. We have to do it ourselves.
ShadingInfo ComputeShadingInfo(const Shape& shape, const PathVertex& vertex);

inline void SetMaterialId(Shape& shape, int material_id)
{
    std::visit([&](auto& s) { s.materialId = material_id; }, shape);
}

inline void SetAreaLightId(Shape& shape, int area_light_id)
{
    std::visit([&](auto& s) { s.areaLightId = area_light_id; }, shape);
}

inline void SetInteriorMediumId(Shape& shape, int interior_medium_id)
{
    std::visit([&](auto& s) { s.interiorMediumId = interior_medium_id; }, shape);
}

inline void SetExteriorMediumId(Shape& shape, int exterior_medium_id)
{
    std::visit([&](auto& s) { s.exteriorMediumId = exterior_medium_id; }, shape);
}

inline int GetMaterialId(const Shape& shape)
{
    return std::visit([&](const auto& s) { return s.materialId; }, shape);
}

inline int GetAreaLightId(const Shape& shape)
{
    return std::visit([&](const auto& s) { return s.areaLightId; }, shape);
}

inline int GetInteriorMediumId(const Shape& shape)
{
    return std::visit([&](const auto& s) { return s.interiorMediumId; }, shape);
}

inline int GetExteriorMediumId(const Shape& shape)
{
    return std::visit([&](const auto& s) { return s.exteriorMediumId; }, shape);
}

inline bool IsLight(const Shape& shape)
{
    return GetAreaLightId(shape) >= 0;
}

} // namespace elma