/* =============================================================================
   MÓDULO 03 - LECCIÓN 01: std::string y std::string_view
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
      std::string es el tipo principal para manejar texto en C++ moderno.
      A diferencia de los arrays de char de C, std::string gestiona su propia
      memoria dinámicamente: crece y se encoge según sea necesario. Internamente
      suele usar "Small String Optimization" (SSO): si el texto es corto (<15-22
      chars según implementación), lo almacena directamente en el objeto sin
      pedir memoria al heap.

      std::string_view (C++17) es una "ventana" de solo lectura sobre cualquier
      texto existente (literal, string, array de chars). NO copia datos. Es ideal
      para parámetros de funciones que solo leen texto — evita alocaciones
      innecesarias.

   2. ¿QUÉ PASA POR DEBAJO?
      Un std::string típicamente contiene: un puntero a char en el heap, un
      size (cantidad de caracteres) y un capacity (espacio reservado). Cuando
      size == capacity y agregás más caracteres, hace realloc (generalmente
      duplica capacity). string_view solo tiene: un puntero y un tamaño —
      nada más. Por eso es O(1) construirla y copiarla.

   3. ERRORES COMUNES:
      - Retornar string_view que apunta a un string local → dangling reference.
      - Mezclar índices negativos (string usa size_t, índices negativos wrappean).
      - Olvidar que find() retorna string::npos cuando no encuentra nada.

   4. PARA PROFUNDIZAR:
      https://en.cppreference.com/w/cpp/string/basic_string
      https://en.cppreference.com/w/cpp/string/basic_string_view
   ============================================================================= */
#include <iostream>
#include <string>
#include <string_view>
#include <algorithm>

// Función que toma vista de solo lectura — no copia nada
void analizar(std::string_view sv) {
    std::cout << "  texto: \"" << sv << "\"\n";
    std::cout << "  longitud: " << sv.size() << "\n";
    std::cout << "  empieza con 'H': " << (sv.starts_with('H') ? "sí" : "no") << "\n";
}

int main() {
    std::cout << "=== 1. CONSTRUCCIÓN Y ASIGNACIÓN ===\n";
    std::string s1 = "Hola";
    std::string s2("Mundo");
    std::string s3(5, 'A'); // "AAAAA"
    std::cout << s1 << " " << s2 << " " << s3 << "\n";

    std::cout << "\n=== 2. CONCATENACIÓN Y APPEND ===\n";
    std::string mensaje = s1 + ", " + s2 + "!";
    mensaje += " Fin.";
    std::cout << mensaje << "\n";

    std::cout << "\n=== 3. MÉTODOS ÚTILES ===\n";
    std::string texto = "El motor de juego crasheó";
    std::cout << "size: "      << texto.size()              << "\n";
    std::cout << "empty: "     << texto.empty()             << "\n";
    std::cout << "substr(3,5): " << texto.substr(3, 5)      << "\n"; // "motor"
    auto pos = texto.find("juego");
    std::cout << "find 'juego': pos=" << pos                << "\n";
    std::cout << "npos? " << (pos == std::string::npos ? "no encontrado" : "encontrado") << "\n";

    std::cout << "\n=== 4. MODIFICACIÓN ===\n";
    std::string str = "  espacio  ";
    // trim manual
    auto inicio = str.find_first_not_of(' ');
    auto fin    = str.find_last_not_of(' ');
    if (inicio != std::string::npos)
        str = str.substr(inicio, fin - inicio + 1);
    std::cout << "|" << str << "|\n";

    std::string lower = "HOLA MUNDO";
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    std::cout << "lowercased: " << lower << "\n";

    std::cout << "\n=== 5. STRING_VIEW ===\n";
    std::string nombre = "Hola C++20";
    std::string_view vista = nombre;        // sin copia
    std::string_view sub   = vista.substr(5, 5); // "C++20"
    std::cout << "vista: " << vista << "\n";
    std::cout << "sub:   " << sub   << "\n";

    analizar("Literal directo");  // también funciona con literales
    analizar(nombre);              // y con std::string

    std::cout << "\n=== 6. COMPARACIÓN ===\n";
    std::string a = "alpha", b = "beta";
    std::cout << "alpha < beta? " << (a < b ? "sí" : "no") << "\n";
    std::cout << "comparación lexicográfica: " << a.compare(b) << " (negativo = a < b)\n";

    return 0;
}
