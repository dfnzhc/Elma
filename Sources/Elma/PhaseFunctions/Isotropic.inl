Spectrum EvalOp::operator()(const IsotropicPhase&) const
{
    return MakeConstSpectrum(kInvFourPi);
}

std::optional<Vector3> SamplePhaseFunctionOp::operator()(const IsotropicPhase&) const
{
    // Uniform sphere sampling
    Real z   = 1 - 2 * randParam.x;
    Real r   = std::sqrt(std::fmax(Real(0), 1 - z * z));
    Real phi = 2 * kPi * randParam.y;
    return Vector3{r * std::cos(phi), r * std::sin(phi), z};
}

Real PdfSamplePhaseOp::operator()(const IsotropicPhase&) const
{
    return kInvFourPi;
}
