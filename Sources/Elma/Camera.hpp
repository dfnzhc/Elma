#pragma once

#include "Elma.hpp"
#include "Filter.hpp"
#include "Matrix.hpp"
#include "Vector.hpp"
#include "Ray.hpp"

namespace elma {

/// Currently we only support a pinhole Perspective camera
struct Camera
{
    Camera() { }

    Camera(const Matrix4x4& cam_to_world,
           Real fov, // in degree
           int width,
           int height,
           const Filter& filter,
           int medium_id);

    Matrix4x4 sampleToCam, camToSample;
    Matrix4x4 camToWorld, worldToCam;
    int width, height;
    Filter filter;

    int mediumId; // for participating media rendering in homework 2
};

/// 从 [0, 1] x [0, 1] 的屏幕位置生成相机光线(Primary Ray)
Ray SamplePrimary(const Camera& camera, const Vector2& screen_pos);

} // namespace elma