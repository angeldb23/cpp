/* =============================================================================
   MÓDULO 05 - SOLUCIÓN 06: Copia profunda con constructor de copia
   =============================================================================
   POR QUÉ FUNCIONA:
     - Cada construcción (o copia) registra su reserva; cada destrucción
       registra su liberación: `recursos() - liberaciones` son las
       fugas y el test lo fuerza a 0.
     - La copia pide SU PROPIO bloque y copia el contenido: los datos
       copiados son independientes (lección 10). La fuga sumaria de
       copia superficial sería: dos objetos, un bloque, doble free.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_06... -o sol06 && ./sol06
   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

class InventarioBug {
public:
    explicit InventarioBug(int capacidad)
        : m_tamano(capacidad), m_ids(nullptr) {
        m_ids = new int[m_tamano] {};   // {} → todo en 0
        ++s_reservas;
    }

    // Constructor de copia PROFUNDA: recurso nuevo + contenido + tamaño.
    InventarioBug(const InventarioBug& otra)
        : m_tamano(otra.m_tamano), m_ids(nullptr) {
        m_ids = new int[m_tamano] {};
        ++s_reservas;
        for (int i = 0; i < m_tamano; ++i) {
            m_ids[i] = otra.m_ids[i];
        }
    }

    ~InventarioBug() {
        delete[] m_ids;      // SOLO el bloque propio
        m_ids = nullptr;
        ++s_liberaciones;
    }

    void marcar(int indice, int id) {
        if (indice >= 0 && indice < m_tamano && m_ids != nullptr) {
            m_ids[indice] = id;
        }
    }

    int en(int indice) const {
        if (indice < 0 || indice >= m_tamano || m_ids == nullptr) return -1;
        return m_ids[indice];
    }

    int tamano() const { return m_tamano; }

    static int recursos() { return s_reservas; }
    static int fugas() { return s_reservas - s_liberaciones; }

private:
    int m_tamano;
    int* m_ids;
    static int s_reservas;
    static int s_liberaciones;
};

int InventarioBug::s_reservas = 0;
int InventarioBug::s_liberaciones = 0;

// ============= TESTS =============
TEST_CASE(TestConstructorYContenido) {
    InventarioBug inv(3);
    inv.marcar(0, 101);
    ASSERT_EQ(inv.en(0), 101);
    ASSERT_EQ(inv.en(2), 0);
    ASSERT_EQ(inv.tamano(), 3);
}

TEST_CASE(TestCopiaProfundaEsIndependiente) {
    InventarioBug original(2);
    original.marcar(0, 500);
    original.marcar(1, 600);

    InventarioBug copia = original;
    copia.marcar(0, 999);

    ASSERT_EQ(original.en(0), 500);
    ASSERT_EQ(copia.en(0), 999);
    ASSERT_EQ(copia.en(1), 600);

    original.marcar(1, -7);
    ASSERT_EQ(copia.en(1), 600);
}

TEST_CASE(TestCadaUnoLiberaSuPropioBloque) {
    ASSERT_EQ(InventarioBug::fugas(), 0);
    const int antes = InventarioBug::recursos();
    {
        InventarioBug a(2);
        InventarioBug b = a;
        InventarioBug c = a;
        ASSERT_EQ(InventarioBug::recursos(), antes + 3);
    }
    ASSERT_EQ(InventarioBug::recursos(), antes + 3);
    ASSERT_EQ(InventarioBug::fugas(), 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 06...\n";
    return RUN_ALL_TESTS();
}
