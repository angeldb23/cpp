/* =============================================================================
   MÓDULO 05 - EJERCICIO 08: La regla de 3 completa
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lección 12 (regla de 3: destructor + copia + asignación en
   un mismo tipo con recurso crudo) — resumen de 10 y 11.

   CONSIGNA: clase `Botiquin` con recurso crudo `double* m_pastillas`
   (m_capacidad dosis):
     1. Destructor que libere.
     2. Constructor de copia PROFUNDA.
     3. operator= de copia SIN auto-asignación problemática que pida
        ANTES y libere DESPUÉS, y devuelva *this.
     4. Métodos: `void poner(int i, double d)`, `double leer(int i)
        const`, `int capacidad() const` y `bool estaVacio() const`
        (true si todas las dosis son 0.0).

   El esqueleto tiene la regla de 0 (nada definido): todos los tests
   del rastreo de vida fallan hasta que armes la regla de 3 completa.
   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

class Botiquin {
public:
    explicit Botiquin(int capacidad)
        : m_capacidad(capacidad), m_pastillas(nullptr) {
        // TODO 1: new double[capacidad]{}
        (void)capacidad;
        return;  // placeholder
    }

    // Regla de 3, pieza 1: el destructor.
    ~Botiquin() {
        // TODO
    }

    // Regla de 3, pieza 2: constructor de copia profunda.
    Botiquin(const Botiquin& otra)
        : m_capacidad(otra.m_capacidad), m_pastillas(nullptr) {
        (void)otra;
        return;  // placeholder (copia superficial implícita: doble free)
    }

    // Regla de 3, pieza 3: asignación de copia.
    Botiquin& operator=(const Botiquin& otra) {
        (void)otra;
        return *this;  // placeholder
    }

    void poner(int indice, double dosis) {
        if (indice >= 0 && indice < m_capacidad && m_pastillas != nullptr) {
            m_pastillas[indice] = dosis;
        }
    }

    double leer(int indice) const {
        if (indice < 0 || indice >= m_capacidad || m_pastillas == nullptr) {
            return -1.0;
        }
        return m_pastillas[indice];
    }

    int capacidad() const { return m_capacidad; }

    bool estaVacio() const {
        for (int i = 0; i < m_capacidad; ++i) {
            if (leer(i) != 0.0) {
                return false;
            }
        }
        return true;
    }

private:
    int m_capacidad;
    double* m_pastillas;
};

// ============= TESTS =============
TEST_CASE(TestUsoBasico) {
    Botiquin b(3);
    ASSERT_TRUE(b.estaVacio());
    b.poner(1, 0.5);
    ASSERT_FALSE(b.estaVacio());
    ASSERT_NEAR(b.leer(1), 0.5, 1e-9);
    ASSERT_EQ(b.capacidad(), 3);
}

TEST_CASE(TestCopiaProfunda) {
    Botiquin base(2);
    base.poner(0, 1.25);
    Botiquin copia = base;
    copia.poner(0, 9.0);
    ASSERT_NEAR(base.leer(0), 1.25, 1e-9);   // independencia
    ASSERT_NEAR(copia.leer(0), 9.0, 1e-9);
    ASSERT_NEAR(copia.leer(1), 0.0, 1e-9);   // el resto, zero
}

TEST_CASE(TestAsignacionProfundaYAuto) {
    Botiquin destino(2), fuente(2);
    fuente.poner(0, 2.5);
    destino = fuente;
    ASSERT_NEAR(destino.leer(0), 2.5, 1e-9);

    Botiquin* pd = &destino;
    destino = *pd;                            // auto-asignación
    ASSERT_NEAR(destino.leer(0), 2.5, 1e-9);  // sobrevive
}

TEST_CASE(TestReglaDe3CompletaNoFuente) {
    // Simulación de lifetime anidado: muere la copia interna, sobrevive
    // la externa (y las dos pidieron recursos separados).
    Botiquin externo(2);
    externo.poner(0, 3.5);
    {
        Botiquin copiaInterna = externo;
        copiaInterna.poner(1, 8.0);
    }   // la copia muere
    ASSERT_NEAR(externo.leer(0), 3.5, 1e-9);
    ASSERT_NEAR(externo.leer(1), 0.0, 1e-9);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 08...\n";
    return RUN_ALL_TESTS();
}
