/* =============================================================================
   MÓDULO 11 - SOLUCIÓN 01: inventario con map (ejercicio 01)
   =============================================================================
   L01 es la lección de respaldo: operator[] como contador, at/contains
   para consultar sin crear, erase para limpiar, y recorrido completo
   para agregados en un contenedor sin accès mixto y sin "huecos".
   ============================================================================= */
#include <iostream>
#include <map>
#include <string>
#include "../../include/test_framework.hpp"

using Inventario = std::map<std::string, int>;

// 1) operator[] ACCESA Y CREA: lo unico que sirve para contador:
void contarElemento(Inventario& inv, const std::string& elemento) {
    ++inv[elemento];    // si no estaba: 0 -> 1
}

// 2) consulta CON find (no operator[]: no crear entrada turbo); y borrado
//    limpio al llegar a cero (el inventario no deja "huecos fantasma"):
bool gastarElemento(Inventario& inv, const std::string& elemento) {
    auto it = inv.find(elemento);
    if (it == inv.end() || it->second <= 0) return false;
    --it->second;
    if (it->second == 0) inv.erase(it);    // erase por iterador: O(log n)
    return true;
}

// 3) insert/refers: si ya estaba (bool == false del insert), fijar cantidad:
bool abastecer(Inventario& inv, const std::string& elemento, int cantidad) {
    auto [pos, nueva] = inv.try_emplace(elemento, cantidad);
    if (!nueva) inv.at(elemento) = cantidad;    // reescribe si ya estaba
    return nueva;
}

int totalUnidades(const Inventario& inv) {
    int total = 0;
    for (const auto& [elemento, cantidad] : inv) {
        (void)elemento;
        if (cantidad > 0) total += cantidad;
    }
    return total;
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
    ASSERT_EQ(totalUnidades(inv), 9);                 // 3 + 2 + 4 (el 0 no)
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 01 (inventario con map)...\n";
    return RUN_ALL_TESTS();
}
