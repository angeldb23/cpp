/* =============================================================================
   MÓDULO 09 - LECCIÓN 08: Reference collapsing y la mecánica de std::forward
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
      En C++ no existen referencias a referencias... salvo cuando el
      compilador está manipulando TIPOS (un alias de tipo, la deducción de
      T en un template, decltype). En esos lugares dos ampersands se
      COLAPSAN en uno, con esta tabla de cuatro casos:

        &  &   ->  &
        &  &&  ->  &
        && &   ->  &
        && &&  ->  &&

      Es decir: si ALGUNA de las dos es & (lvalue-ref), el resultado es &;
      solo && && conserva &&. Esa regla de cuatro renglones es la llave de
      todo el forwarding.

      ¿Dónde aparece el colapso? En DOS lugares principales:
        + En la deducción de T de una referencia universal (lección 06):
          con un lvalue de tipo X se deduce T = X&, así que el T&& real es
          X& && -> X& (por eso liga bien a ese lvalue).
        + En tipos escritos o deducidos a mano: `using B = int&; using C = B&&;`
          hace C = int&.

      Y acá está la mecánica de std::forward<T>: no hay nada mágico, ES
      un static_cast<T&&>(x), y el T&& hace todo el trabajo mediante el
      colapso:
        - el argumento llegó como lvalue   -> T = X&  -> T&& = X&&? NO:
          X& && colapsa a X& -> el cast deja un LVALUE: no se mueve nada.
        - el argumento llegó como rvalue   -> T = X   -> T&& = X&& ->
          el cast deja un RVALUE: se puede mover.
      std::move es la versión simple del mismo truco (un static_cast<T&&>
      con T NO deducido: convierte siempre a rvalue); std::forward es la
      versión condicional que recuerda cómo llegó el valor.

   2. ¿QUÉ PASA POR DEBAJO?
      El colapso es una regla de tipos en compilación, nada de runtime.
      Con static_assert + is_same_v se puede verificar la tabla entera
      (hago eso acá arriba y en el main). Con auto&& pasa exactamente lo
      mismo que con T&&: en un contexto genérico, auto ocupa el lugar del
      T y se deduce con las mismas reglas.

   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
      - decltype(parametro) en un T&& da el TIPO DECLARADO (o sea, T&& ya
        colapsado), no genérico "int": leelo con cuidado.
      - El parámetro x de `void f(T&& x)` es un LVALUE adentro de la
        función por ser una variable con nombre: el colapso trabaja sobre
        tipos, no cambia la categoría de lo que ya tiene nombre.
      - Si especificás el T a mano (forward<int&&>(x)), estás fijando el
        resultado del colapso vos mismo: entendé qué estás pidiendo en
        cada llamada.

   4. PARA PROFUNDIZAR:
      https://en.cppreference.com/w/cpp/language/reference
      https://en.cppreference.com/w/cpp/utility/forward
      C++ Core Guidelines: F.19, "For forwarding parameters, use T&&".
   ============================================================================= */
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

// --- 1. La tabla completa del colapso, verificada en compilación ---
using A1 = int&;        // referencia lvalue
using A2 = int&&;       // referencia rvalue

using B1 = A1&;         // int& &    -> int&
using B2 = A1&&;        // int& &&   -> int&
using B3 = A2&;         // int&& &   -> int&
using B4 = A2&&;        // int&& &&  -> int&&

static_assert(std::is_same_v<B1, int&>);
static_assert(std::is_same_v<B2, int&>);
static_assert(std::is_same_v<B3, int&>);
static_assert(std::is_same_v<B4, int&&>);

template <typename T>
constexpr std::string_view nombreDe() {
    if constexpr (std::is_lvalue_reference_v<T>)  return "&";
    else if constexpr (std::is_rvalue_reference_v<T>) return "&&";
    else                                              return "(sin referencias)";
}

// --- 2. ¿Qué dedujo T? Inspección en runtime con if constexpr ---
template <typename T>
constexpr std::string_view deduccionDe() {
    if constexpr (std::is_same_v<T, int>)               return "int";
    else if constexpr (std::is_same_v<T, int&>)         return "int&";
    else if constexpr (std::is_same_v<T, const int&>)   return "const int&";
    else if constexpr (std::is_same_v<T, std::string>)  return "std::string";
    else if constexpr (std::is_same_v<T, std::string&>) return "std::string&";
    else                                                return "otro";
}

template <typename T>
void colapsar(T&&) {
    // El propio T graba el resultado: deduccionDe<T> lo imprime.
    std::cout << "  T dedujo: " << deduccionDe<T>() << "\n";
}

int main() {
    std::cout << "=== 1. LA TABLA DEL COLAPSO (verificada al compilar) ===\n";
    std::cout << "  int&  &  -> int"  << nombreDe<B1>() << "\n";
    std::cout << "  int&  && -> int"  << nombreDe<B2>() << "\n";
    std::cout << "  int&& &  -> int"  << nombreDe<B3>() << "\n";
    std::cout << "  int&& && -> int" << nombreDe<B4>() << "\n";
    std::cout << "  (gana & con que APAREZCA un &; solo && && conserva &&)\n";

    std::cout << "\n=== 2. DEDUCCION DE T EN UNA REFERENCIA UNIVERSAL ===\n";
    int dato = 7;
    colapsar(dato);                    // lvalue  -> T = int&
    colapsar(7);                       // rvalue  -> T = int
    colapsar(std::move(dato));         // rvalue  -> T = int
    colapsar(std::string("texto"));    // rvalue  -> T = std::string
    std::string nombre = "hola";
    colapsar(nombre);                  // lvalue  -> T = std::string&

    std::cout << "\n=== 3. auto&& DEDUCE CON LA MISMA REGLA ===\n";
    auto&& a = dato;                   // int&
    auto&& b = std::move(dato);        // int&&
    auto&& c = 42;                     // int&&  (prvalue)
    static_assert(std::is_same_v<decltype(a), int&>);
    static_assert(std::is_same_v<decltype(b), int&&>);
    static_assert(std::is_same_v<decltype(c), int&&>);
    a = 8;                              // escribible: es el mismo 'dato'
    std::cout << "  'a' es int&, 'b' y 'c' son int&& (verificado en compilacion),"
                 " dato ahora vale " << dato << "\n";

    std::cout << "\n=== 4. forward ES static_cast<T&&> ===\n";
    // decltype de la llamada devuelve EXACTO el tipo resultante del cast:
    static_assert(std::is_same_v<decltype(std::forward<int&>(dato)), int&>);
    static_assert(std::is_same_v<decltype(std::forward<int&&>(std::move(dato))), int&&>);
    std::cout << "  forward<int&>(x)   -> int&   (lvalue: no se mueve)\n";
    std::cout << "  forward<int&&>(x)  -> int&&  (rvalue: se puede mover)\n";
    std::cout << "  std::move(x)       -> int&&  (siempre rvalue, sin recordar "
                 "cómo llegó)\n";
    std::cout << "\n  Recordá: adentro de 'void f(T&& x)', la variable x es un\n"
              << "  lvalue por tener nombre; el colapso trabaja sobre tipos,\n"
              << "  no sobre variables con nombre.\n";
    return 0;
}
