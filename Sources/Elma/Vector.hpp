#pragma once

#include "Elma.hpp"
#include <cmath>

namespace elma {

template<typename T> struct TVector2
{
    TVector2() { }

    template<typename T2> TVector2(T2 x, T2 y) : x(T(x)), y(T(y)) { }

    template<typename T2> TVector2(const TVector2<T2>& v) : x(T(v.x)), y(T(v.y)) { }

    T& operator[](int i) { return *(&x + i); }

    T operator[](int i) const { return *(&x + i); }

    T x, y;
};

template<typename T> struct TVector3
{
    TVector3() { }

    template<typename T2> TVector3(T2 x, T2 y, T2 z) : x(T(x)), y(T(y)), z(T(z)) { }

    template<typename T2> TVector3(const TVector3<T2>& v) : x(T(v.x)), y(T(v.y)), z(T(v.z)) { }

    T& operator[](int i) { return *(&x + i); }

    T operator[](int i) const { return *(&x + i); }

    T x, y, z;
};

template<typename T> struct TVector4
{
    TVector4() { }

    template<typename T2> TVector4(T2 x, T2 y, T2 z, T2 w) : x(T(x)), y(T(y)), z(T(z)), w(T(w)) { }

    template<typename T2> TVector4(const TVector4<T2>& v) : x(T(v.x)), y(T(v.y)), z(T(v.z)), w(T(v.w)) { }

    T& operator[](int i) { return *(&x + i); }

    T operator[](int i) const { return *(&x + i); }

    T x, y, z, w;
};

using Vector2f = TVector2<float>;
using Vector2d = TVector2<double>;
using Vector2i = TVector2<int>;
using Vector2  = TVector2<Real>;
using Vector3i = TVector3<int>;
using Vector3f = TVector3<float>;
using Vector3d = TVector3<double>;
using Vector3  = TVector3<Real>;
using Vector4f = TVector4<float>;
using Vector4d = TVector4<double>;
using Vector4  = TVector4<Real>;
using Vector4u = TVector4<uint8_t>;

template<typename T> inline TVector2<T> operator+(const TVector2<T>& v0, const TVector2<T>& v1)
{
    return TVector2<T>(v0.x + v1.x, v0.y + v1.y);
}

template<typename T> inline TVector2<T> operator-(const TVector2<T>& v0, const TVector2<T>& v1)
{
    return TVector2<T>(v0.x - v1.x, v0.y - v1.y);
}

template<typename T> inline TVector2<T> operator-(const TVector2<T>& v, Real s)
{
    return TVector2<T>(v.x - s, v.y - s);
}

template<typename T> inline TVector2<T> operator-(Real s, const TVector2<T>& v)
{
    return TVector2<T>(s - v.x, s - v.y);
}

template<typename T> inline TVector2<T> operator*(const T& s, const TVector2<T>& v)
{
    return TVector2<T>(s * v[0], s * v[1]);
}

template<typename T> inline TVector2<T> operator*(const TVector2<T>& v, const T& s)
{
    return TVector2<T>(v[0] * s, v[1] * s);
}

template<typename T> inline TVector2<T> operator/(const TVector2<T>& v, const T& s)
{
    return TVector2<T>(v[0] / s, v[1] / s);
}

template<typename T> inline TVector3<T> operator+(const TVector3<T>& v0, const TVector3<T>& v1)
{
    return TVector3<T>(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z);
}

template<typename T> inline TVector3<T> operator+(const TVector3<T>& v, const T& s)
{
    return TVector3<T>(v.x + s, v.y + s, v.z + s);
}

template<typename T> inline TVector3<T> operator+(const T& s, const TVector3<T>& v)
{
    return TVector3<T>(s + v.x, s + v.y, s + v.z);
}

template<typename T> inline TVector3<T>& operator+=(TVector3<T>& v0, const TVector3<T>& v1)
{
    v0.x += v1.x;
    v0.y += v1.y;
    v0.z += v1.z;
    return v0;
}

template<typename T> inline TVector3<T> operator-(const TVector3<T>& v0, const TVector3<T>& v1)
{
    return TVector3<T>(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
}

template<typename T> inline TVector3<T> operator-(Real s, const TVector3<T>& v)
{
    return TVector3<T>(s - v.x, s - v.y, s - v.z);
}

template<typename T> inline TVector3<T> operator-(const TVector3<T>& v, Real s)
{
    return TVector3<T>(v.x - s, v.y - s, v.z - s);
}

template<typename T> inline TVector3<T> operator-(const TVector3<T>& v)
{
    return TVector3<T>(-v.x, -v.y, -v.z);
}

template<typename T> inline TVector3<T> operator*(const T& s, const TVector3<T>& v)
{
    return TVector3<T>(s * v[0], s * v[1], s * v[2]);
}

template<typename T> inline TVector3<T> operator*(const TVector3<T>& v, const T& s)
{
    return TVector3<T>(v[0] * s, v[1] * s, v[2] * s);
}

template<typename T> inline TVector3<T> operator*(const TVector3<T>& v0, const TVector3<T>& v1)
{
    return TVector3<T>(v0[0] * v1[0], v0[1] * v1[1], v0[2] * v1[2]);
}

template<typename T> inline TVector3<T>& operator*=(TVector3<T>& v, const T& s)
{
    v[0] *= s;
    v[1] *= s;
    v[2] *= s;
    return v;
}

template<typename T> inline TVector3<T>& operator*=(TVector3<T>& v0, const TVector3<T>& v1)
{
    v0[0] *= v1[0];
    v0[1] *= v1[1];
    v0[2] *= v1[2];
    return v0;
}

template<typename T> inline TVector3<T> operator/(const TVector3<T>& v, const T& s)
{
    T inv_s = T(1) / s;
    return TVector3<T>(v[0] * inv_s, v[1] * inv_s, v[2] * inv_s);
}

template<typename T> inline TVector3<T> operator/(const T& s, const TVector3<T>& v)
{
    return TVector3<T>(s / v[0], s / v[1], s / v[2]);
}

template<typename T> inline TVector3<T> operator/(const TVector3<T>& v0, const TVector3<T>& v1)
{
    return TVector3<T>(v0[0] / v1[0], v0[1] / v1[1], v0[2] / v1[2]);
}

template<typename T> inline TVector3<T>& operator/=(TVector3<T>& v, const T& s)
{
    T inv_s  = T(1) / s;
    v       *= inv_s;
    return v;
}

template<typename T> inline T Dot(const TVector3<T>& v0, const TVector3<T>& v1)
{
    return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];
}

template<typename T> inline TVector3<T> Cross(const TVector3<T>& v0, const TVector3<T>& v1)
{
    return TVector3<T>{v0[1] * v1[2] - v0[2] * v1[1], v0[2] * v1[0] - v0[0] * v1[2], v0[0] * v1[1] - v0[1] * v1[0]};
}

template<typename T> inline T DistanceSquared(const TVector3<T>& v0, const TVector3<T>& v1)
{
    return Dot(v0 - v1, v0 - v1);
}

template<typename T> inline T Distance(const TVector3<T>& v0, const TVector3<T>& v1)
{
    return std::sqrt(DistanceSquared(v0, v1));
}

template<typename T> inline T LengthSquared(const TVector3<T>& v)
{
    return Dot(v, v);
}

template<typename T> inline T Length(const TVector3<T>& v)
{
    return std::sqrt(LengthSquared(v));
}

template<typename T> inline TVector3<T> Normalize(const TVector3<T>& v0)
{
    auto l = Length(v0);
    if (l <= 0) {
        return TVector3<T>{0, 0, 0};
    }
    else {
        return v0 / l;
    }
}

template<typename T> inline T Average(const TVector3<T>& v)
{
    return (v.x + v.y + v.z) / 3;
}

template<typename T> inline T Max(const TVector3<T>& v)
{
    return std::max(std::max(v.x, v.y), v.z);
}

template<typename T> inline TVector3<T> Max(const TVector3<T>& v0, const TVector3<T>& v1)
{
    return TVector3<T>{std::max(v0.x, v1.x), std::max(v0.y, v1.y), std::max(v0.z, v1.z)};
}

template<typename T> inline bool IsNan(const TVector2<T>& v)
{
    return std::isnan(v[0]) || std::isnan(v[1]);
}

template<typename T> inline bool IsNan(const TVector3<T>& v)
{
    return std::isnan(v[0]) || std::isnan(v[1]) || std::isnan(v[2]);
}

template<typename T> inline bool IsFinite(const TVector2<T>& v)
{
    return std::isfinite(v[0]) || std::isfinite(v[1]);
}

template<typename T> inline bool IsFinite(const TVector3<T>& v)
{
    return std::isfinite(v[0]) || std::isfinite(v[1]) || std::isfinite(v[2]);
}

template<typename T> inline std::ostream& operator<<(std::ostream& os, const TVector2<T>& v)
{
    return os << "(" << v[0] << ", " << v[1] << ")";
}

template<typename T> inline std::ostream& operator<<(std::ostream& os, const TVector3<T>& v)
{
    return os << "(" << v[0] << ", " << v[1] << ", " << v[2] << ")";
}

inline Real CosTheta(const Vector3& w)
{
    return w.z;
}

inline Real Cos2Theta(const Vector3& w)
{
    return Sqr(w.z);
}

inline Real AbsCosTheta(const Vector3& w)
{
    return std::abs(w.z);
}

inline Real Sin2Theta(const Vector3& w)
{
    return std::max<Real>(0, 1 - Cos2Theta(w));
}

inline Real SinTheta(const Vector3& w)
{
    return std::sqrt(Sin2Theta(w));
}

inline Real TanTheta(const Vector3& w)
{
    return SinTheta(w) / CosTheta(w);
}

inline Real Tan2Theta(const Vector3& w)
{
    return Sin2Theta(w) / Cos2Theta(w);
}

inline Real CosPhi(const Vector3& w)
{
    const auto sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 1 : Clamp(w.x / sinTheta, -1, 1);
}

inline Real SinPhi(const Vector3& w)
{
    const auto sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 0 : Clamp(w.y / sinTheta, -1, 1);
}

template<typename T> inline T AbsDot(const TVector3<T>& v0, const TVector3<T>& v1)
{
    return std::abs(Dot(v0, v1));
}

inline Vector2 SampleUniformDiskPolar(const Vector2& uv)
{
    const auto r     = std::sqrt(uv[0]);
    const auto theta = 2 * kPi * uv[1];

    return {r * std::cos(theta), r * std::sin(theta)};
}

template<typename T> inline T LengthSquared(const TVector2<T>& v)
{
    return Sqr(v.x) + Sqr(v.y);
}

template<typename T>
inline TVector3<T> Reflect(const TVector3<T>& in, const TVector3<T>& normal)
{
    return Normalize(-in + kTwo<Real> * Dot(in, normal) * normal);
}


} // namespace elma