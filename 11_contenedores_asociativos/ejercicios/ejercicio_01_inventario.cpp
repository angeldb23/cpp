/* =============================================================================
   MÓDULO 11 - EJERCICIO 01: inventario con map (lección 01)
   =============================================================================
   DIFICULTAD: Basico
   CONSIGNA (tema 1 de TEMAS.md: map: operator[], at, inserciones):
     Un RPG guarda su inventario del aula en un std::map<std::string,int>
     element -> cantidad.
       1. contarElemento(map, elemento): suma 1 a la cantidad con
          operator[] (L01). Si no estaba, lo crea con 1.
       2. gastarElemento(map, elemento): resta 1 si hay stock y devuelve
          true; si no hay (o no existe), deja el estado intacto y
          devuelve false. IMPORTANTE (L01): no queda el elemento con 0
          en el map TODO el año; si al quedar en 0, BORRA la entrada
          (erase) para que el inventario no tenga "huecos fantasma".
       3. abastecer(map, elemento, cantidad): fija la cantidad EXACTA si
          el elemento ya estaba (SIN toques, L01) o lo inserta con esa
          cantidad si no estaba. Devuelve true si CREÓ la entrada,
          false si solo actualizó.
       4. totalUnidades(map): el TOTAL de unidades sumadas (¡recorrer el
          map y sumar las cantidades por encima de 0!).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_01_inventario.cpp -o ej01 && ./ej01
   ============================================================================= */
#include <iostream>
#include <map>
#include <string>
#include "../../include/test_framework.hpp"

using Inventario = std::map<std::string, int>;

// TODO 1: suma 1 al elemento (crea la entrada si no existe).
void contarElemento([[maybe_unused]] Inventario& inv,
                    [[maybe_unused]] const std::string& elemento) {
    // TU CODIGO AQUI
}

// TODO 2: resta 1 si hay stock (true); no toca nada si no (false).
//         BONUS: si queda en 0, erase() de la entrada.
bool gastarElemento([[maybe_unused]] Inventario& inv,
                    [[maybe_unused]] const std::string& elemento) {
    return false;    // placeholder
}

// TODO 3: fija cantidad (si ya estaba) o crea (devuelve true si CREO).
bool abastecer([[maybe_unused]] Inventario& inv,
               [[maybe_unused]] const std::string& elemento,
               [[maybe_unused]] int cantidad) {
    return false;    // placeholder
}

// TODO 4: suma total de las cantidades (elementos con cantidad > 0).
int totalUnidades([[maybe_unused]] const Inventario& inv) {
    return 0;    // placeholder
}

// ============= TESTS =============
TEST_CASE(ContarCreaYSuma) {
    Inventario inv;
    contarElemento(inv, "pocion");
    contarElemento(inv, "espada");
    contarElemento(inv, "pocion");
    ASSERT_EQ(inv.at("pocion"), 2);
    ASSERT_EQ(inv.at("espada"), 1);
}

TEST_CASE(GastarConYSinStock) {
    Inventario inv{{"pocion", 1}};
    ASSERT_TRUE(gastarElemento(inv, "pocion"));       // 1 -> 0 -> se borra
    ASSERT_FALSE(inv.contains("pocion"));             // erase al llegar a 0
    ASSERT_FALSE(gastarElemento(inv, "pocion"));      // ya no hay
    ASSERT_TRUE(inv.empty());                         // el map queda limpio
}

TEST_CASE(GastarNoTocaEntradasInexistentes) {
    Inventario inv{{"espada", 1}};
    ASSERT_FALSE(gastarElemento(inv, "tinta"));
    ASSERT_EQ(inv.size(), 1u);
}

TEST_CASE(AbastecerInsertaYFija) {
    Inventario inv;
    ASSERT_TRUE(abastecer(inv, "palas", 3));          // creo fila nueva
    ASSERT_FALSE(abastecer(inv, "palas", 1));         // solo actualizo
    ASSERT_EQ(inv.at("palas"), 1);
}

TEST_CASE(TotalSumaPositivos) {
    Inventario inv{{"palas", 3}, {"picos", 2}, {"balde", 0}};
    abastecer(inv, "opcional", 4);
    ASSERT_EQ(totalUnidades(inv), 9);                  // 3 + 2 + 4 (el 0 no cuenta)
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 01 (inventario con map)...\n";
    return RUN_ALL_TESTS();
}
