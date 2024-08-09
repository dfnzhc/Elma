#pragma once

#include "Elma.hpp"
#include "Matrix.hpp"
#include "Shape.hpp"

namespace elma {

/// Parse Stanford PLY files.
TriangleMesh ParsePLY(const fs::path& filename, const Matrix4x4& to_world);

} // namespace elma
