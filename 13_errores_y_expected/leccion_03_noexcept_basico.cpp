/* =============================================================================
   MÓDULO 13 - LECCIÓN 03: noexcept, parte 1: qué promete y cómo se consulta
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       noexcept es una PROMESA que una función le hace al compilador y a
       quien la usa: "esta función no sale lanzando excepciones". La
       firma queda: void f() noexcept;. Y a diferencia de un comentario
       suelto, esta promesa NO es decorativa: trae dos consecuencias:
         1. SI la promesa se rompe (un throw adentro de una noexcept),
            el programa NO busca manejadores: llama std::terminate()
            y termina. (C++11 reemplazó a la vieja especificación
            dinámica throw(); eso quedó eliminado.)
         2. La promesa entra al SISTEMA DE TIPOS: se consulta en tiempo
            de compilación y las bibliotecas la LEEN para elegir
            caminos. Es por esto que el módulo 08 insistió: "el vector
            reubica con move SI el move no lanza": noexcept manda.
       ¿Es un contrato sobre el error? NO: una función noexcept no
       garantiza que no haya errores, garantiza que la falla no SALE
       como excepción: si algo sale mal, reportá por código/estado
       (L01) o el error no sale de ahí. Prometió "no lanzar", no
       "no fallar".
       Cómo se CONSULTA: con el operador noexcept(expr): es una
       expresión constante en tiempo de compilación:
         noexcept(f()) -> true si f está declarada noexcept (no ejecuta nada)
       Ese operador es la base de la L04: "noexcept condicional" (una
       función es noexcept si su pieza lo es). Y hay las traits de tipos
       del módulo 09, que empacan la misma consulta:
         std::is_nothrow_move_constructible_v<T>
         std::is_nothrow_swappable_v<T>
       Una función con noexcept(false) es el "con permiso de lanzar"
       explícito: es el default (las funciones pueden lanzar), pero a
       veces conviene escribirlo a mano cuando el default de una
       operación ES noexcept (el destructor: L08) y vos querés lanzar
       igual. La promesa también la aplican "por familia" varias piezas
       del estándar: los destructores son noexcept por defecto (L08 lo
       justifica), las funciones de deallocation (operator delete) y el
       swap de la STL son noexcept.
     2. ¿QUÉ PASA POR DEBAJO?
         El compilador escribe las tablas del unwinding (L02) y en las
         tablas registra las funciones noexcept: una excepción que
         entra a un frame noexcept no tiene "cómo continuar" y la rutina
         de propagación llama a std::terminate(): el proceso se mata,
         sin buscar catch. La promesa NO se verifica en el cuerpo: si
         `int f() noexcept` hace throw, COMPILA bien y se rompe recién
         en ejecución. En cambio, la CONSULTA sí se verifica: el
         operador noexcept(expr) se evalúa mirando las declaraciones
         (con la sobrecarga elegida por los tipos, módulo 02), todo en
         tiempo de compilación: nada de esto toca la ejecución.
     3. BUENAS PRÁCTICAS:
         - Marcá noexcept en lo que REALMENTE no lanza: swap de tus
           clases, moves de tipos livianos, funciones que mueven bytes
           (la regla del 08: swap + move noexcept = ella misma en la
           regla de 5: es el kit de la L06).
         - No marques noexcept en funciones que hacen memoria o llaman
           código que puede lanzar (L05-L08 estira el tema): si el
           cuerpo puede lanzar, una promesa rota es std::terminate en
           el peor momento.
         - Cuando una función es noexcept CONDICIONAL (no depende de
           agresividad tuya sino de un parámetro/dato), no la escribas
           a mano: deducila con noexcept(expr): L04.
         - El dtor es noexcept por defecto: si tu clase necesita un
           dtor LANZÁNDOLO, poné noexcept(false) a mano y leé la L08
           antes de hacercape un proceso por esa decisión.
   ============================================================================= */
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

namespace Leccion03 {

// (1) la promesa básica: noexcept aparece en la firma y el error de
//     negocio sale por código (L01), no por throw.
bool dividirSeguro(int a, int b, int& resultado) noexcept {
    if (b == 0) { resultado = 0; return false; }
    resultado = a / b;
    return true;
}

// (2) el operador noexcept(expr): CONSULTA sin ejecutar. Es constexpr:
//     el compilador elige la sobrecarga por los tipos (módulo 02) y mira
//     las declaraciones, nada corren).
struct Lanzadora {
    void abre() const { throw 1; }         // firma common: permiso de lanzar
    void cierra() const noexcept {}        // prometió: no lanza
};
static_assert(!noexcept(Lanzadora{}.abre()), "abre() lanza: no es noexcept");
static_assert(noexcept(Lanzadora{}.cierra()), "cierra() prometió noexcept");

// (3) noexcept(false): el "con permiso de lanzar" explícito (igual al
//     default, PERO declara la intención). Acá está el uso clásico de
//     reloj: el destructor default es noexcept y si querés lanzar eps
//     noexcept(false) (y la decisión justa se analiza en la L08).
struct RelojTick {
    ~RelojTick() noexcept(false) {}          // explicito en el DTOR default
};

// (4) la promesa en funciones "bajas": el swap de int no lanza. NOTAS
//     el compilador NO revisa el CUERPO: revisa la DECLARACIÓN:
void swapInts(int& a, int& b) noexcept {
    int tmp = a; a = b; b = tmp;
}
static_assert(noexcept(swapInts(std::declval<int&>(), std::declval<int&>())),
              "nuestra swap promete nothrow");

// (5) la promesa leída por las traits del módulo 09: empaquetan la MISMA
//     consulta, para que una biblioteca pueda decidir caminos según T.
static_assert(std::is_nothrow_move_constructible_v<int>,
              "el move de int es trivial: no lanza");
static_assert(std::is_nothrow_move_constructible_v<std::string>,
              "el move de string puede mover el buffer: no lanza");
static_assert(std::is_nothrow_swappable_v<int>, "swap<int> no lanza");

} // namespace Leccion03

int main() {
    std::cout << "== promesa básica: falla SIN lanzar ==\n";
    int r = 0;
    std::cout << "dividirSeguro(10, 0): " << (Leccion03::dividirSeguro(10, 0, r)
                ? "ok" : "falló sin lanzar") << ", resultado quedó " << r << '\n';
    Leccion03::dividirSeguro(84, 2, r);
    std::cout << "dividirSeguro(84, 2): " << r << '\n';

    std::cout << "== el operador noexcept(expr) como consulta ==\n";
    std::cout << "noexcept(cierra()): "
              << (noexcept(Leccion03::Lanzadora{}.cierra()) ? "true" : "false") << '\n';
    std::cout << "noexcept(abre()):   "
              << (noexcept(Leccion03::Lanzadora{}.abre()) ? "true" : "false") << '\n';

    std::cout << "== is_nothrow_*: la misma promesa, empaquetada ==\n";
    std::cout << "is_nothrow_move_constructible_v<int>       = "
              << std::is_nothrow_move_constructible_v<int> << '\n';
    std::cout << "is_nothrow_move_constructible_v<std::string> = "
              << std::is_nothrow_move_constructible_v<std::string> << '\n';
    return 0;
}
