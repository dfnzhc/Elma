#pragma once

#include "Elma.hpp"
#include "Spectrum.hpp"
#include "Vector.hpp"
#include <optional>
#include <variant>

namespace elma {
struct IsotropicPhase
{ };

struct HenyeyGreenstein
{
    Real g;
};

using PhaseFunction = std::variant<IsotropicPhase, HenyeyGreenstein>;

Spectrum eval(const PhaseFunction& phase_function, const Vector3& dir_in, const Vector3& dir_out);

std::optional<Vector3>
sample_phase_function(const PhaseFunction& phase_function, const Vector3& dir_in, const Vector2& rnd_param);

Real pdf_sample_phase(const PhaseFunction& phase_function, const Vector3& dir_in, const Vector3& dir_out);

} // namespace elma