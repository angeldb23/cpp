/* =============================================================================
   MÓDULO 3 - LECCIÓN 02: Arreglos Fijos y Colecciones Dinámicas (std::vector)
   =============================================================================
   1. Arreglos de tamaño fijo:
      En C++ moderno preferimos 'std::array<Tipo, Tamaño>' en vez del arreglo viejo de C.
      Su tamaño debe conocerse en tiempo de compilación y no puede crecer.

   2. El rey indiscutido: 'std::vector<Tipo>':
      Un arreglo dinámico que crece y se achica automáticamente según lo necesites.
      Maneja la memoria de forma ultra eficiente contigua en la RAM (amigable con la caché).

   3. Bucle moderno 'range-based for':
      for (const auto& elemento : coleccion)
   ============================================================================= */

#include <iostream>
#include <vector>
#include <array>

using namespace std;

int main() {
    cout << "=== 1. ARREGLO FIJO MODERNO (std::array) ===" << endl;
    array<int, 4> puntosControl = {10, 25, 50, 100};
    cout << "Cantidad de puntos: " << puntosControl.size() << endl;
    cout << "Primer punto: " << puntosControl[0] << ", Ultimo: " << puntosControl.back() << endl;
    cout << endl;

    cout << "=== 2. ARREGLO DINÁMICO (std::vector) ===" << endl;
    // Creamos un vector vacío de strings:
    vector<string> enemigos;

    // Agregar elementos al final con push_back()
    enemigos.push_back("Goblin");
    enemigos.push_back("Orco");
    enemigos.push_back("Dragon");

    cout << "Cantidad actual de enemigos: " << enemigos.size() << endl;

    // Acceder a elementos por índice
    cout << "Segundo enemigo: " << enemigos[1] << endl;

    // Eliminar el último elemento con pop_back()
    enemigos.pop_back(); // Elimina "Dragon"
    cout << "Enemigos despues de pop_back: " << enemigos.size() << endl;
    cout << endl;

    cout << "=== 3. RECORRER COLECCIONES CON RANGE-BASED FOR ===" << endl;
    // 'const auto&' evita copiar cada string en cada vuelta del bucle:
    cout << "Lista de enemigos en el mapa:" << endl;
    for (const auto& enemigo : enemigos) {
        cout << " - " << enemigo << endl;
    }
    cout << endl;

    cout << "=== 4. VECTOR DE NÚMEROS Y MODIFICACIÓN ===" << endl;
    vector<int> puntajes = {150, 300, 450};

    // Si queremos modificar los elementos dentro del bucle, usamos 'auto&' (sin const):
    for (auto& puntaje : puntajes) {
        puntaje += 50; // Bonificación a todos
    }

    cout << "Puntajes actualizados: ";
    for (int p : puntajes) {
        cout << p << " ";
    }
    cout << endl;

    return 0;
}
