#pragma once

#include "Elma.hpp"
#include "Matrix.hpp"
#include "Shape.hpp"

/// Load Mitsuba's serialized file format.
TriangleMesh load_serialized(const fs::path& filename, int shape_index, const Matrix4x4& to_world);
