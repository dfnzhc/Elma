#include "ParseScene.hpp"
#include <pugixml.hpp>
#include "LoadSerialized.hpp"
#include "ParseObj.hpp"
#include "ParsePly.hpp"
#include "ShapeUtils.hpp"
#include "Transform.hpp"
#include <map>
#include <regex>
#include "Common/Error.hpp"

namespace elma {

const Real kDefaultFov             = 45.0;
const int kDefaultRes              = 256;
const std::string kDefaultFilename = "image.exr";
const Filter kDefaultFilter        = Box{Real(1)};
;

struct ParsedSampler
{
    int sampleCount = 4;
};

enum class TextureType
{
    BITMAP,
    CHECKERBOARD
};

struct ParsedTexture
{
    TextureType type;
    fs::path filename;
    Spectrum color0, color1; // for checkerboard
    Real uScale = 1, vScale = 1;
    Real uOffset = 0, vOffset = 0;
};

enum class FovAxis
{
    X,
    Y,
    DIAGONAL,
    SMALLER,
    LARGER
};

std::vector<std::string> SplitString(const std::string& str, const std::regex& delim_regex)
{
    std::sregex_token_iterator first{begin(str), end(str), delim_regex, -1}, last;
    std::vector<std::string> list{first, last};
    return list;
}

auto ParseDefaultMap(const std::string& value, const std::map<std::string, std::string>& default_map)
{
    if (!value.empty()) {
        if (value[0] == '$') {
            auto it = default_map.find(value.substr(1));
            if (it == default_map.end()) {
                ELMA_THROW("没有找到默认变量 '{}'。", value);
            }
            return it;
        }
    }
    return default_map.end();
}

bool ParseBoolean(const std::string& value)
{
    if (value == "true") {
        return true;
    }
    else if (value == "false") {
        return false;
    }
    else {
        ELMA_THROW("布尔值解析失败。");
        return false;
    }
}

bool ParseBoolean(const std::string& value, const std::map<std::string, std::string>& default_map)
{
    if (auto it = ParseDefaultMap(value, default_map); it != default_map.end()) {
        return ParseBoolean(it->second);
    }
    return ParseBoolean(value);
}

int ParseInteger(const std::string& value, const std::map<std::string, std::string>& default_map)
{
    if (auto it = ParseDefaultMap(value, default_map); it != default_map.end()) {
        return std::stoi(it->second);
    }
    return std::stoi(value);
}

Real ParseFloat(const std::string& value, const std::map<std::string, std::string>& default_map)
{
    if (auto it = ParseDefaultMap(value, default_map); it != default_map.end()) {
        return std::stof(it->second);
    }
    return std::stof(value);
}

std::string ParseString(const std::string& value, const std::map<std::string, std::string>& default_map)
{
    if (auto it = ParseDefaultMap(value, default_map); it != default_map.end()) {
        return it->second;
    }
    return value;
}

Vector3 ParseVector3(const std::string& value)
{
    std::vector<std::string> list = SplitString(value, std::regex("(,| )+"));
    Vector3 v;
    if (list.size() == 1) {
        v[0] = std::stof(list[0]);
        v[1] = std::stof(list[0]);
        v[2] = std::stof(list[0]);
    }
    else if (list.size() == 3) {
        v[0] = std::stof(list[0]);
        v[1] = std::stof(list[1]);
        v[2] = std::stof(list[2]);
    }
    else {
        ELMA_THROW("Vector3 解析失败。");
    }
    return v;
}

Vector3 ParseVector3(const std::string& value, const std::map<std::string, std::string>& default_map)
{
    if (auto it = ParseDefaultMap(value, default_map); it != default_map.end()) {
        return ParseVector3(it->second);
    }
    return ParseVector3(value);
}

Vector3 Parse_sRGB(const std::string& value)
{
    Vector3 srgb;
    if (value.size() == 7 && value[0] == '#') {
        char* end_ptr = nullptr;
        // parse hex code (#abcdef)
        int encoded = strtol(value.c_str() + 1, &end_ptr, 16);
        if (*end_ptr != '\0') {
            ELMA_THROW("无效的 sRGB 值：{}。", value);
        }
        srgb[0] = float((encoded & 0xFF0000) >> 16) / 255.0f;
        srgb[1] = float((encoded & 0x00FF00) >> 8) / 255.0f;
        srgb[2] = float(encoded & 0x0000FF) / 255.0f;
    }
    else {
        ELMA_THROW("未知的 sRGB 格式：{}。", value);
    }
    return srgb;
}

Vector3 Parse_sRGB(const std::string& value, const std::map<std::string, std::string>& default_map)
{
    if (auto it = ParseDefaultMap(value, default_map); it != default_map.end()) {
        return Parse_sRGB(it->second);
    }
    return Parse_sRGB(value);
}

std::vector<std::pair<Real, Real>> ParseSpectrum(const std::string& value)
{
    std::vector<std::string> list = SplitString(value, std::regex("(,| )+"));
    std::vector<std::pair<Real, Real>> s;
    if (list.size() == 1 && list[0].find(":") == std::string::npos) {
        // a single uniform value for all wavelength
        s.emplace_back(Real(-1), stof(list[0]));
    }
    else {
        for (const auto& val_str : list) {
            std::vector<std::string> pair = SplitString(val_str, std::regex(":"));
            if (pair.size() < 2) {
                ELMA_THROW("解析 Spectrum 失败。");
            }
            s.emplace_back(Real(stof(pair[0])), Real(stof(pair[1])));
        }
    }
    return s;
}

std::vector<std::pair<Real, Real>> ParseSpectrum(const std::string& value,
                                                 const std::map<std::string, std::string>& default_map)
{
    if (auto it = ParseDefaultMap(value, default_map); it != default_map.end()) {
        return ParseSpectrum(it->second);
    }
    return ParseSpectrum(value);
}

Matrix4x4 ParseMatrix4x4(const std::string& value)
{
    std::vector<std::string> list = SplitString(value, std::regex("(,| )+"));
    if (list.size() != 16) {
        ELMA_THROW("解析 Matrix4x4 失败。");
    }

    Matrix4x4 m;
    int k = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            m(i, j) = std::stof(list[k++]);
        }
    }
    return m;
}

Matrix4x4 ParseMatrix4x4(const std::string& value, const std::map<std::string, std::string>& default_map)
{
    if (auto it = ParseDefaultMap(value, default_map); it != default_map.end()) {
        return ParseMatrix4x4(it->second);
    }
    return ParseMatrix4x4(value);
}

