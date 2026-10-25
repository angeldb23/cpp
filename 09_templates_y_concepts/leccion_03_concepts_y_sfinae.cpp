/* =============================================================================
   MÓDULO 09 - LECCIÓN 03: Concepts, if constexpr y SFINAE
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
      Un template acepta CUALQUIER tipo, y si el tipo no sirve el error aparece
      adentro de la función, con mensajes imposibles de leer. C++ tiene tres
      herramientas para decir "este template solo acepta tipos que cumplan X":

        a) SFINAE clásico (C++11): std::enable_if. Funciona, pero es críptico.
        b) if constexpr (C++17): elige código distinto según el tipo, dentro de
           UNA misma función.
        c) Concepts (C++20): la forma moderna. Le ponés nombre a una condición
           (concept Numerico = ...) y la usás como restricción.

      SFINAE = "Substitution Failure Is Not An Error": si reemplazar T en la
      firma de un template produce un tipo inválido, el compilador no da error:
      descarta ESA sobrecarga y prueba las demás.

   2. ¿QUÉ PASA POR DEBAJO?
      Nada de esto existe en el binario: son chequeos en tiempo de compilación.
      Con if constexpr, la rama que no corresponde NI SIQUIERA se compila (dentro
      de un template), por eso podés llamar a .size() en una rama sin romper
      los tipos que no lo tienen. Los concepts, además, participan en la
      resolución de sobrecargas: si dos funciones aplican, gana la MÁS
      restringida.

   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
      - if constexpr solo "apaga" ramas dentro de templates. En una función
        normal, las dos ramas se compilan igual.
      - std::integral<bool> y std::integral<char> son verdaderos: bool y char
        son tipos enteros para C++.
      - Un requires con { expr } -> concept chequea el TIPO del resultado.
      - Buena práctica moderna: en código nuevo, concepts primero;
        enable_if solo para leer código viejo.
      - Los traits de transformación (std::remove_reference_t, std::remove_cv_t)
        devuelven un TIPO nuevo y se pueden encadenar. El ORDEN importa: primero
        se saca la referencia y recién después el const.

   4. PARA PROFUNDIZAR:
      https://en.cppreference.com/w/cpp/language/constraints
      https://en.cppreference.com/w/cpp/header/concepts
      https://en.cppreference.com/w/cpp/types
   ============================================================================= */
#include <concepts>
#include <cstddef>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// --- 1. SFINAE clásico: dos sobrecargas, una se descarta según T ---
template <typename T>
std::enable_if_t<std::is_integral_v<T>, T> mitadViejo(T x) {
    return x / 2;                         // división entera
}
template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, T> mitadViejo(T x) {
    return x * static_cast<T>(0.5);
}

// --- 2. Detection idiom: ¿el tipo tiene .size()? ---
template <typename T, typename = void>
struct TieneSizeViejo : std::false_type {};
template <typename T>
struct TieneSizeViejo<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type {};

// --- 3. if constexpr: una sola función, ramas según el tipo ---
template <typename T>
std::string tipoEn(const T& x) {
    if constexpr (std::is_integral_v<T>) {
        return "entero de " + std::to_string(sizeof(T)) + " byte(s)";
    } else if constexpr (std::is_floating_point_v<T>) {
        return "decimal";
    } else if constexpr (std::is_pointer_v<T>) {
        return "puntero";
    } else if constexpr (TieneSizeViejo<T>::value) {
        return "coleccion de " + std::to_string(x.size()) + " elemento(s)";
    } else {
        return "otro";
    }
}

// --- 4. Concepts: ponerle nombre a una condición ---
template <typename T>
concept Numerico = std::integral<T> || std::floating_point<T>;

template <typename T>
concept ConSize = requires(const T& t) {
    { t.size() } -> std::convertible_to<std::size_t>;
};

// Tres formas equivalentes de restringir:
template <Numerico T>
T doble(T x) { return x * 2; }                       // 1) en el parámetro

template <typename T>
    requires ConSize<T>
std::size_t cuantos(const T& c) { return c.size(); } // 2) cláusula requires

std::string rotulo(const Numerico auto& x) {         // 3) auto restringido
    return "numero " + std::to_string(x);
}

