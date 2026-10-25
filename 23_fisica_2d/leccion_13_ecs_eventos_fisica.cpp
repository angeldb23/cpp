/* =============================================================================
   MÓDULO 23 - LECCIÓN 13: Eventos de física y callbacks al gameplay
   =============================================================================
   TEMA 8 de TEMAS.md (segunda lección): callbacks a eventos de juego.

   EL PROBLEMA
   -----------
   El sistema de física sabe que dos cuerpos se tocaron. El gameplay necesita
   saber cosas como "el jugador pisó el trigger", "el enemigo recibió un
   golpe", "terminó la ronda". Si el sistema de física llama directamente a
   `matarJugador()` en medio del `Step`, se rompe la iteración y aparecen
   bugs difíciles de reproducir (destruir un cuerpo mientras Box2D lo está
   resolviendo es UB).

   SOLUCIÓN: COLA DE EVENTOS (DEFERRED EVENTS)
   -------------------------------------------
       1. La física detecta el cambio de estado (begin/end de contacto).
       2. Encola un evento inmutable: {tipo, entidadA, entidadB, normal}.
       3. Termina el Step.
       4. El sistema de gameplay drena la cola y decide (daño, sonido, borrar).

   ESTADO DE CONTACTOS
   -------------------
   BeginContact se emite una sola vez cuando el par empieza a tocarse.
   EndContact, cuando deja de tocarse. Para eso hay que recordar el conjunto
   de pares activos del frame anterior: `paresActivos`. Sin ese estado, cada
   frame emitirías "begin" y el juego dispararía el trigger 60 veces por
   segundo.

   ORDEN Y DETERMINISMO
   --------------------
   Los eventos se procesan en el orden en que se detectaron (o se ordenan por
   ID para que sea reproducible). Si usás `std::unordered_set` para los pares,
   el orden de iteración no es estable entre plataformas: para el gameplay
   conviene ordenar antes de despachar.

   SENSORES Y TRIGGERS
   -------------------
   Un sensor no genera respuesta física, pero SÍ begin/end. Es el mecanismo
   estándar para pickups, checkpoints y zonas de daño. El evento debe llevar
   el flag `sensor` para que el gameplay sepa si además hay que resolver el
   choque.
   ============================================================================= */
#include "include/fisica2d.hpp"

#include <algorithm>
#include <deque>
#include <functional>
#include <iomanip>
#include <iostream>
#include <set>
#include <vector>

using f2d::AABB;
using f2d::Pair;

namespace {

enum class TipoEvento { BeginContact, EndContact, Trigger };

struct Evento {
    TipoEvento tipo;
    int a = -1;
    int b = -1;
    f2d::Vec2 normal;
    bool sensor = false;
};

class ColaEventos {
public:
    void encolar(const Evento& e) { cola_.push_back(e); }

    // Drena en orden de llegada. El callback puede devolver false para
    // "consumir" el evento y no pasarlo al siguiente sistema.
    void despachar(const std::function<bool(const Evento&)>& fn) {
        while (!cola_.empty()) {
            const Evento e = cola_.front();
            cola_.pop_front();
            if (!fn(e)) continue;
        }
    }

    size_t tam() const { return cola_.size(); }

private:
    std::deque<Evento> cola_;
};

struct Cuerpo {
    int id = -1;
    AABB caja;
    bool sensor = false;
    bool vivo = true;
};

// Detecta pares que se tocan, compara con el frame anterior y emite eventos.
void detectar(std::vector<Cuerpo>& cuerpos, std::set<Pair>& activos,
              ColaEventos& cola) {
    std::set<Pair> ahora;
    for (size_t i = 0; i < cuerpos.size(); ++i) {
        if (!cuerpos[i].vivo) continue;
        for (size_t j = i + 1; j < cuerpos.size(); ++j) {
            if (!cuerpos[j].vivo) continue;
            if (!cuerpos[i].caja.overlaps(cuerpos[j].caja)) continue;
            const Pair p{std::min(cuerpos[i].id, cuerpos[j].id),
                         std::max(cuerpos[i].id, cuerpos[j].id)};
            ahora.insert(p);
            if (!activos.contains(p)) {
                cola.encolar({cuerpos[i].sensor || cuerpos[j].sensor ? TipoEvento::Trigger
                                                                   : TipoEvento::BeginContact,
                              p.a, p.b, {1.0f, 0.0f},
                              cuerpos[i].sensor || cuerpos[j].sensor});
            }
        }
    }
    for (const auto& p : activos) {
        if (!ahora.contains(p)) {
            cola.encolar({TipoEvento::EndContact, p.a, p.b, {}, false});
        }
    }
    activos = std::move(ahora);
}

const char* nombre(TipoEvento t) {
    switch (t) {
        case TipoEvento::BeginContact: return "BeginContact";
        case TipoEvento::EndContact: return "EndContact";
        case TipoEvento::Trigger: return "Trigger";
    }
    return "?";
}

} // namespace

int main() {
    std::cout << std::fixed << std::setprecision(2);

    std::vector<Cuerpo> cuerpos = {
        {0, AABB(0.0f, 0.0f, 1.0f, 1.0f), false, true},
        {1, AABB(0.5f, 0.0f, 1.5f, 1.0f), false, true},
        {2, AABB(3.0f, 0.0f, 4.0f, 1.0f), true, true},  // trigger/pickup
    };

    std::set<Pair> activos;
    ColaEventos cola;

    // Frame 1: 0 y 1 se tocan, 2 está lejos.
    detectar(cuerpos, activos, cola);
    std::cout << "Frame 1 (cola=" << cola.tam() << "):\n";
    cola.despachar([](const Evento& e) {
        std::cout << "  " << nombre(e.tipo) << " (" << e.a << ", " << e.b
                  << ")" << (e.sensor ? " [sensor]" : "") << "\n";
        return true;
    });

    // Frame 2: el cuerpo 1 se mueve y toca el trigger 2. El par (0,1) sigue.
    cuerpos[1].caja = AABB(2.5f, 0.0f, 3.5f, 1.0f);
    detectar(cuerpos, activos, cola);
    std::cout << "Frame 2 (cola=" << cola.tam() << "):\n";
    cola.despachar([](const Evento& e) {
        std::cout << "  " << nombre(e.tipo) << " (" << e.a << ", " << e.b
                  << ")" << (e.sensor ? " [sensor]" : "") << "\n";
        return true;
    });

    // Frame 3: el trigger "consume" el pickup: se marca muerto DESPUÉS del
    // Step, no dentro de la detección.
    cuerpos[2].vivo = false;
    detectar(cuerpos, activos, cola);
    std::cout << "Frame 3 (pickup consumido, cola=" << cola.tam() << "):\n";
    cola.despachar([](const Evento& e) {
        std::cout << "  " << nombre(e.tipo) << " (" << e.a << ", " << e.b
                  << ")" << (e.sensor ? " [sensor]" : "") << "\n";
        return true;
    });

    std::cout << "\nSin el set de pares activos, cada frame emitiría otro\n"
                 "BeginContact y el pickup se consumiría 60 veces por segundo.\n"
                 "Los eventos se despachan SIEMPRE después del Step.\n";
    return 0;
}
