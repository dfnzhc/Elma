#pragma once

#include "Elma.hpp"
#include "Matrix.hpp"
#include "Vector.hpp"

namespace elma {
/// A collection of 3D transformations.
Matrix4x4 Translate(const Vector3& delta);
Matrix4x4 Scale(const Vector3& scale);
Matrix4x4 Rotate(Real angle, const Vector3& axis);
Matrix4x4 LookAt(const Vector3& pos, const Vector3& look, const Vector3& up);
Matrix4x4 Perspective(Real fov);
/// Actually transform the vectors given a transformation.
Vector3 TransformPoint(const Matrix4x4& xform, const Vector3& pt);
Vector3 TransformVector(const Matrix4x4& xform, const Vector3& vec);
Vector3 TransformNormal(const Matrix4x4& inv_xform, const Vector3& n);

} // namespace elma