// --- 5. Sobrecarga por concepts: gana la más restringida ---
template <typename T>
std::string clase(const T&) { return "cualquier cosa"; }
template <Numerico T>
std::string clase(const T&) { return "numerico"; }
template <ConSize T>
std::string clase(const T&) { return "con size()"; }

// Requisito simple: "¿compila a + b?". Sirve para tipos propios con operator+.
template <typename T>
concept Sumable = requires(T a, T b) { a + b; };

// Los parámetros de un requires pueden ser de CUALQUIER tipo, incluso de uno
// que no sea T: por ejemplo un std::ostream& para preguntar "¿se puede enviar
// a un flujo?". Acá preguntamos si dos valores se pueden comparar con <, y
// además exigimos que el resultado sea convertible a bool.
template <typename T>
concept Ordenable = requires(const T& a, const T& b) {
    { a < b } -> std::convertible_to<bool>;
};

// Ejemplo con un flujo como parámetro (el tipo std::ostream viene de <iostream>)
template <typename T>
concept ImprimibleEnFlujo = requires(std::ostream& flujo, const T& valor) {
    flujo << valor;
};

// --- 6. Traits de transformación: devuelven un tipo nuevo ---
// is_same_v sirve para chequear en compilación que dos tipos son iguales.
static_assert(std::is_same_v<std::remove_reference_t<int&>, int>);
static_assert(std::is_same_v<std::remove_cv_t<const volatile int>, int>);
static_assert(std::is_same_v<std::remove_cv_t<std::remove_reference_t<const int&>>, int>);

struct Sorpresa {};

int main() {
    std::cout << "=== 1. SFINAE CLÁSICO (enable_if) ===\n";
    std::cout << "mitadViejo(9)   = " << mitadViejo(9)   << "  (entera)\n";
    std::cout << "mitadViejo(9.0) = " << mitadViejo(9.0) << "\n";

    std::cout << "\n=== 2. DETECTION IDIOM ===\n";
    std::cout << "vector tiene size? " << TieneSizeViejo<std::vector<int>>::value << "\n";
    std::cout << "int tiene size?    " << TieneSizeViejo<int>::value << "\n";

    std::cout << "\n=== 3. IF CONSTEXPR ===\n";
    std::vector<int> v = {1, 2, 3};
    std::cout << "tipoEn(7):          " << tipoEn(7) << "\n";
    std::cout << "tipoEn(2.5):        " << tipoEn(2.5) << "\n";
    std::cout << "tipoEn(vector):     " << tipoEn(v) << "\n";
    std::cout << "tipoEn(Sorpresa):   " << tipoEn(Sorpresa{}) << "\n";

    std::cout << "\n=== 4. CONCEPTS ===\n";
    std::cout << "doble(21)    = " << doble(21) << "\n";
    std::cout << "doble(1.25)  = " << doble(1.25) << "\n";
    std::cout << "cuantos(vec) = " << cuantos(v) << "\n";
    std::cout << "rotulo(5)    = " << rotulo(5) << "\n";
    // doble(std::string("x")) NO compila, y el error dice qué concepto falló.

    std::cout << "\n=== 5. SOBRECARGA POR CONCEPTS ===\n";
    std::cout << "clase(3):          " << clase(3) << "\n";
    std::cout << "clase(vector):     " << clase(v) << "\n";
    std::cout << "clase(Sorpresa):   " << clase(Sorpresa{}) << "\n";

    std::cout << "\n=== 6. REQUISITOS SIMPLES Y TRAITS DE TRANSFORMACIÓN ===\n";
    std::cout << "Sumable<int>:      " << Sumable<int> << "\n";
    std::cout << "Sumable<Sorpresa>: " << Sumable<Sorpresa> << "\n";
    std::cout << "Ordenable<int>:    " << Ordenable<int> << "\n";
    std::cout << "Ordenable<Sorpresa>: " << Ordenable<Sorpresa> << "\n";
    std::cout << "ImprimibleEnFlujo<std::string>: " << ImprimibleEnFlujo<std::string> << "\n";
    std::cout << "ImprimibleEnFlujo<Sorpresa>:    " << ImprimibleEnFlujo<Sorpresa> << "\n";
    const char* texto = "hola";
    std::cout << "tipoEn(const char*): " << tipoEn(texto) << "\n";
    // Los static_assert de arriba ya probaron, al compilar, que los traits
    // remove_reference_t / remove_cv_t hacen lo que dice la lección.

    return 0;
}
