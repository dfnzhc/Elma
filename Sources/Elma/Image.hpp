#pragma once

#include "Vector.hpp"

#include <string>
#include <cstring>
#include <vector>

namespace elma {

/// A N-channel image stored in a contiguous vector
/// The storage format is HWC -- outer dimension is height
/// then width, then channels.
template<typename T> struct Image
{
    Image() { }

    Image(int w, int h) : width(w), height(h)
    {
        data.resize(w * h);
        memset(data.data(), 0, sizeof(T) * data.size());
    }

    T& operator()(int x) { return data[x]; }

    const T& operator()(int x) const { return data[x]; }

    T& operator()(int x, int y) { return data[y * width + x]; }

    const T& operator()(int x, int y) const { return data[y * width + x]; }

    int width;
    int height;
    std::vector<T> data;
};

using Image1  = Image<Real>;
using Image3  = Image<Vector3>;
using Image3f = Image<Vector3f>;
using Image4  = Image<Vector4>;
using Image4u = Image<Vector4u>;
using Image4f = Image<Vector4f>;

/// Read from an 1 channel image. If the image is not actually
/// single channel, the first channel is used.
/// Supported formats: JPG, PNG, TGA, BMP, PSD, GIF, HDR, PIC
Image1 ImageRead1(const fs::path& filename);
/// Read from a 3 channels image.
/// If the image only has 1 channel, we set all 3 channels to the same color.
/// If the image has more than 3 channels, we truncate it to 3.
/// Undefined behavior if the image has 2 channels (does that even happen?)
/// Supported formats: JPG, PNG, TGA, BMP, PSD, GIF, HDR, PIC
Image3 ImageRead3(const fs::path& filename);

/// Save an image to a file.
/// Supported formats: PFM & exr
void ImageWrite(const fs::path& filename, const Image3& image);

inline Image3 ToImage3(const Image1& img)
{
    Image3 out(img.width, img.height);
    std::transform(img.data.cbegin(), img.data.cend(), out.data.begin(), [](Real v) { return Vector3(v, v, v); });
    return out;
}

inline Image1 ToImage1(const Image3& img)
{
    Image1 out(img.width, img.height);
    std::transform(img.data.cbegin(), img.data.cend(), out.data.begin(), [](const Vector3& v) { return Average(v); });
    return out;
}

} // namespace elma