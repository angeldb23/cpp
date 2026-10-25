/* =============================================================================
   SOLUCION 10 (L16): mallas_material_groups
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <sstream>

namespace {

struct Grupo {
    std::string material;
    std::vector<int> indices;
    int baseVertex = 0;
};

std::vector<Grupo> parseOBJGroups(const std::string& objText) {
    std::vector<Grupo> grupos;
    std::string currentMaterial = "";
    std::map<std::tuple<int,int,int>, int> vertexMap;
    int nextVertexIdx = 0;
    Grupo* currentGrupo = nullptr;

    auto getVertexIdx = [&](int v, int vt, int vn) -> int {
        auto key = std::make_tuple(v, vt, vn);
        auto it = vertexMap.find(key);
        if (it != vertexMap.end()) return it->second;
        int idx = nextVertexIdx++;
        vertexMap[key] = idx;
        return idx;
    };

    std::istringstream iss(objText);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ls(line);
        std::string cmd;
        ls >> cmd;
        if (cmd == "usemtl") {
            ls >> currentMaterial;
            if (currentGrupo == nullptr || currentGrupo->material != currentMaterial) {
                grupos.push_back({currentMaterial, {}, 0});
                currentGrupo = &grupos.back();
            }
        } else if (cmd == "f") {
            if (!currentGrupo) {
                grupos.push_back({"default", {}, 0});
                currentGrupo = &grupos.back();
            }
            std::string faceVert;
            int verts[3][3] = {}; // v, vt, vn
            int vcount = 0;
            while (ls >> faceVert && vcount < 3) {
                int v = 0, vt = -1, vn = -1;
                char slash;
                std::istringstream fs(faceVert);
                fs >> v;
                if (fs.peek() == '/') {
                    fs >> slash;
                    if (fs.peek() != '/') fs >> vt;
                    if (fs.peek() == '/') { fs >> slash; fs >> vn; }
                }
                verts[vcount][0] = v - 1; // 1-based to 0-based
                verts[vcount][1] = vt - 1;
                verts[vcount][2] = vn - 1;
                vcount++;
            }
            // triangulación fan si >3 (aquí asumimos triángulos)
            for (int i = 1; i + 1 < vcount; ++i) {
                for (int j : {0, i, i+1}) {
                    int localIdx = getVertexIdx(verts[j][0], verts[j][1], verts[j][2]);
                    if (currentGrupo->baseVertex == 0 && currentGrupo->indices.empty()) {
                        currentGrupo->baseVertex = localIdx;
                    }
                    currentGrupo->indices.push_back(localIdx - currentGrupo->baseVertex);
                }
            }
        }
    }
    return grupos;
}

} // namespace

TEST_CASE(test_dos_materiales) {
    std::string obj = R"(
v 0 0 0
v 1 0 0
v 1 1 0
v 0 1 0
v 0 0 1
v 1 0 1
v 1 1 1
v 0 1 1
usemtl matA
f 1 2 3
f 2 3 4
usemtl matB
f 5 6 7
f 6 7 8
)";
    auto grupos = parseOBJGroups(obj);
    ASSERT_EQ(grupos.size(), 2);
    ASSERT_EQ(grupos[0].material, "matA");
    ASSERT_EQ(grupos[1].material, "matB");
    ASSERT_EQ(grupos[0].indices.size(), 6);
    ASSERT_EQ(grupos[1].indices.size(), 6);
}

TEST_CASE(test_basevertex_matA) {
    std::string obj = R"(
v 0 0 0
v 1 0 0
v 1 1 0
usemtl matA
f 1 2 3
)";
    auto grupos = parseOBJGroups(obj);
    ASSERT_EQ(grupos[0].baseVertex, 0);
}

TEST_CASE(test_basevertex_matB) {
    std::string obj = R"(
v 0 0 0
v 1 0 0
v 1 1 0
v 0 1 0
v 0 0 1
v 1 0 1
v 1 1 1
v 0 1 1
usemtl matA
f 1 2 3
f 2 3 4
usemtl matB
f 5 6 7
f 6 7 8
)";
    auto grupos = parseOBJGroups(obj);
    ASSERT_EQ(grupos[1].baseVertex, 4);
}

TEST_CASE(test_indices_locales) {
    std::string obj = R"(
v 0 0 0
v 1 0 0
v 1 1 0
v 0 1 0
usemtl matA
f 1 2 3
f 2 3 4
)";
    auto grupos = parseOBJGroups(obj);
    std::vector<int> esperado = {0,1,2, 1,2,3};
    ASSERT_EQ(grupos[0].indices.size(), esperado.size());
    for (size_t i = 0; i < esperado.size(); ++i)
        ASSERT_EQ(grupos[0].indices[i], esperado[i]);
}

int main() {
    return RUN_ALL_TESTS();
}