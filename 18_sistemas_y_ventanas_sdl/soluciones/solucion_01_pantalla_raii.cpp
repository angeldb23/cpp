/* =============================================================================
   MÓDULO 18 - SOLUCIÓN 01: Wrapper RAII de ventana/renderer
   =============================================================================
   MISMO ENUNCIADO que ejercicios/ejercicio_01_pantalla_raii.cpp.
   SOLUCIÓN COMENTADA: qué hace cada pieza y por qué.

   PUNTOS CLAVE:
   - El dtor es el dueño de la LIMPIEZA: los punteros a handles
     se nullean al destruir y nada se libera dos veces.
   - El orden de destrucción queda INVERSO a la creación:
     si el renderer falla, la ventana se destruye EN abrir()
     (proactive failure cleanup, no en dtor).
     — Extra: abrir() recibe inyección de falla por parámetro; el
     constructor crea ventana + intenta renderer + destruye W si falla).
   - El move reemplaza a puntero nulo: el origen no puede "volver a
     destruir" lo que ya no le pertenece (std::exchange).
   ============================================================================= */
#include <string>
#include <utility>
#include "../../include/test_framework.hpp"

// ============================ SIMULADOR SDL (no tocar) ======================
struct Ventana  { int id = 0; };
struct Renderer { int id = 0; };

static int         gVentanasVivas   = 0;
static int         gRenderersVivos  = 0;
static std::string gLog;   // log de destrucción: 'R' = renderer, 'W' = ventana

inline Ventana  SDL_CreaVentana()          { ++gVentanasVivas;  return Ventana{1}; }
inline Renderer SDL_CreaRenderer(Ventana)  { ++gRenderersVivos; return Renderer{1}; }
inline void SDL_DstrRenderer(Renderer)     { --gRenderersVivos; gLog += 'R'; }
inline void SDL_DstrVentana(Ventana)       { --gVentanasVivas;  gLog += 'W'; }

inline void resetSimulador() {
    gVentanasVivas = 0;
    gRenderersVivos = 0;
    gLog.clear();
}

// ============================ SOLUCIÓN ======================================
class PantallaRAII {
    Ventana*   ventana_  = nullptr;
    Renderer*  renderer_ = nullptr;

public:
    PantallaRAII() = default;

    // Fábrica: ventana primero; si el renderer falla, destruye la ventana
    // AL DARSE LA FALLA (self-clean desde el día 1, no en dtor).
    static PantallaRAII abrir(bool fallaRenderer) {
        Ventana* w = new Ventana{SDL_CreaVentana()};
        if (fallaRenderer) { SDL_DstrVentana(*w); delete w; return PantallaRAII(); }
        Renderer* r = new Renderer{SDL_CreaRenderer(*w)};
        return PantallaRAII(w, r);
    }

    ~PantallaRAII() { cerrar(); }              // dtor = cerrar (única fuente)

    // Copia borrada: el par es exclusivo (SDL_Window*/SDL_Renderer* no
    // se duplica: 2 dueños -> doble destroy).
    PantallaRAII(const PantallaRAII&) = delete;
    PantallaRAII& operator=(const PantallaRAII&) = delete;

    // Movimiento: transfiere el par con std::exchange (el origen queda vacío)
    PantallaRAII(PantallaRAII&& o) noexcept
        : ventana_(std::exchange(o.ventana_, nullptr)),
          renderer_(std::exchange(o.renderer_, nullptr)) {}

    PantallaRAII& operator=(PantallaRAII&& o) noexcept {
        if (this != &o) {
            cerrar();   // la que ya tenía se libera igual (invariante)
            ventana_  = std::exchange(o.ventana_, nullptr);
            renderer_ = std::exchange(o.renderer_, nullptr);
        }
        return *this;
    }

    void cerrar() {
        // mínimamente idempotente: si el ptr es nullptr ya se liberó
        if (renderer_ != nullptr) { SDL_DstrRenderer(*renderer_); delete renderer_; renderer_ = nullptr; }
        if (ventana_  != nullptr) { SDL_DstrVentana(*ventana_);   delete ventana_;  ventana_  = nullptr; }
    }

    bool valida() const { return ventana_ != nullptr && renderer_ != nullptr; }
    Ventana*  ventana()  const { return ventana_; }
    Renderer* renderer() const { return renderer_; }

private:
    PantallaRAII(Ventana* w, Renderer* r) : ventana_(w), renderer_(r) {}
};

// ============================ TESTS (no tocar) ==============================
TEST_CASE(AbrirValidoYDestruccionEnOrden) {
    resetSimulador();
    {
        PantallaRAII p = PantallaRAII::abrir(false);
        ASSERT_TRUE(p.valida());
        ASSERT_EQ(gVentanasVivas, 1);
        ASSERT_EQ(gRenderersVivos, 1);
    } // dtor: renderer y luego ventana
    ASSERT_EQ(gLog, "RW");          // renderer PRIMERO, ventana después
    ASSERT_EQ(gVentanasVivas, 0);
    ASSERT_EQ(gRenderersVivos, 0);
}

TEST_CASE(DestruyeVentanaSiFallaRenderer) {
    resetSimulador();
    {
        PantallaRAII p = PantallaRAII::abrir(true);
        ASSERT_FALSE(p.valida());
        ASSERT_EQ(gVentanasVivas, 0);   // la ventana se destruyó al fallar
        ASSERT_EQ(gRenderersVivos, 0);
    }
    ASSERT_EQ(gLog, "W");               // UNA sola destrucción
}

TEST_CASE(MoverTransfiereYNoDestruyeDoble) {
    resetSimulador();
    PantallaRAII origen = PantallaRAII::abrir(false);
    ASSERT_TRUE(origen.valida());
    PantallaRAII destino = std::move(origen);
    ASSERT_FALSE(origen.valida());
    ASSERT_TRUE(destino.valida());
    ASSERT_EQ(gVentanasVivas, 1);
    ASSERT_EQ(gRenderersVivos, 1);
    {
        PantallaRAII temp = std::move(destino);
        ASSERT_TRUE(temp.valida());
    }
    ASSERT_EQ(gLog, "RW");
    ASSERT_EQ(gRenderersVivos, 0);
    ASSERT_EQ(gVentanasVivas, 0);
}

TEST_CASE(CerrarIdempotente) {
    resetSimulador();
    PantallaRAII p = PantallaRAII::abrir(false);
    p.cerrar();
    p.cerrar();                         // segunda llamada: no-op
    ASSERT_EQ(gLog, "RW");
    ASSERT_FALSE(p.valida());
    ASSERT_EQ(gVentanasVivas, 0);
}

int main() { return RUN_ALL_TESTS(); }
