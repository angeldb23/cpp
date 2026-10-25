/* =============================================================================
   MÓDULO 05 - SOLUCIÓN 08: La regla de 3 completa
   =============================================================================
   POR QUÉ FUNCIONA:
     - Las TRES piezas están alineadas con el mismo recurso: destructor
       (lo libera), copia (nuevo recurso + contenido), asignación (pide
       antes, libera después, sin auto-asignación).
     - Con la regla de 3 completa, "copia interna que muere antes que
       la externa" no tira abajo a nadie: cada Botiquín tiene su bloque.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_08... -o sol08 && ./sol08
   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

class Botiquin {
public:
    explicit Botiquin(int capacidad)
        : m_capacidad(capacidad), m_pastillas(nullptr) {
        m_pastillas = new double[m_capacidad] {};
    }

    // Regla de 3, pieza 1: destructor.
    ~Botiquin() {
        delete[] m_pastillas;
        m_pastillas = nullptr;
    }

    // Regla de 3, pieza 2: constructor de copia profunda.
    Botiquin(const Botiquin& otra)
        : m_capacidad(otra.m_capacidad), m_pastillas(nullptr) {
        m_pastillas = new double[m_capacidad] {};
        for (int i = 0; i < m_capacidad; ++i) {
            m_pastillas[i] = otra.m_pastillas[i];
        }
    }

    // Regla de 3, pieza 3: asignación de copia.
    Botiquin& operator=(const Botiquin& otra) {
        if (this == &otra) {
            return *this;
        }
        double* nuevos = new double[otra.m_capacidad] {};
        for (int i = 0; i < otra.m_capacidad; ++i) {
            nuevos[i] = otra.m_pastillas[i];
        }
        delete[] m_pastillas;
        m_pastillas = nuevos;
        m_capacidad = otra.m_capacidad;
        return *this;
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
    ASSERT_NEAR(base.leer(0), 1.25, 1e-9);
    ASSERT_NEAR(copia.leer(0), 9.0, 1e-9);
    ASSERT_NEAR(copia.leer(1), 0.0, 1e-9);
}

TEST_CASE(TestAsignacionProfundaYAuto) {
    Botiquin destino(2), fuente(2);
    fuente.poner(0, 2.5);
    destino = fuente;
    ASSERT_NEAR(destino.leer(0), 2.5, 1e-9);

    Botiquin* pd = &destino;
    destino = *pd;
    ASSERT_NEAR(destino.leer(0), 2.5, 1e-9);
}

TEST_CASE(TestReglaDe3CompletaNoFuga) {
    Botiquin externo(2);
    externo.poner(0, 3.5);
    {
        Botiquin copiaInterna = externo;
        copiaInterna.poner(1, 8.0);
    }
    ASSERT_NEAR(externo.leer(0), 3.5, 1e-9);
    ASSERT_NEAR(externo.leer(1), 0.0, 1e-9);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 08...\n";
    return RUN_ALL_TESTS();
}
