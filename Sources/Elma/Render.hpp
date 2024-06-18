#pragma once

#include "Elma.hpp"
#include "Image.hpp"
#include <memory>

struct Scene;

Image3 render(const Scene& scene);
