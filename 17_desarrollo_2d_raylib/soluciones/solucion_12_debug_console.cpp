/* =============================================================================
   MÓDULO 17 - SOLUCIÓN 12: ScreenConsole — Ring Buffer, Decaimiento y Filtro
   =============================================================================
   Resuelve ejercicio_12_debug_console.cpp. La llave es un deque<Entry> con
   push_back + pop_front: la inserción es al final (orden viejo→nuevo gratis)
   y cuando pasa de maxLines se descarta por el frente (la MÁS VIEJA).
   Update resta dt a TODAS las vidas y purga con remove_if (erase idiom);
   Snapshot formatea "[cat] msg" y filtra por categoría exacta ("": todas).

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_12_debug_console.cpp -o sol && ./sol
   ============================================================================= */
#include <deque>
#include <string>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <iostream>
#include "../../include/test_framework.hpp"

// ============================ SOLUCIÓN =======================================
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
        if (cat.empty()) return;   // categoría vacía NO existe: no-op (defensive)
        entries_.push_back({cat, msg, lifetime_});
        if (entries_.size() > maxLines_) {
            entries_.pop_front();  // descarta la más vieja
        }
    }

    // TODO 2: decaimiento + purge de expiradas
    void Update(float dt) {
        for (Entry& e : entries_) e.life -= dt;
        entries_.erase(
            std::remove_if(entries_.begin(), entries_.end(),
                           [](const Entry& e) { return e.life <= 0.0f; }),
            entries_.end());
    }

    // TODO 3: snapshot filtrada en orden de inserción
    void Snapshot(const std::string& filter, std::vector<std::string>& out) const {
        out.clear();
        for (const Entry& e : entries_) {
            if (!filter.empty() && e.cat != filter) continue;
            out.push_back("[" + e.cat + "] " + e.msg);
        }
    }

    void SetFilter(const std::string& cat) { filter_ = cat; }
    std::string GetFilter() const { return filter_; }
    size_t RawCount() const { return entries_.size(); }
};

// ============================ TESTS (iguales) ================================
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
    std::cout << "Solución 12: ScreenConsole (ring, decaimiento, filtro)\n";
    return RUN_ALL_TESTS();
}
