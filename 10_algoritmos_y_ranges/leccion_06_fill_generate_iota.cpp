/* =============================================================================
   MÓDULO 10 - LECCIÓN 06: modificantes II: fill, generate, iota y replace
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       La otra cara de los modificantes: EN VEZ de transformar lo que ya hay,
       los algoritmos de hoy escribe/reescribe lugares completos:
         - `std::fill(prin, fin, valor)` / `std::fill_n(prin, n, valor)`:
           pone el mismo en TODOS los lugares. Casos de uso: resetear
           buffers, "curar más" los HP, vaciar lógicamente un tablero.
         - `std::generate(prin, fin, generador)` / `generate_n`: para cada
           lugar invoca el generador (un callable SIN parámetros) y guarda
           lo que devuelve. La diferencia con fill: el valor cambia por
           llamada (un contador, un random, el next() de otra fuente). El
           generador puede ser un lambda `mutable` (su estado interno avanza
           a cada llamada; mutable se ve a fondo en la L12).
         - `std::iota(prin, fin, valor_inicial)`: escribe valor_inicial,
           valor_inicial+1, valor_inicial+2... uno más por posición. Es el
           generate más barato: sirve para IDs consecutivos y para
           preparar la secuencia de índices.
         - `std::replace(prin, fin, viejo, nuevo)` / `replace_if(..., pred,
           nuevo)`: donde lo que hay calza (es igual, o cumple pred),
           escriba el nuevo valor. Es transform "con condición": no se
           deriva del valor existente, el nuevo es uno fijo.

       Cuidado con las cuatro: TODAS escriben sobre el rango tal como está;
       si esperás que agranden el contenedor, no: como en la L05, hay que
       asegurar el espacio antes.
   2. ¿QUÉ PASA POR DEBAJO?
       fill puede bajar a un memset cuando el tipo es simple y el valor es
       "todos iguales". generate es una llamada del generador por posición,
       ejecutadas en orden de izquierda a derecha: no cuentes con
       paralelismo automático en std::generate (eso es explícito, con las
       políticas de ejecución que se ven en el módulo 14; la STL clásica es
       secuencial).
       iota es exactamente ese bucle: `*prin = value; ++value;` en cada
       vuelta, sin llamadas a funciones: el generate más barato cuando el
       paso es +1.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - Esperar que fill/generate/iota agranden el vector: no lo hacen;
         primero resize o construilos con el tamaño correcto.
       - Generadores que capturan por referencia variables mutadas por
         alguien más: llevalos mutable y con estado propio (como el ejemplo)
         para que su lógica quede encerrada y testeable.
       - replace_if para "borrar" (pasarle un nuevo valioso/no válido no
         borra nada: el elemento SIGUE en el vector): para borrar está la
         técnica de la L07 (remove/erase). replace solo reemplaza valores.
       - iota solo aplica a tipos que soportan `+1` (numéricos); para
         cualquier otra cosa, generate o un bucle.
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/algorithm/fill
       https://en.cppreference.com/w/cpp/algorithm/generate
       https://en.cppreference.com/w/cpp/algorithm/iota
       https://en.cppreference.com/w/cpp/algorithm/replace
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

int main() {
    // --- 1. fill: resetear con un solo algoritmo -----------------------------
    std::vector<int> muni{5, 3, 2, 2, 2};                       // munición
    std::fill(muni.begin(), muni.end(), 30);                    // recarga
    std::fill_n(muni.begin(), 2, 0);                            // las dos cañón a 0
    std::cout << "municion: ";
    for (int m : muni) std::cout << m << ' ';
    std::cout << '\n';

    // --- 2. generate: cada lugar obtiene su propia respuesta -----------------
    std::vector<int> hp_enemigos(5);
    int tanda = 1;                            // el estado que muta la lambda
    std::generate(hp_enemigos.begin(), hp_enemigos.end(),
                  [&tanda]() mutable {
                      return tanda++ * 25;             // 25, 50, 75, 100, 125
                  });
    std::cout << "hp por tanda: ";
    for (int hp : hp_enemigos) std::cout << hp << ' ';
    std::cout << " | tanda siguiente: " << tanda << '\n';

    // --- 3. iota: IDs consecutivos, el caso clásico --------------------------
    std::vector<int> ids(7);
    std::iota(ids.begin(), ids.end(), 100);                     // 100..106
    std::cout << "ids: ";
    for (int i : ids) std::cout << i << ' ';
    std::cout << '\n';

    // --- 4. replace / replace_if: donde ya está malo, corregir ---------------
    std::vector<int> hp_jugadores{100, 0, 85, 0};               // 0 = caído
    std::replace(hp_jugadores.begin(), hp_jugadores.end(), 0, 50);   // revivir
    std::replace_if(hp_jugadores.begin(), hp_jugadores.end(),
                    [](int hp) { return hp > 90; }, 90);        // capar al tope
    std::cout << "hp post reglas: ";
    for (int hp : hp_jugadores) std::cout << hp << ' ';
    std::cout << '\n';

    return 0;
}
