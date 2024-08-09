#include "ParseObj.hpp"
#include "Transform.hpp"
#include "Common/Error.hpp"

#include <fstream>
#include <functional>
#include <map>
#include <regex>
#include <string>

namespace elma {

// https://stackoverflow.com/questions/216823/how-to-trim-a-stdstring
// trim from start
static inline std::string& LeftTrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    return s;
}

// trim from end
static inline std::string& RightTrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string& Trim(std::string& s)
{
    return LeftTrim(RightTrim(s));
}

static std::vector<int> SplitFaceStr(const std::string& s)
{
    std::regex rgx("/");
    std::sregex_token_iterator first{begin(s), end(s), rgx, -1}, last;
    std::vector<std::string> list{first, last};
    std::vector<int> result;
    for (auto& i : list) {
        if (i != "")
            result.push_back(std::stoi(i));
        else
            result.push_back(0);
    }
    while (result.size() < 3)
        result.push_back(0);

    return result;
}

struct ObjVertex
{
    ObjVertex(const std::vector<int>& id) : v(id[0] - 1), vt(id[1] - 1), vn(id[2] - 1) { }

    bool operator<(const ObjVertex& vertex) const
    {
        if (v != vertex.v) {
            return v < vertex.v;
        }
        if (vt != vertex.vt) {
            return vt < vertex.vt;
        }
        if (vn != vertex.vn) {
            return vn < vertex.vn;
        }
        return false;
    }

    int v, vt, vn;
};

size_t GetVertexId(const ObjVertex& vertex,
                   const std::vector<Vector3>& pos_pool,
                   const std::vector<Vector2>& st_pool,
                   const std::vector<Vector3>& nor_pool,
                   const Matrix4x4& to_world,
                   std::vector<Vector3>& pos,
                   std::vector<Vector2>& st,
                   std::vector<Vector3>& nor,
                   std::map<ObjVertex, size_t>& vertex_map)
{
    auto it = vertex_map.find(vertex);
    if (it != vertex_map.end()) {
        return it->second;
    }
    size_t id = pos.size();
    pos.push_back(TransformPoint(to_world, pos_pool[vertex.v]));
    if (vertex.vt != -1)
        st.push_back(st_pool[vertex.vt]);
    if (vertex.vn != -1) {
        nor.push_back(TransformNormal(Inverse(to_world), nor_pool[vertex.vn]));
    }
    vertex_map[vertex] = id;
    return id;
}

TriangleMesh ParseObj(const fs::path& filename, const Matrix4x4& to_world)
{
    std::vector<Vector3> pos_pool;
    std::vector<Vector3> nor_pool;
    std::vector<Vector2> st_pool;
    std::map<ObjVertex, size_t> vertex_map;
    TriangleMesh mesh;

    std::ifstream ifs(filename.c_str(), std::ifstream::in);
    if (!ifs.is_open()) {
        ELMA_THROW("无法打开 obj 文件 {}.", filename.string());
    }
    while (ifs.good()) {
        std::string line;
        std::getline(ifs, line);
        line = Trim(line);
        if (line.size() == 0 || line[0] == '#') { // comment
            continue;
        }

        std::stringstream ss(line);
        std::string token;
        ss >> token;
        if (token == "v") { // vertices
            Real x, y, z, w = 1;
            ss >> x >> y >> z >> w;
            pos_pool.push_back(Vector3{x, y, z} / w);
        }
        else if (token == "vt") {
            Real s, t, w;
            ss >> s >> t >> w;
            st_pool.push_back(Vector2{s, 1 - t});
        }
        else if (token == "vn") {
            Real x, y, z;
            ss >> x >> y >> z;
            nor_pool.push_back(normalize(Vector3{x, y, z}));
        }
        else if (token == "f") {
            std::string i0, i1, i2;
            ss >> i0 >> i1 >> i2;
            std::vector<int> i0f = SplitFaceStr(i0);
            std::vector<int> i1f = SplitFaceStr(i1);
            std::vector<int> i2f = SplitFaceStr(i2);

            ObjVertex v0(i0f), v1(i1f), v2(i2f);
            size_t v0id = GetVertexId(
                v0, pos_pool, st_pool, nor_pool, to_world, mesh.positions, mesh.uvs, mesh.normals, vertex_map);
            size_t v1id = GetVertexId(
                v1, pos_pool, st_pool, nor_pool, to_world, mesh.positions, mesh.uvs, mesh.normals, vertex_map);
            size_t v2id = GetVertexId(
                v2, pos_pool, st_pool, nor_pool, to_world, mesh.positions, mesh.uvs, mesh.normals, vertex_map);
            mesh.indices.push_back(Vector3i{v0id, v1id, v2id});

            std::string i3;
            if (ss >> i3) {
                std::vector<int> i3f = SplitFaceStr(i3);
                ObjVertex v3(i3f);
                size_t v3id = GetVertexId(
                    v3, pos_pool, st_pool, nor_pool, to_world, mesh.positions, mesh.uvs, mesh.normals, vertex_map);
                mesh.indices.push_back(Vector3i{v0id, v2id, v3id});
            }
            std::string i4;
            if (ss >> i4) {
                ELMA_THROW("obj 文件 {} 包含非三角面数据，读取失败。", filename.string());
            }
        } // Currently ignore other tokens
    }

    return mesh;
}

} // namespace elma