/* =============================================================================
   MÓDULO 05 - EJERCICIO 06: Copia profunda with constructor de copia
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lección 10 (constructor de copia, copia superficial vs
   profunda, miembros con recursos crudos).

   CONSIGNA: clase `InventarioBug` (inventario de un bug tracker):
     - recurso crudo: `int* m_ids` con `m_tamano` elementos (new[] /
       delete[]).
     - Constructor (capacidad): reserva un recurso de esa capacidad
       (todos en 0).
     - Constructor de copia: copia PROFUNDA: nuevo recurso propio +
       copiar el contenido + copiar el tamaño. Los datos copiados no
       comparten memoria con el original.
     - Método `void marcar(int indice, int id)`.
     - Getter `int en(int indice) const`, `int tamano() const`.
     - Getters estáticos para el test: `recursos()`: cuántas
       reservas de memoria se hicieron en total (construcciones +
       copias). `fugas()`: reservas - liberaciones.

   PISTA: el ejercicio es el test de la independencia: cambiar la
   copia NO debe cambiar el original y viceversa, y cada objeto libe-
   ra EXACTAMENTE su propio bloque en el destructor.

   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

class InventarioBug {
public:
    explicit InventarioBug(int capacidad)
        : m_tamano(capacidad), m_ids(nullptr) {
        // TODO: new int[capacidad] {} y asigná m_ids.
        (void)capacidad;
        return;  // placeholder
    }

    // TODO: constructor de copia PROFUNDA.
    // PISTA: new de recurso nuevo, copiar contenido y tamaño, y
    // registrar otra reserva en el contador static.
    InventarioBug(const InventarioBug& otra)
        : m_tamano(otra.m_tamano), m_ids(nullptr) {
        (void)otra;
        return;  // placeholder: NO copia (copia superficial: los tests fallan)
    }

    ~InventarioBug() {
        // TODO: delete[] m_ids; y registrar la liberación.
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

    // TODO 3-4: statics del ejercicio.
    static int recursos() { return 0; }   // placeholder
    static int fugas() { return 0; }      // placeholder

private:
    int m_tamano;
    int* m_ids;
    // TODO: static int s_reservas; static int s_liberaciones;
};

// Definiciones (la clase solo declara):
// int InventarioBug::s_reservas = 0;
// int InventarioBug::s_liberaciones = 0;

// ============= TESTS =============
TEST_CASE(TestConstructorYContenido) {
    InventarioBug inv(3);
    inv.marcar(0, 101);
    ASSERT_EQ(inv.en(0), 101);
    ASSERT_EQ(inv.en(2), 0);       // new[]{} inicializa en 0
    ASSERT_EQ(inv.tamano(), 3);
}

TEST_CASE(TestCopiaProfundaEsIndependiente) {
    InventarioBug original(2);
    original.marcar(0, 500);
    original.marcar(1, 600);

    InventarioBug copia = original;    // constructor de copia
    copia.marcar(0, 999);              // tocar la copia...

    ASSERT_EQ(original.en(0), 500);    // ...NO cambia el original
    ASSERT_EQ(copia.en(0), 999);
    ASSERT_EQ(copia.en(1), 600);

    original.marcar(1, -7);            // tocar el original...
    ASSERT_EQ(copia.en(1), 600);       // ...NO cambia la copia
}

TEST_CASE(TestCadaUnoLiberaSuPropioBloque) {
    ASSERT_EQ(InventarioBug::fugas(), 0);
    const int antes = InventarioBug::recursos();
    {
        InventarioBug a(2);
        InventarioBug b = a;
        InventarioBug c = a;
        ASSERT_EQ(InventarioBug::recursos(), antes + 3);  // a + b + c
    }
    // Los 3 llegaron a su destructor: reservas == liberaciones.
    ASSERT_EQ(InventarioBug::recursos(), antes + 3);
    ASSERT_EQ(InventarioBug::fugas(), 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 06...\n";
    return RUN_ALL_TESTS();
}
