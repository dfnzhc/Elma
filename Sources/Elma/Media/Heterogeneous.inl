#include "../Volume.hpp"

Spectrum get_majorant_op::operator()(const HeterogeneousMedium& m)
{
    if (Intersect(m.density, ray)) {
        return GetMaxValue(m.density);
    }
    else {
        return MakeZeroSpectrum();
    }
}

Spectrum get_sigma_s_op::operator()(const HeterogeneousMedium& m)
{
    Spectrum density = Lookup(m.density, p);
    Spectrum albedo  = Lookup(m.albedo, p);
    return density * albedo;
}

Spectrum get_sigma_a_op::operator()(const HeterogeneousMedium& m)
{
    Spectrum density = Lookup(m.density, p);
    Spectrum albedo  = Lookup(m.albedo, p);
    return density * (Real(1) - albedo);
}
