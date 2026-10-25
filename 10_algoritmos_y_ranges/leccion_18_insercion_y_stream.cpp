/* =============================================================================
   MÓDULO 10 - LECCIÓN 18: iteradores de inserción y de stream
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       La cuenta pendiente de la L05: ¿cómo copiar a un vector VACÍO si
       los algoritmos nunca hacen push_back? Con los ADAPTADORES: envolver
       el contenedor en un ITERADOR cuyo `=` hace la inserción por vos:

         - `std::back_inserter(v)`: cada *it = x es v.push_back(x). Es el
           caso práctico del 90% (vector/deque/list). Con copy/copy_if/
           transform: el destino CRECE solo.
         - `std::front_inserter(d)`: hace d.push_front (solo en
           contenedores que la tienen: deque/list). Cuidado con el orden
           que casi nadie espera: copiar {1,2,3} con front_inserter da
           3,2,1 (es el físico del push_front).
         - `std::inserter(c, pos)`: hace c.insert(pos, x) y AVANZA la
           posición en cada inserción: sirve para insertar en orden en
           una list, y para map/set (donde pos es solo una pista).

       La forma de materializar un pipeline (L14-L16) queda corta con
       ranges: std::ranges::copy(view, std::back_inserter(v)). Ni resize
       ni nada: el vector crece a medida que entra cada elemento.

       Y el otro lote del tema: los iteradores de STREAM:
         - `std::istream_iterator<T>`: lee T del stream con >> de a uno;
           el constructor sin stream es el "fin de la entrada".
         - `std::ostream_iterator<T>(os, sep)`: escribir cada valor seguido
           del separador sep.
       Con copy cierran el TRÍO clásico: leer los ints de un stream,
       escribirlos a otro con separador, sin bucle. Acá se usa istringstream
       (in-memory); con ifstream/ofstream (módulo 12) la mecánica es la
       misma.
   2. ¿QUÉ PASA POR DEBAJO?
       back_inserter(c) devuelve un std::back_insert_iterator: un objeto
       cuyo operator= está REDEFINIDO para llamar c.push_back(x). Por eso
       el algoritmo escribe siempre de la misma forma (*salida = valor) y
       el adaptador lo redirige: la escritura de copy se convierte en
       inserción. Los de stream usan operator>> / operator<< en cada paso
       (++ avanza la lectura). Es un uso especial del contrato `=` de la
       L01: en vez de escribir donde apunta, se invierte la operación.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - back_inserter SOLO para contenedores con push_back (map/set no
         lo tienen: ahí va el inserter(c, c.end())).
       - front_inserter invierte el orden (es el físico del push_front):
         con orden idéntico te alcanza back_inserter.
       - istream_iterator corta la lectura apenas el stream falla (un
         dato inválido en medio termina la copia en silencio): con
         entrada de confianza pasa, con entrada externa hay que
         validar (el módulo 12 entra a fondo).
       - transform a un back_inserter: la salida crece UN elemento por
         entrada (no OJO: exacto). Para un vector final con MEMORIA
         ajustada, reservá antes o usá el patrón resize del L05.
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/iterator/back_inserter
       https://en.cppreference.com/w/cpp/iterator/istream_iterator
       https://en.cppreference.com/w/cpp/iterator/ostream_iterator
   ============================================================================= */
#include <algorithm>
#include <deque>
#include <iostream>
#include <list>
#include <iterator>
#include <numeric>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

int main() {
    // --- 1. back_inserter: el destino vacío ya no es problema ----------------
    const std::vector<int> hp{45, 80, 12, 92};
    std::vector<int> filtro;
    std::copy_if(hp.begin(), hp.end(), std::back_inserter(filtro),
                 [](int x) { return x > 40; });     // el vector CRECE solo
    std::cout << "copiados al vuelo: ";
    for (int x : filtro) std::cout << x << ' ';
    std::cout << '\n';

    // --- 2. materializar un pipeline (L14-L16) en una línea ------------------
    auto pipeline = std::views::iota(1, 20)
        | std::views::filter([](int n) { return n % 3 == 0; })
        | std::views::transform([](int n) { return n * 100; });
    std::vector<int> material;
    std::ranges::copy(pipeline, std::back_inserter(material));
    std::cout << "pipeline materializado: ";
    for (int x : material) std::cout << x << ' ';
    std::cout << '\n';

    // --- 3. front_inserter: el orden invertido que casi nadie espera ---------
    std::vector<int> llegan{1, 2, 3};
    std::deque<int> pila;
    std::copy(llegan.begin(), llegan.end(), std::front_inserter(pila));
    std::cout << "front_inserter copiando 1 2 3 da: ";
    for (int x : pila) std::cout << x << ' ';          // 3 2 1 (push_front)
    std::cout << '\n';

    // --- 4. inserter con list: insertar manteniendo orden --------------------
    std::list<int> orden{10, 30, 50};
    const std::vector<int> nuevos{5, 25, 45};
    // para cada nuevo, insertar en el lugar que le viene (positivo):
    for (int n : nuevos) {
        auto pos = std::lower_bound(orden.begin(), orden.end(), n);  // L09
        *std::inserter(orden, pos) = n;         // el `=` INSERTA en `pos`
    }
    std::cout << "list en orden: ";
    for (int x : orden) std::cout << x << ' ';
    std::cout << '\n';

    // --- 5. stream iterators: leer y escribir sin bucle ----------------------
    std::istringstream entrada("10 20 30 40");
    const std::istream_iterator<int> ini(entrada);
    const std::istream_iterator<int> fin;             // sin arg = fin de entrada
    std::vector<int> leidos;
    std::copy(ini, fin, std::back_inserter(leidos));  // parsing + copy en 1

    std::ostringstream salida;
    std::copy(leidos.begin(), leidos.end(),
              std::ostream_iterator<int>(salida, ", ")); // separador fijo
    std::cout << "leido '10 20 30 40' -> reproducido como: " << salida.str()
              << '\n';

    return 0;
}
