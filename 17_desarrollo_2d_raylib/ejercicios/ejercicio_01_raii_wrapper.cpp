/* =============================================================================
   MÓDULO 17 - EJERCICIO 01: Wrapper RAII Move-Only para Resource de raylib
   =============================================================================
   DIFICULTAD: Básico
   APOYO: Lección 01 (Wrappers RAII), Módulo 07 (RAII) y 08 (movimiento)

   CONSIGNA:
   Completa la clase TextureRAII para que cumpla el contrato RAII de la
   lección 01 con relación al "recurso" simulado (contadores estáticos):

   ESTRUCTURA REQUERIDA:
   - Constructor default: sin recurso (Valid() == false)
   - Constructor desde Texture2D: toma posesión (Valid() == true)
   - NO copiable (borrar copia): propiedad exclusiva
   - Movible: el origen queda vacío tras el movimiento (doble-unload prohibido)
   - Destructor: si hay recurso, llama UnloadTexture (contador baja)
   - Reset(): libera manual; es idempotente (puede llamarse dos veces)

   El archivo ya trae los contadores estáticos (Load/Unload simuladas).

   TESTS (no tocar): ciclo de vida, movimiento, reset idempotente y
   cero-unload-con-doble-cierre (el destructor no re-libera lo ya reseteado).

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_01_raii_wrapper.cpp -o ej && ./ej
   ============================================================================= */
#include <iostream>
#include <utility>
#include <cassert>
#include "../../include/test_framework.hpp"

// ============================ CONTADORES SIMULADOS (no tocar) ================
struct Texture2D { int id = 0; };

static int gUp = 0;      // "LoadTexture": cantidad de recursos vivos AHORA
static int gCreated = 0; // LoadTexture total
static int gDestroyed = 0; // UnloadTexture total

inline Texture2D SimulateLoad() { ++gUp; ++gCreated; return Texture2D{gCreated}; }
inline void UnloadTexture(Texture2D) { --gUp; ++gDestroyed; }

// ============================ TU CÓDIGO AQUÍ ================================
class TextureRAII {
    Texture2D tex_{};
    bool valid_ = false;

public:
    TextureRAII() = default;

    // TODO 1: constructor desde handle crudo (toma posesión)
    explicit TextureRAII(Texture2D t) {
        // Pista: guardar el handle y marcar válido
        (void)t;
    }

    TextureRAII(const TextureRAII&) = delete;
    TextureRAII& operator=(const TextureRAII&) = delete;

    // TODO 2: constructor de movimiento (el origen queda inválido)
    TextureRAII(TextureRAII&& other) noexcept {
        // Pista: robar handle + flag, y vaciar al other para que su dtor
        //        NO haga el unload (doble-unload = BUG)
        (void)other;
    }

    // TODO 3: asignación de movimiento (reset antes de robar, self-check)
    TextureRAII& operator=(TextureRAII&& other) noexcept {
        (void)other;
        return *this;
    }

    // TODO 4: destructor que libera si es válido
    ~TextureRAII() {
    }

    // TODO 5: reset() — libera si hay recurso, y es idempotente
    void Reset() {
    }

    bool Valid() const { return valid_; }
    Texture2D Get() const { return tex_; }
};

// ============================ TESTS (no tocar) ==============================
TEST_CASE(LoadYDtorAutomático) {
    gUp = 0; gCreated = 0; gDestroyed = 0;
    {
        TextureRAII tex{SimulateLoad()};
        ASSERT_TRUE(tex.Valid());
        ASSERT_EQ(gUp, 1);
    } // el destructor libera
    ASSERT_EQ(gUp, 0);
    ASSERT_EQ(gDestroyed, 1);
}

TEST_CASE(MoveTransfiereSinDobleUnload) {
    gUp = 1; gCreated = 1; gDestroyed = 0;
    TextureRAII origen{Texture2D{1}};
    TextureRAII destino = std::move(origen);
    ASSERT_TRUE(destino.Valid());
    ASSERT_FALSE(origen.Valid());   // moved-from: inválido
    destino.Reset();                // única liberación
    ASSERT_EQ(gDestroyed, 1);
    ASSERT_EQ(gUp, 0);
}

TEST_CASE(MoveAssignReseteaAntesDeRobar) {
    gUp = 2; gCreated = 2; gDestroyed = 0;
    TextureRAII a{Texture2D{1}};
    TextureRAII b{Texture2D{2}};
    a = std::move(b);   // a soltaba su {1} antes de tomar el de b
    ASSERT_EQ(gDestroyed, 1);       // el recurso {1} fue liberado
    ASSERT_TRUE(a.Valid());
    ASSERT_EQ(a.Get().id, 2);
    ASSERT_EQ(gUp, 1);
}

TEST_CASE(ResetEsIdempotente) {
    gUp = 1; gCreated = 1; gDestroyed = 0;
    TextureRAII tex{Texture2D{1}};
    tex.Reset();
    ASSERT_EQ(gDestroyed, 1);
    ASSERT_EQ(gUp, 0);
    tex.Reset();   // segunda vez: NO vuelve a liberar
    ASSERT_EQ(gDestroyed, 1);
    ASSERT_FALSE(tex.Valid());
}

TEST_CASE(DefaultEsInvalidoYResetSinRecursoNoRompe) {
    gUp = 0; gCreated = 0; gDestroyed = 0;
    TextureRAII vacio;
    ASSERT_FALSE(vacio.Valid());
    vacio.Reset();          // liberar algo que no existe: no-op
    ASSERT_EQ(gDestroyed, 0);
    ASSERT_EQ(gUp, 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 01 (RAII Wrapper)...\n";
    return RUN_ALL_TESTS();
}
