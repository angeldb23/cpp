/* =============================================================================
   MÓDULO 20 - SOLUCIÓN 09: Parser OBJ mini + dedup de vértices
   =============================================================================
   DIFICULTAD: Alta
   APOYO: Lección 18. Resuelve el ejercicio_09_parser_obj.cpp.

   Lo que acá hacemos a mano — indexing 1-based, esquinas v/vt/vn y dedup
   con tabla de símbolos — es exactamente lo que tinyobjloader entrega en
   attrib.vertices + shapes[0].mesh.indices (la lección lo muestra).
   ============================================================================ */
#include "../../include/test_framework.hpp"
#include <array>
#include <map>
#include <sstream>
#include <string>
#include <vector>

struct Triple { int v = 0, vt = 0, vn = 0; };   // 0-based; -1 = ausente
struct Cara   { Triple c[3]; };
struct Malla {
    std::vector<std::array<float, 3>> v;
    std::vector<std::array<float, 2>> vt;
    std::vector<std::array<float, 3>> vn;
    std::vector<Cara> f;
};

// parsea UNA línea de vértice "v 1.5 2 0.25" en el array de salida
template <size_t N>
static bool parseNumeros(const std::string& linea, std::array<float, N>& out) {
    std::istringstream in(linea);
    std::string tag;
    if (!(in >> tag)) return false;
    for (size_t i = 0; i < N; ++i)
        if (!(in >> out[i])) return false;      // no hay suficientes números
    if (in >> std::ws, in.peek() != EOF) return false;   // sobra basura
    return true;
}

// solo un entero con signo (los índices del OBJ siempre son enteros)
static bool soloNumero(const std::string& p) {
    if (p.empty()) return false;
    size_t k = (p[0] == '-') ? 1 : 0;
    if (k >= p.size()) return false;
    for (size_t q = k; q < p.size(); ++q)
        if (p[q] < '0' || p[q] > '9') return false;
    return true;
}

// parsea UNA esquina "a", "a/b", "a/b/c" o "a//c" → Triple 0-based
static bool parseEsquina(const std::string& s, Triple& t) {
    t = Triple{0, -1, -1};
    int   comp[3] = {0, 0, 0};
    bool  presente[3] = {false, false, false};
    int   n = 0;
    size_t pos = 0;
    while (true) {
        if (n >= 3) return false;                    // "a/b/c/d": se ignora
        size_t slash = s.find('/', pos);
        std::string parte = s.substr(pos,
            (slash == std::string::npos ? s.size() : slash) - pos);
        if (!parte.empty()) {
            if (!soloNumero(parte)) return false;    // basura en la esquina
            comp[n] = std::atoi(parte.c_str()) - 1;  // 1-based → 0-based
            presente[n] = true;
        } else if (n == 0) {
            return false;                            // la esquina no arranca vacía
        }
        ++n;
        if (slash == std::string::npos) break;
        pos = slash + 1;
        if (pos == s.size()) break;                  // "a/" o "a//"
    }
    t.v = presente[0] ? comp[0] : -1;
    if (n >= 2) t.vt = presente[1] ? comp[1] : -1;
    if (n >= 3) t.vn = presente[2] ? comp[2] : -1;
    return true;
}

// TODO 1
static Malla parseO(const std::string& texto) {
    Malla m;
    Cara caraActual;
    std::istringstream in(texto);
    std::string linea;
    while (std::getline(in, linea)) {
        if (!linea.empty() && linea.back() == '\r') linea.pop_back();   // OBJ de Windows
        size_t arranca = linea.find_first_not_of(" \t");
        if (arranca == std::string::npos) continue;      // línea vacía
        linea = linea.substr(arranca);
        if (!linea.empty() && linea[0] == '#') continue; // comentario
        std::istringstream tok(linea);
        std::string tag;
        tok >> tag;
        if (tag == "v") {
            std::array<float, 3> p{};
            if (parseNumeros(linea, p)) m.v.push_back(p);
        } else if (tag == "vt") {
            std::array<float, 2> p{};
            if (parseNumeros(linea, p)) m.vt.push_back(p);
        } else if (tag == "vn") {
            std::array<float, 3> p{};
            if (parseNumeros(linea, p)) m.vn.push_back(p);
        } else if (tag == "f") {
            Cara c;
            std::string esq;
            int n = 0;
            bool ok = true;
            while (tok >> esq) {
                if (n >= 3) { ok = false; break; }    // no triángulo: se ignora
                if (!parseEsquina(esq, c.c[n])) { ok = false; break; }
                ++n;
            }
            if (ok && n == 3) m.f.push_back(c);
        }
        // otros tags ('o', 's', 'mtllib', ...): se ignoran
    }
    return m;
}

