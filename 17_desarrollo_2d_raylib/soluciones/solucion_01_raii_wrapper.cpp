/* =============================================================================
   MÓDULO 17 - SOLUCIÓN 01: Wrapper RAII Move-Only para Resource de raylib
   =============================================================================
   Resuelve ejercicio_01_raii_wrapper.cpp. Los TODOs están resueltos con
   comentarios de explicación: el patrón clave es "robar y vaciar al origen"
   en el movimiento, y un flag valid_ que hace idempotente a Reset/dtor.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_01_raii_wrapper.cpp -o sol && ./sol
   ============================================================================= */
#include <iostream>
#include <utility>
#include <cassert>
#include "../../include/test_framework.hpp"

// ============================ CONTADORES SIMULADOS (no tocar) ================
struct Texture2D { int id = 0; };

static int gUp = 0;
static int gCreated = 0;
static int gDestroyed = 0;

inline Texture2D SimulateLoad() { ++gUp; ++gCreated; return Texture2D{gCreated}; }
inline void UnloadTexture(Texture2D) { --gUp; ++gDestroyed; }

// ============================ SOLUCIÓN =======================================
class TextureRAII {
    Texture2D tex_{};
    bool valid_ = false;

public:
    TextureRAII() = default;

    // TODO 1: constructor desde handle crudo (toma posesión)
    explicit TextureRAII(Texture2D t) : tex_(t), valid_(true) {}

    TextureRAII(const TextureRAII&) = delete;
    TextureRAII& operator=(const TextureRAII&) = delete;

    // TODO 2: constructor de movimiento — el corazón del patrón RAII move-only.
    // Tomo el handle y la validez del other; en el origen dejo valid_=false
    // para que su destructor NO haga el UnloadTexture (doble-liberación).
    TextureRAII(TextureRAII&& other) noexcept : tex_(other.tex_), valid_(other.valid_) {
        other.valid_ = false;   // el moved-from queda sin recurso (estado "vacío")
    }

    // TODO 3: asignación de movimiento. Orden exacto importado del módulo 08:
    // 1) self-check (a = std::move(a) NO debe destruirse a sí mismo),
    // 2) reset del propio recurso (libera lo que tenia YA),
    // 3) robar el recurso del other y vaciarlo.
    TextureRAII& operator=(TextureRAII&& other) noexcept {
        if (this != &other) {
            Reset();                 // devuelve al mundo lo que ya tenía
            tex_ = other.tex_;
            valid_ = other.valid_;
            other.valid_ = false;
        }
        return *this;
    }

    // TODO 4: destructor. Con UnloadFn seguro (el flag evita el doble error).
    ~TextureRAII() { Reset(); }

    // TODO 5: reset() idempotente: el guard de valid_ bloquea el segundo Unload.
    // Es la MISMA lógica que usa std::unique_ptr::reset con su deleter.
    void Reset() {
        if (valid_) {
            UnloadTexture(tex_);
            valid_ = false;
        }
    }

    bool Valid() const { return valid_; }
    Texture2D Get() const { return tex_; }
};

// ============================ TESTS (iguales que el ejercicio) ===============
TEST_CASE(LoadYDtorAutomático) {
    gUp = 0; gCreated = 0; gDestroyed = 0;
    {
        TextureRAII tex{SimulateLoad()};
        ASSERT_TRUE(tex.Valid());
        ASSERT_EQ(gUp, 1);
    }
    ASSERT_EQ(gUp, 0);
    ASSERT_EQ(gDestroyed, 1);
}

TEST_CASE(MoveTransfiereSinDobleUnload) {
    gUp = 1; gCreated = 1; gDestroyed = 0;
    TextureRAII origen{Texture2D{1}};
    TextureRAII destino = std::move(origen);
    ASSERT_TRUE(destino.Valid());
    ASSERT_FALSE(origen.Valid());
    destino.Reset();
    ASSERT_EQ(gDestroyed, 1);
    ASSERT_EQ(gUp, 0);
}

TEST_CASE(MoveAssignReseteaAntesDeRobar) {
    gUp = 2; gCreated = 2; gDestroyed = 0;
    TextureRAII a{Texture2D{1}};
    TextureRAII b{Texture2D{2}};
    a = std::move(b);
    ASSERT_EQ(gDestroyed, 1);
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
    tex.Reset();
    ASSERT_EQ(gDestroyed, 1);
    ASSERT_FALSE(tex.Valid());
}

TEST_CASE(DefaultEsInvalidoYResetSinRecursoNoRompe) {
    gUp = 0; gCreated = 0; gDestroyed = 0;
    TextureRAII vacio;
    ASSERT_FALSE(vacio.Valid());
    vacio.Reset();
    ASSERT_EQ(gDestroyed, 0);
    ASSERT_EQ(gUp, 0);
}

int main() {
    std::cout << "Solución 01: Wrapper RAII Move-Only\n";
    return RUN_ALL_TESTS();
}
