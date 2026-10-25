/* =============================================================================
   MÓDULO 10 - LECCIÓN 16: filter/transform/take/drop: detalles y pureza
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Las cuatro views de trabajo diario, ahora con lupa:
         - `views::filter(pred)`: pasa solo lo que cumple. Dos detalles
           con fuerza: (a) re-evalúa pred en cada iteración: la pereza
           implica que el filtro NO cachea resultados; (b) pred debe ser
           PURO (sin efectos externos): el filter revisa posiciones
           cuando le hacen falta y una misma puede revisarse más de una
           vez; un pred con estado dá resultados no reproducibles.
         - `views::transform(f)`: mapea cada elemento con f. Igual regla
           de pureza: f se invoca CADA VEZ que se lee la posición (el
           view no cachea: iterar dos veces son dos pasadas completas de
           f). Para un cálculo caro, materializá antes de transformar.
         - `views::take(n)` / `take_while(pred)`: cortar por arriba por
           N (counted, barato para generadores) o por pred (hasta el
           primero que no cumpla).
         - `views::drop(n)` / `drop_while(pred)`: saltar los n primeros /
           hasta el primero que NO cumple.
         - Bonus de familia: `views::reverse` (de atrás para adelante,
           SIN ordenar ni copiar) y, para mapas de la L11, keys/values
       Los views soportan EXACTAMENTE el contrato de la L01: begin/end,
       range-for y el paso de algoritmos. La advertencia sigue valiendo:
       guardar un view es guardar una referencia al contenedor; si los
       datos deben SER TUYOS, materializá (el patrón de la L14).
   2. ¿QUÉ PASA POR DEBAJO?
       filter_view/transform_view son WRAPPERS sobre el view subyacente:
       al iterarlos, cada pregunta de "¿siguiente?" la delegan al
       underlying: filter consulta el elemento y le aplica pred (y salta
       el que no), transform guarda f y la aplica al leer. take/drop son
       wrappers de límites. Resultado: cada recorrida REPITE los
       predicados (nada cachea) y el take corta temprano: en el ejemplo,
       take(2) deja de pedir elementos y las posiciones siguientes ni se
       miran: pereza de los dos lados.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - filter con pred que cambia estado: la misma posición puede
         revisarse dos veces y el resultado depende del recorrido. Evitá
         mutate dentro de los parámetros; si el estado es necesario,
         materializá primero.
       - transform de cálculo caro usado dos veces: son dos pasadas de f.
         Materializá el resultado en un vector para pagar una sola.
       - take_while corta en el PRIMER no-cumplido (no toma los que
         siguen): si tus datos no tienen una zona inicial uniforme,
         take_while no sirve para "tomar los que cumplen": tomá filter
         o usá un take con n.
       - Guardar un view como si poseyera datos: no posee nada.
         Guardá materializados (resize+copy, L14) o re-usá el pipeline.
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/ranges/filter_view
       https://en.cppreference.com/w/cpp/ranges/take_while_view
       https://en.cppreference.com/w/cpp/ranges/drop_while_view
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <ranges>
#include <vector>

int main() {
    const std::vector<std::string> palabras{"alpha", "Beta", "beta", "GAMMA"};

    // --- 1. filter + take: se corta temprano (pereza en ambos lados) ---------
    int leidas = 0;
    auto contadas = [&leidas](const std::string& s) {
        ++leidas;                                   // cuenta cada REVISION
        return s.find_first_of("bB") != std::string::npos;   // tiene 'b' o 'B'?
    };
    auto filtro = palabras
        | std::views::filter(contadas)
        | std::views::take(2);
    // pasan alpha? no; Beta? si. beta? si y el take cortó; GAMMA se
    // revisa SOLO porque take(2) probea la altura para descubrir el fin
    std::cout << "las 2 con b/B: ";
    for (const std::string& s : filtro) std::cout << s << ' ';
    std::cout << "| revisiones: " << leidas << " (alpha..GAMMA, el 5mo no existe)\n";

    // --- 2. transform: se llama EN cada lectura ------------------------------
    int transforms = 0;
    auto costosa = [&transforms](int hp) {
        ++transforms;
        return hp * hp;
    };
    const std::vector<int> hp{3, 5, 8};
    auto alCubo = hp | std::views::transform(costosa);
    std::cout << "transform 1da pasada: ";
    for (int v : alCubo) std::cout << v << ' ';
    std::cout << "| transform corridos: " << transforms << '\n';
    std::cout << "2da pasada (recalcula): ";
    for (int v : alCubo) std::cout << v << ' ';
    std::cout << "|total: " << transforms << '\n';

    // --- 3. drop_while / take_while: el corte por predicado ------------------
    const std::vector<int> fila{0, 0, 0, 5, 9, 2};
    auto sinLosCeros = fila | std::views::drop_while(
        [](int x) { return x == 0; });              // salta los ceros del frente
    std::cout << "drop_while: ";
    for (int x : sinLosCeros) std::cout << x << ' ';
    std::cout << '\n';

    const std::vector<int> ranked{7, 6, 5, 2, 9, 1};
    auto mejorTrio = ranked | std::views::take_while(
        [](int x) { return x > 4; });               // corta AL PRIMER no-cumplido
    std::cout << "take_while: ";
    for (int x : mejorTrio) std::cout << x << ' ';  // 7 6 5 (el 9 llegó tarde y no entra)
    std::cout << '\n';

    // --- 4. reverse: view bidireccional, SIN ordenar ni copiar ---------------
    std::cout << "reversa: ";
    const std::vector<int> datos{1, 2, 3, 4};
    for (int x : datos | std::views::reverse) std::cout << x << ' ';
    std::cout << '\n';

    return 0;
}
