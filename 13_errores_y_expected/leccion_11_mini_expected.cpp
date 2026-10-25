/* =============================================================================
   MÓDULO 13 - LECCIÓN 11: mini_expected: implementá tu expected en C++20
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       L10 mostró el std::expected real (C++23). Pero el proyecto del
       curso compila con -std=c++20... y encima ver la implementación
       aconseja entender LO QUE ES un expected (y no solo su uso). Con
       lo que ya sabés (templates del módulo 09, optional del 11, regla
       de cero del 08) se puede escribir UNA clase propia con la misma
       API: la llamamos mini::expected y la tenés en el header
       include/mini_expected.hpp del curso.

       Las piezas que la componen (la lista del diseño):
         - el ESTADO: un T o un E, nunca los dos. En la implementación
           didáctica vive en dos std::optional (uno para el valor y uno
           para el error); el std::expected real usa una UNION con el
           bool activo (la diferencia es de memoria, no de semántica).
         - el bool "hay valor": has_value() y el operator bool, EXACTO
           igual que optional (modulo 11).
         - la CONSTRUCCIÓN: implícita desde T (el caso feliz) e
           implícita desde E (el caso error) — por eso una función que
           devuelve expected<int, Codigo> hace `return 5;` o
           `return Codigo::FormatoInvalido;` sin armar nada.—también un
           constructor con la ETIQUETA mini::unexpect para el caso
           explícito de error (el espejo de std::unexpect de C++23):
             expected<int, Codigo>(mini::unexpect, Codigo::FormatoInvalido)
         - value(): si no hay valor, lanza (módulo 07: la decisión no es
           del llamado silencioso: el programa NO ignora el check):
           en la mini lanzamos std::runtime_error; el std::expected
           lanza std::bad_expected_access<E> (el tipo del error viajando
           dentro).
         - value_or(porDefecto): la versión que NUNCA lanza.
         - las MONÁDICAS (L12): and_then, transform, transform_error y
           or_else: en la mini son 4 templates con invoke_result_t del
           módulo 09 para DEDUCIR el tipo devuelto de cada callable.
       ¿Por qué NO se reemplaza el std::expected real? Porque la API
       completa (monádicas sobre lvalue/rvalue con ref-qualifiers, el
       swap, operator==, bad_expected_access especializado... más 300
       líneas) la tiene la biblioteca. Nuestra mini enseña el corazón;
       el resto es pulido de interfaz.
    2. ¿QUÉ PASA POR DEBAJO?
        Con dos optionals y el bool implícito (has_value de valor_):
         - la CAJA del estado no es gratis: dos optionals tamaño max de
           (T, E) dos veces; el std::expected real usa UNA unión (solo
           un lado se guarda). Para aprender la semántica alcanza; para
           hot code de motor, el one real de la STL.
         - las MONÁDICAS están hecho con invoke_result_t (módulo 09):
           `using Res = std::decay_t<std::invoke_result_t<F&, const T&>>;`
           y el Res es el expected del TIPO que retorna tu callable:
           la deducción hace que las pipes de L12 no escriban tipos a
           mano.
         - value() lanza si no hay valor: es el espejo del mandatory del
           expected real (bad_expected_access): la MINI lanza
           std::runtime_error "a mano"; el estándar trae el tipo
           especializado y stub array register en el E.
   ============================================================================= */
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

#include "../include/mini_expected.hpp"

namespace Leccion11 {

// La firma declara T y E: ese es el "contrato" del resultado.
mini::expected<int, std::string> parseEntero(const std::string& s) {
    if (s.empty()) return std::string("string vacío");   // E implícito
    for (char c : s) {
        if (c < '0' || c > '9') return std::string("carácter inválido: ") + c;
    }
    return std::stoi(s);                                  // T implícito
}

} // namespace Leccion11

int main() {
    std::cout << "== construcción implícita a los dos lados ==\n";
    auto bien = Leccion11::parseEntero("42");
    auto mal = Leccion11::parseEntero("4a2");
    std::cout << "42:   " << (bien ? "ok: " + std::to_string(bien.value()) : "error") << '\n';
    std::cout << "4a2:  " << (mal ? "ok" : "error: " + mal.error()) << '\n';
    std::cout << "vacio: " << (Leccion11::parseEntero("").has_value() ? "ok" : "error") << '\n';

    std::cout << "== value_or: la versión sin ifs ==\n";
    std::cout << "parseEntero(\"\").value_or(-1) = "
              << Leccion11::parseEntero("").value_or(-1) << '\n';

    std::cout << "== value() sobre error lanza (la decisión del programador) ==\n";
    try {
        (void)Leccion11::parseEntero("").value();
    } catch (const std::runtime_error& e) {
        std::cout << "runtime_error atrapada: " << e.what() << '\n';
    }

    std::cout << "== el constructor con mini::unexpect (el explícito) ==\n";
    mini::expected<int, std::string> soloError(mini::unexpect, std::string("proyectado"));
    std::cout << "bool = " << static_cast<bool>(soloError)
              << ", error = " << soloError.error() << '\n';
    (void)soloError;
    return 0;
}