Matrix4x4 ParseTransform(pugi::xml_node node, const std::map<std::string, std::string>& default_map)
{
    Matrix4x4 tform = Matrix4x4::identity();
    for (auto child : node.children()) {
        std::string name = ToLowercase(child.name());
        if (name == "scale") {
            Real x = 1.0;
            Real y = 1.0;
            Real z = 1.0;
            if (!child.attribute("x").empty()) {
                x = ParseFloat(child.attribute("x").value(), default_map);
            }
            if (!child.attribute("y").empty()) {
                y = ParseFloat(child.attribute("y").value(), default_map);
            }
            if (!child.attribute("z").empty()) {
                z = ParseFloat(child.attribute("z").value(), default_map);
            }
            if (!child.attribute("value").empty()) {
                Vector3 v = ParseVector3(child.attribute("value").value(), default_map);
                x         = v.x;
                y         = v.y;
                z         = v.z;
            }
            tform = Scale(Vector3{x, y, z}) * tform;
        }
        else if (name == "translate") {
            Real x = 0.0;
            Real y = 0.0;
            Real z = 0.0;
            if (!child.attribute("x").empty()) {
                x = ParseFloat(child.attribute("x").value(), default_map);
            }
            if (!child.attribute("y").empty()) {
                y = ParseFloat(child.attribute("y").value(), default_map);
            }
            if (!child.attribute("z").empty()) {
                z = ParseFloat(child.attribute("z").value(), default_map);
            }
            if (!child.attribute("value").empty()) {
                Vector3 v = ParseVector3(child.attribute("value").value(), default_map);
                x         = v.x;
                y         = v.y;
                z         = v.z;
            }
            tform = Translate(Vector3{x, y, z}) * tform;
        }
        else if (name == "rotate") {
            Real x     = 0.0;
            Real y     = 0.0;
            Real z     = 0.0;
            Real angle = 0.0;
            if (!child.attribute("x").empty()) {
                x = ParseFloat(child.attribute("x").value(), default_map);
            }
            if (!child.attribute("y").empty()) {
                y = ParseFloat(child.attribute("y").value(), default_map);
            }
            if (!child.attribute("z").empty()) {
                z = ParseFloat(child.attribute("z").value(), default_map);
            }
            if (!child.attribute("angle").empty()) {
                angle = ParseFloat(child.attribute("angle").value(), default_map);
            }
            tform = Rotate(angle, Vector3(x, y, z)) * tform;
        }
        else if (name == "lookat") {
            Vector3 pos    = ParseVector3(child.attribute("origin").value(), default_map);
            Vector3 target = ParseVector3(child.attribute("target").value(), default_map);
            Vector3 up     = ParseVector3(child.attribute("up").value(), default_map);
            tform          = LookAt(pos, target, up) * tform;
        }
        else if (name == "matrix") {
            Matrix4x4 trans = ParseMatrix4x4(std::string(child.attribute("value").value()), default_map);
            tform           = trans * tform;
        }
    }
    return tform;
}

Spectrum ParseColor(pugi::xml_node node, const std::map<std::string, std::string>& default_map)
{
    std::string type = node.name();
    if (type == "spectrum") {
        std::vector<std::pair<Real, Real>> spec = ParseSpectrum(node.attribute("value").value(), default_map);
        if (spec.size() > 1) {
            Vector3 xyz = IntegrateXYZ(spec);
            return FromRGB(XYZ_ToRGB(xyz));
        }
        else if (spec.size() == 1) {
            return FromRGB(Vector3{1, 1, 1});
        }
        else {
            return FromRGB(Vector3{0, 0, 0});
        }
    }
    else if (type == "rgb") {
        return FromRGB(ParseVector3(node.attribute("value").value(), default_map));
    }
    else if (type == "srgb") {
        Vector3 srgb = Parse_sRGB(node.attribute("value").value(), default_map);
        return FromRGB(sRGBToRGB(srgb));
    }
    else if (type == "float") {
        return MakeConstSpectrum(ParseFloat(node.attribute("value").value(), default_map));
    }
    else {
        ELMA_THROW("未知的颜色类型：{}。", type);
        return MakeZeroSpectrum();
    }
}

/// We don't load the images to memory at this stage. Only record their names.
ParsedTexture ParseTexture(pugi::xml_node node, const std::map<std::string, std::string>& default_map)
{
    std::string type = node.attribute("type").value();
    if (type == "bitmap") {
        std::string filename;
        Real uscale  = 1;
        Real vscale  = 1;
        Real uoffset = 0;
        Real voffset = 0;
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "filename") {
                filename = ParseString(child.attribute("value").value(), default_map);
            }
            else if (name == "uvscale") {
                uscale = vscale = ParseFloat(child.attribute("value").value(), default_map);
            }
            else if (name == "uscale") {
                uscale = ParseFloat(child.attribute("value").value(), default_map);
            }
            else if (name == "vscale") {
                vscale = ParseFloat(child.attribute("value").value(), default_map);
            }
            else if (name == "uoffset") {
                uoffset = ParseFloat(child.attribute("value").value(), default_map);
            }
            else if (name == "voffset") {
                voffset = ParseFloat(child.attribute("value").value(), default_map);
            }
        }
        return ParsedTexture{TextureType::BITMAP,
                             fs::path(filename),
                             MakeZeroSpectrum(),
                             MakeZeroSpectrum(),
                             uscale,
                             vscale,
                             uoffset,
                             voffset};
    }
    else if (type == "checkerboard") {
        Spectrum color0 = FromRGB(Vector3{Real(0.4), Real(0.4), Real(0.4)});
        Spectrum color1 = FromRGB(Vector3{Real(0.2), Real(0.2), Real(0.2)});
        Real uscale     = 1;
        Real vscale     = 1;
        Real uoffset    = 0;
        Real voffset    = 0;
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "color0") {
                color0 = ParseColor(child, default_map);
            }
            else if (name == "color1") {
                color1 = ParseColor(child, default_map);
            }
            else if (name == "uvscale") {
                uscale = vscale = ParseFloat(child.attribute("value").value(), default_map);
            }
            else if (name == "uscale") {
                uscale = ParseFloat(child.attribute("value").value(), default_map);
            }
            else if (name == "vscale") {
                vscale = ParseFloat(child.attribute("value").value(), default_map);
            }
            else if (name == "uoffset") {
                uoffset = ParseFloat(child.attribute("value").value(), default_map);
            }
            else if (name == "voffset") {
                voffset = ParseFloat(child.attribute("value").value(), default_map);
            }
        }
        return ParsedTexture{TextureType::CHECKERBOARD, "", color0, color1, uscale, vscale, uoffset, voffset};
    }
    ELMA_THROW("未知的纹理类型：{}。", type);
    return ParsedTexture{};
}

Texture<Spectrum> ParseSpectrumTexture(pugi::xml_node node,
                                       const std::map<std::string /* name id */, ParsedTexture>& texture_map,
                                       TexturePool& texture_pool,
                                       const std::map<std::string, std::string>& default_map)
{
    std::string type = node.name();
    if (type == "spectrum") {
        std::vector<std::pair<Real, Real>> spec = ParseSpectrum(node.attribute("value").value(), default_map);
        if (spec.size() > 1) {
            Vector3 xyz = IntegrateXYZ(spec);
            return MakeConstantSpectrumTexture(FromRGB(XYZ_ToRGB(xyz)));
        }
        else if (spec.size() == 1) {
            return MakeConstantSpectrumTexture(FromRGB(Vector3{1, 1, 1}));
        }
        else {
            return MakeConstantSpectrumTexture(FromRGB(Vector3{0, 0, 0}));
        }
    }
    else if (type == "rgb") {
        return MakeConstantSpectrumTexture(FromRGB(ParseVector3(node.attribute("value").value(), default_map)));
    }
    else if (type == "srgb") {
        Vector3 srgb = Parse_sRGB(node.attribute("value").value(), default_map);
        return MakeConstantSpectrumTexture(FromRGB(sRGBToRGB(srgb)));
    }
    else if (type == "ref") {
        // referencing a texture
        std::string ref_id = node.attribute("id").value();
        auto t_it          = texture_map.find(ref_id);
        if (t_it == texture_map.end()) {
            ELMA_THROW("未找到 Spectrum 纹理：ID = {}。", ref_id);
        }
        const ParsedTexture t = t_it->second;
        if (t.type == TextureType::BITMAP) {
            return MakeImageSpectrumTexture(ref_id, t.filename, texture_pool, t.uScale, t.vScale, t.uOffset, t.vOffset);
        }
        else if (t.type == TextureType::CHECKERBOARD) {
            return MakeCheckerboardSpectrumTexture(t.color0, t.color1, t.uScale, t.vScale, t.uOffset, t.vOffset);
        }
        else {
            return MakeConstantSpectrumTexture(FromRGB(Vector3{0, 0, 0}));
        }
    }
    else if (type == "texture") {
        ParsedTexture t = ParseTexture(node, default_map);
        // increment ref_id_counter until we can't find the name in texture_pool
        static int ref_id_counter = 0; // should switch to atomic<int> if we parse scenes using multiple threads
        std::string tmp_ref_name  = "$inline_spectrum_texture";
        while (TextureIdExists(texture_pool, tmp_ref_name + std::to_string(ref_id_counter))) {
            ref_id_counter++;
        }
        tmp_ref_name = tmp_ref_name + std::to_string(ref_id_counter);
        if (t.type == TextureType::BITMAP) {
            return MakeImageSpectrumTexture(
                tmp_ref_name, t.filename, texture_pool, t.uScale, t.vScale, t.uOffset, t.vOffset);
        }
        else if (t.type == TextureType::CHECKERBOARD) {
            return MakeCheckerboardSpectrumTexture(t.color0, t.color1, t.uScale, t.vScale, t.uOffset, t.vOffset);
        }
        else {
            return MakeConstantSpectrumTexture(FromRGB(Vector3{0, 0, 0}));
        }
    }
    else {
        ELMA_THROW("未找到 Spectrum 纹理类型：{}。", type);
        return ConstantTexture<Spectrum>{MakeZeroSpectrum()};
    }
}

