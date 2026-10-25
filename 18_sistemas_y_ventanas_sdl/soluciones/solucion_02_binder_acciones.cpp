/* =============================================================================
   MÓDULO 18 - SOLUCIÓN 02: Binder de eventos a acciones
   =============================================================================
   MISMO ENUNCIADO que ejercicios/ejercicio_02_binder_acciones.cpp.
   SOLUCIÓN COMENTADA: qué hace cada pieza y por qué.

   PUNTOS CLAVE:
   - La tabla es un vector de {codigo, accion, requiereCtrl} para teclado
     y otro "espacio" (el PAD usa los mismos códigos lógicos de binding:
     se distingue por el TIPO de evento, no por la tabla).
   - aplicar() no guarda estado: el "edge" vive en el EVENTO (Press = fresco,
     repeat = falso Press). Menos estado = menos bugs (lección 04).
   - Un mismo código puede mapearse con y sin ctrl (ESPACIO sin ctrl y
     guardado con ctrl en la L05): la REGLA es "el binding exige, el
     evento provee".
   ============================================================================= */
#include <vector>
#include <string>
#include <algorithm>
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

// ============================ SOLUCIÓN ======================================
class Binder {
    struct Binding {
        int    codigo = 0;
        Accion accion = Accion::Cerrar;
        bool   requiereCtrl = false;
    };

    std::vector<Binding> teclado_;
    std::vector<Binding> pad_;

public:
    Binder& bind(int tecla, Accion a, bool requiereCtrl = false) {
        teclado_.push_back(Binding{tecla, a, requiereCtrl});
        return *this;
    }

    Binder& bindPad(int boton, Accion a) {
        pad_.push_back(Binding{boton, a, false}); // gamepad no usa ctrl
        return *this;
    }

    std::vector<Accion> aplicar(const EventoCola& e) {
        std::vector<Accion> out;
        // SOLO las transiciones 'apretado' disparan algo
        if (e.tipo == EventoCola::Tipo::Press && !e.repeat) {
            for (const Binding& b : teclado_) {
                if (b.codigo == e.codigo && (!b.requiereCtrl || e.ctrl))
                    out.push_back(b.accion);
            }
        }
        if (e.tipo == EventoCola::Tipo::PadPress) {
            for (const Binding& b : pad_) {
                if (b.codigo == e.codigo)
                    out.push_back(b.accion);
            }
        }
        // Release / PadRelease / Press-repeat: no dispara (transición de
        // soltar o repetición del SO: el edge es el Press fresco)
        return out;
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
