/* =============================================================================
   MÓDULO 20 - EJERCICIO 03: Política de buffers (hint + operación de subida)
   =============================================================================
   DIFICULTAD: Baja-media
   APOYO: Lección 07 (glBufferData vs glBufferSubData; STATIC/DYNAMIC/STREAM).

   CONSIGNA:
   La lección 07 muestra la tabla de decisiones por PATRÓN DE USO. Armá la
   función de política EXACTA del curso (lógica pura, sin GL):

     struct Patron {
         int  usosDeDibujo   = 0;     // draws que vivirá el buffer
         int  borrados       = -1;    // cuántas veces se reescribe COMPLETO
         bool subidasParciales = false; // usa glBufferSubData por rangos
     }

     1. `usagePara(p)`: la constante GL como string:
        - GL_STREAM_DRAW si reescribís completo AL MENOS TANTO como la
          cantidad de draws (borrados >= usosDeDibujo).
        - GL_DYNAMIC_DRAW si hay subidas parciales (pero NO alcanza el caso
          stream del punto anterior).
        - GL_STATIC_DRAW en el resto (subo una vez, dibujo muchas).
        (¡Igual que la lección! Verificación en los tests de abajo.)
     2. `operacionPara(p)`: "glBufferData" si hay reescrituras completas
        (borrados >= 1) o no hay subidas parciales; "glBufferSubData" si
        hay subidas parciales (el caso DYNAMIC: update por rangos).
     3. `bytesInstances(cant)`: bytes del VBO de `cant` instancias del
        struct de la L14 (2 floats de offset + 3 de color). Es el tamaño
        que la subida completa (STREAM) pide por subida.
     4. `subDataValida(tamAsignado, offset, tamSubida)`: la regla dura del
        glBufferSubData: offset + tamSubida <= tamAsignado.

   TESTS (no tocar). COMPILACIÓN Y CORRIDA:
     g++ -std=c++20 -Wall -Wextra ejercicio_03_politica_buffers.cpp -o ej && ./ej
   ============================================================================ */
#include "../../include/test_framework.hpp"

struct Patron {
    int  usosDeDibujo   = 0;
    int  borrados       = -1;
    bool subidasParciales = false;
};

/* ------------------------------ TU CÓDIGO AQUÍ ---------------------------- */

// TODO 1: STATIC/DYNAMIC/STREAM como string (sin el "GL_")
static std::string usagePara(const Patron& p) {
    (void)p;
    return "";
}

// TODO 2: glBufferData | glBufferSubData según el patrón
static std::string operacionPara(const Patron& p) {
    (void)p;
    return "";
}

// TODO 3: bytes totales del VBO de instances por el struct dado en sizeT
struct Instancia { float off[2]; float color[3]; };   // 5 floats (L14)
static int bytesInstances(int cant) {
    (void)cant;
    return 0;
}

// TODO 4: ¿la llamada glBufferSubData es válida? Regla del GL (lección 07):
// offset + tamSubida <= tamAsignado. (Si no: GL_INVALID_VALUE.)
static bool subDataValida(int tamAsignado, int offset, int tamSubida) {
    (void)tamAsignado; (void)offset; (void)tamSubida;
    return false;
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
    // 5 floats x 4 bytes x cant
    ASSERT_EQ(bytesInstances(1), 20);
    ASSERT_EQ(bytesInstances(4000), 80000);   // la lección 07 con STREAM
}

TEST_CASE(SubDataValida) {
    // 48000 bytes asignados (la lección 07 con 4000 instancias)
    ASSERT_TRUE(subDataValida(48000, 0, 48000));      // todo, exacto
    ASSERT_TRUE(subDataValida(48000, 24000, 24000));  // segunda mitad
    ASSERT_FALSE(subDataValida(48000, 24000, 24001)); // se pasa 1 byte
    ASSERT_FALSE(subDataValida(48000, 48001, 1));     // offset fuera de rango
    ASSERT_TRUE(subDataValida(48000, 48000, 0));      // vacía: no-op válida
}

int main() { return RUN_ALL_TESTS(); }
