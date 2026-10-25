/* =============================================================================
   MÓDULO 10 - LECCIÓN 07: remove/erase idiom y erase_if
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       El algoritmo `std::remove(prin, fin, valor)` NO borra nada. Lo que
       hace es "compactar los que sobreviven": mueve los elementos que NO
       son ese valor al frente del rango y devuelve el iterador al nuevo
       fin LÓGICO. ¿Por qué no puede borrar de verdad? Porque es un
       algoritmo de rango: solo conoce iteradores, no al contenedor que
       contiene el rango; solo el contenedor puede cambiar su TAMAÑO
       (recordá v.size()).

       Por eso el idiom histórico de una línea:
           v.erase(std::remove(v.begin(), v.end(), 50), v.end());
       El rango [nuevo_fin, fin_viejo) queda lleno de "sobrantes sin
       valor" (típicamente dos copias del último elemento conservado: es el
       efecto visible del corrimiento); el erase de la cola es lo que
       restaura el tamaño real.

       C++20 lo simplifica: `std::erase(v, valor)` y `std::erase_if(v,
       pred)` son funciones libres EN el header del contenedor (acá llega
       `<vector>`): por conocerlo, sí cambian su tamaño y devuelven cuántos
       borraron. Es la forma moderna y preferida para vector/string/deque.

       Dos notas de familia:
         - `std::unique(prin, fin)`: tampoco borra. Deja una sola copia de
           cada grupo de adyacentes iguales al frente y el resto queda como
           valores sueltos: mismo idiom de arriba, unique + erase.
         - `std::list::remove/remove_if`: SÍ borran de verdad (list borra
           su nodo y su tamaño en el método): otra interfaz porque el
           método sí conoce el contenedor.
   2. ¿QUÉ PASA POR DEBAJO?
       remove es un bucle con dos punteros: uno LEE, otro ESCRIBE donde va
       el próximo sobreviviente. Todo lo que queda detrás en
       [nuevo_fin, fin) no guarda nada útil. Borrar con un bucle de
       v.erase(it) dentro del for es O(n^2) (cada erase corre a desplazar
       todos los siguientes, y además invalida el iterador: módulo 03);
       remove+erase completo es O(n).
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - Usar remove a secas y creer que se borró algo (el tamaño no
         cambia: son solo corrimientos lógicos).
       - Iterar y hacer v.erase(it) a mano dentro de un for: además de
         O(n^2), hay que armarse con `it = v.erase(it)` para no usar un
         iterador invalidado. erase_if mata este patrón completo.
       - unique sin erase al final: deja los únicos al frente y basura en
         la cola; va siempre acompañado del erase de la cola.
       - erase_if en list: ahí la list ya tiene sus métodos propios (list::remove/
         remove_if) que hacen todo en una llamada; para map/set, recién C++20
         trae erase_if en el header de cada contenedor asociativo.
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/algorithm/remove
       https://en.cppreference.com/w/cpp/container/vector/erase2
       https://en.cppreference.com/w/cpp/container/list/remove
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <list>
#include <string>
#include <vector>

int main() {
    // --- 1. remove a secas: NO cambia el tamaño ------------------------------
    std::vector<int> hp{100, 0, 80, 0, 45};
    const auto nuevo_fin = std::remove(hp.begin(), hp.end(), 0);
    std::cout << "despues de remove: ";
    for (int x : hp) std::cout << x << ' ';            // 100 80 45 + basura
    std::cout << "| size SI SIGUE " << hp.size() << '\n';
    hp.erase(nuevo_fin, hp.end());                     // ahora si se caga el final
    std::cout << "despues de erase: size " << hp.size() << " -> ";
    for (int x : hp) std::cout << x << ' ';
    std::cout << '\n';

    // --- 2. idiom completo en una linea (la forma clasica) -------------------
    std::vector<int> pts{10, 50, 10, 90, 10};
    pts.erase(std::remove(pts.begin(), pts.end(), 10), pts.end());
    std::cout << "pts sin los 10: ";
    for (int x : pts) std::cout << x << ' ';
    std::cout << '\n';

    // --- 3. C++20: erase / erase_if devolviendote cuantos --------------------
    std::vector<std::string> inventario{
        "pocion", "roto", "flecha", "roto", "pocion",
    };
    const std::size_t borrados = std::erase(inventario, "roto");
    std::cout << "erase borro " << borrados << " rotos; quedan: ";
    for (const std::string& s : inventario) std::cout << s << ' ';
    std::cout << '\n';

    std::vector<int> niveles{1, 45, 90, 8, 50, 203, 200};
    const int sin_maximos = std::erase_if(niveles,
        [](int n) { return n >= 200; });               // la lista queda real
    std::cout << "erase_if borro " << sin_maximos << "; niveles: ";
    for (int n : niveles) std::cout << n << ' ';
    std::cout << '\n';

    // --- 4. unique: mismo idiom (tambien devuelve el nuevo fin) --------------
    std::vector<int> bufazo{1, 1, 2, 3, 3, 3, 4};
    bufazo.erase(std::unique(bufazo.begin(), bufazo.end()), bufazo.end());
    std::cout << "sin adyacentes repetidos: ";
    for (int x : bufazo) std::cout << x << ' ';
    std::cout << '\n';

    // y en la list, la version que si borra de una vez:
    std::list<int> litros{7, 7, 5};
    litros.remove(7);                                    // metodo de list
    std::cout << "list despues de remove: size " << litros.size() << '\n';

    return 0;
}
