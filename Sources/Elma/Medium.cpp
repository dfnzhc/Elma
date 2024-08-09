#include <variant>
#include "Medium.hpp"

namespace elma {
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

Spectrum GetMajorant(const std::variant<HomogeneousMedium, HeterogeneousMedium>& medium, const Ray& ray)
{
    return std::visit(get_majorant_op{ray}, medium);
}

Spectrum GetSigmaS(const std::variant<HomogeneousMedium, HeterogeneousMedium>& medium, const Vector3& p)
{
    return std::visit(get_sigma_s_op{p}, medium);
}

Spectrum GetSigmaA(const std::variant<HomogeneousMedium, HeterogeneousMedium>& medium, const Vector3& p)
{
    return std::visit(get_sigma_a_op{p}, medium);
}

} // namespace elma