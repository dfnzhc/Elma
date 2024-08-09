#include "Scene.hpp"
#include "TableDist.hpp"

namespace elma {

Scene::Scene(const RTCDevice& embree_device,
             const Camera& camera,
             const std::vector<Material>& materials,
             const std::vector<Shape>& shapes,
             const std::vector<Light>& lights,
             const std::vector<Medium>& media,
             int envmap_light_id,
             const TexturePool& texture_pool,
             const RenderOptions& options,
             const std::string& output_filename)
: embreeDevice(embree_device),
  camera(camera),
  materials(materials),
  shapes(shapes),
  lights(lights),
  media(media),
  envmapLightId(envmap_light_id),
  texturePool(texture_pool),
  options(options),
  outputFilename(output_filename)
{
    // Register the geometry to Embree
    embreeScene = rtcNewScene(embree_device);
    // We don't care about build time.
    rtcSetSceneBuildQuality(embreeScene, RTC_BUILD_QUALITY_HIGH);
    rtcSetSceneFlags(embreeScene, RTC_SCENE_FLAG_ROBUST);
    for (const Shape& shape : this->shapes) {
        RegisterEmbree(shape, embree_device, embreeScene);
    }
    rtcCommitScene(embreeScene);

    // Get scene bounding box from Embree
    RTCBounds embree_bounds;
    rtcGetSceneBounds(embreeScene, &embree_bounds);
    Vector3 lb{embree_bounds.lower_x, embree_bounds.lower_y, embree_bounds.lower_z};
    Vector3 ub{embree_bounds.upper_x, embree_bounds.upper_y, embree_bounds.upper_z};
    bounds = BSphere{Distance(ub, lb) / 2, (lb + ub) / Real(2)};

    // build shape & light sampling distributions if necessary
    // TODO: const_cast is a bit ugly...
    std::vector<Shape>& mod_shapes = const_cast<std::vector<Shape>&>(this->shapes);
    for (Shape& shape : mod_shapes) {
        InitSamplingDist(shape);
    }
    std::vector<Light>& mod_lights = const_cast<std::vector<Light>&>(this->lights);
    for (Light& light : mod_lights) {
        InitSamplingDist(light, *this);
    }

    // build a sampling distributino for all the lights
    std::vector<Real> power(this->lights.size());
    for (int i = 0; i < (int)this->lights.size(); i++) {
        power[i] = LightPower(this->lights[i], *this);
    }
    lightDist = MakeTableDist1d(power);
}

Scene::~Scene()
{
    // This decreses the reference count of embree_scene in Embree,
    // if it reaches zero, Embree will deallocate the scene.
    rtcReleaseScene(embreeScene);
}

int SampleLight(const Scene& scene, Real u)
{
    return Sample(scene.lightDist, u);
}

Real LightPmf(const Scene& scene, int light_id)
{
    return Pmf(scene.lightDist, light_id);
}

} // namespace elma