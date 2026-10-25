/* =============================================================================
   MÓDULO 11 - EJERCICIO 06: registro con presupuesto de comparaciones
   (lecciones 07 y 08)
   =============================================================================
   DIFICULTAD: Medio
   CONSIGNA (tema 4 de TEMAS.md: elección de contenedor por complejidad /
   localidad):
     Un registro responde "¿está esta llave y qué valor tiene?" muchas
     veces; hay que elegir el CONTENEDOR correcto para que el costo no
     explote. El test no mide tiempo: cuenta COMPARACIONES (L07: cada
     O(log n) llama al comparador; L08: por qué una búsqueda lineal no
     es "gratis").

     Llave abajo incrementa un contador en CADA operator<. Tu Rotulo
     agrega / esta / valor (más abajo): "cast" que funcionar y, además,
     el test exige que 200 inserciones seguidas de 100 búsquedas
     produzcan MENOS de 4000 comparaciones AL BUSCAR (un vector con
     búsqueda lineal hace ~10.000 en esa fase: reprobado; un map o
     una tabla hash pasan de sobra).

       struct Rotulo {
         // ELEGE tu contenedor acá (map / unordered_map / set+contenido /
         // lo que quieras: y si elegis unordered_map,
         // definis hasher como en la L06).
         void agrega(const Llave& k, int v);
         bool esta(const Llave& k) const;
         int  valor(const Llave& k) const;   // 0 si no está
       };
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_06_registro.cpp -o ej06 && ./ej06
   ============================================================================= */
#include <cstddef>
#include <functional>
#include <iostream>
#include "../../include/test_framework.hpp"

// Contador de comparaciones (NO lo uses para elegir: solo herramienta):
std::size_t& comparaciones() {
    static std::size_t n = 0;
    return n;
}

struct Llave {
    int id;

    bool operator<(const Llave& otra) const {
        ++comparaciones();
        return id < otra.id;
    }
};

// TODO: completá el struct Rotulo con el contenedor que elijas (L07-L08).
//       El comparador de tu contenedor llamará al operator< de Llave:
//       ese es el "termómetro" del test.
class Rotulo {
  public:
    // TU CODIGO AQUI
};

// TODO 1: agrega la llave (o reescribe su valor si ya estaba):
void agrega([[maybe_unused]] Rotulo& r, [[maybe_unused]] const Llave& k,
            [[maybe_unused]] int v) {
    // TU CODIGO AQUI
}

// TODO 2: true si la llave está en el registro:
bool esta([[maybe_unused]] const Rotulo& r, [[maybe_unused]] const Llave& k) {
    return false;    // placeholder
}

// TODO 3: el valor de la llave, o 0 si no está (sin crear entrada):
int valor([[maybe_unused]] const Rotulo& r, [[maybe_unused]] const Llave& k) {
    return 0;    // placeholder
}

// ============= TESTS =============
TEST_CASE(RegistraConsultaDevuelve) {
    Rotulo r;
    agrega(r, Llave{10}, 1);
    agrega(r, Llave{20}, 2);
    ASSERT_TRUE(esta(r, Llave{10}));
    ASSERT_FALSE(esta(r, Llave{30}));
    ASSERT_EQ(valor(r, Llave{20}), 2);
    ASSERT_EQ(valor(r, Llave{30}), 0);
}

TEST_CASE(ActualizaElValorSinDuplicar) {
    Rotulo r;
    agrega(r, Llave{1}, 5);
    agrega(r, Llave{1}, 7);
    ASSERT_EQ(valor(r, Llave{1}), 7);
    ASSERT_TRUE(esta(r, Llave{1}));
}

TEST_CASE(PresupuestoDeComparaciones_LogaritmoOHash) {
    Rotulo r;
    for (int i = 0; i < 200; ++i) {
        agrega(r, Llave{i * 3}, i);    // las llaves son, dispersos
    }
    comparaciones() = 0;               // la fase medida: SOLO busquedas
    for (int i = 0; i < 100; ++i) {
        (void)esta(r, Llave{i * 6});       // mitad existen / mitad no
    }
    const std::size_t gastadas = comparaciones();
    std::cout << "  (comparaciones al buscar: " << gastadas << ")\n";
    ASSERT_TRUE(gastadas < 4000);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 06 (registro con presupuesto)...\n";
    return RUN_ALL_TESTS();
}
