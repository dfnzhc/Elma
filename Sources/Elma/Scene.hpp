#pragma once

#include "Elma.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "Material.hpp"
#include "Medium.hpp"
#include "Shape.hpp"
#include "Volume.hpp"

#include <memory>
#include <vector>

namespace elma {
enum class Integrator
{
    Depth,
    ShadingNormal,
    MeanCurvature,
    RayDifferential, // visualize radius & spread
    MipmapLevel,
    Path,
    VolPath
};

struct RenderOptions
{
    Integrator integrator = Integrator::Path;
    int samplesPerPixel   = 4;
    int accumulateCount   = 0;
    int maxDepth          = -1;
    int rrDepth           = 5;
    int volPathVersion    = 0;
    int maxNullCollisions = 1'000;
};

/// Bounding sphere
struct BSphere
{
    Real radius;
    Vector3 center;
};

/// A "Scene" contains the camera, materials, geometry (shapes), lights,
/// and also the rendering options such as number of samples per pixel or
/// the parameters of our renderer.
struct Scene
{
    Scene() { }

    Scene(const RTCDevice& embree_device,
          const Camera& camera,
          const std::vector<Material>& materials,
          const std::vector<Shape>& shapes,
          const std::vector<Light>& lights,
          const std::vector<Medium>& media,
          int envmap_light_id, /* -1 if the scene has no envmap */
          const TexturePool& texture_pool,
          const RenderOptions& options,
          const std::string& output_filename);
    ~Scene();
    Scene(const Scene& t)            = delete;
    Scene& operator=(const Scene& t) = delete;

    RTCDevice embreeDevice;
    RTCScene embreeScene;
    // We decide to maintain a copy of the scene here.
    // This allows us to manage the memory of the scene ourselves and decouple
    // from the scene parser, but it's obviously less efficient.
    Camera camera;
    // For now we use stl vectors to store scene content.
    // This wouldn't work if we want to extend this to run on GPUs.
    // If we want to port this to GPUs later, we need to maintain a thrust vector or something similar.
    const std::vector<Material> materials;
    const std::vector<Shape> shapes;
    const std::vector<Light> lights;
    const std::vector<Medium> media;
    int envmapLightId;
    const TexturePool texturePool;

    // Bounding sphere of the scene.
    BSphere bounds;

    RenderOptions options;
    std::string outputFilename;

    // For sampling lights
    TableDist1D lightDist;
};

/// Sample a light source from the scene given a random number u \in [0, 1]
int SampleLight(const Scene& scene, Real u);

/// The probability mass function of the sampling procedure above.
Real LightPmf(const Scene& scene, int light_id);

inline bool HasEnvmap(const Scene& scene)
{
    return scene.envmapLightId != -1;
}

inline const Light& GetEnvmap(const Scene& scene)
{
    assert(scene.envmapLightId != -1);
    return scene.lights[scene.envmapLightId];
}

inline Real GetShadowEpsilon(const Scene& scene)
{
    return Min(scene.bounds.radius * Real(1e-5), Real(0.01));
}

inline Real GetIntersectionEpsilon(const Scene& scene)
{
    return Min(scene.bounds.radius * Real(1e-5), Real(0.01));
}

} // namespace elma