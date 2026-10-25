/* =============================================================================
   MÓDULO 03 - LECCIÓN 03: std::span y Rangos de Datos
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
      std::span (C++20) es la versión para arrays de lo que string_view es para
      strings: una vista de solo lectura (o lectura/escritura) sobre un rango
      contiguo de datos. Acepta arrays de C, std::array, std::vector o cualquier
      bloque contiguo. No posee los datos — solo los observa.

      ¿Por qué existe?
      Antes de span, si querías escribir una función que procesara "cualquier
      array de enteros", tenías que pasar (T* ptr, size_t n) — dos parámetros
      separados, fácil de equivocar. span los une en uno solo y añade acceso
      por rango, iteradores y bounds-checking opcional.

   2. ¿QUÉ PASA POR DEBAJO?
      span<T> internamente es: (T* data, size_t size). Son 2 palabras de
      máquina — tan barato de construir y copiar como string_view. No hace
      ninguna alocación de memoria. El tamaño puede ser dinámico (en runtime)
      o estático (como parámetro de template: span<int, 5>).

   3. ERRORES COMUNES:
      - Span que sobrevive al contenedor original → dangling pointer.
      - Confundir span de solo lectura (span<const T>) con modificable.
      - Usar span en lugar de range-for cuando solo necesitas iterar.

   4. PARA PROFUNDIZAR:
      https://en.cppreference.com/w/cpp/container/span
   ============================================================================= */
#include <iostream>
#include <span>
#include <vector>
#include <array>
#include <numeric>

// Acepta cualquier rango contiguo de int: array[], std::array, std::vector
void imprimirElementos(std::span<const int> datos) {
    std::cout << "[";
    for (std::size_t i = 0; i < datos.size(); ++i) {
        std::cout << datos[i];
        if (i + 1 < datos.size()) std::cout << ", ";
    }
    std::cout << "]\n";
}

double promedio(std::span<const double> datos) {
    if (datos.empty()) return 0.0;
    double suma = 0.0;
    for (double d : datos) suma += d;
    return suma / static_cast<double>(datos.size());
}

// span<T> (sin const) permite modificar los datos originales
void escalarPorDos(std::span<int> datos) {
    for (int& x : datos) x *= 2;
}

int main() {
    std::cout << "=== 1. SPAN DESDE DISTINTOS ORÍGENES ===\n";
    int arr_c[] = {1, 2, 3, 4, 5};
    std::array<int, 4>  arr_std = {10, 20, 30, 40};
    std::vector<int>    vec     = {100, 200, 300};

    std::cout << "Array C:     "; imprimirElementos(arr_c);
    std::cout << "std::array:  "; imprimirElementos(arr_std);
    std::cout << "std::vector: "; imprimirElementos(vec);

    std::cout << "\n=== 2. SUBSPAN — VISTA PARCIAL ===\n";
    std::span<int> vista_total(arr_c);
    auto primeros3 = vista_total.first(3);   // primeros 3 elementos
    auto ultimos2  = vista_total.last(2);    // últimos 2 elementos
    auto medio     = vista_total.subspan(1, 3); // desde índice 1, 3 elementos
    std::cout << "primeros3:  "; imprimirElementos(primeros3);
    std::cout << "ultimos2:   "; imprimirElementos(ultimos2);
    std::cout << "medio:      "; imprimirElementos(medio);

    std::cout << "\n=== 3. SPAN MODIFICABLE ===\n";
    std::vector<int> datos = {1, 2, 3, 4, 5};
    std::cout << "antes:  "; imprimirElementos(datos);
    escalarPorDos(datos);
    std::cout << "después: "; imprimirElementos(datos);

    std::cout << "\n=== 4. SPAN ESTÁTICO (TAMAÑO EN TIEMPO DE COMPILACIÓN) ===\n";
    // span<int, 3> — el compilador sabe en compilación que son 3 elementos
    std::span<int, 3> span_estatico(datos.data(), 3);
    std::cout << "span estático size: " << span_estatico.size() << "\n";
    std::cout << "sizeof(span_estatico): " << sizeof(span_estatico) << " bytes\n";

    std::cout << "\n=== 5. PROMEDIO CON SPAN<CONST DOUBLE> ===\n";
    std::vector<double> notas = {8.5, 7.0, 9.2, 6.8, 10.0};
    std::cout << "promedio notas: " << promedio(notas) << "\n";
    double arr_notas[] = {5.0, 7.5, 8.0};
    std::cout << "promedio arr:   " << promedio(arr_notas) << "\n";

    std::cout << "\n=== 6. DATOS Y TAMAÑO ===\n";
    std::span<int> s(datos);
    std::cout << "data ptr: " << static_cast<void*>(s.data()) << "\n";
    std::cout << "size: " << s.size() << "\n";
    std::cout << "size_bytes: " << s.size_bytes() << "\n";

    return 0;
}
