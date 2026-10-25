/* =============================================================================
   MÓDULO 10 - LECCIÓN 10: numéricos: accumulate, reduce y partial_sum
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       <numeric> trae las herramientas de "pliegue" (fold): consumir el rango
       de a un elemento, acumulando en un solo valor.

       `std::accumulate(prin, fin, inicial)`: suma acumulada a partir de
       `inicial`. La firma esconde una trampa clásica: el TIPO del
       acumulador es el tipo de `inicial`. `accumulate(v.begin(), v.end(),
       0)` sobre un vector<double> te acumula en INT: cada paso truncado.
       La versión correcta pasa un inicial double: `0.0`.
       Con op: `std::accumulate(prin, fin, inicial, op)` acumula con
       CUALQUIER op binario (por ejemplo, `std::multiplies<>{}` para el
       producto). op recibe (acumulado, elemento).

       `std::reduce(prin, fin)` / `reduce(prin, fin, inicial)` / con op: ES
       accumulate en la versión moderna (C++17), con UNA diferencia de
       contrato: no garantiza el orden de las operaciones (asume que op es
       ASOCIATIVO y CONMUTATIVO (la resta NO sirve). Precisamente por esa
       libertad puede ser más rápido (el compilador puede agrupar, y las
       versiones con políticas de ejecución -módulo 14- pueden paraleizar).
       Regla práctica: querés determinismo y op cualquiera -> accumulate;
       el op es asociativo/conmutativo y buscás velocidad -> reduce.

       `std::partial_sum(prin, fin, salida)`: escribe en `salida` los
       acumulados PARCIALES (v[i], v[i]+v[i-1], ...): n resultados, no uno.
       Para "¿cuánto llevo después de i rondas?" sin re-sumar cada vez.
       Realmente si se le pasa el mismo rango como salida, lo cubre, incluso
       (cansada la cadena: 2 golpes: extra visita a la línea).

       BONUS del set: `std::inner_product(p1, fin1, p2, inicial)`: recorre
       DOS secuencias juntas acumulando el valor de cada par (el clásico
       producto punto: sumar a_i*b_i). Dos ops: una COMBINA cada par, otra
       ACUMULA los resultados.
   2. ¿QUÉ PASA POR DEBAJO?
       accumulate es un fold FIJO de izquierda a derecha: "((0 + a) + b) +
       c". Cumplir ese orden le permite op asociativos pero NO conmutativos
       (la concatenación de strings es asociativa sin ser conmutativa: vale
       para accumulate, no para reduce). reduce deja el orden libre:
       ((a+b)+(c+d)) o (((a+b)+c)+d) le dan lo mismo. Con floats esa
       libertad puede cambiar los últimos bits: el orden libre no es gratis,
       se paga con la exigencia al op.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - Acumulador de tipo equivocado (init 0 vs 0.0): el truncamiento
         silencioso pasa igual en accumulate y en reduce: el tipo que
         decide es el del init.
       - reduce con un op NO asociativo (la resta): el resultado depende
         del orden y el contrato de la función no lo garantiza
       - partial_sum sin espacio en la salida para n resultados: resize
         antes, como siempre.
       - inner_product con secuencias de TAMAÑO distinto: recorre hasta el
         final de la primera e ignora el resto de la segunda (no valida
         nada).
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/algorithm/accumulate
       https://en.cppreference.com/w/cpp/algorithm/reduce
       https://en.cppreference.com/w/cpp/algorithm/partial_sum
   ============================================================================= */
#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>
#include <vector>

int main() {
    const std::vector<int> danos{15, 20, 8, 32};       // daño por ronda

    // --- 1. accumulate con el init correcto ----------------------------------
    const int total = std::accumulate(danos.begin(), danos.end(), 0);
    std::cout << "danos total: " << total << '\n';

    const std::vector<double> probas{0.25, 0.10, 0.05};
    // el init ES el tipo del acumulador: 0.0 (double), no 0 (int):
    const double proba_total = std::accumulate(probas.begin(), probas.end(), 0.0);
    std::cout << "proba total: " << proba_total << '\n';

    // --- 2. accumulate con op: el producto -----------------------------------
    const std::vector<int> multipliers{2, 3, 2};
    const int combo = std::accumulate(multipliers.begin(), multipliers.end(), 1,
                                      std::multiplies<int>{});
    std::cout << "combo x" << combo << '\n';

    // --- 3. reduce: el mismo pliegue con orden libre -------------------------
    const int total_r = std::reduce(danos.begin(), danos.end(), 0);
    std::cout << "danos total (reduce): " << total_r << '\n';

    // --- 4. partial_sum: la curva de acumulados ------------------------------
    std::vector<int> curva{};
    curva.resize(danos.size());                    // lugar para n resultados
    std::partial_sum(danos.begin(), danos.end(), curva.begin());
    std::cout << "acumulado por ronda: ";
    for (int r : curva) std::cout << r << ' ';
    std::cout << '\n';

    // --- 5. inner_product: dos secuencias, le producto punto -----------------
    const std::vector<int> danos_jugador{15, 20, 8, 32};
    const std::vector<int> armadura_enemigo{2, 2, 0, 1};   // dano efectivo
    const std::size_t n = std::min(danos_jugador.size(), armadura_enemigo.size());
    const int neto = std::inner_product(
        danos_jugador.begin(), danos_jugador.begin() + static_cast<long>(n),
        armadura_enemigo.begin(), 0,                              // init
        std::plus<int>{},                           // como acumular
        [](int d, int arm) { return d - arm; });    // como combinar el par
    std::cout << "danos netos acumulados: " << neto << '\n';

    return 0;
}
