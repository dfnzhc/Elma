#pragma once

#include "Elma.hpp"
#include "Scene.hpp"
#include <string>
#include <memory>

namespace elma {

/// Parse Mitsuba's XML scene format.
std::unique_ptr<Scene> parse_scene(const fs::path& filename, const RTCDevice& embree_device);

} // namespace elma
