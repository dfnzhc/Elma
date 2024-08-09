#pragma once

#include "Elma.hpp"
#include "Image.hpp"
#include "Intersection.hpp"
#include "Mipmap.hpp"
#include <map>
#include <variant>

namespace elma {
/// Can be replaced by a more advanced texture caching system,
/// where we only load images from files when necessary.
/// See OpenImageIO for example https://github.com/OpenImageIO/oiio
struct TexturePool
{
    std::map<std::string, int> image1sMap;
    std::map<std::string, int> image3sMap;

    std::vector<Mipmap1> image1s;
    std::vector<Mipmap3> image3s;
};

inline bool TextureIdExists(const TexturePool& pool, const std::string& texture_name)
{
    return pool.image1sMap.find(texture_name) != pool.image1sMap.end() ||
           pool.image3sMap.find(texture_name) != pool.image3sMap.end();
}

inline int InsertImage1(TexturePool& pool, const std::string& texture_name, const fs::path& filename)
{
    if (pool.image1sMap.find(texture_name) != pool.image1sMap.end()) {
        // We don't check if img is the same as the one in the cache!
        return pool.image1sMap[texture_name];
    }
    int id                        = (int)pool.image1s.size();
    pool.image1sMap[texture_name] = id;
    pool.image1s.push_back(MakeMipmap(ImageRead1(filename)));
    return id;
}

inline int InsertImage1(TexturePool& pool, const std::string& texture_name, const Image1& img)
{
    if (pool.image1sMap.find(texture_name) != pool.image1sMap.end()) {
        // We don't check if img is the same as the one in the cache!
        return pool.image1sMap[texture_name];
    }
    int id                        = (int)pool.image1s.size();
    pool.image1sMap[texture_name] = id;
    pool.image1s.push_back(MakeMipmap(img));
    return id;
}

inline int InsertImage3(TexturePool& pool, const std::string& texture_name, const fs::path& filename)
{
    if (pool.image3sMap.find(texture_name) != pool.image3sMap.end()) {
        // We don't check if img is the same as the one in the cache!
        return pool.image3sMap[texture_name];
    }
    int id                        = (int)pool.image3s.size();
    pool.image3sMap[texture_name] = id;
    pool.image3s.push_back(MakeMipmap(ImageRead3(filename)));
    return id;
}

inline int InsertImage3(TexturePool& pool, const std::string& texture_name, const Image3& img)
{
    if (pool.image3sMap.find(texture_name) != pool.image3sMap.end()) {
        // We don't check if img is the same as the one in the cache!
        return pool.image3sMap[texture_name];
    }
    int id                        = (int)pool.image3s.size();
    pool.image3sMap[texture_name] = id;
    pool.image3s.push_back(MakeMipmap(img));
    return id;
}

inline const Mipmap1& GetImage1(const TexturePool& pool, int texture_id)
{
    assert(texture_id >= 0 && texture_id < (int)pool.image1s.size());
    return pool.image1s[texture_id];
}

inline const Mipmap3& GetImage3(const TexturePool& pool, int texture_id)
{
    assert(texture_id >= 0 && texture_id < (int)pool.image3s.size());
    return pool.image3s[texture_id];
}

template<typename T> struct ConstantTexture
{
    T value;
};

template<typename T> struct ImageTexture
{
    int texture_id;
    Real uScale, vScale;
    Real uOffset, vOffset;
};

template<typename T> struct CheckerboardTexture
{
    T color0, color1;
    Real uScale, vScale;
    Real uOffset, vOffset;
};

template<typename T> inline const Mipmap<T>& GetImage(const ImageTexture<T>& t, const TexturePool& pool)
{
    return Mipmap<T>{};
}

template<> inline const Mipmap<Real>& GetImage(const ImageTexture<Real>& t, const TexturePool& pool)
{
    return GetImage1(pool, t.texture_id);
}

template<> inline const Mipmap<Vector3>& GetImage(const ImageTexture<Vector3>& t, const TexturePool& pool)
{
    return GetImage3(pool, t.texture_id);
}

template<typename T> using Texture = std::variant<ConstantTexture<T>, ImageTexture<T>, CheckerboardTexture<T>>;
using Texture1                     = Texture<Real>;
using TextureSpectrum              = Texture<Spectrum>;

template<typename T> struct EvalTextureOp
{
    T operator()(const ConstantTexture<T>& t) const;
    T operator()(const ImageTexture<T>& t) const;
    T operator()(const CheckerboardTexture<T>& t) const;

    const Vector2& uv;
    const Real& footprint;
    const TexturePool& pool;
};

template<typename T> T EvalTextureOp<T>::operator()(const ConstantTexture<T>& t) const
{
    return t.value;
}

template<typename T> T EvalTextureOp<T>::operator()(const ImageTexture<T>& t) const
{
    const auto& img = GetImage(t, pool);
    Vector2 local_uv{Modulo(uv[0] * t.uScale + t.uOffset, Real(1)), Modulo(uv[1] * t.vScale + t.vOffset, Real(1))};
    Real scaled_footprint = Max(GetWidth(img), GetHeight(img)) * Max(t.uScale, t.vScale) * footprint;
    Real level            = std::log2(Max(scaled_footprint, Real(1e-8f)));
    return Lookup(img, local_uv[0], local_uv[1], level);
}

template<typename T> T EvalTextureOp<T>::operator()(const CheckerboardTexture<T>& t) const
{
    Vector2 local_uv{Modulo(uv[0] * t.uScale + t.uOffset, Real(1)), Modulo(uv[1] * t.vScale + t.vOffset, Real(1))};
    int x = 2 * Modulo((int)(local_uv.x * 2), 2) - 1, y = 2 * Modulo((int)(local_uv.y * 2), 2) - 1;

    if (x * y == 1) {
        return t.color0;
    }
    else {
        return t.color1;
    }
}

/// Evaluate the texture at location uv.
/// Footprint should be approximatedly Min(du/dx, du/dy, dv/dx, dv/dy) for texture filtering.
template<typename T> T Eval(const Texture<T>& texture, const Vector2& uv, Real footprint, const TexturePool& pool)
{
    return std::visit(EvalTextureOp<T>{uv, footprint, pool}, texture);
}

inline ConstantTexture<Spectrum> MakeConstantSpectrumTexture(const Spectrum& spec)
{
    return ConstantTexture<Spectrum>{spec};
}

inline ConstantTexture<Real> MakeConstantFloatTexture(Real f)
{
    return ConstantTexture<Real>{f};
}

inline ImageTexture<Spectrum> MakeImageSpectrumTexture(const std::string& texture_name,
                                                       const fs::path& filename,
                                                       TexturePool& pool,
                                                       Real uscale  = 1,
                                                       Real vscale  = 1,
                                                       Real uoffset = 0,
                                                       Real voffset = 0)
{
    return ImageTexture<Spectrum>{InsertImage3(pool, texture_name, filename), uscale, vscale, uoffset, voffset};
}

inline ImageTexture<Spectrum> MakeImageSpectrumTexture(const std::string& texture_name,
                                                       const Image3& img,
                                                       TexturePool& pool,
                                                       Real uscale  = 1,
                                                       Real vscale  = 1,
                                                       Real uoffset = 0,
                                                       Real voffset = 0)
{
    return ImageTexture<Spectrum>{InsertImage3(pool, texture_name, img), uscale, vscale, uoffset, voffset};
}

inline ImageTexture<Real> MakeImageFloatTexture(const std::string& texture_name,
                                                const fs::path& filename,
                                                TexturePool& pool,
                                                Real uscale  = 1,
                                                Real vscale  = 1,
                                                Real uoffset = 0,
                                                Real voffset = 0)
{
    return ImageTexture<Real>{InsertImage1(pool, texture_name, filename), uscale, vscale, uoffset, voffset};
}

inline ImageTexture<Real> MakeImageFloatTexture(const std::string& texture_name,
                                                const Image1& img,
                                                TexturePool& pool,
                                                Real uscale  = 1,
                                                Real vscale  = 1,
                                                Real uoffset = 0,
                                                Real voffset = 0)
{
    return ImageTexture<Real>{InsertImage1(pool, texture_name, img), uscale, vscale, uoffset, voffset};
}

inline CheckerboardTexture<Spectrum> MakeCheckerboardSpectrumTexture(const Spectrum& color0,
                                                                     const Spectrum& color1,
                                                                     Real uscale  = 1,
                                                                     Real vscale  = 1,
                                                                     Real uoffset = 0,
                                                                     Real voffset = 0)
{
    return CheckerboardTexture<Spectrum>{color0, color1, uscale, vscale, uoffset, voffset};
}

inline CheckerboardTexture<Real> MakeCheckerboardFloatTexture(
    Real color0, Real color1, Real uscale = 1, Real vscale = 1, Real uoffset = 0, Real voffset = 0)
{
    return CheckerboardTexture<Real>{color0, color1, uscale, vscale, uoffset, voffset};
}

} // namespace elma