Texture<Real> ParseFloatTexture(pugi::xml_node node,
                                const std::map<std::string /* name id */, ParsedTexture>& texture_map,
                                TexturePool& texture_pool,
                                const std::map<std::string, std::string>& default_map)
{
    std::string type = node.name();
    if (type == "ref") {
        // referencing a texture
        std::string ref_id = node.attribute("id").value();
        auto t_it          = texture_map.find(ref_id);
        if (t_it == texture_map.end()) {
            ELMA_THROW("未找到 Float 纹理：ID = {}。", type);
        }
        const ParsedTexture t = t_it->second;
        if (t.type == TextureType::BITMAP) {
            return MakeImageFloatTexture(ref_id, t.filename, texture_pool, t.uScale, t.vScale, t.uOffset, t.vOffset);
        }
        else if (t.type == TextureType::CHECKERBOARD) {
            return MakeCheckerboardFloatTexture(Avg(t.color0), Avg(t.color1), t.uScale, t.vScale, t.uOffset, t.vOffset);
        }
        else {
            return MakeConstantFloatTexture(0);
        }
    }
    else if (type == "float") {
        return MakeConstantFloatTexture(ParseFloat(node.attribute("value").value(), default_map));
    }
    else if (type == "texture") {
        ParsedTexture t = ParseTexture(node, default_map);
        // increment ref_id_counter until we can't find the name in texture_pool
        static int ref_id_counter = 0; // should switch to atomic<int> if we parse scenes using multiple threads
        std::string tmp_ref_name  = "$inline_float_texture";
        while (TextureIdExists(texture_pool, tmp_ref_name + std::to_string(ref_id_counter))) {
            ref_id_counter++;
        }
        tmp_ref_name = tmp_ref_name + std::to_string(ref_id_counter);
        if (t.type == TextureType::BITMAP) {
            return MakeImageFloatTexture(
                tmp_ref_name, t.filename, texture_pool, t.uScale, t.vScale, t.uOffset, t.vOffset);
        }
        else if (t.type == TextureType::CHECKERBOARD) {
            return MakeCheckerboardFloatTexture(Avg(t.color0), Avg(t.color1), t.uScale, t.vScale, t.uOffset, t.vOffset);
        }
        else {
            return MakeConstantFloatTexture(0);
        }
    }
    else {
        ELMA_THROW("未找到 Float 纹理：{}。", type);
        return MakeConstantFloatTexture(Real(0));
    }
}

Spectrum ParseIntensity(pugi::xml_node node, const std::map<std::string, std::string>& default_map)
{
    std::string rad_type = node.name();
    if (rad_type == "spectrum") {
        std::vector<std::pair<Real, Real>> spec = ParseSpectrum(node.attribute("value").value(), default_map);
        if (spec.size() == 1) {
            // For a light source, the white point is
            // XYZ(0.9505, 1.0, 1.0888) instead
            // or XYZ(1, 1, 1). We need to handle this special case when
            // we don't have the full spectrum data.
            Vector3 xyz{Real(0.9505), Real(1.0), Real(1.0888)};
            return FromRGB(XYZ_ToRGB(xyz * spec[0].second));
        }
        else {
            Vector3 xyz = IntegrateXYZ(spec);
            return FromRGB(XYZ_ToRGB(xyz));
        }
    }
    else if (rad_type == "rgb") {
        return FromRGB(ParseVector3(node.attribute("value").value(), default_map));
    }
    else if (rad_type == "srgb") {
        Vector3 srgb = Parse_sRGB(node.attribute("value").value(), default_map);
        return FromRGB(sRGBToRGB(srgb));
    }
    return MakeConstSpectrum(1);
}

void ParseDefaultMap(pugi::xml_node node, std::map<std::string, std::string>& default_map)
{
    if (node.attribute("name")) {
        std::string name = node.attribute("name").value();
        if (node.attribute("value")) {
            std::string value = node.attribute("value").value();
            default_map[name] = value;
        }
    }
}

RenderOptions ParseIntegrator(pugi::xml_node node, const std::map<std::string, std::string>& default_map)
{
    RenderOptions options;
    std::string type = node.attribute("type").value();
    if (type == "path") {
        options.integrator = Integrator::Path;
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "maxDepth") {
                options.maxDepth = ParseInteger(child.attribute("value").value(), default_map);
            }
            else if (name == "rrDepth") {
                options.rrDepth = ParseInteger(child.attribute("value").value(), default_map);
            }
        }
    }
    else if (type == "volpath") {
        options.integrator = Integrator::VolPath;
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "maxDepth" || name == "max_depth") {
                options.maxDepth = ParseInteger(child.attribute("value").value(), default_map);
            }
            else if (name == "rrDepth" || name == "rr_depth") {
                options.rrDepth = ParseInteger(child.attribute("value").value(), default_map);
            }
            else if (name == "version") {
                options.volPathVersion = ParseInteger(child.attribute("value").value(), default_map);
            }
            else if (name == "maxNullCollisions" || name == "max_null_collisions") {
                options.maxNullCollisions = ParseInteger(child.attribute("value").value(), default_map);
            }
        }
    }
    else if (type == "direct") {
        options.integrator = Integrator::Path;
        options.maxDepth   = 2;
    }
    else if (type == "depth") {
        options.integrator = Integrator::Depth;
    }
    else if (type == "shadingNormal" || type == "shading_normal") {
        options.integrator = Integrator::ShadingNormal;
    }
    else if (type == "meanCurvature" || type == "mean_curvature") {
        options.integrator = Integrator::MeanCurvature;
    }
    else if (type == "rayDifferential" || type == "ray_differential") {
        options.integrator = Integrator::RayDifferential;
    }
    else if (type == "mipmapLevel" || type == "mipmap_level") {
        options.integrator = Integrator::MipmapLevel;
    }
    else {
        ELMA_THROW("不支持的积分器(Integrator)类型：{}。", type);
    }
    return options;
}

