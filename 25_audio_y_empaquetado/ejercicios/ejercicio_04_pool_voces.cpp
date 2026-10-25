/* =============================================================================
   MÓDULO 25 - EJERCICIO 04: pool de voces con prioridad y voice stealing
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 04 (ma_engine, voces, pool, política de robo).

   CONSIGNA
   --------
   Completá `PoolVoces` (slots fijos, "sin allocation" en play):
   `play(id, prioridad, reloj)` devuelve el índice de slot usado o -1:
   1. Si hay slot libre (id == -1): ocupa el PRIMER libre. (La prioridad y
      el reloj quedan en el slot.)
   2. Si no hay libres: roba la voz con prioridad ESTRICTAMENTE MENOR
      (nunca una igual ni superior). Entre varias robables, gana la de
      MENOR prioridad; a igual prioridad, la MÁS VIEJA (menor reloj).
   3. Si no hay ninguna robable: devuelve -1 (rechazo explícito, sin
      mutación de estado).
   Otras interfaces (completar igual): `stop(slot)` libera (id - ignore, no
   toca inválidos), `activas()`, `idDe(slot)` (-1 si el slot no existe).
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <vector>

class PoolVoces {
public:
    explicit PoolVoces(int slots) : slots_(slots, Voz{}) {}

    int play(int id, int prioridad, long reloj) {
        // TODO
        (void)id; (void)prioridad; (void)reloj;
        return -1;
    }

    void stop(int slot) {
        // TODO
        (void)slot;
    }

    int activas() const {
        // TODO
        return 0;
    }

    int idDe(int slot) const {
        // TODO
        (void)slot;
        return -1;
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
    /* stop libera el slot y el próximo play lo reusa. */
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
    /* lleno. Play con prioridad 5: roba la de menor prioridad = 102. */
    const int slot = p.play(105, 5, 4);
    ASSERT_TRUE(slot == 1);
    ASSERT_TRUE(p.idDe(1) == 105);
}

TEST_CASE(EmpateRobaLaMasVieja) {
    PoolVoces p(2);
    ASSERT_TRUE(p.play(201, 3, 10) == 0);
    ASSERT_TRUE(p.play(202, 3, 20) == 1);
    /* dos voces con prioridad 3: la robable más vieja es 201 (reloj 10). */
    const int slot = p.play(203, 5, 30);
    ASSERT_TRUE(slot == 0);
    ASSERT_TRUE(p.idDe(0) == 203);
    ASSERT_TRUE(p.idDe(1) == 202);
}

TEST_CASE(RechazoSinRobo) {
    PoolVoces p(2);
    ASSERT_TRUE(p.play(301, 7, 0) == 0);
    ASSERT_TRUE(p.play(302, 7, 1) == 1);
    /* lleno de prioridad 7 y el play pide 7: sin robables => -1 */
    ASSERT_TRUE(p.play(303, 7, 2) == -1);
    /* y con prioridad 2 TAMBIÉN rechaza (no se roba nadie). */
    ASSERT_TRUE(p.play(303, 2, 3) == -1);
    /* el estado quedó intacto */
    ASSERT_TRUE(p.idDe(0) == 301 && p.idDe(1) == 302);
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
