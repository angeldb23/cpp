/* =============================================================================
   MÓDULO 03 - LECCIÓN 04: Algoritmos e iteradores sobre colecciones
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
      Las lecciones anteriores mostraron los contenedores. Esta muestra las
      herramientas de <algorithm> que los ejercicios necesitan todo el tiempo.
      Todas trabajan con ITERADORES: objetos que apuntan a un elemento y que se
      pueden avanzar, comparar y desreferenciar con * (como un puntero).

        - v.begin() apunta al primer elemento; v.end() apunta UNA POSICIÓN
          DESPUÉS del último (nunca se desreferencia).
        - std::max_element / std::min_element devuelven un ITERADOR, no el
          valor: para el valor hay que desreferenciar con *.
        - std::distance(a, b) cuenta cuántos pasos hay entre dos iteradores.
        - std::sort + std::unique + erase = quitar duplicados.
        - rbegin() / rend(): iteradores INVERSOS, recorren de atrás hacia
          adelante. std::reverse da vuelta una secuencia.
        - std::equal compara dos secuencias elemento a elemento.
        - std::toupper / std::tolower (de <cctype>) trabajan de a un carácter.
        - std::string_view::find(c, pos) busca desde una posición dada.

   2. ¿QUÉ PASA POR DEBAJO?
      Para un std::vector, un iterador es casi un puntero: avanzar es sumar 1 y
      desreferenciar es leer memoria contigua. Por eso los algoritmos son tan
      rápidos: el compilador los reduce a bucles simples. std::unique no libera
      memoria: reordena los elementos y devuelve el nuevo "final lógico"; el
      erase posterior es el que achica el vector. La resta de iteradores (y
      std::distance) devuelve std::ptrdiff_t, un entero CON signo.

   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
      - Desreferenciar end() o el resultado de max_element sobre un contenedor
        vacío es comportamiento indefinido: chequeá empty() primero.
      - std::unique solo colapsa duplicados ADYACENTES: ordená antes.
      - std::toupper(c) con un char negativo (letras con tilde en UTF-8) es
        comportamiento indefinido: convertí a unsigned char antes de llamar.
      - Mezclar ptrdiff_t (con signo) con size_t (sin signo) genera warnings;
        convertí con static_cast cuando haga falta.
      - Modificar un vector mientras lo recorrés con iteradores puede
        invalidarlos.

   4. PARA PROFUNDIZAR:
      https://en.cppreference.com/w/cpp/algorithm
      https://en.cppreference.com/w/cpp/iterator/distance
      https://en.cppreference.com/w/cpp/string/byte/toupper
   ============================================================================= */
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Cuenta cuántas veces aparece 'c', buscando desde una posición cada vez
std::size_t contarCaracter(std::string_view texto, char c) {
    std::size_t cuenta = 0;
    std::size_t pos = texto.find(c, 0);
    while (pos != std::string_view::npos) {
        ++cuenta;
        pos = texto.find(c, pos + 1);       // seguimos DESPUÉS de lo encontrado
    }
    return cuenta;
}

int main() {
    std::cout << "=== 1. MAX_ELEMENT / MIN_ELEMENT DEVUELVEN ITERADORES ===\n";
    std::vector<int> puntos = {40, 15, 90, 7, 62};
    auto itMax = std::max_element(puntos.begin(), puntos.end());
    auto itMin = std::min_element(puntos.begin(), puntos.end());
    std::cout << "máximo = " << *itMax << " en el índice "
              << std::distance(puntos.begin(), itMax) << "\n";
    std::cout << "mínimo = " << *itMin << " en el índice "
              << std::distance(puntos.begin(), itMin) << "\n";
    std::cout << "max(3, 8) = " << std::max(3, 8) << ", min(3, 8) = " << std::min(3, 8) << "\n";

    std::cout << "\n=== 2. QUITAR DUPLICADOS: SORT + UNIQUE + ERASE ===\n";
    std::vector<std::string> nombres = {"Luz", "Ana", "Luz", "Beto", "Ana", "Ana"};
    std::sort(nombres.begin(), nombres.end());
    auto nuevoFin = std::unique(nombres.begin(), nombres.end());
    std::cout << "duplicados que sobran: " << std::distance(nuevoFin, nombres.end()) << "\n";
    nombres.erase(nuevoFin, nombres.end());
    std::cout << "sin repetir:";
    for (const auto& n : nombres) std::cout << " " << n;
    std::cout << "\n";

    std::cout << "\n=== 3. ITERADORES INVERSOS ===\n";
    std::vector<int> v = {1, 2, 3};
    std::cout << "al revés:";
    for (auto it = v.rbegin(); it != v.rend(); ++it) std::cout << " " << *it;
    std::cout << "\n";
    std::vector<int> copia = v;
    std::reverse(copia.begin(), copia.end());
    std::cout << "reverse:";
    for (int x : copia) std::cout << " " << x;
    std::cout << "\n";
    // ¿Es capicúa? comparamos la primera mitad contra la última leída al revés
    std::vector<int> cap = {4, 8, 1, 8, 4};
    bool esCapicua = std::equal(cap.begin(),
                                cap.begin() + static_cast<std::ptrdiff_t>(cap.size() / 2),
                                cap.rbegin());
    std::cout << "{4,8,1,8,4} es capicúa? " << (esCapicua ? "sí" : "no") << "\n";

    std::cout << "\n=== 4. CARACTERES: TOUPPER / TOLOWER ===\n";
    std::string titulo = "motor grafico";
    titulo[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(titulo[0])));
    std::cout << titulo << "\n";
    int mayusculas = 0;
    for (char c : std::string("Hola Mundo ENGINE")) {
        if (std::isupper(static_cast<unsigned char>(c))) ++mayusculas;
    }
    std::cout << "mayúsculas en \"Hola Mundo ENGINE\": " << mayusculas << "\n";
    std::string chico = "ABC";
    for (char& c : chico) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    std::cout << "en minúsculas: " << chico << "\n";

    std::cout << "\n=== 5. STRING_VIEW::FIND CON POSICIÓN ===\n";
    std::cout << "cantidad de 'a' en \"banana\": " << contarCaracter("banana", 'a') << "\n";
    std::cout << "cantidad de 'z' en \"banana\": " << contarCaracter("banana", 'z') << "\n";

    std::cout << "\n=== 6. SWAP ===\n";
    int a = 1, b = 2;
    std::swap(a, b);
    std::cout << "a = " << a << ", b = " << b << "\n";

    return 0;
}
