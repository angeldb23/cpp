/* =============================================================================
   MÓDULO 05 - EJERCICIO 07: Asignación de copia (operator=)
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: leccion 11 (operator= de copia: auto-asignación, liberar
   lo propio, copiar, devolver *this) sobre la base de la lección 10.

   CONSIGNA: clase `Cofre` que guarda un recurso crudo `int*` de
   `m_capacidad` elementos:
     1. Constructor con capacidad (new[] de capacidad, todo en 0).
     2. Destructor con delete[].
     3. `operator=(const Cofre& otra)` que:
        - detecte la AUTO-asignación (this == &otra) y no rompa nada;
        - pida un recurso nuevo ANTES de liberar el propio;
        - copie la capacidad y el contenido de `otra`;
        - libere su propio recurso viejo DESPUÉS (no antes: así, si el
          new falla, el objeto queda entero);
        - devuelva `*this` (encadenable).
     4. Método `void poner(int indice, int valor)` y `int valor(int
        indice) const` (con chequeo de índice) y `int capacidad() const`.

   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

class Cofre {
public:
    explicit Cofre(int capacidad)
        : m_capacidad(capacidad), m_datos(nullptr) {
        // TODO 1: new int[capacidad]{}
        (void)capacidad;
        return;  // placeholder
    }

    // Destructor TODO 2: delete[] m_datos;
    ~Cofre() {
        // TODO
    }

    // TODO 3: la asignación de copia completa.
    Cofre& operator=(const Cofre& otra) {
        std::cout << "  operator= invocado\n";
        (void)otra;
        return *this;  // placeholder
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
    ASSERT_EQ(c.valor(0), 0);      // new[]{} empieza en 0
    c.poner(0, 7);
    ASSERT_EQ(c.valor(0), 7);
}

TEST_CASE(TestAsignacionCopiaContenido) {
    Cofre un(2), dos(3);
    un.poner(0, 100);
    un.poner(1, 200);
    ASSERT_EQ(dos.capacidad(), 3);   // la capacidad se copia
    dos = un;
    ASSERT_EQ(dos.capacidad(), 2);
    ASSERT_EQ(dos.valor(0), 100);
    ASSERT_EQ(dos.valor(1), 200);
}

TEST_CASE(TestAutoAsignacionSana) {
    Cofre c(2);
    c.poner(0, 55);
    Cofre* pc = &c;
    c = *pc;                      // auto-asignación disfrazada
    ASSERT_EQ(c.capacidad(), 2);
    ASSERT_EQ(c.valor(0), 55);     // contenido intacto (no se destruyó)
}

TEST_CASE(TestEncadenado) {
    Cofre a(1), b(2), fuente(2);
    fuente.poner(0, 42);
    b = a = fuente;       // primero a = fuente, después b = a
    ASSERT_EQ(a.valor(0), 42);
    ASSERT_EQ(b.valor(0), 42);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 07...\n";
    return RUN_ALL_TESTS();
}
