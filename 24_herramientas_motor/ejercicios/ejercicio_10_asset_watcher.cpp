/* =============================================================================
   MÓDULO 24 - EJERCICIO 10: Watcher de assets con debounce
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 10 (hot-reload, snapshot/diff y coalescencia).

   CONSIGNA
   --------
   1. `diffSnapshots(antes, despues)` compara dos mapas path -> mtime y
      devuelve eventos ORDENADOS POR PATH:
        - path sólo en `despues`: Added,
        - en ambos con mtime distinto: Modified,
        - sólo en `antes`: Removed.
   2. `Debouncer(cooldown)`:
        - `notificar(path, ahora)`: si el path NO está pendiente, registra
          `ahora`. Si ya está pendiente, NO cambia el tiempo (coalescencia).
        - `listos(ahora)`: devuelve los paths cuyo primer aviso cumplió
          `ahora - aviso >= cooldown`, ordenados por path, y los elimina de
          pendientes.
        - `pendientes()`: cuántos quedan.
   ============================================================================= */
#include "../include/gui_util.hpp"
#include "../../include/test_framework.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

enum class Cambio { Agregado, Modificado, Borrado };

struct Evento {
    std::string path;
    Cambio cambio = Cambio::Modificado;
};

using Snapshot = std::map<std::string, std::int64_t>;

std::vector<Evento> diffSnapshots(const Snapshot& antes, const Snapshot& despues) {
    // TODO
    (void)antes;
    (void)despues;
    return {};
}

class Debouncer {
public:
    explicit Debouncer(double cooldown) : cooldown_(cooldown) {}

    void notificar(const std::string& path, double ahora) {
        // TODO
        (void)path;
        (void)ahora;
    }

    std::vector<std::string> listos(double ahora) {
        // TODO
        (void)ahora;
        return {};
    }

    std::size_t pendientes() const { return pendientes_.size(); }

private:
    double cooldown_ = 0.0;
    std::map<std::string, double> pendientes_;
};

// ============================================================================
// TESTS (no tocar)
// ============================================================================
TEST_CASE(DiffAgregaModificaYBorra) {
    const Snapshot antes = {{"a.png", 1}, {"b.png", 2}, {"c.png", 3}};
    const Snapshot despues = {{"b.png", 20}, {"c.png", 3}, {"d.png", 4}};
    const auto ev = diffSnapshots(antes, despues);
    ASSERT_EQ(ev.size(), 3u);
    ASSERT_EQ(ev[0].path, std::string("a.png"));
    ASSERT_EQ(static_cast<int>(ev[0].cambio), static_cast<int>(Cambio::Borrado));
    ASSERT_EQ(ev[1].path, std::string("b.png"));
    ASSERT_EQ(static_cast<int>(ev[1].cambio), static_cast<int>(Cambio::Modificado));
    ASSERT_EQ(ev[2].path, std::string("d.png"));
    ASSERT_EQ(static_cast<int>(ev[2].cambio), static_cast<int>(Cambio::Agregado));
}

TEST_CASE(DiffSinCambios) {
    const Snapshot s = {{"a", 1}, {"b", 2}};
    ASSERT_TRUE(diffSnapshots(s, s).empty());
}

TEST_CASE(DebounceEsperaCooldown) {
    Debouncer d(0.25);
    d.notificar("a.png", 10.0);
    ASSERT_EQ(d.pendientes(), 1u);
    ASSERT_TRUE(d.listos(10.20).empty());
    const auto r = d.listos(10.25);
    ASSERT_EQ(r.size(), 1u);
    ASSERT_EQ(r[0], std::string("a.png"));
    ASSERT_EQ(d.pendientes(), 0u);
    ASSERT_TRUE(d.listos(100.0).empty());
}

TEST_CASE(DebounceCoalesceRafaga) {
    Debouncer d(0.25);
    d.notificar("shader.glsl", 0.0);
    d.notificar("shader.glsl", 0.10);
    d.notificar("shader.glsl", 0.20);
    ASSERT_EQ(d.pendientes(), 1u);
    // Si el timer se reiniciara con cada aviso, no estaria listo hasta 0.45.
    const auto r = d.listos(0.30);
    ASSERT_EQ(r.size(), 1u);
    ASSERT_EQ(r[0], std::string("shader.glsl"));
}

TEST_CASE(DebounceVariosOrdenados) {
    Debouncer d(1.0);
    d.notificar("zeta", 0.5);
    d.notificar("alfa", 0.0);
    ASSERT_EQ(d.pendientes(), 2u);
    const auto r = d.listos(2.0);
    ASSERT_EQ(r.size(), 2u);
    ASSERT_EQ(r[0], std::string("alfa"));
    ASSERT_EQ(r[1], std::string("zeta"));
}

int main() {
    std::cout << "Ejercicio 10 (asset watcher)...\n";
    return RUN_ALL_TESTS();
}
