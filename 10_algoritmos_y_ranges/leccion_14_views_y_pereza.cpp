/* =============================================================================
   MÓDULO 10 - LECCIÓN 14: views: qué son y por qué no corre hasta que iterás
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Hasta acá los algoritmos EJECUTABAN trabajo en el momento de la
       llamada: sort ordena YA, transform escribe YA. Las views de
       <ranges> (C++20) son otra cosa: describen el recorrido SIN
       ejecutarlo. Un view es una RECETA de recorrido, no los datos.

       Un view es un OBSERVADOR de elementos:
         - NO posee los elementos: apunta a un rango que ya existe (o los
           genera al vuelo, como iota). Copiar un view es copiar
           iteradores/límites/predicados: barato, chico, sin datos.
         - NO modifica: un view es *read-only* respecto de los elementos;
           algunos views (transform bidireccional con mutables) permiten
           escribir a través, pero lo izquierdo del módulo es "el view
           mira, el contenedor tiene".
         - ES PEREZOSO (lazy): nada corre hasta que ITERÁS el view. Como
           lo que se escribió es una RECETA, iterarla una vez la calcula,
           dos veces la recalcula (el detalle técnico se ve en la L15),
           y si el contenedor base cambia, la receta ve el cambio.
       Los views base del set:
         - `views::iota(primer, ultimo)`: genera PRIMER, primer+1, ...
           hasta ultimo-1 (EXCLUSIVE: el rango [primer, ultimo) entero,
           sin contenedor ni copia: los números se generan al vuelo).
         - `views::drop(n)` / `views::take(n)`: saltar los primeros n /
           quedarse con los primeros n, por la DERECHA del view recibido.
         - `views::reverse`: de atrás para adelante (pide bidirectional).
       Para MATERIALIZAR el resultado (tener los datos en un contenedor
       propio) se combinan dos cosas ya vistas: un vector y el algoritmo
       copy (el patrón resize+copy de la L05; back_inserter llega en L18):
           std::vector<int> salida{...};
           std::copy(view.begin(), view.end(), salida.begin());  // con lugar
       El "iterar y guardar" del bucle común funciona igual sobre un view
       (la receta ES un rango: begin/end y range-for desde la L01).
   2. ¿QUÉ PASA POR DEBAJO?
       OJO con materializar: un std::copy(primero, ultimo, out) necesita que
       begin() y end() sean del MISMO tipo; los views con take/sentinel de
       un generador sin fin pueden tener end() de otro tipo (el sentinel).
       Por eso el mundo ranges trajo copy(view, out) de UN argumento: acepta
       begin y end heterogéneos. Es la punta del iceberg de por qué los
       algoritmos ranges son cambios reales (L17). Los views con contenedor
       de pares (vector: begin/end son del mismo tipo) siguen compilando
       con el std::copy clásico.
       iota es el caso "sin datos detrás": si iterás iota(0,10) | take(5),
       los números 0..4 se GENERAN al vuelo y los 5..9 nunca existieron
       (take cortó antes): solo se produce lo que se consume. Copiar un
       view copia su INTERFAZ (referencia al rango base, el límite y el
       predicado), no los datos. Y como el view de un contenedor NO es
       dueño: si el contenedor muere (o se realoca, módulo 03), el view
       queda inválido como cualquier iterador guardado.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - Esperar un view con elementos propios: no; guardó una receta.
         Para guardar, materializá (copy a un vector).
       - iterar un view SIN límite (iota infinito): el bucle no termina
         SIEMPRE cerralo con take o drop antes de iterar.
       - Guardar un view a un contenedor que muere antes: dangling. Si
         el view debe sobrevivir al contexto, materializá los datos.
       - drop con n más grande que el rango: queda vacío (y n negativo
         no está permitido).
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/ranges
       https://en.cppreference.com/w/cpp/ranges/iota_view
       https://en.cppreference.com/w/cpp/ranges/take_view
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <ranges>
#include <vector>

int main() {
    // --- 1. iota: números generados sin contenedor ---------------------------
    // la receta: de 5 a 9 (9 exclusive)
    auto quinto = std::views::iota(5, 9);
    std::cout << "iota(5, 9): ";
    for (int n : quinto) std::cout << n << ' ';        // se genera AL iterar
    std::cout << '\n';

    // --- 2. take / drop sobre el generador -----------------------------------
    auto primeros3 = std::views::iota(100) | std::views::take(3);
    std::cout << "iota(100) | take(3): ";
    for (int n : primeros3) std::cout << n << ' ';     // 100 101 102 y FIN
    std::cout << '\n';

    const std::vector<int> niveles{4, 10, 22, 5, 90, 55};
    auto cola = niveles | std::views::drop(2);         // sin los primeros 2
    std::cout << "drop(2): ";
    for (int n : cola) std::cout << n << ' ';
    std::cout << '\n';

    // --- 3. el view es barato: copiar la receta no copia datos ---------------
    auto receta = niveles | std::views::drop(1) | std::views::take(3);
    auto copiaDeReceta = receta;                       // copiar apuntadores
    std::cout << "la receta copiada ve lo mismo: ";
    for (int n : copiaDeReceta) std::cout << n << ' ';
    std::cout << "| sizeof(view): " << sizeof(receta) << " bytes\n";

    // --- 4. LA PEREZA: nada se calcula hasta iterar --------------------------
    // Construir la receta NO toca los datos: solo iterarla los produce
    int llamadas = 0;
    auto pesada = [&llamadas](int n) {
        ++llamadas;
        return n * 2;
    };
    auto diferido = niveles | std::views::transform(pesada);
    std::cout << "llamadas despues de CONSTRUIR: " << llamadas << '\n';
    for (int n : diferido | std::views::take(2)) (void)n;       // itero: corren 2
    std::cout << "llamadas despues de iterar 2: " << llamadas << '\n';

    auto finitos = std::views::iota(100, 103);           // con fin, tipo simple
    std::vector<int> top;
    top.resize(3);                              // 1) reservo espacio
    std::copy(finitos.begin(), finitos.end(), top.begin());     // 2) copy
    std::cout << "materializado: ";
    for (int n : top) std::cout << n << ' ';
    std::cout << '\n';

    // ATENCION: primeros3 (iota INFINITA | take) no compile con std::copy(a, b,
    // c): su end() es de un TIPO distinto a begin() (el sentinel del take).
    // La version RANGES acepta el range entero como un solo argumento:
    std::vector<int> top2;
    top2.resize(3);
    std::ranges::copy(primeros3, top2.begin());  // ranges::copy(view, out)
    std::cout << "materializado con ranges::copy: ";
    for (int n : top2) std::cout << n << ' ';
    std::cout << '\n';

    return 0;
}
