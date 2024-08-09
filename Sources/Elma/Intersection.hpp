#pragma once

#include "Elma.hpp"
#include "Frame.hpp"
#include "Ray.hpp"
#include "Spectrum.hpp"
#include "Vector.hpp"

#include <optional>

namespace elma {
struct Scene;

/// An "PathVertex" represents a vertex of a light path.
/// We store the information we need for computing any sort of path contribution & sampling density.
struct PathVertex
{
    Vector3 position;
    Vector3 normal; // always face at the same direction at shading_frame.n
    Frame shadingFrame;
    Vector2 st;     // A 2D parametrization of the surface. Irrelavant to UV mapping.
                    // for triangle this is the barycentric coordinates, which we use
                    // for interpolating the uv map.
    Vector2 uv;     // The actual UV we use for texture fetching.
    // For texture filtering, stores approximatedly min(abs(du/dx), abs(dv/dx), abs(du/dy), abs(dv/dy))
    Real uvScreenSize;
    Real meanCurvature;   // For ray differential propagation.
    Real rayRadius;       // For ray differential propagation.
    int shapeId     = -1;
    int primitiveId = -1; // For triangle meshes. This indicates which triangle it hits.
    int materialId  = -1;

    // If the path vertex is inside a medium, these two IDs
    // are the same.
    int interiorMediumId = -1;
    int exteriorMediumId = -1;
};

/// Intersect a ray with a scene. If the ray doesn't hit anything,
/// returns an invalid optional output.
std::optional<PathVertex>
Intersect(const Scene& scene, const Ray& ray, const RayDifferential& ray_diff = RayDifferential{});

/// Test is a ray segment Intersect with anything in a scene.
bool Occluded(const Scene& scene, const Ray& ray);

/// Computes the Emission at a path vertex v, with the viewing direction
/// pointing outwards of the intersection.
Spectrum Emission(const PathVertex& v, const Vector3& view_dir, const Scene& scene);

} // namespace elma