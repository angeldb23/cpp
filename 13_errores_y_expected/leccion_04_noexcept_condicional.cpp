/* =============================================================================
   MÓDULO 13 - LECCIÓN 04: noexcept condicional y su efecto en movimiento
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       L03 dejó el operador noexcept(expr) como consulta. Ahora lo usa-
       mos del otro lado: dentro de la propia firma, y aparece la forma
       más útil del módulo:

         template <typename T>
         void intercambiar(T& a, T& b) noexcept(noexcept(a.swap(b))) {
             a.swap(b);
         }

       Se lee: "esta función promete no lanzar SI la llamada a swap lo
       es". El noexcept INTERIOR es una CONSULTA sobre una expresión
       concreta (como el módulo 09: cuando T es un template, la consulta
       se hace al tipos que lleguen); el noexcept EXTERIOR es la
       promesa. Es el noexcept CONDICIONAL: la promesa se deduce, y no
       hay lugar para mentir: si el swap de T lanza, tu función no
       promete; si el swap de T es no-throw, la promesa se hereda gratis.

       ¿Por qué carga tanto este detalle? Recordemos el módulo 08.
       El std::vector reubica memoria cuando crece, y para reubicar
       traslada cada elemento: si el move de T promete noexcept, mueve
       cada uno al bloque nuevo (rápido, sin copiar nada) y libera el
       bloque viejo; pero si
       el move de T PODRÍA lanzar, mover a mitad del traslado deja el
       vector roto: la mitad en el bloque nuevo y la mitad "desparramada". Para no romper la invariante (tema de la L05) el
       vector baja del move y usa COPIAS: copia uno por uno al bloque
       nuevo; si una copia lanza, el original quedó intacto (garantía
       sólida, pero CARA). Todo ese diseño depende de UNA pregunta que el compilador
       contesta leyendo firmas: ¿el move de T promete noexcept?
       Regla práctica: si tu clase está bien diseñada (swap + move
       noexcept para la regla de 5, como enseñó el módulo 08), te
       conviene que SI o SI escribas la promesa: si la olvidás,
       los contenedores caen al camino de la copia y pagás velocidad
       sin haber hecho nada "grande" mal.
       ¿Dónde más pesa noexcept? En plantillas genéricas es la pieza
       de toda la eficiencia de la STL: la promesa se DEDUCE pieza a
       pieza (el noexcept condicional) y "sube" por las paredes de los
       templates: std::vector<T> es cosa seria SI y SOLO SI el move y
       el swap de T prometen. Nada de esto es "oculto": todo se consulta
       con los traits del módulo 09, en tiempo de compilación.

       En fin: la pregunta "¿prometo noexcept o no?" NO se contesta con
       idealismo: se contesta mirando lo que el cuerpo llama. Si el
       cuerpo llama cosas que pueden lanzar, no prometas... salvo que
       las atrapes ADENTRO con try/catch (que es lo que hace la STL en
       varios noexcept: "el cuerpo lanza TORPE, y yo lo salvo EN TODO"
       como intentaParsear más abajo).
    2. ¿QUÉ PASA POR DEBAJO?
        Con noexcept condicional, el compilador hace la CONSULTA de la
        L03 en los tipos REALES en cada instanciación del template y
        "pinta" cada instancia con true o false independientemente: una
        misma plantilla puede tener un vector<InstanciaNoexcept> y otra
        sin: lo aguja alto. Y las std::move y std::forward son
        noexcept (son casts, L08): no dependen de T. El std::swap
        global usa "la tripleta move"/swap de clase: es noexcept si TU
        swap (o el move de tu clase) lo es; en módulo 13, la L06 armar
        dos clases para ver el desarrollo y el costo.
                La consulta puede ser COMPUESTA (&&, ||) sobre varios traits,
        como trasladaSinLanzar más abajo: si T falla en un solo
        trait, la constexpr sale false y el contenedor elige el otro
        camino en tiempo de COMPILACIÓN: cero costo de conflicto en
        ejecución.
    3. BUENAS PRÁCTICAS:
        - swap, move ctor y move assign de clases con recursos:
          noexcept: es el kit de la regla de 5 (módulo 08).
        - En funciones genéricas, no escribas noexcept(true) a ciegas:
          deducila: noexcept(condición) sobre una expresión representativa
          del cuerpo. Si el cuerpo llama tres cosas y cualquiera puede
          lanzar, la condición las incluye a las tres.
        - No prometas noexcept en funciones de alto nivel (manejo de
          I/O, lógica de juego): garantibase estáticas al menos; el
          módulo 13 te da las formas de fallar (los otros canales,
          T) para no mentir con la promesa.
        - los traits estándar (is_nothrow_*) son LA forma de consultar;
          usalos en static_assert para documentar diseño (módulo 09:
          estático, gratis y te avisa en compilación si cambiás algo).
   ============================================================================= */
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace Leccion04 {

// (1) swap genérico noexcept condicional: la receta pro de la STL.
//     "promete no lanzar SI el swap de T no lanza".
template <typename T>
void intercambiarSinLanzar(T& a, T& b) noexcept(noexcept(std::swap(a, b))) {
    std::swap(a, b);   // para clases: usa el swap de la clase si existe
}

// Segmento de consulta compuesta, tipo módulo 09: dos traits juntas.
template <typename T>
constexpr bool trasladaSinLanzar =
    std::is_nothrow_move_constructible_v<T> && std::is_nothrow_swappable_v<T>;

static_assert(trasladaSinLanzar<int>, "int se traslada sin lanzar");
static_assert(trasladaSinLanzar<std::string>, "string también");

// (2) un "move bueno" y uno "malo" para ver el efecto EN REALIDAD
//     en el vector (la misma receta que contemplamos arriba).
struct SeMueveLigero {
    int v = 0;
    explicit SeMueveLigero(int v_) : v(v_) {}
    SeMueveLigero(const SeMueveLigero&) : v(-1) {
        std::cout << "    copia llamada\n";
    }
    SeMueveLigero(SeMueveLigero&& otro) noexcept : v(otro.v) {
        std::cout << "    move noexcept llamado\n";
    }
};
static_assert(std::is_nothrow_move_constructible_v<SeMueveLigero>);

struct SeMueveLanzando {
    int v = 0;
    explicit SeMueveLanzando(int v_) : v(v_) {}
    SeMueveLanzando(const SeMueveLanzando&) : v(-1) {
        std::cout << "    copia llamada\n";
    }
    SeMueveLanzando(SeMueveLanzando&& otro) : v(otro.v) {  // SIN noexcept
        std::cout << "    move SIN noexcept llamado\n";
    }
};
static_assert(!std::is_nothrow_move_constructible_v<SeMueveLanzando>);

// (3) la promesa estrambótica DE SEGURO existencial: si el cuerpo llama
//     cosas que pueden lanzar y las ATRAPA adentro con try/catch,
//     la promesa es honesta. La STL hace esto en varios noexcept.
bool intentaParsear(const std::string& s) noexcept {   // perí la promesa
    try {
        std::stoi(s);                    // puede lanzar... y acá lo atrapo
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

} // namespace Leccion04

int main() {
    std::cout << "== swap condicional sobre int y string ==\n";
    int x = 7, y = 13;
    Leccion04::intercambiarSinLanzar(x, y);
    std::cout << "x = " << x << ", y = " << y
              << " (noexcept con int: "
              << (noexcept(Leccion04::intercambiarSinLanzar(x, y)) ? "sí" : "no") << ")\n";

    std::string a = "vida", b = "mana";
    Leccion04::intercambiarSinLanzar(a, b);
    std::cout << "a = " << a << ", b = " << b << '\n';

    std::cout << "== consulta compuesta (el análisis del vector) ==\n";
    std::cout << "trasladaSinLanzar<int> = "
              << (Leccion04::trasladaSinLanzar<int> ? "true" : "false")
              << "; trasladaSinLanzar<std::string> = "
              << (Leccion04::trasladaSinLanzar<std::string> ? "true" : "false") << '\n';

    std::cout << "== el efecto EN EL vector de cada promesa de move ==\n";
    std::cout << "vector<SeMueveLigero> (move noexcept):\n";
    std::vector<Leccion04::SeMueveLigero> vec1;
    vec1.push_back(Leccion04::SeMueveLigero{1});
    std::cout << "  tamaño 1; acelero a 8:\n";
    vec1.reserve(8);
    std::cout << "vector<SeMueveLanzando> (move sin noexcept):\n";
    std::vector<Leccion04::SeMueveLanzando> vec2;
    vec2.push_back(Leccion04::SeMueveLanzando{1});
    std::cout << "  tamaño 1; acelero a 8:\n";
    vec2.reserve(8);

    std::cout << "== promesa honesta con try/catch dentro ==\n";
    std::cout << "intentaParsear(\"42\"):  " << (Leccion04::intentaParsear("42") ? "true" : "false")
              << "; intentaParsear(\"basura\"): " << (Leccion04::intentaParsear("basura") ? "true" : "false")
              << " (y ambas NO lanzan: noexcept respetada)\n";
    return 0;
}
