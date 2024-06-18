#include "Medium.hpp"

struct get_majorant_op
{
    Spectrum operator()(const HomogeneousMedium& m);
    Spectrum operator()(const HeterogeneousMedium& m);

    const Ray& ray;
};

struct get_sigma_s_op
{
    Spectrum operator()(const HomogeneousMedium& m);
    Spectrum operator()(const HeterogeneousMedium& m);

    const Vector3& p;
};

struct get_sigma_a_op
{
    Spectrum operator()(const HomogeneousMedium& m);
    Spectrum operator()(const HeterogeneousMedium& m);

    const Vector3& p;
};

#include "Media/Homogeneous.inl"
#include "Media/Heterogeneous.inl"

Spectrum get_majorant(const Medium& medium, const Ray& ray)
{
    return std::visit(get_majorant_op{ray}, medium);
}

Spectrum get_sigma_s(const Medium& medium, const Vector3& p)
{
    return std::visit(get_sigma_s_op{p}, medium);
}

Spectrum get_sigma_a(const Medium& medium, const Vector3& p)
{
    return std::visit(get_sigma_a_op{p}, medium);
}