std::tuple<int /* width */, int /* height */, std::string /* filename */, Filter>
ParseFilm(pugi::xml_node node, const std::map<std::string, std::string>& default_map)
{
    int width = kDefaultRes, height = kDefaultRes;
    std::string filename = kDefaultFilename;
    Filter filter        = kDefaultFilter;

    for (auto child : node.children()) {
        std::string type = child.name();
        std::string name = child.attribute("name").value();
        if (name == "width") {
            width = ParseInteger(child.attribute("value").value(), default_map);
        }
        else if (name == "height") {
            height = ParseInteger(child.attribute("value").value(), default_map);
        }
        else if (name == "filename") {
            filename = ParseString(child.attribute("value").value(), default_map);
        }
        if (type == "rfilter") {
            std::string filter_type = child.attribute("type").value();
            if (filter_type == "box") {
                Real filter_width = Real(1);
                for (auto grand_child : child.children()) {
                    if (std::string(grand_child.attribute("name").value()) == "width") {
                        filter_width = ParseFloat(grand_child.attribute("value").value(), default_map);
                    }
                }
                filter = Box{filter_width};
            }
            else if (filter_type == "tent") {
                Real filter_width = Real(2);
                for (auto grand_child : child.children()) {
                    if (std::string(grand_child.attribute("name").value()) == "width") {
                        filter_width = ParseFloat(grand_child.attribute("value").value(), default_map);
                    }
                }
                filter = Tent{filter_width};
            }
            else if (filter_type == "gaussian") {
                Real filter_stddev = Real(0.5);
                for (auto grand_child : child.children()) {
                    if (std::string(grand_child.attribute("name").value()) == "stddev") {
                        filter_stddev = ParseFloat(grand_child.attribute("value").value(), default_map);
                    }
                }
                filter = Gaussian{filter_stddev};
            }
        }
    }
    return std::make_tuple(width, height, filename, filter);
}

VolumeSpectrum ParseVolumeSpectrum(pugi::xml_node node, const std::map<std::string, std::string>& default_map)
{
    std::string type = node.attribute("type").value();
    if (type == "constvolume") {
        Spectrum value = MakeZeroSpectrum();
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "value") {
                value = ParseColor(child, default_map);
            }
        }
        return ConstantVolume<Spectrum>{value};
    }
    else if (type == "gridvolume") {
        std::string filename;
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "filename") {
                filename = ParseString(child.attribute("value").value(), default_map);
            }
        }
        if (filename.empty()) {
            ELMA_THROW("未设置 Grid Volume 的引用。");
        }
        return LoadVolumeFromFile<Spectrum>(filename);
    }
    else {
        ELMA_THROW("不支持的 Volume 类型：{}。", type);
    }
    return ConstantVolume<Spectrum>{MakeZeroSpectrum()};
}

PhaseFunction ParsePhaseFunction(pugi::xml_node node, const std::map<std::string, std::string>& default_map)
{
    std::string type = node.attribute("type").value();
    if (type == "isotropic") {
        return IsotropicPhase{};
    }
    else if (type == "hg") {
        Real g = 0;
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "g") {
                g = ParseFloat(child.attribute("value").value(), default_map);
            }
        }
        return HenyeyGreenstein{g};
    }
    else {
        ELMA_THROW("不支持的相位函数(Phase Function)类型：{}。", type);
    }
    return IsotropicPhase{};
}

std::tuple<std::string /* ID */, Medium> ParseMedium(pugi::xml_node node,
                                                     const std::map<std::string, std::string>& default_map)
{
    PhaseFunction phase_func = IsotropicPhase{};

    std::string type = node.attribute("type").value();
    std::string id;
    if (!node.attribute("id").empty()) {
        id = node.attribute("id").value();
    }
    if (type == "homogeneous") {
        Vector3 sigma_a{0.5, 0.5, 0.5};
        Vector3 sigma_s{0.5, 0.5, 0.5};
        Real scale = 1;
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "sigmaA" || name == "sigma_a") {
                sigma_a = ParseColor(child, default_map);
            }
            else if (name == "sigmaS" || name == "sigma_s") {
                sigma_s = ParseColor(child, default_map);
            }
            else if (name == "scale") {
                scale = ParseFloat(child.attribute("value").value(), default_map);
            }
            else if (std::string(child.name()) == "phase") {
                phase_func = ParsePhaseFunction(child, default_map);
            }
        }
        return std::make_tuple(id, HomogeneousMedium{{phase_func}, sigma_a * scale, sigma_s * scale});
    }
    else if (type == "heterogeneous") {
        VolumeSpectrum albedo  = ConstantVolume<Spectrum>{MakeConstSpectrum(1)};
        VolumeSpectrum density = ConstantVolume<Spectrum>{MakeConstSpectrum(1)};
        Real scale             = 1;
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "albedo") {
                albedo = ParseVolumeSpectrum(child, default_map);
            }
            else if (name == "density") {
                density = ParseVolumeSpectrum(child, default_map);
            }
            else if (name == "scale") {
                scale = ParseFloat(child.attribute("value").value(), default_map);
            }
            else if (std::string(child.name()) == "phase") {
                phase_func = ParsePhaseFunction(child, default_map);
            }
        }
        // scale only applies to density!!
        SetScale(density, scale);
        return std::make_tuple(id, HeterogeneousMedium{{phase_func}, albedo, density});
    }
    else {
        ELMA_THROW("不支持的介质(Medium)类型：{}。", type);
    }
}

std::tuple<Camera, std::string /* output filename */, ParsedSampler>
ParseSensor(pugi::xml_node node,
            std::vector<Medium>& media,
            std::map<std::string /* name id */, int /* index id */>& medium_map,
            const std::map<std::string, std::string>& default_map)
{
    Real fov           = kDefaultFov;
    Matrix4x4 to_world = Matrix4x4::identity();
    int width = kDefaultRes, height = kDefaultRes;
    std::string filename = kDefaultFilename;
    Filter filter        = kDefaultFilter;
    FovAxis fov_axis     = FovAxis::X;
    ParsedSampler sampler;
    int medium_id = -1;

    std::string type = node.attribute("type").value();
    if (type == "perspective") {
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "fov") {
                fov = ParseFloat(child.attribute("value").value(), default_map);
            }
            else if (name == "toWorld" || name == "to_world") {
                to_world = ParseTransform(child, default_map);
            }
            else if (name == "fovAxis" || name == "fov_axis") {
                std::string value = child.attribute("value").value();
                if (value == "x") {
                    fov_axis = FovAxis::X;
                }
                else if (value == "y") {
                    fov_axis = FovAxis::Y;
                }
                else if (value == "diagonal") {
                    fov_axis = FovAxis::DIAGONAL;
                }
                else if (value == "smaller") {
                    fov_axis = FovAxis::SMALLER;
                }
                else if (value == "larger") {
                    fov_axis = FovAxis::LARGER;
                }
                else {
                    ELMA_THROW("不支持的 fovAxis 类型：{}。", type);
                }
            }
        }
    }
    else {
        ELMA_THROW("不支持的相机类型：{}。", type);
    }

    for (auto child : node.children()) {
        if (std::string(child.name()) == "film") {
            std::tie(width, height, filename, filter) = ParseFilm(child, default_map);
        }
        else if (std::string(child.name()) == "sampler") {
            std::string name = child.attribute("type").value();
            if (name != "independent") {
                std::cerr << "Warning: the renderer currently only supports independent samplers." << std::endl;
            }
            for (auto grand_child : child.children()) {
                std::string name = grand_child.attribute("name").value();
                if (name == "sampleCount" || name == "sample_count") {
                    sampler.sampleCount = ParseInteger(grand_child.attribute("value").value(), default_map);
                }
            }
        }
        else if (std::string(child.name()) == "ref") {
            // A reference to a medium
            pugi::xml_attribute id = child.attribute("id");
            if (id.empty()) {
                ELMA_THROW("未设置介质(Medium)的引用。");
            }
            auto it = medium_map.find(id.value());
            if (it == medium_map.end()) {
                ELMA_THROW("没有找到介质(Medium)引用 '{}'。", id.value());
            }
            medium_id = it->second;
        }
        else if (std::string(child.name()) == "medium") {
            Medium m;
            std::string medium_name;
            std::tie(medium_name, m) = ParseMedium(child, default_map);
            if (!medium_name.empty()) {
                medium_map[medium_name] = media.size();
            }
            std::string name_value = child.attribute("name").value();
            medium_id              = media.size();
            media.push_back(m);
        }
    }

    // convert to fovX (code taken from
    // https://github.com/mitsuba-renderer/mitsuba/blob/master/src/librender/sensor.cpp)
    if (fov_axis == FovAxis::Y || (fov_axis == FovAxis::SMALLER && height < width) ||
        (fov_axis == FovAxis::LARGER && width < height))
    {
        Real aspect = width / Real(height);
        fov         = Degrees(2 * std::atan(std::tan(Radians(fov) / 2) * aspect));
    }
    else if (fov_axis == FovAxis::DIAGONAL) {
        Real aspect   = width / Real(height);
        Real diagonal = 2 * std::tan(Radians(fov) / 2);
        Real width    = diagonal / std::sqrt(1 + 1 / (aspect * aspect));
        fov           = Degrees(2 * std::atan(width / 2));
    }

    return std::make_tuple(Camera(to_world, fov, width, height, filter, medium_id), filename, sampler);
}

