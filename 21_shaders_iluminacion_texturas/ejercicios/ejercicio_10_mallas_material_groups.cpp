/* =============================================================================
   EJERCICIO 10 (L16): mallas_material_groups
   split por usemtl inline - parser mini OBJ
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include <string>
#include <vector>
#include <map>
#include <tuple>

namespace {

/* --- EJERCICIO ---
   Dado OBJ string simple con 2 usemtl, parsear y devolver grupos:
   1) parseOBJGroups(objText) -> vector<Grupo>
      Grupo: {material, indices, baseVertex}
*/

struct Grupo {
    std::string material;
    std::vector<int> indices;  // locales al grupo (baseVertex remapea)
    int baseVertex = 0;
};

std::vector<Grupo> parseOBJGroups(const std::string& objText) { (void)objText; return {{"", {}}}; }

/* OBJ de prueba:
   v ... (8 vértices únicos)
   usemtl matA
   f 1 2 3
   f 2 3 4
   usemtl matB
   f 5 6 7
   f 6 7 8
   -> 2 grupos: matA (6 idx, base=0), matB (6 idx, base=4 si vértices únicos empiezan en 5)
   En realidad: vértices 1-4 son matA, 5-8 matB. Dedup no aplica aquí.
   baseVertex matA = 0, matB = 4 (vértices 0-based: 4,5,6,7)
*/

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
    if (grupos.size() < 2) return;
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
    ASSERT_EQ(grupos.size(), 1);
    if (grupos.empty()) return;
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
    ASSERT_EQ(grupos.size(), 2);
    if (grupos.size() < 2) return;
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