// TODO 2
static void expandir(const Malla& m,
                     std::vector<Triple>& vertices,
                     std::vector<unsigned>& indices) {
    std::map<std::tuple<int, int, int>, unsigned> tabla;   // key → índice en vertices
    for (const Cara& c : m.f) {
        for (const Triple& e : c.c) {
            auto key = std::make_tuple(e.v, e.vt, e.vn);
            auto it = tabla.find(key);
            if (it == tabla.end()) {
                unsigned nuevo = (unsigned)vertices.size();
                tabla[key] = nuevo;
                vertices.push_back(e);
                indices.push_back(nuevo);
            } else {
                indices.push_back(it->second);   // dedup: reuso
            }
        }
    }
}

/* --------------------------- TESTS (no tocar) -------------------------- */
static const std::string OBJ_DEMO =
    "# caja de prueba\n"
    "o Caja\n"
    "v 0 0 0\n"
    "v 1 0 0\n"
    "v 0 1 0\n"
    "vt 0 0\n"
    "vt 1 0\n"
    "vt 0 1\n"
    "vn 0 0 1\n"
    "f 1/1/1 2/2/1 3/3/1\n"
    "f 1/1/1 3/3/1 2/2/1\n";

TEST_CASE(ParsearConteos) {
    Malla m = parseO(OBJ_DEMO);
    ASSERT_EQ(m.v.size(),  (size_t)3);
    ASSERT_EQ(m.vt.size(), (size_t)3);
    ASSERT_EQ(m.vn.size(), (size_t)1);
    ASSERT_EQ(m.f.size(),  (size_t)2);
    ASSERT_NEAR(m.v[1][0], 1.0f, 1e-6f);
    ASSERT_NEAR(m.vt[2][1], 1.0f, 1e-6f);
    ASSERT_NEAR(m.vn[0][2], 1.0f, 1e-6f);
    ASSERT_EQ(m.f[0].c[0].v, 0); ASSERT_EQ(m.f[0].c[0].vt, 0); ASSERT_EQ(m.f[0].c[0].vn, 0);
    ASSERT_EQ(m.f[0].c[2].v, 2);
    ASSERT_EQ(m.f[1].c[1].vt, 2);
}

TEST_CASE(ParserIgnoraNoRelevante) {
    Malla m = parseO("v 0 0 0\nv 1 0 0\nv 0 1 0\no Meta\ns 1\n\n# mediolínea\nf 1 2 3\n");
    ASSERT_EQ(m.v.size(), (size_t)3);
    ASSERT_EQ(m.f.size(), (size_t)1);
    ASSERT_EQ(m.f[0].c[0].v, 0);
    ASSERT_EQ(m.f[0].c[0].vt, -1);
    ASSERT_EQ(m.f[0].c[0].vn, -1);
    ASSERT_EQ(m.f[0].c[2].v, 2);
    Malla d = parseO("v 0 0 0\nv 1 0 0\nf 1 2\n");
    ASSERT_EQ(d.f.size(), (size_t)0);
    ASSERT_FALSE(d.v.empty());
}

TEST_CASE(ExpandirDedup) {
    Malla m = parseO(OBJ_DEMO);
    ASSERT_EQ(m.f.size(), (size_t)2);
    std::vector<Triple> vertices;
    std::vector<unsigned> indices;
    expandir(m, vertices, indices);
    ASSERT_EQ(vertices.size(), (size_t)3);
    ASSERT_EQ(indices.size(), (size_t)6);
    ASSERT_EQ(indices[0], 0u); ASSERT_EQ(indices[1], 1u); ASSERT_EQ(indices[2], 2u);
    ASSERT_EQ(indices[3], 0u);
    ASSERT_EQ(indices[4], 2u); ASSERT_EQ(indices[5], 1u);
    ASSERT_EQ(vertices[1].v, 1); ASSERT_EQ(vertices[1].vt, 1); ASSERT_EQ(vertices[1].vn, 0);
    Malla q = parseO("v 0 0 0\nv 1 0 0\nvt 0 0\nf 1 2/1 2/1\n");
    std::vector<Triple> v2; std::vector<unsigned> i2;
    ASSERT_EQ(q.f.size(), (size_t)1);
    expandir(q, v2, i2);
    ASSERT_EQ(v2.size(), (size_t)2);
    ASSERT_EQ(i2.size(), (size_t)3);
    ASSERT_EQ(v2[0].vt, -1);
    ASSERT_EQ(v2[1].vt, 0);
    ASSERT_EQ(i2[1], 1u); ASSERT_EQ(i2[2], 1u);
}

int main() { return RUN_ALL_TESTS(); }