Texture<Real> AlphaToRoughness(pugi::xml_node node,
                               const std::map<std::string /* name id */, ParsedTexture>& texture_map,
                               TexturePool& texture_pool,
                               const std::map<std::string, std::string>& default_map)
{
    // Alpha in microfacet models requires special treatment since we need to convert
    // the values to roughness
    std::string type = node.name();
    if (type == "ref") {
        // referencing a texture
        std::string ref_id = node.attribute("id").value();
        auto t_it          = texture_map.find(ref_id);
        if (t_it == texture_map.end()) {
            ELMA_THROW("未找到纹理：ID = {}。", ref_id);
        }
        const ParsedTexture t = t_it->second;
        if (t.type == TextureType::BITMAP) {
            Image1 alpha = ImageRead1(t.filename);
            // Convert alpha to roughness.
            Image1 roughness_img(alpha.width, alpha.height);
            for (int i = 0; i < alpha.width * alpha.height; i++) {
                roughness_img.data[i] = std::sqrt(alpha.data[i]);
            }
            return MakeImageFloatTexture(ref_id, roughness_img, texture_pool, t.uScale, t.vScale);
        }
        else if (t.type == TextureType::CHECKERBOARD) {
            Real roughness0 = std::sqrt(Avg(t.color0));
            Real roughness1 = std::sqrt(Avg(t.color1));
            return MakeCheckerboardFloatTexture(roughness0, roughness1, t.uScale, t.vScale, t.uOffset, t.vOffset);
        }
        else {
            return MakeConstantFloatTexture(Real(0.1));
        }
    }
    else if (type == "float") {
        Real alpha = ParseFloat(node.attribute("value").value(), default_map);
        return elma::MakeConstantFloatTexture(std::sqrt(alpha));
    }
    else if (type == "texture") {
        ParsedTexture t = ParseTexture(node, default_map);
        // increment ref_id_counter until we can't find the name in texture_pool
        static int ref_id_counter = 0; // should switch to atomic<int> if we parse scenes using multiple threads
        std::string tmp_ref_name  = "$inline_alpha_texture";
        while (TextureIdExists(texture_pool, tmp_ref_name + std::to_string(ref_id_counter))) {
            ref_id_counter++;
        }
        tmp_ref_name = tmp_ref_name + std::to_string(ref_id_counter);
        if (t.type == TextureType::BITMAP) {
            Image1 alpha = ImageRead1(t.filename);
            // Convert alpha to roughness.
            Image1 roughness_img(alpha.width, alpha.height);
            for (int i = 0; i < alpha.width * alpha.height; i++) {
                roughness_img.data[i] = std::sqrt(alpha.data[i]);
            }
            return MakeImageFloatTexture(
                tmp_ref_name, t.filename, texture_pool, t.uScale, t.vScale, t.uOffset, t.vOffset);
        }
        else if (t.type == TextureType::CHECKERBOARD) {
            Real roughness0 = std::sqrt(Avg(t.color0));
            Real roughness1 = std::sqrt(Avg(t.color1));
            return MakeCheckerboardFloatTexture(roughness0, roughness1, t.uScale, t.vScale, t.uOffset, t.vOffset);
        }
        else {
            return MakeConstantFloatTexture(Real(0.1));
        }
    }
    else {
        ELMA_THROW("不支持的 Float 纹理类型 '{}'。", type);
    }
}

