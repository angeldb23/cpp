/* =============================================================================
   MÓDULO 20 - EJERCICIO 09: Parser OBJ mini + dedup de vértices
   =============================================================================
   DIFICULTAD: Alta
   APOYO: Lección 18 (los motores cargan .obj con tinyobjloader; el parser
          mini de este ejercicio muestra LO QUE APORTA el loader: indexing
          1-based, triples v/vt/vn por esquina y el dedup que produce el
          index buffer final).

   CONSIGNA (lógica pura; sin GL ni file system):
   Formato del subconjunto:
     "v fx fy fz"          → posición
     "vt fu fv"            → uv
     "vn fx fy fz"         → normal
     "f a/b/c x/y/z n/m/p" → cara TRIANGULAR (3 esquinas); cada esquina es
       "v", "v/vt", "v/vt/vn" o "v//vn". Los índices OBJ son 1-BASED y se
       convierten a 0-based ("1" → 0). Componente AUSENTE = -1 (nunca 0:
       el 0 es un índice válido).
   Se IGNORAN (sin error): líneas vacías, comentarios que empiezan con '#',
   y tags desconocidos ("o", "s", "mtllib", etc.). Un "f" con MÁS o MENOS
   de 3 esquinas también se ignora (la triangulación de ngons la maneja el
   loader real de la lección).

     struct Triple { int v = 0, vt = 0, vn = 0; };   // 0-based; -1 = ausente
     struct Cara   { Triple c[3]; };
     struct Malla {
         std::vector<std::array<float, 3>> v;    // posiciones
         std::vector<std::array<float, 2>> vt;   // uvs
         std::vector<std::array<float, 3>> vn;   // normales
         std::vector<Cara> f;
     };

     1. `parseO(texto)`: parsea TODAS las líneas (cortá también por '\r':
        los OBJ de Windows vienen con \r\n). Devuelve la Malla.
     2. `expandir(malla, vertices, indices)`: materializa las caras en
        (vertex buffer, index buffer): por cada ESQUINA de cada cara arma
        la key (v, vt, vn) y la deduplica en `vertices` (un std::map hace
        de tabla de símbolos); `indices` recibe el índice del vértice
        deduplicado en el orden de aparición.

   TESTS (no tocar). COMPILACIÓN Y CORRIDA:
     g++ -std=c++20 -Wall -Wextra ejercicio_09_parser_obj.cpp -o ej && ./ej
   ============================================================================ */
#include "../../include/test_framework.hpp"
#include <array>
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

/* ------------------------------ TU CÓDIGO AQUÍ ---------------------------- */

// TODO 1: el parser mini
static Malla parseO(const std::string& texto) {
    (void)texto;
    return {};
}

// TODO 2: materializar caras → vertex buffer dedup + index buffer
static void expandir(const Malla& m,
                     std::vector<Triple>& vertices,
                     std::vector<unsigned>& indices) {
    (void)m; (void)vertices; (void)indices;
}

/* --------------------------- TESTS (no tocar) -------------------------- */
// triángulo con 3 v + 3 vt + 1 vn; la segunda cara REUTILIZA la esquina
// (0,0,0): tal cual un quad triángulado. Los 1-based OBJ van a 0-based.
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
    ASSERT_EQ(m.v.size(),  (size_t)3);   // guard del stub vacío
    ASSERT_EQ(m.vt.size(), (size_t)3);
    ASSERT_EQ(m.vn.size(), (size_t)1);
    ASSERT_EQ(m.f.size(),  (size_t)2);
    ASSERT_NEAR(m.v[1][0], 1.0f, 1e-6f);         // el "v 1 0 0" va a m.v[1]
    ASSERT_NEAR(m.vt[2][1], 1.0f, 1e-6f);
    ASSERT_NEAR(m.vn[0][2], 1.0f, 1e-6f);
    // la esquina 0 de la cara 0 "1/1/1" → (0, 0, 0) ya 0-based
    ASSERT_EQ(m.f[0].c[0].v, 0); ASSERT_EQ(m.f[0].c[0].vt, 0); ASSERT_EQ(m.f[0].c[0].vn, 0);
    ASSERT_EQ(m.f[0].c[2].v, 2);                 // "3/3/1" → v=2, vn=0
    ASSERT_EQ(m.f[1].c[1].vt, 2);                // "3/3/1" en la cara 2
}

TEST_CASE(ParserIgnoraNoRelevante) {
    Malla m = parseO("v 0 0 0\nv 1 0 0\nv 0 1 0\no Meta\ns 1\n\n# mediolínea\nf 1 2 3\n");
    ASSERT_EQ(m.v.size(), (size_t)3);            // guard anti-crash del stub vacío
    ASSERT_EQ(m.f.size(), (size_t)1);            // el 'o' y el 's' se van al olvido
    // esquina "1" solo con v: vt y vn quedan AUSENTES (-1), no 0
    ASSERT_EQ(m.f[0].c[0].v, 0);
    ASSERT_EQ(m.f[0].c[0].vt, -1);
    ASSERT_EQ(m.f[0].c[0].vn, -1);
    ASSERT_EQ(m.f[0].c[2].v, 2);
    // cara de 2 esquinas (prohibida en este mini): IGNORADA, no rompe
    Malla d = parseO("v 0 0 0\nv 1 0 0\nf 1 2\n");
    ASSERT_EQ(d.f.size(), (size_t)0);
    ASSERT_FALSE(d.v.empty());                   // guard anti-crash del stub vacío
}

TEST_CASE(ExpandirDedup) {
    Malla m = parseO(OBJ_DEMO);
    ASSERT_EQ(m.f.size(), (size_t)2);            // guard del stub vacío
    std::vector<Triple> vertices;
    std::vector<unsigned> indices;
    expandir(m, vertices, indices);
    ASSERT_EQ(vertices.size(), (size_t)3);       // 6 esquinas → 3 vértices ÚNICOS
    ASSERT_EQ(indices.size(), (size_t)6);
    ASSERT_EQ(indices[0], 0u); ASSERT_EQ(indices[1], 1u); ASSERT_EQ(indices[2], 2u);
    ASSERT_EQ(indices[3], 0u);                   // la cara 2 reusa la esquina (0,0,0)
    ASSERT_EQ(indices[4], 2u); ASSERT_EQ(indices[5], 1u);
    ASSERT_EQ(vertices[1].v, 1); ASSERT_EQ(vertices[1].vt, 1); ASSERT_EQ(vertices[1].vn, 0);
    // mismo v con distintos vt: son vértices DISTINTOS en el dedup
    // esquinas: "1" → (0,-1,-1) y "2/1" → (1,0,-1) (la cara la repite)
    Malla q = parseO("v 0 0 0\nv 1 0 0\nvt 0 0\nf 1 2/1 2/1\n");
    std::vector<Triple> v2; std::vector<unsigned> i2;
    ASSERT_EQ(q.f.size(), (size_t)1);            // guard anti-crash del stub vacío
    expandir(q, v2, i2);
    ASSERT_EQ(v2.size(), (size_t)2);             // (0,-1,-1) y (1,0,-1): NO deduplean
    ASSERT_EQ(i2.size(), (size_t)3);
    ASSERT_EQ(v2[0].vt, -1);                     // la esquina "1" lleva vt ausente
    ASSERT_EQ(v2[1].vt, 0);                      // la esquina "2/1" trae el vt 0
    ASSERT_EQ(i2[1], 1u); ASSERT_EQ(i2[2], 1u);  // la repetida reusa el índice
}

int main() { return RUN_ALL_TESTS(); }
