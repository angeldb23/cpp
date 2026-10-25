/* =============================================================================
   MÓDULO 09 - EJERCICIO 12: Sobrecargas por categoría de valor (lección 09)
   =============================================================================
   DIFICULTAD: Avanzado
   CONSIGNA (tema 8 de TEMAS.md: sobrecargas por categoría de valor):
     La lección 09 mostró la alternativa al forwarding reference: SOBRECARGAS
     explícitas por categoría de valor, y los ref-qualifiers de las
     funciones miembro. Implementá:

     1. fusionar(l, r) le agrega la derecha a la IZQUIERDA en una Frase nueva:
        - {const Frase&, const Frase&}: los dos textos son prestados: la
          nueva Frase se arma SIN mover a los dueños.
        - {Frase&&, Frase&&}: los dos llegaron como temporarios: absorbé
          los dos textos por move.
     2. struct Almacen: guarda un std::string interno y expone la lectura
        por ref-qualifiers:
        - dato() const& -> devuelve UNA COPIA del interno (el objeto sigue vivo).
        - dato() &&     -> devuelve el interno MOVIDO (el objeto ya se va).
     3. leerTemporal(cola): en UNA línea, toma la lectura && de un Almacen
        temporario y concaténale `cola` (tiene que resultar "renglon final").
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_12_sobrecargas.cpp -o ej12 && ./ej12
   ============================================================================= */
#include <iostream>
#include <string>
#include <utility>
#include "../../include/test_framework.hpp"

// Un dato simple, sin contadores: acá lo que importa es la CATEGORÍA.
class Frase {
public:
    std::string texto;
    explicit Frase(std::string t) : texto(std::move(t)) {}
};

// TODO 1: la sobrecarga para LVALUES: los textos NO se mueven.
// TU CÓDIGO AQUÍ:
Frase fusionar([[maybe_unused]] const Frase& izq, [[maybe_unused]] const Frase& der) {
    return Frase("");   // placeholder
}

// TODO 2: la sobrecarga para RVALUES: los dos textos se pueden absorber.
// TU CÓDIGO AQUÍ:
Frase fusionar([[maybe_unused]] Frase&& izq, [[maybe_unused]] Frase&& der) {
    return Frase("");   // placeholder
}

// TODO 3: las DOS lecturas de Almacen, una por ref-qualifier.
struct Almacen {
    std::string interno = "renglon";

    // TU CÓDIGO AQUÍ:
    std::string dato() const {
        return interno;      // placeholder: no distingue todavía && de const&
    }
};

// TODO 4: en UNA línea: lectura && de un Almacen temporario + `cola`.
// TU CÓDIGO AQUÍ:
std::string leerTemporal([[maybe_unused]] const std::string& cola) {
    return "";   // placeholder
}

// ============= TESTS =============
TEST_CASE(TestFusionarLvaluesNoArruinaLosDuenios) {
    Frase a("mar"), b("luna");
    Frase r = fusionar(a, b);                  // dos lvalues: sobrecarga const&
    ASSERT_EQ(r.texto, std::string("mar luna"));
    ASSERT_EQ(a.texto, std::string("mar"));    // el dueño quedó entero
    ASSERT_EQ(b.texto, std::string("luna"));   // el dueño quedó entero
}

TEST_CASE(TestFusionarRvaluesAbsorbeLosDos) {
    Frase r = fusionar(Frase("sol"), Frase("cielo"));   // dos temporarios -> &&
    ASSERT_EQ(r.texto, std::string("sol cielo"));
}

TEST_CASE(TestAlmacenPorRefQualifiers) {
    Almacen vivo;
    std::string copiada = vivo.dato();         // lvalue: copia
    ASSERT_EQ(copiada, std::string("renglon"));
    ASSERT_EQ(vivo.interno, std::string("renglon"));   // el interno sigue ahí

    std::string robada = Almacen{}.dato();     // rvalue: el interno se va movido
    ASSERT_EQ(robada, std::string("renglon"));
    // el temporario ya no existe: no hay interno que verificar
}

TEST_CASE(TestLeerTemporalConLaVersionRvalue) {
    ASSERT_EQ(leerTemporal(" final"), std::string("renglon final"));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 12 (sobrecargas por categoría)...\n";
    return RUN_ALL_TESTS();
}