std::tuple<std::string /* ID */, Material>
ParseBSDF(pugi::xml_node node,
          const std::map<std::string /* name id */, ParsedTexture>& texture_map,
          TexturePool& texture_pool,
          const std::map<std::string, std::string>& default_map,
          const std::string& parent_id = "")
{
    std::string type = node.attribute("type").value();
    std::string id   = parent_id;
    if (!node.attribute("id").empty()) {
        id = node.attribute("id").value();
    }
    if (type == "twosided") {
        // In lajolla, all BSDFs are twosided.
        for (auto child : node.children()) {
            if (std::string(child.name()) == "bsdf") {
                return ParseBSDF(child, texture_map, texture_pool, default_map, id);
            }
        }
    }
    else if (type == "diffuse") {
        Texture<Spectrum> reflectance = MakeConstantSpectrumTexture(FromRGB(Vector3{0.5, 0.5, 0.5}));
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "reflectance") {
                reflectance = ParseSpectrumTexture(child, texture_map, texture_pool, default_map);
            }
        }
        return std::make_tuple(id, Lambertian{reflectance});
    }
    else if (type == "roughplastic" || type == "plastic") {
        Texture<Spectrum> diffuse_reflectance  = MakeConstantSpectrumTexture(FromRGB(Vector3{0.5, 0.5, 0.5}));
        Texture<Spectrum> specular_reflectance = MakeConstantSpectrumTexture(FromRGB(Vector3{1, 1, 1}));
        Texture<Real> roughness                = MakeConstantFloatTexture(Real(0.1));
        if (type == "plastic") {
            // Approximate plastic materials with very small roughness
            roughness = MakeConstantFloatTexture(Real(0.01));
        }
        Real intIOR = 1.49;
        Real extIOR = 1.000277;
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "diffuseReflectance" || name == "diffuse_reflectance") {
                diffuse_reflectance = ParseSpectrumTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "specularReflectance" || name == "specular_reflectance") {
                specular_reflectance = ParseSpectrumTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "alpha") {
                roughness = AlphaToRoughness(child, texture_map, texture_pool, default_map);
            }
            else if (name == "roughness") {
                roughness = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "intIOR" || name == "int_ior") {
                intIOR = ParseFloat(child.attribute("value").value(), default_map);
            }
            else if (name == "extIOR" || name == "ext_ior") {
                extIOR = ParseFloat(child.attribute("value").value(), default_map);
            }
        }
        return std::make_tuple(id, RoughPlastic{diffuse_reflectance, specular_reflectance, roughness, intIOR / extIOR});
    }
    else if (type == "roughdielectric" || type == "dielectric") {
        Texture<Spectrum> specular_reflectance   = MakeConstantSpectrumTexture(FromRGB(Vector3{1, 1, 1}));
        Texture<Spectrum> specular_transmittance = MakeConstantSpectrumTexture(FromRGB(Vector3{1, 1, 1}));
        Texture<Real> roughness                  = MakeConstantFloatTexture(Real(0.1));
        if (type == "dielectric") {
            // Approximate plastic materials with very small roughness
            roughness = MakeConstantFloatTexture(Real(0.01));
        }
        Real intIOR = 1.5046;
        Real extIOR = 1.000277;
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "specularReflectance" || name == "specular_reflectance") {
                specular_reflectance = ParseSpectrumTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "specularTransmittance" || name == "specular_transmittance") {
                specular_transmittance = ParseSpectrumTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "alpha") {
                roughness = AlphaToRoughness(child, texture_map, texture_pool, default_map);
            }
            else if (name == "roughness") {
                roughness = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "intIOR" || name == "int_ior") {
                intIOR = ParseFloat(child.attribute("value").value(), default_map);
            }
            else if (name == "extIOR" || name == "ext_ior") {
                extIOR = ParseFloat(child.attribute("value").value(), default_map);
            }
        }
        return std::make_tuple(
            id, RoughDielectric{specular_reflectance, specular_transmittance, roughness, intIOR / extIOR});
    }
    else if (type == "disneydiffuse") {
        Texture<Spectrum> base_color = MakeConstantSpectrumTexture(FromRGB(Vector3{0.5, 0.5, 0.5}));
        Texture<Real> roughness      = MakeConstantFloatTexture(Real(0.5));
        Texture<Real> subsurface     = MakeConstantFloatTexture(Real(0));
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "baseColor" || name == "base_color") {
                base_color = ParseSpectrumTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "roughness") {
                roughness = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "subsurface") {
                subsurface = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
        }
        return std::make_tuple(id, DisneyDiffuse{base_color, roughness, subsurface});
    }
    else if (type == "disneymetal") {
        Texture<Spectrum> base_color = MakeConstantSpectrumTexture(FromRGB(Vector3{0.5, 0.5, 0.5}));
        Texture<Real> roughness      = MakeConstantFloatTexture(Real(0.5));
        Texture<Real> anisotropic    = MakeConstantFloatTexture(Real(0.0));
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "baseColor" || name == "base_color") {
                base_color = ParseSpectrumTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "roughness") {
                roughness = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "anisotropic") {
                anisotropic = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
        }
        return std::make_tuple(id, DisneyMetal{base_color, roughness, anisotropic});
    }
    else if (type == "disneyglass") {
        Texture<Spectrum> base_color = MakeConstantSpectrumTexture(FromRGB(Vector3{0.5, 0.5, 0.5}));
        Texture<Real> roughness      = MakeConstantFloatTexture(Real(0.5));
        Texture<Real> anisotropic    = MakeConstantFloatTexture(Real(0.0));
        Real eta                     = Real(1.5);
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "baseColor" || name == "base_color") {
                base_color = ParseSpectrumTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "roughness") {
                roughness = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "anisotropic") {
                anisotropic = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "eta") {
                eta = ParseFloat(child.attribute("value").value(), default_map);
            }
        }
        return std::make_tuple(id, DisneyGlass{base_color, roughness, anisotropic, eta});
    }
    else if (type == "disneyclearcoat") {
        Texture<Real> clearcoat_gloss = MakeConstantFloatTexture(Real(1.0));
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "clearcoatGloss") {
                clearcoat_gloss = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
        }
        return std::make_tuple(id, DisneyClearcoat{clearcoat_gloss});
    }
    else if (type == "disneysheen") {
        Texture<Spectrum> base_color = MakeConstantSpectrumTexture(FromRGB(Vector3{0.5, 0.5, 0.5}));
        Texture<Real> sheen_tint     = MakeConstantFloatTexture(Real(0.5));
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "baseColor" || name == "base_color") {
                base_color = ParseSpectrumTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "sheenTint" || name == "sheen_tint") {
                sheen_tint = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
        }
        return std::make_tuple(id, DisneySheen{base_color, sheen_tint});
    }
    else if (type == "disneybsdf" || type == "principled") {
        Texture<Spectrum> base_color        = MakeConstantSpectrumTexture(FromRGB(Vector3{0.5, 0.5, 0.5}));
        Texture<Real> specular_transmission = MakeConstantFloatTexture(Real(0));
        Texture<Real> metallic              = MakeConstantFloatTexture(Real(0));
        Texture<Real> subsurface            = MakeConstantFloatTexture(Real(0));
        Texture<Real> specular              = MakeConstantFloatTexture(Real(0.5));
        Texture<Real> roughness             = MakeConstantFloatTexture(Real(0.5));
        Texture<Real> specular_tint         = MakeConstantFloatTexture(Real(0));
        Texture<Real> anisotropic           = MakeConstantFloatTexture(Real(0));
        Texture<Real> sheen                 = MakeConstantFloatTexture(Real(0));
        Texture<Real> sheen_tint            = MakeConstantFloatTexture(Real(0.5));
        Texture<Real> clearcoat             = MakeConstantFloatTexture(Real(0));
        Texture<Real> clearcoat_gloss       = MakeConstantFloatTexture(Real(1));
        Real eta                            = Real(1.5);
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "baseColor" || name == "base_color") {
                base_color = ParseSpectrumTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "specularTransmission" || name == "specular_transmission" || name == "specTrans" ||
                     name == "spec_trans")
            {
                specular_transmission = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "metallic") {
                metallic = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "subsurface") {
                subsurface = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "specular") {
                specular = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "roughness") {
                roughness = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "specularTint" || name == "specular_tint" || name == "specTint" || name == "spec_tint") {
                specular_tint = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "anisotropic") {
                anisotropic = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "sheen") {
                sheen = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "sheenTint" || name == "sheen_tint") {
                sheen_tint = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "clearcoat") {
                clearcoat = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "clearcoatGloss" || name == "clearcoat_gloss") {
                clearcoat_gloss = ParseFloatTexture(child, texture_map, texture_pool, default_map);
            }
            else if (name == "eta") {
                eta = ParseFloat(child.attribute("value").value(), default_map);
            }
        }
        return std::make_tuple(id,
                               DisneyBSDF{base_color,
                                          specular_transmission,
                                          metallic,
                                          subsurface,
                                          specular,
                                          roughness,
                                          specular_tint,
                                          anisotropic,
                                          sheen,
                                          sheen_tint,
                                          clearcoat,
                                          clearcoat_gloss,
                                          eta});
    }
    else if (type == "null") {
        // TODO: implement actual null BSDF (the ray will need to pass through the shape)
        return std::make_tuple(id, Lambertian{MakeConstantSpectrumTexture(FromRGB(Vector3{0.0, 0.0, 0.0}))});
    }
    else {
        ELMA_THROW("不支持的 BSDF 类型 '{}'。", type);
    }
    return std::make_tuple("", Material{});
}

