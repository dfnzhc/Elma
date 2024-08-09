#pragma once

#include "Elma.hpp"
#include "Vector.hpp"
#include <vector>

namespace elma {

// Numerical robust computation of angle between unit vectors
inline Real UnitAngle(const Vector3& u, const Vector3& v)
{
    if (Dot(u, v) < 0)
        return (kPi - 2) * asin(Real(0.5) * Length(v + u));
    else
        return 2 * asin(Real(0.5) * Length(v - u));
}

inline std::vector<Vector3> ComputeNormal(const std::vector<Vector3>& vertices, const std::vector<Vector3i>& indices)
{
    std::vector<Vector3> normals(vertices.size(), Vector3{0, 0, 0});

    // Nelson Max, "Computing Vertex Normals from Facet Normals", 1999
    for (auto& index : indices) {
        Vector3 n = Vector3{0, 0, 0};
        for (int i = 0; i < 3; ++i) {
            const Vector3& v0 = vertices[index[i]];
            const Vector3& v1 = vertices[index[(i + 1) % 3]];
            const Vector3& v2 = vertices[index[(i + 2) % 3]];
            Vector3 side1 = v1 - v0, side2 = v2 - v0;
            if (i == 0) {
                n      = Cross(side1, side2);
                Real l = Length(n);
                if (l == 0) {
                    break;
                }
                n = n / l;
            }
            Real angle        = UnitAngle(normalize(side1), normalize(side2));
            normals[index[i]] = normals[index[i]] + n * angle;
        }
    }

    for (auto& n : normals) {
        Real l = Length(n);
        if (l != 0) {
            n = n / l;
        }
        else {
            // degenerate normals, set it to 0
            n = Vector3{0, 0, 0};
        }
    }
    return normals;
}

} // namespace elma