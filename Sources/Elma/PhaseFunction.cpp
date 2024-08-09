#include <variant>
#include "PhaseFunction.hpp"

namespace elma {

struct EvalOp
{
    Spectrum operator()(const IsotropicPhase& p) const;
    Spectrum operator()(const HenyeyGreenstein& p) const;

    const Vector3& dirIn;
    const Vector3& dirOut;
};

struct SamplePhaseFunctionOp
{
    std::optional<Vector3> operator()(const IsotropicPhase& p) const;
    std::optional<Vector3> operator()(const HenyeyGreenstein& p) const;

    const Vector3& dirIn;
    const Vector2& randParam;
};

struct PdfSamplePhaseOp
{
    Real operator()(const IsotropicPhase& p) const;
    Real operator()(const HenyeyGreenstein& p) const;

    const Vector3& dirIn;
    const Vector3& dirOut;
};

#include "PhaseFunctions/Isotropic.inl"
#include "PhaseFunctions/HenyeyGreenstein.inl"

Spectrum Eval(const std::variant<IsotropicPhase, HenyeyGreenstein>& phase_function,
              const Vector3& dir_in,
              const Vector3& dir_out)
{
    return std::visit(EvalOp{dir_in, dir_out}, phase_function);
}

std::optional<Vector3> SamplePhaseFunction(const std::variant<IsotropicPhase, HenyeyGreenstein>& phase_function,
                                           const Vector3& dir_in,
                                           const Vector2& rnd_param)
{
    return std::visit(SamplePhaseFunctionOp{dir_in, rnd_param}, phase_function);
}

Real PdfSamplePhase(const std::variant<IsotropicPhase, HenyeyGreenstein>& phase_function,
                    const Vector3& dir_in,
                    const Vector3& dir_out)
{
    return std::visit(PdfSamplePhaseOp{dir_in, dir_out}, phase_function);
}

} // namespace elma