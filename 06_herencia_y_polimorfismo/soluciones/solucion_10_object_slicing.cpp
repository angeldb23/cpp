/* =============================================================================
   MÓDULO 06 - SOLUCIÓN 10: Object slicing: la firma que rebana al boss
   =============================================================================
   Clave de la lección 13:
   - Por `const&` (o puntero): la copia NO existe, el polimorfismo corre.
   - En colecciones: contenedor de punteros, no de valor base.
   ============================================================================= */
#include <iostream>
#include <vector>
#include "../../include/test_framework.hpp"

class Enemigo {
public:
    virtual ~Enemigo() = default;
    virtual int danoBase() const { return 1; }
};

class Zombi : public Enemigo {
public:
    int danoBase() const override { return 4; }
};

class Boss : public Enemigo {
public:
    int danoBase() const override { return 90; }
};

int atacar(const Enemigo& e) {        // FIX: referencia, no valor
    return e.danoBase();
}

int danioTotal(const std::vector<Enemigo*>& lista) {   // FIX: punteros
    int total = 0;
    for (const Enemigo* e : lista) total += e->danoBase();
    return total;
}

TEST_CASE(TestAtacarConservaPolimorfismo) {
    Boss boss;
    Zombi  z;
    ASSERT_EQ(atacar(boss), 90);
    ASSERT_EQ(atacar(z), 4);
}

TEST_CASE(TestDanioTotalConPunteros) {
    Boss boss;
    Zombi  z1;
    Zombi  z2;
    std::vector<Enemigo*> ronda = {&boss, &z1, &z2};
    ASSERT_EQ(danioTotal(ronda), 98);
}

int main() {
    std::cout << "Solucion Ejercicio 10: firmas que no rebanan.\n";
    return RUN_ALL_TESTS();
}
