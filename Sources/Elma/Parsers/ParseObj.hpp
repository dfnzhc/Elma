#pragma once

#include "Elma.hpp"
#include "Matrix.hpp"
#include "Shape.hpp"

/// Parse Wavefront obj files. Currently only supports triangles and quads.
/// Throw errors if encountered general polygons.
TriangleMesh parse_obj(const fs::path& filename, const Matrix4x4& to_world);
