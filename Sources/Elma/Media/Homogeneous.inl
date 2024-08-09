Spectrum get_majorant_op::operator()(const HomogeneousMedium& m)
{
    return m.sigmaA + m.sigmaS;
}

Spectrum get_sigma_s_op::operator()(const HomogeneousMedium& m)
{
    return m.sigmaS;
}

Spectrum get_sigma_a_op::operator()(const HomogeneousMedium& m)
{
    return m.sigmaA;
}
