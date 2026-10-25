/* =============================================================================
   MÓDULO 03 - LECCIÓN 02: std::vector y std::array
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
      std::vector es el contenedor más usado en C++. Es un array dinámico que
      se redimensiona automáticamente. std::array es un wrapper de array
      estático con tamaño fijo conocido en tiempo de compilación. Ambos
      garantizan almacenamiento contiguo en memoria (como arrays de C).

      Regla práctica:
        - Tamaño conocido en compilación y pequeño → std::array
        - Tamaño variable o desconocido → std::vector

   2. ¿QUÉ PASA POR DEBAJO?
      std::vector almacena: puntero a buffer en heap, size y capacity.
      Al push_back cuando size==capacity, realoca duplicando capacity
      (amortizado O(1)). std::array<T,N> es exactamente sizeof(T)*N bytes
      en stack — cero overhead, ideal para datos pequeños y fijos.

   3. ERRORES COMUNES:
      - Acceder fuera de rango con [] → undefined behavior. Usar .at() para
        lanzar excepciones en debug.
      - Invalidación de iteradores: cualquier operación que cause realocación
        invalida todos los iteradores/referencias al vector.
      - Comparar .size() (unsigned) con int negativo → warning signed/unsigned.

   4. PARA PROFUNDIZAR:
      https://en.cppreference.com/w/cpp/container/vector
      https://en.cppreference.com/w/cpp/container/array
   ============================================================================= */
#include <iostream>
#include <vector>
#include <array>
#include <numeric>   // std::accumulate
#include <algorithm> // std::sort, std::find

int main() {
    std::cout << "=== 1. std::array — TAMAÑO FIJO ===\n";
    std::array<int, 5> dados = {3, 1, 4, 1, 5};
    std::cout << "dados[2] = " << dados[2] << "\n";
    std::cout << "size = " << dados.size() << "\n";
    // Iterar con range-for
    std::cout << "elementos: ";
    for (int d : dados) std::cout << d << " ";
    std::cout << "\n";
    // Ordenar
    std::sort(dados.begin(), dados.end());
    std::cout << "ordenado: ";
    for (int d : dados) std::cout << d << " ";
    std::cout << "\n";

    std::cout << "\n=== 2. std::vector — TAMAÑO DINÁMICO ===\n";
    std::vector<std::string> inventario;
    inventario.push_back("Espada");
    inventario.push_back("Escudo");
    inventario.push_back("Poción");
    inventario.emplace_back("Arco"); // construye in-place
    std::cout << "inventario (" << inventario.size() << " items):\n";
    for (std::size_t i = 0; i < inventario.size(); ++i)
        std::cout << "  [" << i << "] " << inventario[i] << "\n";

    std::cout << "\n=== 3. RESERVAR CAPACIDAD ===\n";
    std::vector<int> nums;
    nums.reserve(100); // evita realocaciones futuras
    std::cout << "size=" << nums.size() << " capacity=" << nums.capacity() << "\n";
    for (int i = 0; i < 10; ++i) nums.push_back(i * i);
    std::cout << "tras 10 push_backs: size=" << nums.size() << " capacity=" << nums.capacity() << "\n";

    std::cout << "\n=== 4. OPERACIONES ÚTILES ===\n";
    // Eliminar elemento (swap-and-pop O(1) si orden no importa)
    auto it = std::find(inventario.begin(), inventario.end(), "Escudo");
    if (it != inventario.end()) {
        std::swap(*it, inventario.back());
        inventario.pop_back();
    }
    std::cout << "tras eliminar Escudo: ";
    for (auto& s : inventario) std::cout << s << " ";
    std::cout << "\n";

    // erase-remove idiom
    std::vector<int> vals = {1,2,3,2,4,2,5};
    vals.erase(std::remove(vals.begin(), vals.end(), 2), vals.end());
    std::cout << "tras erase-remove(2): ";
    for (int v : vals) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "\n=== 5. ALGORITMOS NUMÉRICOS ===\n";
    std::vector<double> pesos = {1.5, 2.3, 0.8, 4.1};
    double suma = std::accumulate(pesos.begin(), pesos.end(), 0.0);
    std::cout << "suma pesos: " << suma << "\n";

    std::cout << "\n=== 6. VECTOR DE VECTORES (MATRIZ 2D) ===\n";
    std::vector<std::vector<int>> matriz(3, std::vector<int>(4, 0));
    matriz[1][2] = 42;
    for (auto& fila : matriz) {
        for (int x : fila) std::cout << x << "\t";
        std::cout << "\n";
    }

    return 0;
}
