#include "Render.hpp"
#include "Intersection.hpp"
#include "Material.hpp"
#include "Parallel.hpp"
#include "PathTracing.hpp"
#include "VolPathTracing.hpp"
#include "Pcg.hpp"
#include "ProgressReporter.hpp"
#include "Scene.hpp"
#include "Common/Error.hpp"

namespace elma {

/// Render auxiliary buffers e.g., depth.
Image3 AuxRender(const Scene& scene)
{
    int w = scene.camera.width, h = scene.camera.height;
    Image3 img(w, h);

    constexpr int tile_size = 16;
    int num_tiles_x         = (w + tile_size - 1) / tile_size;
    int num_tiles_y         = (h + tile_size - 1) / tile_size;

    ParallelFor(
        [&](const Vector2i& tile) {
            int x0 = tile[0] * tile_size;
            int x1 = Min(x0 + tile_size, w);
            int y0 = tile[1] * tile_size;
            int y1 = Min(y0 + tile_size, h);
            for (int y = y0; y < y1; y++) {
                for (int x = x0; x < x1; x++) {
                    Ray ray = SamplePrimary(scene.camera, Vector2((x + Real(0.5)) / w, (y + Real(0.5)) / h));
                    RayDifferential ray_diff = InitRayDifferential(w, h);
                    if (std::optional<PathVertex> vertex = Intersect(scene, ray, ray_diff)) {
                        Real dist = Distance(vertex->position, ray.org);
                        Vector3 color{0, 0, 0};
                        if (scene.options.integrator == Integrator::Depth) {
                            color = Vector3{dist, dist, dist};
                        }
                        else if (scene.options.integrator == Integrator::ShadingNormal) {
                            // color = (vertex->shading_frame.n + Vector3{1, 1, 1}) / Real(2);
                            color = vertex->shadingFrame.n;
                        }
                        else if (scene.options.integrator == Integrator::MeanCurvature) {
                            Real kappa = vertex->meanCurvature;
                            color      = Vector3{kappa, kappa, kappa};
                        }
                        else if (scene.options.integrator == Integrator::RayDifferential) {
                            color = Vector3{ray_diff.radius, ray_diff.spread, Real(0)};
                        }
                        else if (scene.options.integrator == Integrator::MipmapLevel) {
                            const auto& mat     = scene.materials[vertex->materialId];
                            const auto& texture = GetTexture(mat);
                            auto* t             = std::get_if<ImageTexture<Spectrum>>(&texture);
                            if (t != nullptr) {
                                const Mipmap3& mipmap = GetImage3(scene.texturePool, t->texture_id);
                                Vector2 uv{Modulo(vertex->uv[0] * t->uScale, Real(1)),
                                           Modulo(vertex->uv[1] * t->vScale, Real(1))};
                                // ray_diff.radius stores approximatedly dpdx,
                                // but we want dudx -- we get it through
                                // dpdx / dpdu
                                Real footprint = vertex->uvScreenSize;
                                Real scaled_footprint =
                                    Max(GetWidth(mipmap), GetHeight(mipmap)) * Max(t->uScale, t->vScale) * footprint;
                                Real level = log2(Max(scaled_footprint, Real(1e-8f)));
                                color      = Vector3{level, level, level};
                            }
                        }
                        img(x, y) = color;
                    }
                    else {
                        img(x, y) = Vector3{0, 0, 0};
                    }
                }
            }
        },
        Vector2i(num_tiles_x, num_tiles_y));

    return img;
}

Image3 PathRender(const Scene& scene)
{
    int w = scene.camera.width, h = scene.camera.height;
    Image3 img(w, h);

    constexpr int tile_size = 16;
    int num_tiles_x         = (w + tile_size - 1) / tile_size;
    int num_tiles_y         = (h + tile_size - 1) / tile_size;
    int num_acc             = scene.options.accumulateCount;

    ProgressReporter reporter(num_tiles_x * num_tiles_y);
    ParallelFor(
        [&](const Vector2i& tile) {
            // Use a different rng stream for each thread.
            const auto idx = tile[1] * num_tiles_x + tile[0];
            Pcg32State rng = InitPcg32(idx, wyhash64(wyhash64(num_acc) + idx));
            int x0         = tile[0] * tile_size;
            int x1         = Min(x0 + tile_size, w);
            int y0         = tile[1] * tile_size;
            int y1         = Min(y0 + tile_size, h);
            for (int y = y0; y < y1; y++) {
                for (int x = x0; x < x1; x++) {
                    Spectrum radiance = MakeZeroSpectrum();
                    int spp           = scene.options.samplesPerPixel;
                    for (int s = 0; s < spp; s++) {
                        radiance += PathTracing(scene, x, y, rng);
                    }
                    img(x, y) = radiance / Real(spp);
                }
            }
            reporter.update(1);
        },
        Vector2i(num_tiles_x, num_tiles_y));
    reporter.done();
    return img;
}

Image3 VolPathRender(const Scene& scene)
{
    int w = scene.camera.width, h = scene.camera.height;
    Image3 img(w, h);

    constexpr int tile_size = 16;
    int num_tiles_x         = (w + tile_size - 1) / tile_size;
    int num_tiles_y         = (h + tile_size - 1) / tile_size;

    auto f = VolPathTracing;
    if (scene.options.volPathVersion == 1) {
        f = VolPathTracing1;
    }
    else if (scene.options.volPathVersion == 2) {
        f = VolPathTracing2;
    }
    else if (scene.options.volPathVersion == 3) {
        f = VolPathTracing3;
    }
    else if (scene.options.volPathVersion == 4) {
        f = VolPathTracing4;
    }
    else if (scene.options.volPathVersion == 5) {
        f = VolPathTracing5;
    }
    else if (scene.options.volPathVersion == 6) {
        f = VolPathTracing;
    }

    ProgressReporter reporter(num_tiles_x * num_tiles_y);
    ParallelFor(
        [&](const Vector2i& tile) {
            // Use a different rng stream for each thread.
            Pcg32State rng = InitPcg32(tile[1] * num_tiles_x + tile[0]);
            int x0         = tile[0] * tile_size;
            int x1         = Min(x0 + tile_size, w);
            int y0         = tile[1] * tile_size;
            int y1         = Min(y0 + tile_size, h);
            for (int y = y0; y < y1; y++) {
                for (int x = x0; x < x1; x++) {
                    Spectrum radiance = MakeZeroSpectrum();
                    int spp           = scene.options.samplesPerPixel;
                    for (int s = 0; s < spp; s++) {
                        Spectrum L = f(scene, x, y, rng);
                        if (IsFinite(L)) {
                            // Hacky: exclude NaNs in the rendering.
                            radiance += L;
                        }
                    }
                    img(x, y) = radiance / Real(spp);
                }
            }
            reporter.update(1);
        },
        Vector2i(num_tiles_x, num_tiles_y));
    reporter.done();
    return img;
}

Image3 Render(const Scene& scene)
{
    if (scene.options.integrator == Integrator::Depth || scene.options.integrator == Integrator::ShadingNormal ||
        scene.options.integrator == Integrator::MeanCurvature ||
        scene.options.integrator == Integrator::RayDifferential || scene.options.integrator == Integrator::MipmapLevel)
    {
        return AuxRender(scene);
    }
    else if (scene.options.integrator == Integrator::Path) {
        return PathRender(scene);
    }
    else if (scene.options.integrator == Integrator::VolPath) {
        return VolPathRender(scene);
    }
    else {
        ELMA_UNREACHABLE();
        return {};
    }
}

} // namespace elma