/* =============================================================================
   MÓDULO 09 - LECCIÓN 06: Perfect forwarding y referencias universales
   (migrada y ampliada desde el módulo 02 - lección 06)
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
      Perfect forwarding = pasar un argumento a otra función SIN cambiar su
      categoría de valor: si el que llama te dio un lvalue, lo que llega
      allá es un lvalue (se copia, se usa por referencia); si te dio un
      rvalue, lo que llega es un rvalue (se puede mover).

      La herramienta es un parámetro de template con doble ampersand,
      T&&, cuando el tipo T SE ESTÁ DEDUCIENDO:
        template <typename T> void f(T&& x);
      Ahí T&& no significa "referencia a rvalue acá", significa "ligate a
      lo que venga": se llaman referencias universales (o forwarding
      references). La deducción juega una doble cartilla:
        - Si te pasan un lvalue de tipo X,  T deduce X&   -> T&& = X&
        - Si te pasan un rvalue de tipo X,  T deduce X    -> T&& = X&&
      Con `auto&&` pasa lo mismo (es la misma regla: se usa a lot en
      range-for genérico).

      Después, std::forward<T>(x) devuelve x con su categoría ORIGINAL:
        - T = X&  -> forward(x) es un lvalue (no se mueve nada)
        - T = X&& -> forward(x) es un rvalue (se puede mover)
      Sin forward, x adentro de la función es SIEMPRE un lvalue (tiene
      nombre), ¡aunque lo que llegó haya sido un rvalue! Perderías la
      oportunidad de moverlo.

      Cuidado: T&& solo es referencia universal si T se deduce PARA ESO.
      Acá T&& es universal:    template <typename T> void f(T&&);
      Acá NO lo es (T está fijado): void f(int&&);   -> referencia rvalue
      normal, acepta solo rvalues. Y en un std::vector<T>&& tampoco.

   2. ¿QUÉ PASA POR DEBAJO?
      La deducción de T a partir de un lvalue (T = X&) es la que hace que
      T&& colapsen en una referencia a lvalue: X& && -> X&. Ese "colapso de
      referencias" es la maquinaria real (se mira a fondo en la lección 08).
      std::forward<T> es un cast expresivo, un static_cast<T&&>: resulta
      idéntico según qué T dedujo el compilador.

   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
      - Llamá a forward SIEMPRE con el parámetro de template explícito:
        std::forward<T>(x), no std::forward(x): sin T no hace nada útil.
      - Adentro de la función, usar x dos veces después de forwardarlo:
        la segunda vez puede que ya esté movida.
      - No confundí "referencia universal" con "referencia que acepta
        todo":
      - Preferí auto&& en range-for sobre rangos de proxy (map): cambiando
        la categoría el rango no se rompe.

   4. PARA PROFUNDIZAR:
      cppreference: https://en.cppreference.com/w/cpp/utility/forward
      C++ Core Guidelines: C.45: "Don't insist on an argument being an
      rvalue when it should be a forwarding reference".
   ============================================================================= */
#include <concepts>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// --- 1. ¿Qué dedujo T? Un lector de deducción con if constexpr (lección 03)
template <typename T>
constexpr std::string_view deduccion() {
    if constexpr (std::is_same_v<T, int>)                return "int";
    else if constexpr (std::is_same_v<T, int&>)          return "int&";
    else if constexpr (std::is_same_v<T, const int&>)    return "const int&";
    else if constexpr (std::is_same_v<T, std::string>)   return "std::string";
    else if constexpr (std::is_same_v<T, std::string&>)  return "std::string&";
    else                                                 return "otro";
}

template <typename T>
void mirarDeduccion(T&&) {
    std::cout << "  argumento. T dedujo: " << deduccion<T>() << "\n";
}

// --- 2. forwarding básico a un par de sobrecargas ---
// El lvalue cae en esta: no se puede mover nada.
void procesar(const int& x) {
    std::cout << "  procesar(lvalue): " << x << " (sin tocar)\n";
}
// El rvalue cae en esta: adentro podría absorberse (moverse).
void procesar(int&& x) {
    std::cout << "  procesar(rvalue):  " << x << " (absorbible)\n";
}

template <typename T>
void redirigir(T&& x) {
    // forward<T>(x): conserva la categoría del argumento ORIGINAL.
    procesar(std::forward<T>(x));
}

// --- 3. auto&&: la misma regla para variables ---
// Útil en range-for genérico: acepta lvalues y proxies sin copiar.
template <typename Contenedor>
long largoTotal(const Contenedor& c) {
    long suma = 0;
    for (auto&& x : c) {                 // auto&& = referencia universal
        if constexpr (requires { x.size(); { x[0] } -> std::convertible_to<char>; })
            suma += static_cast<long>(x.size());
        else
            suma += static_cast<long>(x);
    }
    return suma;
}

int main() {
    std::cout << "=== 1. DEDUCION DE T EN T&& ===\n";
    int edad = 42;
    mirarDeduccion(edad);                // lvalue  -> T = int&
    mirarDeduccion(99);                  // rvalue  -> T = int
    const int& alias = edad;
    mirarDeduccion(alias);               // lvalue a const -> T = const int&
    std::string nombre = "hola";
    mirarDeduccion(nombre);              // lvalue  -> T = std::string&
    mirarDeduccion(std::string("chau")); // rvalue  -> T = std::string

    // Verificación al compilar de la tabla completa:
    std::string& refNombre = nombre;
    static_assert(std::is_same_v<decltype(edad), int>);                    // lvalue int: es int
    static_assert(std::is_same_v<decltype(99), int>);                      // prvalue
    static_assert(std::is_same_v<decltype(alias), const int&>);            // lvalue const int&
    static_assert(std::is_same_v<decltype(refNombre), std::string&>);
    std::cout << "  static_asserts de tipos: ok en compilacion\n";

    std::cout << "\n=== 2. FORWARD PRESERVA LA CATEGORIA ===\n";
    int dato = 7;
    redirigir(dato);                     // lvalue   -> const int&
    redirigir(7);                        // rvalue   -> int&&
    redirigir(std::move(dato));          // rvalue explicito -> int&&

    std::cout << "\n=== 3. SIN FORWARD LA CATEGORIA SE MUERE ===\n";
    // Adentro de redirigir, `x` tiene nombre: es un lvalue SIEMPRE.
    // std::forward<T> devuelve la categoría que tenía el argumento original;
    // si la sacás, todo caería en la sobrecarga de const int&.
    std::cout << "  (la deduccion de arriba ya lo mostro: T int& vs int)\n";

    std::cout << "\n=== 4. auto&& EN RANGE-FOR GENERICO ===\n";
    std::vector<std::string> palabras = {"sol", "luna", "estrella"};
    std::vector<int> enteros = {1, 2, 3};
    std::cout << "  suma de largos: " << largoTotal(palabras) << "\n";
    std::cout << "  suma de enteros: " << largoTotal(enteros) << "\n";

    return 0;
}
