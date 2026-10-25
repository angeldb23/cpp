/* =============================================================================
   MÓDULO 09 - SOLUCIÓN 12: Sobrecargas por categoría de valor
   =============================================================================
   POR QUÉ FUNCIONA:
     - Con dos lvalues la versión const& es la elegida: los textos van por
       referencia constante y la Frase nueva concatena las COPIAS que
       hacen falta (los dueños quedan intactos).
     - Con dos temporarios gana la versión &&: puede absorber los dos
       textos y armar la frase sin copiar nada: los originales se
       iban a morir igual.
     - En Almacen, `dato() const&` copia (el objeto sigue vivo, alguien
       puede querer leerlo de nuevo) y `dato() &&` mueve el interno
       (nadie va a leerlo después: fue temporario).
     - leerTemporal llama dato() && sobre el temporario en la misma línea,
       concatenando `cola` al string devuelto.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_12_sobrecargas.cpp -o sol12 && ./sol12
   ============================================================================= */
#include <iostream>
#include <string>
#include <utility>
#include "../../include/test_framework.hpp"

class Frase {
public:
    std::string texto;
    explicit Frase(std::string t) : texto(std::move(t)) {}
};

// LVALUES: prestados, nadie se mueve.
Frase fusionar(const Frase& izq, const Frase& der) {
    Frase resultado(izq.texto);              // copiando la izquierda
    resultado.texto += " " + der.texto;      // concatenando la copia de la derecha
    return resultado;
}

// RVALUES: derecho a absorber.
Frase fusionar(Frase&& izq, Frase&& der) {
    std::string unido = std::move(izq.texto);
    unido += " " + std::move(der.texto);      // der.texto va por move, sin copiar
    return Frase(std::move(unido));
}

struct Almacen {
    std::string interno = "renglon";

    std::string dato() const& { return interno; }              // vivo: copia
    std::string dato() && { return std::move(interno); }       // temporario: move
};

std::string leerTemporal(const std::string& cola) {
    return Almacen{}.dato() + cola;
}

// ============= TESTS (los mismos del ejercicio) =============
TEST_CASE(TestFusionarLvaluesNoArruinaLosDuenios) {
    Frase a("mar"), b("luna");
    Frase r = fusionar(a, b);
    ASSERT_EQ(r.texto, std::string("mar luna"));
    ASSERT_EQ(a.texto, std::string("mar"));
    ASSERT_EQ(b.texto, std::string("luna"));
}

TEST_CASE(TestFusionarRvaluesAbsorbeLosDos) {
    Frase r = fusionar(Frase("sol"), Frase("cielo"));
    ASSERT_EQ(r.texto, std::string("sol cielo"));
}

TEST_CASE(TestAlmacenPorRefQualifiers) {
    Almacen vivo;
    std::string copiada = vivo.dato();
    ASSERT_EQ(copiada, std::string("renglon"));
    ASSERT_EQ(vivo.interno, std::string("renglon"));

    std::string robada = Almacen{}.dato();
    ASSERT_EQ(robada, std::string("renglon"));
}

TEST_CASE(TestLeerTemporalConLaVersionRvalue) {
    ASSERT_EQ(leerTemporal(" final"), std::string("renglon final"));
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 12 (sobrecargas por categoría)...\n";
    return RUN_ALL_TESTS();
}
