/* =============================================================================
   MÓDULO 20 - SOLUCIÓN 03: Política de buffers (hint + operación de subida)
   =============================================================================
   DIFICULTAD: Baja-media
   APOYO: Lección 07. Resuelve el ejercicio_03_politica_buffers.cpp.

   La política es la tabla de la lección (STREAM si el borrado alcanza el
   uso, DYNAMIC si hay updates parciales, STATIC en el resto), y la regla
   dura del subData (offset+size <= asignado) tal como la specifica el GL
   (GL_INVALID_VALUE en el error).
   ============================================================================ */
#include "../../include/test_framework.hpp"
#include <string>

struct Patron {
    int  usosDeDibujo   = 0;
    int  borrados       = -1;
    bool subidasParciales = false;
};

// TODO 1
static std::string usagePara(const Patron& p) {
    if (p.borrados >= p.usosDeDibujo) return "STREAM_DRAW";
    if (p.subidasParciales)           return "DYNAMIC_DRAW";
    return "STATIC_DRAW";
}

// TODO 2
static std::string operacionPara(const Patron& p) {
    if (p.subidasParciales && p.borrados < 1) return "glBufferSubData";
    return "glBufferData";
}

struct Instancia { float off[2]; float color[3]; };

// TODO 3
static int bytesInstances(int cant) {
    return cant * (int)sizeof(Instancia);   // 5 floats compactos = 20 bytes
}

// TODO 4
static bool subDataValida(int tamAsignado, int offset, int tamSubida) {
    return offset + tamSubida <= tamAsignado;   // regla GL_INVALID_VALUE
}

/* --------------------------- TESTS (no tocar) -------------------------- */
TEST_CASE(MallaEstatica) {
    Patron nivela{1000, 1, false};
    ASSERT_EQ(usagePara(nivela), "STATIC_DRAW");
    ASSERT_EQ(operacionPara(nivela), "glBufferData");
}

TEST_CASE(ParticulasDinamicas) {
    Patron part{1000, 0, true};
    ASSERT_EQ(usagePara(part), "DYNAMIC_DRAW");
    ASSERT_EQ(operacionPara(part), "glBufferSubData");
}

TEST_CASE(QuadsDelHUDStream) {
    Patron hud{1000, 1000, false};
    ASSERT_EQ(usagePara(hud), "STREAM_DRAW");
    ASSERT_EQ(operacionPara(hud), "glBufferData");
}

TEST_CASE(BytesDeInstancias) {
    ASSERT_EQ(bytesInstances(1), 20);
    ASSERT_EQ(bytesInstances(4000), 80000);
}

TEST_CASE(SubDataValida) {
    ASSERT_TRUE(subDataValida(48000, 0, 48000));
    ASSERT_TRUE(subDataValida(48000, 24000, 24000));
    ASSERT_FALSE(subDataValida(48000, 24000, 24001));
    ASSERT_FALSE(subDataValida(48000, 48001, 1));
    ASSERT_TRUE(subDataValida(48000, 48000, 0));
}

int main() { return RUN_ALL_TESTS(); }
