#pragma once

#include "Elma.hpp"
#include "Matrix.hpp"
#include "Shape.hpp"

/// Parse Stanford PLY files.
TriangleMesh parse_ply(const fs::path& filename, const Matrix4x4& to_world);
