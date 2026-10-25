/* =============================================================================
   MÓDULO 18 - EJERCICIO 01: Wrapper RAII de ventana/renderer (con orden de
   destrucción verificable)
   =============================================================================
   DIFICULTAD: Básico
   APOYO: Lección 03 (flags/vsync/RAII), Módulos 07 (RAII) y 08 (movimiento)

   CONSIGNA:
   Completá PantallaRAII: el envoltorio del par (ventana, renderer) de la
   lección 03, pero sobre "handles" SIMULADOS y contables, para verificar
   el ciclo de vida SIN abrir ventanas.

   ESTE ARCHIVO YA TRAE (no tocar: el simulador del SDL):
   - Ventana/Renderer: los "handles" (a mano, como structs POD).
   - SDL_CreaVentana/SDL_CreaRenderer/SDL_DstrRenderer/SDL_DstrVentana:
     registran vivos y acumulan un LOG de destrucción (gLog: 'R' = renderer,
     'W' = ventana, en el ORDEN en que ocurrieron).
   - resetSimulador() para que cada test empiece limpio.

   TU CLASE:
   1. Constructor default: sin handles (valida() == false).
   2. static abrir(bool fallaRenderer): crea la ventana PRIMERO; luego el
      renderer. Si fallaRenderer == true, simula que el renderer falló:
      la ventana ya creada se destruye EN ESE MOMENTO (antes de salir) y
      se devuelve un objeto inválido.
   3. NO copiable (borrada): el par no se duplica.
   4. Movible: el move transfiere los dos handles y deja al origen CERRADO
      (el origen no puede destruir el par: prohibido doble 'R','W').
   5. Destructor: si valida(), destruye renderer PRIMERO y ventana después
      (orden inverso a la creación: 'R' antes que 'W' en el log).
   6. cerrar(): idempotente (llamarlo 2 veces destruye una sola vez).

   TESTS (no tocar): ciclo de vida, orden de destrucción, window-destruida-
   al-fallar-renderer, movimiento sin doble destrucción, cierre idempotente.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_01_pantalla_raii.cpp -o ej && ./ej
   ============================================================================= */
#include <string>
#include <utility>
#include "../../include/test_framework.hpp"

// ============================ SIMULADOR SDL (no tocar) ======================
struct Ventana  { int id = 0; };
struct Renderer { int id = 0; };

static int         gVentanasVivas  = 0;
static int         gRenderersVivos = 0;
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

// ============================ TU CÓDIGO AQUÍ ================================
class PantallaRAII {
    Ventana*   ventana_  = nullptr;
    Renderer*  renderer_ = nullptr;

public:
    PantallaRAII() = default;

    // TODO 1: fábrica. Ventana primero; si fallaRenderer, destruí la
    // ventana YA (una sola vez) y devolvé un objeto inválido.
    static PantallaRAII abrir(bool fallaRenderer) {
        (void)fallaRenderer;
        return PantallaRAII();   // reemplazá esto
    }

    void cerrar() {
        // TODO 6: idempotente (destruye si hay handles, y nullear los
        // punteros vuelve cualquier llamada extra un no-op)
        renderer_ = nullptr;
        ventana_ = nullptr;
    }

    // TODO 2: copia borrada (propiedad exclusiva del par)

    // TODO 3: constructor de movimiento (deja el origen cerrado)

    // TODO 4: asignación de movimiento (cerrar() propio antes de robar)

    // TODO 5: destructor — renderer PRIMERO, ventana después


    bool valida() const { return ventana_ != nullptr && renderer_ != nullptr; }

    Ventana*   ventana()  const { return ventana_; }
    Renderer*  renderer() const { return renderer_; }
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
    ASSERT_EQ(gLog, "W");               // UNA sola destrucción de ventana
}

TEST_CASE(MoverTransfiereYNoDestruyeDoble) {
    resetSimulador();
    PantallaRAII origen = PantallaRAII::abrir(false);
    ASSERT_TRUE(origen.valida());
    PantallaRAII destino = std::move(origen);
    ASSERT_FALSE(origen.valida());      // origen queda cerrado
    ASSERT_TRUE(destino.valida());
    ASSERT_EQ(gVentanasVivas, 1);
    ASSERT_EQ(gRenderersVivos, 1);
    {
        PantallaRAII temp = std::move(destino);
        ASSERT_TRUE(temp.valida());
    } // dtor de temp: única destrucción del par
    ASSERT_EQ(gLog, "RW");
    ASSERT_EQ(gRenderersVivos, 0);
    ASSERT_EQ(gVentanasVivas, 0);
}

TEST_CASE(CerrarIdempotente) {
    resetSimulador();
    PantallaRAII p = PantallaRAII::abrir(false);
    p.cerrar();
    p.cerrar();                         // segunda llamada: no-op
    ASSERT_EQ(gLog, "RW");              // una sola vez
    ASSERT_FALSE(p.valida());
    ASSERT_EQ(gVentanasVivas, 0);
}

/* NOTA: no agregues formateos ni includes extra; los headers con std::move
   y std::string ya están. Resolver es TODO el bloque "TU CÓDIGO AQUÍ". */

int main() { return RUN_ALL_TESTS(); }
