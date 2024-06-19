#include "Filter.hpp"

namespace elma {

struct sample_op
{
    Vector2 operator()(const Box& filter) const;
    Vector2 operator()(const Tent& filter) const;
    Vector2 operator()(const Gaussian& filter) const;

    const Vector2& rnd_param;
};

// Implementations of the individual filters.
#include "Filters/Box.inl"
#include "Filters/Tent.inl"
#include "Filters/Gaussian.inl"

Vector2 sample(const Filter& filter, const Vector2& rnd_param)
{
    return std::visit(sample_op{rnd_param}, filter);
}

} // namespace elma