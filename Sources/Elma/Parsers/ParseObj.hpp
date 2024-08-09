#pragma once

#include "Elma.hpp"
#include "Matrix.hpp"
#include "Shape.hpp"

namespace elma {

/// Parse Wavefront obj files. Currently only supports triangles and quads.
/// Throw errors if encountered general polygons.
TriangleMesh ParseObj(const fs::path& filename, const Matrix4x4& to_world);

} // namespace elma
