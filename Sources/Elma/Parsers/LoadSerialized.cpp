#include "LoadSerialized.hpp"
#include <miniz.h>
#include "Transform.hpp"
#include "Common/Error.hpp"
#include <fstream>
#include <iostream>

#define MTS_FILEFORMAT_VERSION_V3 0x0003
#define MTS_FILEFORMAT_VERSION_V4 0x0004

#define ZSTREAM_BUFSIZE 32'768

namespace elma {

enum ETriMeshFlags
{
    EHasNormals      = 0x0001,
    EHasTexcoords    = 0x0002,
    EHasTangents     = 0x0004, // unused
    EHasColors       = 0x0008,
    EFaceNormals     = 0x0010,
    ESinglePrecision = 0x1000,
    EDoublePrecision = 0x2000
};

class ZStream
{
public:
    /// Create a new compression stream
    ZStream(std::fstream& fs);
    void read(void* ptr, size_t size);
    virtual ~ZStream();

private:
    std::fstream& _fs;
    size_t _fileSize;
    z_stream _inflateStream;
    uint8_t _inflateBuffer[ZSTREAM_BUFSIZE];
};

ZStream::ZStream(std::fstream& fs) : _fs(fs)
{
    std::streampos pos = fs.tellg();
    fs.seekg(0, fs.end);
    _fileSize = (size_t)fs.tellg();
    fs.seekg(pos, fs.beg);

    int windowBits          = 15;
    _inflateStream.zalloc   = Z_NULL;
    _inflateStream.zfree    = Z_NULL;
    _inflateStream.opaque   = Z_NULL;
    _inflateStream.avail_in = 0;
    _inflateStream.next_in  = Z_NULL;

    int retval = inflateInit2(&_inflateStream, windowBits);
    if (retval != Z_OK) {
        ELMA_THROW("ZLib 初始化失败");
    }
}

void ZStream::read(void* ptr, size_t size)
{
    uint8_t* targetPtr = (uint8_t*)ptr;
    while (size > 0) {
        if (_inflateStream.avail_in == 0) {
            size_t remaining        = _fileSize - _fs.tellg();
            _inflateStream.next_in  = _inflateBuffer;
            _inflateStream.avail_in = (uInt)Min(remaining, sizeof(_inflateBuffer));
            if (_inflateStream.avail_in == 0) {
                ELMA_THROW("预期数据缺失，读取失败。");
            }

            _fs.read((char*)_inflateBuffer, _inflateStream.avail_in);
        }

        _inflateStream.avail_out = (uInt)size;
        _inflateStream.next_out  = targetPtr;

        int retval = inflate(&_inflateStream, Z_NO_FLUSH);
        switch (retval) {
        case Z_STREAM_ERROR :
            {
                ELMA_THROW("inflate(): stream error!");
            }
        case Z_NEED_DICT :
            {
                ELMA_THROW("inflate(): need dictionary!");
            }
        case Z_DATA_ERROR :
            {
                ELMA_THROW("inflate(): data error!");
            }
        case Z_MEM_ERROR :
            {
                ELMA_THROW("inflate(): memory error!");
            }
        };

        size_t outputSize  = size - (size_t)_inflateStream.avail_out;
        targetPtr         += outputSize;
        size              -= outputSize;

        if (size > 0 && retval == Z_STREAM_END) {
            ELMA_THROW("inflate(): attempting to read past the end of the stream!");
        }
    }
}

ZStream::~ZStream()
{
    inflateEnd(&_inflateStream);
}

void SkipToIdx(std::fstream& fs, const short version, const size_t idx)
{
    // Go to the end of the file to see how many components are there
    fs.seekg(-sizeof(uint32_t), fs.end);
    uint32_t count = 0;
    fs.read((char*)&count, sizeof(uint32_t));
    size_t offset = 0;
    if (version == MTS_FILEFORMAT_VERSION_V4) {
        fs.seekg(-sizeof(uint64_t) * (count - idx) - sizeof(uint32_t), fs.end);
        fs.read((char*)&offset, sizeof(size_t));
    }
    else { // V3
        fs.seekg(-sizeof(uint32_t) * (count - idx + 1), fs.end);
        uint32_t upos = 0;
        fs.read((char*)&upos, sizeof(unsigned int));
        offset = upos;
    }
    fs.seekg(offset, fs.beg);
    // Skip the header
    fs.ignore(sizeof(short) * 2);
}

template<typename Precision> std::vector<Vector3> LoadPosition(ZStream& zs, int num_vertices)
{
    std::vector<Vector3> vertices(num_vertices);
    for (int i = 0; i < (int)num_vertices; i++) {
        Precision x, y, z;
        zs.read(&x, sizeof(Precision));
        zs.read(&y, sizeof(Precision));
        zs.read(&z, sizeof(Precision));
        vertices[i] = Vector3{x, y, z};
    }
    return vertices;
}

template<typename Precision> std::vector<Vector3> LoadNormal(ZStream& zs, int num_vertices)
{
    std::vector<Vector3> normals(num_vertices);
    for (int i = 0; i < (int)normals.size(); i++) {
        Precision x, y, z;
        zs.read(&x, sizeof(Precision));
        zs.read(&y, sizeof(Precision));
        zs.read(&z, sizeof(Precision));
        normals[i] = Vector3{x, y, z};
    }
    return normals;
}

template<typename Precision> std::vector<Vector2> LoadUV(ZStream& zs, int num_vertices)
{
    std::vector<Vector2> uvs(num_vertices);
    for (int i = 0; i < (int)uvs.size(); i++) {
        Precision u, v;
        zs.read(&u, sizeof(Precision));
        zs.read(&v, sizeof(Precision));
        uvs[i] = Vector2{u, v};
    }
    return uvs;
}

template<typename Precision> std::vector<Vector3> LoadColor(ZStream& zs, int num_vertices)
{
    std::vector<Vector3> colors(num_vertices);
    for (int i = 0; i < num_vertices; i++) {
        Precision r, g, b;
        zs.read(&r, sizeof(Precision));
        zs.read(&g, sizeof(Precision));
        zs.read(&b, sizeof(Precision));
        colors[i] = Vector3{r, g, b};
    }
    return colors;
}

TriangleMesh LoadSerialized(const fs::path& filename, int shape_index, const Matrix4x4& to_world)
{
    std::fstream fs(filename.c_str(), std::fstream::in | std::fstream::binary);
    // Format magic number, ignore it
    fs.ignore(sizeof(short));
    // Version number
    short version = 0;
    fs.read((char*)&version, sizeof(short));
    if (shape_index > 0) {
        SkipToIdx(fs, version, shape_index);
    }
    ZStream zs(fs);

    uint32_t flags;
    zs.read((char*)&flags, sizeof(uint32_t));
    std::string name;
    if (version == MTS_FILEFORMAT_VERSION_V4) {
        char c;
        while (true) {
            zs.read((char*)&c, sizeof(char));
            if (c == '\0')
                break;
            name.push_back(c);
        }
    }
    size_t vertex_count = 0;
    zs.read((char*)&vertex_count, sizeof(size_t));
    size_t triangle_count = 0;
    zs.read((char*)&triangle_count, sizeof(size_t));

    bool file_double_precision = flags & EDoublePrecision;
    // bool face_normals = flags & EFaceNormals;

    TriangleMesh mesh;
    if (file_double_precision) {
        mesh.positions = LoadPosition<double>(zs, vertex_count);
    }
    else {
        mesh.positions = LoadPosition<float>(zs, vertex_count);
    }
    for (auto& p : mesh.positions) {
        p = TransformPoint(to_world, p);
    }

    if (flags & EHasNormals) {
        if (file_double_precision) {
            mesh.normals = LoadNormal<double>(zs, vertex_count);
        }
        else {
            mesh.normals = LoadNormal<float>(zs, vertex_count);
        }
        for (auto& n : mesh.normals) {
            n = TransformNormal(Inverse(to_world), n);
        }
    }

    if (flags & EHasTexcoords) {
        if (file_double_precision) {
            mesh.uvs = LoadUV<double>(zs, vertex_count);
        }
        else {
            mesh.uvs = LoadUV<float>(zs, vertex_count);
        }
    }

    if (flags & EHasColors) {
        // Ignore the color attributes.
        if (file_double_precision) {
            LoadColor<double>(zs, vertex_count);
        }
        else {
            LoadColor<float>(zs, vertex_count);
        }
    }

    mesh.indices.resize(triangle_count);
    for (size_t i = 0; i < triangle_count; i++) {
        int i0, i1, i2;
        zs.read(&i0, sizeof(int));
        zs.read(&i1, sizeof(int));
        zs.read(&i2, sizeof(int));
        mesh.indices[i] = Vector3i{i0, i1, i2};
    }

    return mesh;
}

} // namespace elma