/* =============================================================================
   MÓDULO 25 - SOLUCIÓN 04: pool de voces con prioridad y voice stealing
   =============================================================================
   El criterio de robo YA ESTÁ en la pasada única: buscar simultáneamente el
   mejor candidato (menor prioridad, empate por reloj más viejo) recorriendo
   los slots UNA vez. El "rechazo sin estado" hace el pool determinista y
   testable: si nadie es robable, no se toca nada (idempotencia).
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <vector>

class PoolVoces {
public:
    explicit PoolVoces(int slots) : slots_(static_cast<std::size_t>(slots), Voz{}) {}

    int play(int id, int prioridad, long reloj) {
        /* 1) primer slot libre. */
        for (std::size_t i = 0; i < slots_.size(); ++i) {
            if (slots_[i].id == -1) {
                slots_[i].id = id;
                slots_[i].prioridad = prioridad;
                slots_[i].reloj = reloj;
                return static_cast<int>(i);
            }
        }
        /* 2) robo: mejor candidato en una pasada. */
        int mejor = -1;
        for (std::size_t i = 0; i < slots_.size(); ++i) {
            const Voz& v = slots_[i];
            if (v.prioridad >= prioridad) {
                continue; /* nunca robo iguales o superiores */
            }
            if (mejor == -1) {
                mejor = static_cast<int>(i);
                continue;
            }
            const Voz& b = slots_[static_cast<std::size_t>(mejor)];
            const bool gana = (v.prioridad < b.prioridad)
                              || (v.prioridad == b.prioridad && v.reloj < b.reloj);
            if (gana) {
                mejor = static_cast<int>(i);
            }
        }
        if (mejor == -1) {
            return -1; /* 3) rechazo explícito: estado intacto */
        }
        slots_[static_cast<std::size_t>(mejor)].id = id;
        slots_[static_cast<std::size_t>(mejor)].prioridad = prioridad;
        slots_[static_cast<std::size_t>(mejor)].reloj = reloj;
        return mejor;
    }

    void stop(int slot) {
        if (slot >= 0 && slot < static_cast<int>(slots_.size())) {
            slots_[static_cast<std::size_t>(slot)].id = -1;
        }
    }

    int activas() const {
        int n = 0;
        for (const Voz& v : slots_) {
            if (v.id != -1) { ++n; }
        }
        return n;
    }

    int idDe(int slot) const {
        if (slot < 0 || slot >= static_cast<int>(slots_.size())) {
            return -1;
        }
        return slots_[static_cast<std::size_t>(slot)].id;
    }

private:
    struct Voz {
        int id = -1;
        int prioridad = 0;
        long reloj = 0;
    };
    std::vector<Voz> slots_;
};

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(OcupaPrimerLibre) {
    PoolVoces p(4);
    ASSERT_TRUE(p.play(101, 5, 0) == 0);
    ASSERT_TRUE(p.play(102, 3, 1) == 1);
    ASSERT_TRUE(p.activas() == 2);
    ASSERT_TRUE(p.idDe(1) == 102);
    p.stop(1);
    ASSERT_TRUE(p.activas() == 1);
    ASSERT_TRUE(p.idDe(1) == -1);
    ASSERT_TRUE(p.play(103, 4, 2) == 1);
}

TEST_CASE(RoboConPrioridad) {
    PoolVoces p(4);
    ASSERT_TRUE(p.play(101, 5, 0) == 0);
    ASSERT_TRUE(p.play(102, 3, 1) == 1);
    ASSERT_TRUE(p.play(103, 4, 2) == 2);
    ASSERT_TRUE(p.play(104, 6, 3) == 3);
    const int slot = p.play(105, 5, 4);
    ASSERT_TRUE(slot == 1);
    ASSERT_TRUE(p.idDe(1) == 105);
}

TEST_CASE(EmpateRobaLaMasVieja) {
    PoolVoces p(2);
    ASSERT_TRUE(p.play(201, 3, 10) == 0);
    ASSERT_TRUE(p.play(202, 3, 20) == 1);
    const int slot = p.play(203, 5, 30);
    ASSERT_TRUE(slot == 0);
    ASSERT_TRUE(p.idDe(0) == 203);
    ASSERT_TRUE(p.idDe(1) == 202);
}

TEST_CASE(RechazoSinRobo) {
    PoolVoces p(2);
    ASSERT_TRUE(p.play(301, 7, 0) == 0);
    ASSERT_TRUE(p.play(302, 7, 1) == 1);
    ASSERT_TRUE(p.play(303, 7, 2) == -1);
    ASSERT_TRUE(p.play(303, 2, 3) == -1);
    ASSERT_TRUE(p.idDe(0) == 301 && p.idDe(1) == 302);
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
