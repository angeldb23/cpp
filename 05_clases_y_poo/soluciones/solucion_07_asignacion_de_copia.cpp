/* =============================================================================
   MÓDULO 05 - SOLUCIÓN 07: Asignación de copia (operator=)
   =============================================================================
   POR QUÉ FUNCIONA:
     - La auto-asignación sale por el if: nada se libera ni se copia.
     - El recurso nuevo se pide ANTES del delete: si la copia fallara,
       el objeto receptor queda entero (base de la garantía fuerte).
     - `return *this;` habilita el encadenado a = b = fuente.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_07... -o sol07 && ./sol07
   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

class Cofre {
public:
    explicit Cofre(int capacidad)
        : m_capacidad(capacidad), m_datos(nullptr) {
        m_datos = new int[m_capacidad] {};
    }

    ~Cofre() {
        delete[] m_datos;
        m_datos = nullptr;
    }

    Cofre& operator=(const Cofre& otra) {
        if (this == &otra) {
            return *this;                     // 1) auto-asignación
        }
        int* nuevos = new int[otra.m_capacidad] {};   // 2) pedir ANTES
        for (int i = 0; i < otra.m_capacidad; ++i) {
            nuevos[i] = otra.m_datos[i];      // 3) copiar contenido
        }
        delete[] m_datos;                     // 4) liberar DESPUÉS
        m_datos = nuevos;
        m_capacidad = otra.m_capacidad;       // 5) copiar la capacidad
        return *this;                          // 6) encadenable
    }

    void poner(int indice, int valor) {
        if (indice >= 0 && indice < m_capacidad && m_datos != nullptr) {
            m_datos[indice] = valor;
        }
    }

    int valor(int indice) const {
        if (indice < 0 || indice >= m_capacidad || m_datos == nullptr) {
            return -1;
        }
        return m_datos[indice];
    }

    int capacidad() const { return m_capacidad; }

private:
    int m_capacidad;
    int* m_datos;
};

// ============= TESTS =============
TEST_CASE(TestConstructorYAcceso) {
    Cofre c(4);
    ASSERT_EQ(c.capacidad(), 4);
    ASSERT_EQ(c.valor(0), 0);
    c.poner(0, 7);
    ASSERT_EQ(c.valor(0), 7);
}

TEST_CASE(TestAsignacionCopiaContenido) {
    Cofre un(2), dos(3);
    un.poner(0, 100);
    un.poner(1, 200);
    ASSERT_EQ(dos.capacidad(), 3);
    dos = un;
    ASSERT_EQ(dos.capacidad(), 2);
    ASSERT_EQ(dos.valor(0), 100);
    ASSERT_EQ(dos.valor(1), 200);
}

TEST_CASE(TestAutoAsignacionSana) {
    Cofre c(2);
    c.poner(0, 55);
    Cofre* pc = &c;
    c = *pc;
    ASSERT_EQ(c.capacidad(), 2);
    ASSERT_EQ(c.valor(0), 55);
}

TEST_CASE(TestEncadenado) {
    Cofre a(1), b(2), fuente(2);
    fuente.poner(0, 42);
    b = a = fuente;
    ASSERT_EQ(a.valor(0), 42);
    ASSERT_EQ(b.valor(0), 42);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 07...\n";
    return RUN_ALL_TESTS();
}
