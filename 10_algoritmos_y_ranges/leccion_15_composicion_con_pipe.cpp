/* =============================================================================
   MÓDULO 10 - LECCIÓN 15: composición con el pipe | y lo que pasa al iterar
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Los views se convinan con el operador `|` (pipe): la salida de una
       entra como la entrada de la otra. Se LEE como un conducto:
           v | filtro | transformacion | corte
       es "del vector v, tomá los que pasan el filtro, transformalos, y
       quedate con los primeros N". Dos propiedades de negocio:
         - El orden del pipeline es exactamente el del cable: cada stage
           ve el resultado del stage anterior.
         - NADA se ejecuta en la construcción: armá el pipeline hace
           NADA (la L14 lo demostró con el transform pesado); el trabajo
           corre POR ETAPA al iterar: para cada elemento, el take pide
           uno al filter, el filter pide uno al contenedor, y así
           retrospectivamente (pull: el consumido ARRIBA pide a la
           izquierda empuja el dato del siguiente pedido).

       Las consecuencias que hay que tener en la cabeza:
         1. Así como filter pide elementos al contenedor UNO A LA VEZ y
            el predicado corre sobre CADA elemento de la secuencia
            ANTES de pasarlo al siguiente stage: si iterás el pipeline
            DOS veces, el filter corre DOS veces. Repetir iteración
            de un view = recalcular: NADA se cachea.
         2. El view NO es una snapshot: si el vector cambió debajo
            (borrar/agregar elementos), la PRÓXIMA iteración ve el nuevo
            estado. Es justamente lo que NO pasa con materializar.
         3. Un pipeline lazy es UNA sola pasada con N etapas, en vez de
            N pasadas de los algoritmos clásicos (L03-L10) con
            contenedores intermedios: ésa es la ganancia de fondo.
   2. ¿QUÉ PASA POR DEBAJO?
       Cada stage del pipe ES un view wrapper que adentro tiene a los
       ANTERIORES. filter | transform | take es tipos ANINADOS (una tarea
       del tipo exacto de cada stage del compositor: los views son de
       tipos anidados (el take adentro tiene al transform, que adentro
       tiene al filter). El costo del pipeline es el canasto de los stages de
       crear moves/copies (el view es barato, digo la L14). La
       complementación es -compilada en el operador pipe de las views:
       el `|` (operator|) disponible en la C++20 dentro de <ranges>.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - Se anadie dos veces o iterate/computation calculation (a estilo
         trying to assign) CALQUIER datum nuevo: iterar el mismo pipeline
         RECORRE la de nuevo (si el original cambió, cambia la salida).
         Nunca asumir que el view es una foto del estado al build-time.
       - Views con predicados/transformaciones impuras (que cambian
         estado): se re-evalúan en cada iteración y nada te avisa. La
         pureza es la regla (L16).
       - COMPOSITION vs a curva: si un pipeline usa varias el mismo
         vector y la relación entre ellos es inicialmente------+------
         espera; es mejor computar el pipeline UNA vez y materializar.
       - Guardar un view y hacer push_back del vector después: si
         realoca, el view queda inválido (como un iterador guardado,
         módulo 03).
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/ranges/filter_view
       https://en.cppreference.com/w/cpp/ranges/transform_view
       https://en.cppreference.com/w/cpp/language/range-for
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <ranges>
#include <vector>

int main() {
    const std::vector<int> rondas{320, 90, 150, 700, 240, 60, 180};

    // --- 1. el pipeline: se lee de izquierda a derecha -----------------------
    // "de las rondas, quedate con las de > 100, sumales bonus 10, quedate
    // con las 3 primeras":
    auto pipeline = rondas
        | std::views::filter([](int hp) { return hp > 100; })
        | std::views::transform([](int hp) { return hp + 10; })
        | std::views::take(3);
    std::cout << "pipeline: ";
    for (int v : pipeline) std::cout << v << ' ';      // 330 160 710
    std::cout << '\n';

    // --- 2. iterar DOS veces: el filter corre DE NUEVO -----------------------
    int llamados = 0;
    auto contada = [&llamados](int hp) {
        ++llamados;
        return hp > 100;
    };
    auto conContador = rondas | std::views::filter(contada);
    (void)std::ranges::distance(conContador);          // iteracion 1
    (void)std::ranges::distance(conContador);          // iteracion 2
    std::cout << "el filtro corrio " << llamados
              << " veces entre las dos iteraciones (se recalcula: nada\n";
    std::cout << "queda cacheado; la segunda pasada puede correr\n";
    std::cout << "menos porque filter guarda el inicio)\n";

    // --- 3. el view NO es una foto: el vector cambia, el view ve el cambio ---
    std::vector<int> vivo{1, 2, 3};
    auto pares = vivo | std::views::filter(
        [](int x) { return x % 2 == 0; });
    std::cout << "pares ahora: " << std::ranges::distance(pares) << '\n';
    vivo.push_back(4);
    std::cout << "pares despues de push_back(4): "
              << std::ranges::distance(pares) << '\n';

    // --- 4. composicion vs el camino de algoritmos: una pasada vs tres -------
    // el pipeline anterior (filter+) equivaleria en la STL clasica a
    // copy_if + transformer + copy y 3 vectores intermedios; lazy lo hace
    // en UNA recorrida sin nada intermedio:
    std::cout << "uno y el recorrido lazily: min de pipeline = "
              << *std::ranges::min_element(pipeline) << '\n';

    return 0;
}
