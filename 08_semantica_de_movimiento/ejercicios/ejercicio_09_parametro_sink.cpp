/* =============================================================================
   MÓDULO 08 - EJERCICIO 09: Parámetro sink: por valor y mover adentro
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lección 11 (diseño de APIs: si la función VA A GUARDAR, la
   firma es por valor + std::move adentro; si solo mira, const&).

   CONSIGNA:
   La carpeta guarda nombres completos en un vector. Implementá:

      1. `archivar(std::string nombre)`: recepción por VALOR
         (functador de creación "sink"): lo mete al vector con
         std::move (roba el argumento). Devuelve el nuevo size del
         vector.
      2. `chismear(const std::string& nombre)`: la versión que SOLO
         MIRA y guarda una copia (el parámetro es const&; aquí hay
         una copia inevitable, y el origen NO se toca). Devuelve el
         nuevo size.

   Los tests de esta doble lectura están escritos acá abajo con la
   cesión en los call sites: `archivar(std::move(pesado))` (el caller
   cede) frente a `chismear(original)` (const&: no toca).

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_09_parametro_sink.cpp -o ej09 && ./ej09
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

class Carpeta {
public:
    // TODO 1: sink: por valor, roba el argumento con std::move,
    //         y devuelve (int)nombres_.size().
    int archivar(std::string nombre) {
        (void)nombre;
        return -1;                                 // PLACEHOLDER
    }

    // TODO 2: copia simple (no hay robo: el dato sigue vivo a fuera).
    int chismear(const std::string& nombre) {
        (void)nombre;
        return -1;                                 // PLACEHOLDER
    }

    const std::string& etiqueta(std::size_t i) const { return nombres_[i]; }
    std::size_t cantidad() const { return nombres_.size(); }

private:
    std::vector<std::string> nombres_;
};

// ============= TESTS =============
TEST_CASE(ArchivarRobaElArgumento) {
    Carpeta carpeta;
    std::string pesado = "/niveles/08/nombre_muy_largo_del_boss_final.png";
    const int n = carpeta.archivar(std::move(pesado));   // CESION del caller
    ASSERT_EQ(n, 1);
    ASSERT_EQ(carpeta.etiqueta(0), "/niveles/08/nombre_muy_largo_del_boss_final.png");
    ASSERT_EQ(pesado.size(), std::size_t(0));      // garantia estandar: vacio
}

TEST_CASE(ArchivarAceptaTemporales) {
    Carpeta carpeta;
    ASSERT_EQ(carpeta.archivar(std::string("temporal directo")), 1);
    ASSERT_EQ(carpeta.archivar("literal"), 2);     // const char* convierte: se mueve
}

TEST_CASE(ChismearCopiaSinTocarElOrigen) {
    Carpeta carpeta;
    std::string original = "solo la miro";
    const int n = carpeta.chismear(original);
    ASSERT_EQ(n, 1);
    ASSERT_EQ(original, "solo la miro");           // const&: origen ENTERO
    ASSERT_EQ(carpeta.etiqueta(0), "solo la miro");
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 09...\n";
    return RUN_ALL_TESTS();
}
