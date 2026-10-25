/* =============================================================================
   MÓDULO 17 - EJERCICIO 12: ScreenConsole — Ring Buffer, Decaimiento y Filtro
   =============================================================================
   DIFICULTAD: Intermedio
   APOYO: Lección 11 (Consola en pantalla)

   CONSIGNA:
   Implementa la consola de debug "en pantalla" (lección 11):

   1) Log(cat, msg): agrega una línea con vida útil de lifetime. Si hay más
      de maxLines, la MÁS VIEJA se descarta (ring con deque).
   2) Update(dt): resta dt a las vidas (solo a las que tienen edad mayor a 0)
      y PURGA las expiradas (life <= 0).
   3) Snapshot(filter): devuelve SOLO las líneas visibles con el filtro:
          filter == ""  => todas (en orden de inserción, viejo→nuevo)
          filter == cat => solo esa categoría
   4) Categoría vacía no existe: guard de "cat vacía" => no-op (defensive).

   TESTS (no tocar): ring de max, orden viejo→nuevo, decae en purga
   progresiva y filtro por categoría.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_12_debug_console.cpp -o ej && ./ej
   ============================================================================= */
#include <deque>
#include <string>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <iostream>
#include "../../include/test_framework.hpp"

// ============================ TU CÓDIGO AQUÍ ================================
class ScreenConsole {
    struct Entry {
        std::string cat;
        std::string msg;
        float life = 0.0f;
    };
    std::deque<Entry> entries_;
    std::string filter_ = "";
    float lifetime_;
    size_t maxLines_;

public:
    explicit ScreenConsole(size_t maxLines = 8, float lifetime = 5.0f)
        : lifetime_(lifetime), maxLines_(maxLines) {}

    // TODO 1: log con ring (la MÁS VIEJA sale si pasa de maxLines)
    void Log(const std::string& cat, const std::string& msg) {
        (void)cat; (void)msg;
    }

    // TODO 2: decaimiento + purge de expiradas
    void Update(float dt) {
        (void)dt;
    }

    // TODO 3: snapshot filtrada en orden de inserción
    void Snapshot(const std::string& filter, std::vector<std::string>& out) const {
        (void)filter; (void)out;
    }

    void SetFilter(const std::string& cat) { filter_ = cat; }
    std::string GetFilter() const { return filter_; }
    size_t RawCount() const { return entries_.size(); }
};

// ============================ TESTS (no tocar) ==============================
TEST_CASE(RingMantieneMaxLineas) {
    ScreenConsole c(3, 10.0f);
    c.Log("col",  "a");
    c.Log("audio", "b");
    c.Log("col",  "c");
    ASSERT_EQ(c.RawCount(), 3u);
    c.Log("ui", "d");               // 4ta: descarta la "a" (la más vieja)
    ASSERT_EQ(c.RawCount(), 3u);
    std::vector<std::string> all;
    c.Snapshot("", all);
    // quedaron b, c, d (a descartada)
    ASSERT_EQ(all.size(), 3u);
    ASSERT_EQ(all[0], "[audio] b");
    ASSERT_EQ(all[2], "[ui] d");
}

TEST_CASE(SnapshotEnOrdenInsercionViejoANuevo) {
    ScreenConsole c(5, 10.0f);
    c.Log("col", "uno");
    c.Log("col", "dos");
    c.Log("audio", "tres");
    std::vector<std::string> all;
    c.Snapshot("", all);
    ASSERT_EQ(all.size(), 3u);
    ASSERT_EQ(all[0], "[col] uno");
    ASSERT_EQ(all[1], "[col] dos");
    ASSERT_EQ(all[2], "[audio] tres");
    // formato: "[cat] msg"
}

TEST_CASE(FiltroPorCategoria) {
    ScreenConsole c(5, 10.0f);
    c.Log("col",   "piso");
    c.Log("audio", "boom");
    c.Log("col",   "dano");
    c.Log("ui",    "pausa");
    std::vector<std::string> col;
    c.Snapshot("col", col);
    ASSERT_EQ(col.size(), 2u);
    ASSERT_EQ(col[0], "[col] piso");
    ASSERT_EQ(col[1], "[col] dano");
    // filtro plural con nada
    std::vector<std::string> nada;
    c.Snapshot("superbus", nada);
    ASSERT_EQ(nada.size(), 0u);
}

TEST_CASE(DecaeYPurgaProgresivamente) {
    ScreenConsole c(5, 2.0f);          // vida 2s por línea
    c.Log("col", "m1");
    c.Update(1.0f);                     // 1s decurrido: sigue visible
    std::vector<std::string> s1;
    c.Snapshot("", s1);
    ASSERT_EQ(s1.size(), 1u);
    c.Update(1.5f);                     // 2.5s TOTAL: expira
    c.Snapshot("", s1);
    ASSERT_EQ(s1.size(), 0u);
}

TEST_CASE(LogConCatVaciaEsNoOp) {
    ScreenConsole c(3, 10.0f);
    c.Log("", "mensaje huérfano");     // categoría vacía NO entra
    ASSERT_EQ(c.RawCount(), 0u);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 12 (Debug Console)...\n";
    return RUN_ALL_TESTS();
}
