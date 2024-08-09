#pragma once

#include "Elma.hpp"
#include "Vector.hpp"
#include "Filter.hpp"
#include <vector>

namespace elma {
/// TableDist1D stores a tabular discrete distribution
/// that we can Sample from using the functions below.
/// Useful for light source sampling.
struct TableDist1D
{
    std::vector<Real> pmf;
    std::vector<Real> cdf;
};

/// Construct the tabular discrete distribution given a vector of positive numbers.
TableDist1D MakeTableDist1d(const std::vector<Real>& f);

/// Sample an entry from the discrete table given a random number in [0, 1]
int Sample(const TableDist1D& table, Real rnd_param);

/// The probability mass function of the sampling procedure above.
Real Pmf(const TableDist1D& table, int id);

/// TableDist2D stores a 2D piecewise constant distribution
/// that we can Sample from using the functions below.
/// Useful for envmap sampling.
struct TableDist2D
{
    // cdf_rows & pdf_rows store a 1D piecewise constant distribution
    // for each row.
    std::vector<Real> cdfRows, pdfRows;
    // cdf_maringlas & pdf_marginals store a single 1D piecewise
    // constant distribution for sampling a row
    std::vector<Real> cdfMarginals, pdfMarginals;
    Real totalValues;
    int width, height;
};

/// Construct the 2D piecewise constant distribution given a vector of positive numbers
/// and width & height.
TableDist2D MakeTableDist2d(const std::vector<Real>& f, int width, int height);

/// Given two random number in [0, 1]^2, Sample a point in the 2D domain [0, 1]^2
/// with distribution proportional to f above.
Vector2 Sample(const TableDist2D& table, const Vector2& rnd_param);

/// Probability density of the sampling procedure above.
Real Pdf(const TableDist2D& table, const Vector2& xy);

} // namespace elma