#pragma once

#include "Elma.hpp"
#include "Image.hpp"
#include <memory>

namespace elma {
struct Scene;

Image3 render(const Scene& scene);

} // namespace elma