Shape parse_shape(pugi::xml_node node,
                  std::vector<Material>& materials,
                  std::map<std::string /* name id */, int /* index id */>& material_map,
                  const std::map<std::string /* name id */, ParsedTexture>& texture_map,
                  TexturePool& texture_pool,
                  std::vector<Medium>& media,
                  std::map<std::string /* name id */, int /* index id */>& medium_map,
                  std::vector<Light>& lights,
                  const std::vector<Shape>& shapes,
                  const std::map<std::string, std::string>& default_map)
{
    int material_id        = -1;
    int interior_medium_id = -1;
    int exterior_medium_id = -1;
    for (auto child : node.children()) {
        std::string name = child.name();
        if (name == "ref") {
            std::string name_value = child.attribute("name").value();
            pugi::xml_attribute id = child.attribute("id");
            if (id.empty()) {
                ELMA_THROW("未设置材质(Material)/介质(Medium)的索引。");
            }
            if (name_value == "interior") {
                auto it = medium_map.find(id.value());
                if (it == medium_map.end()) {
                    ELMA_THROW("没有找到介质(Medium)引用 '{}'。", id.value());
                }
                interior_medium_id = it->second;
            }
            else if (name_value == "exterior") {
                auto it = medium_map.find(id.value());
                if (it == medium_map.end()) {
                    ELMA_THROW("没有找到介质(Medium)引用 '{}'。", id.value());
                }
                exterior_medium_id = it->second;
            }
            else {
                auto it = material_map.find(id.value());
                if (it == material_map.end()) {
                    ELMA_THROW("没有找到介质(medium)引用 '{}'", id.value());
                }
                material_id = it->second;
            }
        }
        else if (name == "bsdf") {
            Material m;
            std::string material_name;
            std::tie(material_name, m) = ParseBSDF(child, texture_map, texture_pool, default_map);
            if (!material_name.empty()) {
                material_map[material_name] = materials.size();
            }
            material_id = materials.size();
            materials.push_back(m);
        }
        else if (name == "medium") {
            Medium m;
            std::string medium_name;
            std::tie(medium_name, m) = ParseMedium(child, default_map);
            if (!medium_name.empty()) {
                medium_map[medium_name] = media.size();
            }
            std::string name_value = child.attribute("name").value();
            if (name_value == "interior") {
                interior_medium_id = media.size();
            }
            else if (name_value == "exterior") {
                exterior_medium_id = media.size();
            }
            else {
                ELMA_THROW("无法识别的介质(Medium)名称：'{}'。", name_value);
            }
            media.push_back(m);
        }
    }

    Shape shape;
    std::string type = node.attribute("type").value();
    if (type == "obj") {
        std::string filename;
        Matrix4x4 to_world = Matrix4x4::identity();
        bool face_normals  = false;
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "filename") {
                filename = ParseString(child.attribute("value").value(), default_map);
            }
            else if (name == "toWorld" || name == "to_world") {
                if (std::string(child.name()) == "transform") {
                    to_world = ParseTransform(child, default_map);
                }
            }
            else if (name == "faceNormals" || name == "face_normals") {
                face_normals = ParseBoolean(child.attribute("value").value(), default_map);
            }
        }
        shape      = ParseObj(filename, to_world);
        auto& mesh = std::get<TriangleMesh>(shape);
        if (face_normals) {
            mesh.normals = std::vector<Vector3>{};
        }
        else {
            if (mesh.normals.size() == 0) {
                mesh.normals = ComputeNormal(mesh.positions, mesh.indices);
            }
        }
    }
    else if (type == "serialized") {
        std::string filename;
        int shape_index    = 0;
        Matrix4x4 to_world = Matrix4x4::identity();
        bool face_normals  = false;
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "filename") {
                filename = ParseString(child.attribute("value").value(), default_map);
            }
            else if (name == "toWorld" || name == "to_world") {
                if (std::string(child.name()) == "transform") {
                    to_world = ParseTransform(child, default_map);
                }
            }
            else if (name == "shapeIndex" || name == "shape_index") {
                shape_index = ParseInteger(child.attribute("value").value(), default_map);
            }
            else if (name == "faceNormals" || name == "face_normals") {
                face_normals = ParseBoolean(child.attribute("value").value(), default_map);
            }
        }
        shape      = LoadSerialized(filename, shape_index, to_world);
        auto& mesh = std::get<TriangleMesh>(shape);
        if (face_normals) {
            mesh.normals = std::vector<Vector3>{};
        }
        else {
            if (mesh.normals.empty()) {
                mesh.normals = ComputeNormal(mesh.positions, mesh.indices);
            }
        }
    }
    else if (type == "ply") {
        std::string filename;
        int shape_index    = 0;
        Matrix4x4 to_world = Matrix4x4::identity();
        bool face_normals  = false;
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "filename") {
                filename = ParseString(child.attribute("value").value(), default_map);
            }
            else if (name == "toWorld" || name == "to_world") {
                if (std::string(child.name()) == "transform") {
                    to_world = ParseTransform(child, default_map);
                }
            }
            else if (name == "shapeIndex" || name == "shape_index") {
                shape_index = ParseInteger(child.attribute("value").value(), default_map);
            }
            else if (name == "faceNormals" || name == "face_normals") {
                face_normals = ParseBoolean(child.attribute("value").value(), default_map);
            }
        }
        shape      = ParsePLY(filename, to_world);
        auto& mesh = std::get<TriangleMesh>(shape);
        if (face_normals) {
            mesh.normals = std::vector<Vector3>{};
        }
        else {
            if (mesh.normals.empty()) {
                mesh.normals = ComputeNormal(mesh.positions, mesh.indices);
            }
        }
    }
    else if (type == "sphere") {
        Vector3 center{0, 0, 0};
        Real radius = 1;
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "center") {
                center = Vector3{ParseFloat(child.attribute("x").value(), default_map),
                                 ParseFloat(child.attribute("y").value(), default_map),
                                 ParseFloat(child.attribute("z").value(), default_map)};
            }
            else if (name == "radius") {
                radius = ParseFloat(child.attribute("value").value(), default_map);
            }
        }
        shape = Sphere{{}, center, radius};
    }
    else if (type == "rectangle") {
        Matrix4x4 to_world = Matrix4x4::identity();
        bool flip_normals  = false;
        TriangleMesh mesh;
        mesh.positions = {
          Vector3{-1, -1, 0},
          Vector3{ 1, -1, 0},
          Vector3{ 1,  1, 0},
          Vector3{-1,  1, 0}
        };
        mesh.indices = {
          Vector3i{0, 1, 2},
          Vector3i{0, 2, 3}
        };
        mesh.uvs = {
          Vector2{0, 0},
          Vector2{1, 0},
          Vector2{1, 1},
          Vector2{0, 1}
        };
        mesh.normals = {
          Vector3{0, 0, 1},
          Vector3{0, 0, 1},
          Vector3{0, 0, 1},
          Vector3{0, 0, 1}
        };
        for (auto child : node.children()) {
            std::string name = child.attribute("name").value();
            if (name == "toWorld" || name == "to_world") {
                if (std::string(child.name()) == "transform") {
                    to_world = ParseTransform(child, default_map);
                }
            }
            else if (name == "flipNormals" || name == "flip_normals") {
                flip_normals = ParseBoolean(child.attribute("value").value(), default_map);
            }
        }
        if (flip_normals) {
            for (auto& n : mesh.normals) {
                n = -n;
            }
        }
        for (auto& p : mesh.positions) {
            p = TransformPoint(to_world, p);
        }
        for (auto& n : mesh.normals) {
            n = TransformNormal(Inverse(to_world), n);
        }
        shape = mesh;
    }
    else {
        ELMA_THROW("不支持的形状(Shape)类型：'{}'。", type);
    }
    SetMaterialId(shape, material_id);
    SetInteriorMediumId(shape, interior_medium_id);
    SetExteriorMediumId(shape, exterior_medium_id);

    for (auto child : node.children()) {
        std::string name = child.name();
        if (name == "emitter") {
            Spectrum radiance = FromRGB(Vector3{1, 1, 1});
            for (auto grand_child : child.children()) {
                std::string name = grand_child.attribute("name").value();
                if (name == "radiance") {
                    radiance = ParseIntensity(grand_child, default_map);
                }
            }
            SetAreaLightId(shape, lights.size());
            lights.push_back(DiffuseAreaLight{(int)shapes.size() /* shape ID */, radiance});
        }
    }

    return shape;
}

