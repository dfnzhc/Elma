#include "Shape.hpp"
#include "Intersection.hpp"
#include "PointAndNormal.hpp"
#include "Ray.hpp"
#include <embree4/rtcore.h>
#include <variant>

namespace elma {

struct RegisterEmbreeOp
{
    uint32_t operator()(const Sphere& sphere) const;
    uint32_t operator()(const TriangleMesh& mesh) const;

    const RTCDevice& device;
    const RTCScene& scene;
};

struct SamplePointOnShapeOp
{
    PointAndNormal operator()(const Sphere& sphere) const;
    PointAndNormal operator()(const TriangleMesh& mesh) const;

    const Vector3& ref_point;
    const Vector2& uv; // for selecting a point on a 2D surface
    const Real& w;     // for selecting triangles
};

struct SurfaceAreaOp
{
    Real operator()(const Sphere& sphere) const;
    Real operator()(const TriangleMesh& mesh) const;
};

struct PdfPointOnShapeOp
{
    Real operator()(const Sphere& sphere) const;
    Real operator()(const TriangleMesh& mesh) const;

    const PointAndNormal& pointOnShape;
    const Vector3& refPoint;
};

struct InitSamplingDistOp
{
    void operator()(Sphere& sphere) const;
    void operator()(TriangleMesh& mesh) const;
};

struct ComputeShadingInfoOp
{
    ShadingInfo operator()(const Sphere& sphere) const;
    ShadingInfo operator()(const TriangleMesh& mesh) const;

    const PathVertex& vertex;
};

#include "Shapes/Sphere.inl"
#include "Shapes/TriangleMesh.inl"

uint32_t RegisterEmbree(const Shape& shape, const RTCDevice& device, const RTCScene& scene)
{
    return std::visit(RegisterEmbreeOp{device, scene}, shape);
}

PointAndNormal SamplePointOnShape(const Shape& shape, const Vector3& ref_point, const Vector2& uv, Real w)
{
    return std::visit(SamplePointOnShapeOp{ref_point, uv, w}, shape);
}

Real PdfPointOnShape(const std::variant<Sphere, TriangleMesh>& shape,
                     const PointAndNormal& point_on_shape,
                     const Vector3& ref_point)
{
    return std::visit(PdfPointOnShapeOp{point_on_shape, ref_point}, shape);
}

Real SurfaceArea(const std::variant<Sphere, TriangleMesh>& shape)
{
    return std::visit(SurfaceAreaOp{}, shape);
}

void InitSamplingDist(std::variant<Sphere, TriangleMesh>& shape)
{
    return std::visit(InitSamplingDistOp{}, shape);
}

ShadingInfo ComputeShadingInfo(const std::variant<Sphere, TriangleMesh>& shape, const PathVertex& vertex)
{
    return std::visit(ComputeShadingInfoOp{vertex}, shape);
}

} // namespace elma