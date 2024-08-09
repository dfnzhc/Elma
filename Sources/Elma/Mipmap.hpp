#pragma once

#include "Elma.hpp"

namespace elma {
constexpr int kMaxMipmapLevels = 8;

template<typename T> struct Mipmap
{
    std::vector<Image<T>> images;
};

template<typename T> inline int GetWidth(const Mipmap<T>& mipmap)
{
    assert(mipmap.images.size() > 0);
    return mipmap.images[0].width;
}

template<typename T> inline int GetHeight(const Mipmap<T>& mipmap)
{
    assert(mipmap.images.size() > 0);
    return mipmap.images[0].height;
}

template<typename T> inline Mipmap<T> MakeMipmap(const Image<T>& img)
{
    Mipmap<T> mipmap;
    int size       = Max(img.width, img.height);
    int num_levels = std::min((int)std::ceil(std::log2(Real(size)) + 1), kMaxMipmapLevels);
    mipmap.images.push_back(img);
    for (int i = 1; i < num_levels; i++) {
        const Image<T>& prev_img = mipmap.images.back();
        int next_w               = Max(prev_img.width / 2, 1);
        int next_h               = Max(prev_img.height / 2, 1);
        Image<T> next_img(next_w, next_h);
        for (int y = 0; y < next_img.height; y++) {
            for (int x = 0; x < next_img.width; x++) {
                // 2x2 box filter
                next_img(x, y) = (prev_img(2 * x, 2 * y) + prev_img(2 * x + 1, 2 * y) + prev_img(2 * x, 2 * y + 1) +
                                  prev_img(2 * x + 1, 2 * y + 1)) /
                                 Real(4);
            }
        }
        mipmap.images.push_back(next_img);
    }
    return mipmap;
}

/// Bilinear Lookup of a mipmap at location (uv) with an integer level
template<typename T> inline T Lookup(const Mipmap<T>& mipmap, Real u, Real v, int level)
{
    assert(level >= 0 && level < (int)mipmap.images.size());
    // Bilinear interpolation
    // (-0.5 to match Mitsuba's coordinates)
    u          = u * mipmap.images[level].width - Real(0.5);
    v          = v * mipmap.images[level].height - Real(0.5);
    int ufi    = Modulo(int(u), mipmap.images[level].width);
    int vfi    = Modulo(int(v), mipmap.images[level].height);
    int uci    = Modulo(ufi + 1, mipmap.images[level].width);
    int vci    = Modulo(vfi + 1, mipmap.images[level].height);
    Real u_off = u - ufi;
    Real v_off = v - vfi;
    T val_ff   = mipmap.images[level](ufi, vfi);
    T val_fc   = mipmap.images[level](ufi, vci);
    T val_cf   = mipmap.images[level](uci, vfi);
    T val_cc   = mipmap.images[level](uci, vci);
    return val_ff * (1 - u_off) * (1 - v_off) + val_fc * (1 - u_off) * v_off + val_cf * u_off * (1 - v_off) +
           val_cc * u_off * v_off;
}

/// Trilinear look of of a mipmap at (u, v, level)
template<typename T> inline T Lookup(const Mipmap<T>& mipmap, Real u, Real v, Real level)
{
    if (level <= 0) {
        return Lookup(mipmap, u, v, 0);
    }
    else if (level < Real(mipmap.images.size() - 1)) {
        int flevel     = std::clamp((int)std::floor(level), 0, (int)mipmap.images.size() - 1);
        int clevel     = std::clamp(flevel + 1, 0, (int)mipmap.images.size() - 1);
        Real level_off = level - flevel;
        return Lookup(mipmap, u, v, flevel) * (1 - level_off) + Lookup(mipmap, u, v, clevel) * level_off;
    }
    else {
        return Lookup(mipmap, u, v, int(mipmap.images.size() - 1));
    }
}

using Mipmap1 = Mipmap<Real>;
using Mipmap3 = Mipmap<Vector3>;

} // namespace elma