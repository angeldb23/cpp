/* =============================================================================
   MÓDULO 18 - EJERCICIO 02: Binder de eventos a acciones (con filtros de
   repeat y modificadores)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lecciones 04 (event loop) y 05 (texto/gamepad/acciones), Módulo 16
          (input mapping / edge detection)

   CONSIGNA:
   En SDL los eventos de teclado llegan como hecho con NUEVO estado por
   repeat del SO (lección 04: event.key.repeat). Completá el BINDER: la
   tabla que traduce eventos a ACCIONES (Jump, Shoot, Sale?) como los del
   módulo 16, pero con la SALIDA invertida: SDL te da eventos, no estados.
   - SDL te da eventos; raylib te daba el estado. El binder necesita EDGE:
     un KeyDown con repeat==true NUNCA dispara acción nueva.
   - Un combo (mod + la tecla) sólo se activa si el mod está AL presionar.
   - El gamepad mapea su botón abstracto (South/East, L05) directo a
     bindings; eventos de ambos dispositivos que alimentan la misma acción.

   ESTRUCTURA (espejo del SDL_KeyboardEvent de la L04, en tipos propios):
     struct EventoCola {
         enum class Tipo { Press, Release, PadPress, PadRelease };
         Tipo tipo;
         int  codigo;        // scancode (teclado o botón gamepad)
         bool repeat;        // SOLO para Press de teclado
         bool ctrl;          // modificador activo AL llegar el evento
     };
   El binder mantiene ISA tabla y expone:
     void bind(Tecla t, Accion a);          (registra binding de teclado)
     void bindPad(Ppad b, Accion a);         (registra binding gamepad)
     const std::vector<Accion>& aplicar(const EventoCola& e); // devuelve
        las acciones DISPARADAS por ese evento (0, 1 o 2): un Press con
        repeat dispara UNA sola vez (no en cada repeat del SO).

   REGLAS CLAVE (las que prueban):
   1. Press (no repeat) de teclado o PadPress: dispara UNA vez la acción
      del binding (y exige ctrl si el binding lo pidió).
   2. Press con repeat == true: no dispara NADA (son los repeats del SO).
   3. Binding con requiereCtrl: sólo dispara si el evento trae ctrl==true.
   4. Gamepad y teclado alimentan la MISMA acción mediante bindings separados
      (ej.: Espacio y PAD_SOUTH -> Saltar).
   5. Release / PadRelease: no acción (transición de soltar, no de apretar).

   TESTS (no tocar). COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_02_binder_acciones.cpp -o ej && ./ej
   ============================================================================= */
#include <vector>
#include <string>
#include "../../include/test_framework.hpp"

// ============================ MODELO (no tocar) ==============================
struct EventoCola {
    enum class Tipo { Press, Release, PadPress, PadRelease };
    Tipo tipo;
    int  codigo;
    bool repeat;
    bool ctrl;
};

enum class Accion : int {
    Saltar, Disparar, Guardar, MoverX, Cerrar
};

constexpr int TECLA_ESPACIO = 32;
constexpr int TECLA_S = 18;
constexpr int TECLA_ES = 5;
constexpr int PAD_SOUTH = 0;
constexpr int PAD_EAST = 1;
constexpr int PAD_START = 6;

// ============================ TU CÓDIGO AQUÍ ================================
class Binder {
public:
    // requiereCtrl: sólo dispara si el evento llega con ctrl == true
    Binder& bind(int tecla, Accion a, bool requiereCtrl = false) {
        (void)tecla; (void)a; (void)requiereCtrl;
        return *this;
    }

    Binder& bindPad(int boton, Accion a) {
        (void)boton; (void)a;
        return *this;
    }

    // Devuelve las acciones disparadas por ese evento (0, 1 o más).
    std::vector<Accion> aplicar(const EventoCola& e) {
        (void)e;
        return {};   // TODO: presión/combo/edge aquí
    }
};

// ============================ TESTS (no tocar) ==============================
TEST_CASE(PresTeclaDisparaUnaSolaVez) {
    Binder b;
    b.bind(TECLA_ESPACIO, Accion::Saltar);
    ASSERT_EQ(b.aplicar({EventoCola::Tipo::Press, TECLA_ESPACIO, false, false}).size(), std::size_t(1));
    ASSERT_EQ(b.aplicar({EventoCola::Tipo::Press, TECLA_ESPACIO, true, false}).size(), std::size_t(0));
    ASSERT_EQ(b.aplicar({EventoCola::Tipo::Release, TECLA_ESPACIO, false, false}).size(), std::size_t(0));
}

TEST_CASE(RepeatDelSistemaNoDispara) {
    Binder b;
    b.bind(TECLA_ESPACIO, Accion::Saltar);
    // Total: el usuario presiona el espacio y el SO repite el evento; en
    // SDL cada repeat llega como SDL_EVENT_KEY_DOWN con key.repeat == true:
    ASSERT_EQ(b.aplicar({EventoCola::Tipo::Press, TECLA_ESPACIO, false, false}).size(), std::size_t(1));
    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(b.aplicar({EventoCola::Tipo::Press, TECLA_ESPACIO, true, false}).size(), std::size_t(0));
    }
}

TEST_CASE(ComboCtrlS) {
    Binder b;
    b.bind(TECLA_S, Accion::Guardar, true);  // <- exige ctrl
    b.bind(TECLA_ESPACIO, Accion::Disparar);
    // sin Ctrl: S no dispara nada (está reservado al combo)
    ASSERT_EQ(b.aplicar({EventoCola::Tipo::Press, TECLA_S, false, false}).size(), std::size_t(0));
    // con Ctrl: Guardar
    auto r = b.aplicar({EventoCola::Tipo::Press, TECLA_S, false, true});
    ASSERT_EQ(r.size(), std::size_t(1));
    ASSERT_EQ(int(r[0]), int(Accion::Guardar));
}

TEST_CASE(GamepadYTecladoAlimentanLaMismaAccion) {
    Binder b;
    b.bind(TECLA_ESPACIO, Accion::Saltar);   // salta con espacio
    b.bindPad(PAD_SOUTH, Accion::Saltar);    // salta con PAD South
    b.bindPad(PAD_EAST, Accion::Disparar);
    auto r = b.aplicar({EventoCola::Tipo::PadPress, PAD_SOUTH, false, false});
    ASSERT_EQ(r.size(), std::size_t(1));
    ASSERT_EQ(int(r[0]), int(Accion::Saltar));
    // un PadRelease NO dispara (solo transiciones de press)
    ASSERT_EQ(b.aplicar({EventoCola::Tipo::PadRelease, PAD_SOUTH, false, false}).size(), std::size_t(0));
}

TEST_CASE(TeclaSinBindingNoDisparaNada) {
    Binder b;
    b.bind(TECLA_ESPACIO, Accion::Saltar);
    auto r = b.aplicar({EventoCola::Tipo::Press, TECLA_ES, false, false});
    ASSERT_EQ(r.size(), std::size_t(0));
}

int main() { return RUN_ALL_TESTS(); }