std::unique_ptr<Scene> ParseScene(pugi::xml_node node, const RTCDevice& embree_device)
{
    RenderOptions options;
    Camera camera(Matrix4x4::identity(), kDefaultFov, kDefaultRes, kDefaultRes, kDefaultFilter, -1 /*medium_id*/);
    std::string filename = kDefaultFilename;
    std::vector<Material> materials;
    std::map<std::string /* name id */, int /* index id */> material_map;
    TexturePool texture_pool;
    std::map<std::string /* name id */, ParsedTexture> texture_map;
    std::vector<Medium> media;
    std::map<std::string /* name id */, int /* index id */> medium_map;
    std::vector<Shape> shapes;
    std::vector<Light> lights;
    // For <default> tags
    // e.g., <default name="spp" value="4096"/> will map "spp" to "4096"
    std::map<std::string, std::string> default_map;

    int envmap_light_id = -1;
    for (auto child : node.children()) {
        std::string name = child.name();
        if (name == "default") {
            ParseDefaultMap(child, default_map);
        }
        else if (name == "integrator") {
            options = ParseIntegrator(child, default_map);
        }
        else if (name == "sensor") {
            ParsedSampler sampler;
            std::tie(camera, filename, sampler) = ParseSensor(child, media, medium_map, default_map);
            options.samplesPerPixel             = sampler.sampleCount;
        }
        else if (name == "bsdf") {
            std::string material_name;
            Material m;
            std::tie(material_name, m) = ParseBSDF(child, texture_map, texture_pool, default_map);
            if (!material_name.empty()) {
                material_map[material_name] = materials.size();
                materials.push_back(m);
            }
        }
        else if (name == "shape") {
            Shape s = parse_shape(child,
                                  materials,
                                  material_map,
                                  texture_map,
                                  texture_pool,
                                  media,
                                  medium_map,
                                  lights,
                                  shapes,
                                  default_map);
            shapes.push_back(s);
        }
        else if (name == "texture") {
            std::string id = child.attribute("id").value();
            if (texture_map.find(id) != texture_map.end()) {
                ELMA_THROW("重复的纹理 ID：'{}'。", id);
            }
            texture_map[id] = ParseTexture(child, default_map);
        }
        else if (name == "emitter") {
            std::string type = child.attribute("type").value();
            if (type == "envmap") {
                std::string filename;
                Real scale         = 1;
                Matrix4x4 to_world = Matrix4x4::identity();
                for (auto grand_child : child.children()) {
                    std::string name = grand_child.attribute("name").value();
                    if (name == "filename") {
                        filename = ParseString(grand_child.attribute("value").value(), default_map);
                    }
                    else if (name == "toWorld" || name == "to_world") {
                        to_world = ParseTransform(grand_child, default_map);
                    }
                    else if (name == "Scale") {
                        scale = ParseFloat(grand_child.attribute("value").value(), default_map);
                    }
                }
                if (filename.size() > 0) {
                    Texture<Spectrum> t = MakeImageSpectrumTexture("__envmap_texture__", filename, texture_pool, 1, 1);
                    Matrix4x4 to_local  = Inverse(to_world);
                    lights.push_back(Envmap{t, to_world, to_local, scale});
                    envmap_light_id = (int)lights.size() - 1;
                }
                else {
                    ELMA_THROW("未设置环境贴图(Envmap)的文件名。");
                }
            }
            else if (type == "point") {
                LogWarn("将点光源转换为小型球光源。");
                Vector3 position   = Vector3{0, 0, 0};
                Spectrum intensity = MakeConstSpectrum(1);
                for (auto grand_child : child.children()) {
                    std::string name = grand_child.attribute("name").value();
                    if (name == "position") {
                        if (!grand_child.attribute("x").empty()) {
                            position.x = ParseFloat(grand_child.attribute("x").value(), default_map);
                        }
                        if (!grand_child.attribute("y").empty()) {
                            position.y = ParseFloat(grand_child.attribute("y").value(), default_map);
                        }
                        if (!grand_child.attribute("z").empty()) {
                            position.z = ParseFloat(grand_child.attribute("z").value(), default_map);
                        }
                    }
                    else if (name == "intensity") {
                        intensity = ParseIntensity(grand_child, default_map);
                    }
                }
                Shape s          = Sphere{{}, position, Real(1e-4)};
                intensity       *= (kFourPi / SurfaceArea(s));
                Material m       = Lambertian{MakeConstantSpectrumTexture(MakeZeroSpectrum())};
                int material_id  = materials.size();
                materials.push_back(m);
                SetMaterialId(s, material_id);
                SetAreaLightId(s, lights.size());
                lights.push_back(DiffuseAreaLight{(int)shapes.size() /* shape ID */, intensity});
                shapes.push_back(s);
            }
            else if (type == "directional") {
                LogWarn("将方向光源转换为小型球光源。");
                Vector3 direction  = Vector3{0, 0, 1};
                Spectrum intensity = MakeConstSpectrum(1);
                for (auto grand_child : child.children()) {
                    std::string name = grand_child.attribute("name").value();
                    if (name == "direction") {
                        if (!grand_child.attribute("x").empty()) {
                            direction.x = ParseFloat(grand_child.attribute("x").value(), default_map);
                        }
                        if (!grand_child.attribute("y").empty()) {
                            direction.y = ParseFloat(grand_child.attribute("y").value(), default_map);
                        }
                        if (!grand_child.attribute("z").empty()) {
                            direction.z = ParseFloat(grand_child.attribute("z").value(), default_map);
                        }
                    }
                    else if (name == "toWorld" || name == "to_world") {
                        Matrix4x4 to_world = ParseTransform(grand_child, default_map);
                        direction          = TransformVector(to_world, direction);
                    }
                    else if (name == "irradiance") {
                        intensity = ParseIntensity(grand_child, default_map);
                    }
                }
                direction = normalize(direction);
                Vector3 tangent, bitangent;
                std::tie(tangent, bitangent) = CoordinateSystem(-direction);
                TriangleMesh mesh;
                Real length    = Real(1e-3);
                Real dist      = Real(1e3);
                mesh.positions = {Real(0.5) * length * (-tangent - bitangent) - dist * direction,
                                  Real(0.5) * length * (tangent - bitangent) - dist * direction,
                                  Real(0.5) * length * (tangent + bitangent) - dist * direction,
                                  Real(0.5) * length * (-tangent + bitangent) - dist * direction};
                mesh.indices   = {
                  Vector3i{0, 1, 2},
                  Vector3i{0, 2, 3}
                };
                mesh.normals     = {direction, direction, direction, direction};
                intensity       *= ((dist * dist) / (length * length));
                Shape s          = mesh;
                Material m       = Lambertian{MakeConstantSpectrumTexture(MakeZeroSpectrum())};
                int material_id  = materials.size();
                materials.push_back(m);
                SetMaterialId(s, material_id);
                SetAreaLightId(s, lights.size());
                lights.push_back(DiffuseAreaLight{(int)shapes.size() /* shape ID */, intensity});
                shapes.push_back(s);
            }
            else {
                ELMA_THROW("不支持的光源类型：'{}'。", type);
            }
        }
        else if (name == "medium") {
            std::string medium_name;
            Medium m;
            std::tie(medium_name, m) = ParseMedium(child, default_map);
            if (!medium_name.empty()) {
                medium_map[medium_name] = media.size();
                media.push_back(m);
            }
        }
    }
    return std::make_unique<Scene>(
        embree_device, camera, materials, shapes, lights, media, envmap_light_id, texture_pool, options, filename);
}

std::unique_ptr<Scene> ParseScene(const fs::path& filename, const RTCDevice& embree_device)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filename.c_str());
    if (!result) {
        LogError("描述：{}\n位置：{}", result.description(), result.offset);
        ELMA_THROW("解析错误。");
    }
    // back up the current working directory and switch to the parent folder of the file
    fs::path old_path = fs::current_path();
    fs::current_path(filename.parent_path());
    std::unique_ptr<Scene> scene = ParseScene(doc.child("scene"), embree_device);
    // switch back to the old current working directory
    fs::current_path(old_path);
    return scene;
}

} // namespace elma