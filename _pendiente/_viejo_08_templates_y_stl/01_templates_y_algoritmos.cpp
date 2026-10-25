/* =============================================================================
   MÓDULO 8 - LECCIÓN 01: Templates (Plantillas), Algoritmos y Lambdas
   =============================================================================
   1. TEMPLATES (Programación Genérica):
      Permite escribir código que funciona con CUALQUIER tipo de dato sin repetir
      la lógica para int, float, string, etc.

   2. FUNCIONES ANÓNIMAS (LAMBDAS):
      Sintaxis: [captura](parametros) -> retorno { cuerpo }
      Permiten pasar pequeños bloques de código como argumentos a otras funciones.

   3. ALGORITMOS DE LA STL (<algorithm>):
      C++ incluye funciones ultra optimizadas para ordenar ('std::sort'), buscar
      ('std::find_if'), transformar y filtrar colecciones.
   ============================================================================= */

#include <iostream>
#include <vector>
#include <algorithm> // sort, find_if
#include <string>

using namespace std;

// 1. FUNCIÓN TEMPLATE (Genérica para cualquier tipo T)
template <typename T>
T obtenerMaximo(T a, T b) {
    return (a > b) ? a : b;
}

struct Item {
    string nombre;
    int precio;
};

int main() {
    cout << "=== 1. USANDO FUNCIÓN TEMPLATE ===" << endl;
    cout << "Maximo entre enteros (10, 25): " << obtenerMaximo(10, 25) << endl;
    cout << "Maximo entre decimales (3.14, 2.71): " << obtenerMaximo(3.14, 2.71) << endl;
    cout << "Maximo alfabetico ('Zelda', 'Mario'): " << obtenerMaximo(string("Zelda"), string("Mario")) << endl;
    cout << endl;

    cout << "=== 2. ALGORITMOS Y LAMBDAS (std::sort) ===" << endl;
    vector<Item> tienda = {
        {"Pocion", 50},
        {"Espada Legendaria", 1500},
        {"Escudo de Madera", 120},
        {"Arco Elfico", 450}
    };

    // Ordenamos la tienda de menor a mayor precio usando una función Lambda como criterio:
    sort(tienda.begin(), tienda.end(), [](const Item& a, const Item& b) {
        return a.precio < b.precio;
    });

    cout << "Items ordenados por precio ascendente:" << endl;
    for (const auto& item : tienda) {
        cout << " - " << item.nombre << ": $" << item.precio << endl;
    }
    cout << endl;

    cout << "=== 3. BÚSQUEDA CONDICIONAL (std::find_if) ===" << endl;
    // Buscamos el primer ítem que cueste más de $400:
    auto it = find_if(tienda.begin(), tienda.end(), [](const Item& item) {
        return item.precio > 400;
    });

    if (it != tienda.end()) {
        cout << "Primer item con precio > $400: " << it->nombre << " ($" << it->precio << ")" << endl;
    }

    return 0;
}
