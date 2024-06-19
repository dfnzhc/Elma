#pragma once

#include "Elma.hpp"
#include "Filter.hpp"
#include "Matrix.hpp"
#include "Vector.hpp"
#include "Ray.hpp"

namespace elma {

/// Currently we only support a pinhole perspective camera
struct Camera
{
    Camera() { }

    Camera(const Matrix4x4& cam_to_world,
           Real fov, // in degree
           int width,
           int height,
           const Filter& filter,
           int medium_id);

    Matrix4x4 sample_to_cam, cam_to_sample;
    Matrix4x4 cam_to_world, world_to_cam;
    int width, height;
    Filter filter;

    int medium_id; // for participating media rendering in homework 2
};

/// Given screen position in [0, 1] x [0, 1],
/// generate a camera ray.
Ray sample_primary(const Camera& camera, const Vector2& screen_pos);

} // namespace elma