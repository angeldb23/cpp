/* =============================================================================
   MÓDULO 20 - SOLUCIÓN 02: Layout de vértices (stride y offsets en bytes)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 08. Resuelve el ejercicio_02_layout_vertices.cpp.

   La idea: nothing más y nada menos que bytes acumulados en el orden de
   los campos. La "lógica GL" es EXACTA: al enlazar el VAO los offsets
   resultan (const void*)(size_t)offsets[i] con stride = sizeof(los campos).
   (uv[12] con pos[12]); el Ubyte4 (color bajado a 4 bytes) es el caso
   práctico de la lección: mismo offset (20), stride 24 (los bytes del
   atributo SON del tipo básico del pointer: GL_UNSIGNED_BYTE con 4 comps).
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
    int stride = 0;
    std::vector<int> offsets;
};

// TODO 1
static int bytesDe(AtributoTipo t) {
    int comps = 0, bytesBase = 0;
    switch (t) {
        case AtributoTipo::Float:  comps = 1; bytesBase = 4; break;
        case AtributoTipo::Vec2:   comps = 2; bytesBase = 4; break;
        case AtributoTipo::Vec3:   comps = 3; bytesBase = 4; break;
        case AtributoTipo::Vec4:   comps = 4; bytesBase = 4; break;
        case AtributoTipo::Ubyte4: comps = 4; bytesBase = 1; break; // GLubyte
    }
    return comps * bytesBase;
}

// TODO 2
static Layout calcularLayout(const std::vector<Campo>& campos) {
    Layout l;
    l.offsets.reserve(campos.size());
    int cursor = 0;
    for (const Campo& c : campos) {
        l.offsets.push_back(cursor);
        cursor += bytesDe(c.tipo);        // tight: cero gaps (float pack)
    }
    l.stride = cursor;
    return l;
}

// TODO 3
static int offsetDe(const Layout& l, const std::vector<Campo>& campos,
                    const std::string& nombre) {
    for (size_t i = 0; i < campos.size(); ++i) {
        if (campos[i].nombre == nombre) return l.offsets[i];
    }
    return -1;
}

// TODO 4
static int compsPara(AtributoTipo t) {
    switch (t) {
        case AtributoTipo::Float:  return 1;
        case AtributoTipo::Vec2:   return 2;
        case AtributoTipo::Vec3:   return 3;
        case AtributoTipo::Vec4:   return 4;
        case AtributoTipo::Ubyte4: return 4;      // 4 elementos de GLubyte
    }
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
    ASSERT_EQ(bytesDe(AtributoTipo::Ubyte4), 4);
}

TEST_CASE(LayoutBaseDelCurso) {
    Layout l = calcularLayout(VERTEX_BASE);
    ASSERT_EQ(l.stride, 36);
    ASSERT_EQ(l.offsets.size(), (size_t)3);
    ASSERT_EQ(l.offsets[0], 0);
    ASSERT_EQ(l.offsets[1], 12);
    ASSERT_EQ(l.offsets[2], 20);
    ASSERT_EQ(offsetDe(l, VERTEX_BASE, "color"), 20);
    ASSERT_EQ(offsetDe(l, VERTEX_BASE, "no-existe"), -1);
}

TEST_CASE(StrideCambiaConUbyte) {
    std::vector<Campo> campos = VERTEX_BASE;
    campos[2].tipo = AtributoTipo::Ubyte4;
    Layout l = calcularLayout(campos);
    ASSERT_EQ(l.stride, 24);
    ASSERT_EQ(l.offsets[2], 20);
}

TEST_CASE(CompsParaGL) {
    ASSERT_EQ(compsPara(AtributoTipo::Float),  1);
    ASSERT_EQ(compsPara(AtributoTipo::Vec2),   2);
    ASSERT_EQ(compsPara(AtributoTipo::Vec3),   3);
    ASSERT_EQ(compsPara(AtributoTipo::Vec4),   4);
    ASSERT_EQ(compsPara(AtributoTipo::Ubyte4), 4);
}

int main() { return RUN_ALL_TESTS(); }
