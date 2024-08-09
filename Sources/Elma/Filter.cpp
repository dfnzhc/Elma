#include "Filter.hpp"

namespace elma {

struct SampleOp
{
    Vector2 operator()(const Box& filter) const;
    Vector2 operator()(const Tent& filter) const;
    Vector2 operator()(const Gaussian& filter) const;

    const Vector2& rndParam;
};

// Implementations of the individual filters.
#include "Filters/Box.inl"
#include "Filters/Tent.inl"
#include "Filters/Gaussian.inl"

Vector2 Sample(const std::variant<Box, Tent, Gaussian>& filter, const Vector2& rnd_param)
{
    return std::visit(SampleOp{rnd_param}, filter);
}

} // namespace elma