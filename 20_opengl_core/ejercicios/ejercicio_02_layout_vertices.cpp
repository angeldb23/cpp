/* =============================================================================
   MÓDULO 20 - EJERCICIO 02: Layout de vértices (stride y offsets en bytes)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 08 (glVertexAttribPointer, stride, offset, sizeof(Vertex)).

   CONSIGNA:
   El VAO apunta bytes: si la cuenta está mal, el quad sale rotables o el
   driver larga GL_INVALID_VALUE. Completá el calculador de layout EN C++
   (lo que la lección muestra completo; acá lo armás vos):

     struct Campo   { std::string nombre; AtributoTipo tipo; }
     struct Layout  { int stride; std::vector<int> offsets; }

     1. int bytesDe(AtributoTipo) : bytes del ATRIBUTO COMPLETO
        (Vec3 = 12, Vec2 = 8, Vec4 = 16, Ubyte4 = 4 (4x sizeof(GLubyte))).
     2. Layout calcularLayout(campos): tight packing SIN gaps, en el orden
        dato: stride = suma de bytes; offsets acumulados pe campo.
     3. int offsetDe(layout, campos, nombre): el offset del campo `nombre`
        (si no existe: -1).
     4. int culposDe(AtributoTipo): cuántos COMPONENTES pide glVertexAttrib-
        uterPointer (Vec3 -> 3; Ubyte4 -> 4: float vs byte es del `tipo`).

   VERIFICACIONES CLAVE (el test de abajo NO se toca: si tu cuenta está
   mal, el static_assert MENTIRÁ en la lección 08):
     Vertex{Vec3 pos, Vec2 uv, Vec4 color} -> stride 36, offsets 0/12/20.
     Vertex con color Ubyte4 en vez de Vec4 -> stride 24, offsets 0/12/20.
     ¡Atención! NO hay padding: float tight packing (lección 08); el GL
     es el que decide con stride que DEVUELVE TU función al enlazar.

   TESTS (no tocar). COMPILACIÓN Y CORRIDA:
     g++ -std=c++20 -Wall -Wextra ejercicio_02_layout_vertices.cpp -o ej && ./ej
   (Sin GL: es matemática de bytes.)
   ============================================================================ */
#include "../../include/test_framework.hpp"
#include <string>
#include <vector>

enum class AtributoTipo { Float, Vec2, Vec3, Vec4, Ubyte4 };

struct Campo {
    std::string  nombre;
    AtributoTipo tipo;
};

struct Layout {
    int stride = 0;                    // bytes por VERTEX (el `stride` de GL)
    std::vector<int> offsets;          // per campo, en orden
};

/* ------------------------------ TU CÓDIGO AQUÍ ---------------------------- */

// TODO 1: bytes del atributo completo
static int bytesDe(AtributoTipo t) {
    (void)t;
    return 0;   // stub
}

// TODO 2: layúar layout por campos (tight, ordenamiento del vector)
static Layout calcularLayout(const std::vector<Campo>& campos) {
    (void)campos;
    return Layout{};   // stub
}

// TODO 3: offset del campo por nombre (-1 si no está)
static int offsetDe(const Layout& l, const std::vector<Campo>& campos,
                    const std::string& nombre) {
    (void)l; (void)campos; (void)nombre;
    return -1;
}

// TODO 4: cantidad de COMPONENTES para glVertexAttribPointer
static int compsPara(AtributoTipo t) {
    (void)t;
    return 0;
}

/* --------------------------- TESTS (no tocar) -------------------------- */
static const std::vector<Campo> VERTEX_BASE = {
    {"pos",   AtributoTipo::Vec3},
    {"uv",    AtributoTipo::Vec2},
    {"color", AtributoTipo::Vec4},
};

TEST_CASE(BytesPorTipo) {
    ASSERT_EQ(bytesDe(AtributoTipo::Float),  4);
    ASSERT_EQ(bytesDe(AtributoTipo::Vec2),   8);
    ASSERT_EQ(bytesDe(AtributoTipo::Vec3),  12);
    ASSERT_EQ(bytesDe(AtributoTipo::Vec4),  16);
    ASSERT_EQ(bytesDe(AtributoTipo::Ubyte4), 4);   // 4 x sizeof(GLubyte)
}

TEST_CASE(LayoutBaseDelCurso) {
    Layout l = calcularLayout(VERTEX_BASE);
    ASSERT_EQ(l.stride, 36);                       // sizeof(Vertex) de la L08
    ASSERT_EQ(l.offsets.size(), (size_t)3);
    ASSERT_EQ(l.offsets[0], 0);                    // pos
    ASSERT_EQ(l.offsets[1], 12);                   // uv
    ASSERT_EQ(l.offsets[2], 20);                   // color
    ASSERT_EQ(offsetDe(l, VERTEX_BASE, "color"), 20);
    ASSERT_EQ(offsetDe(l, VERTEX_BASE, "no-existe"), -1);
}

TEST_CASE(StrideCambiaConUbyte) {
    std::vector<Campo> campos = VERTEX_BASE;       // color pasa a GLubyte[4]
    campos[2].tipo = AtributoTipo::Ubyte4;
    Layout l = calcularLayout(campos);
    ASSERT_EQ(l.stride, 24);                       // 12 + 8 + 4
    ASSERT_EQ(l.offsets[2], 20);                   // el offset NO cambia
}

TEST_CASE(CompsParaGL) {
    ASSERT_EQ(compsPara(AtributoTipo::Float),  1);
    ASSERT_EQ(compsPara(AtributoTipo::Vec2),   2);
    ASSERT_EQ(compsPara(AtributoTipo::Vec3),   3);
    ASSERT_EQ(compsPara(AtributoTipo::Vec4),   4);
    ASSERT_EQ(compsPara(AtributoTipo::Ubyte4), 4);
}

int main() { return RUN_ALL_TESTS(); }
