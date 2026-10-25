/* =============================================================================
   MÓDULO 11 - LECCIÓN 09: adaptadores: std::stack y std::queue
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
        Los contenedores REALES (vector, deque, list) exponen toda su
        superficie: indexás, insertás al medio, borrás por valor. Los
        ADAPTADORES hacen lo contrario: son una FUNDA sobre otro
        contenedor que expone SOLO las operaciones de un modelo de datos
        conocido (y deja el resto afuera, hasta para tu propio bien):

          - std::stack<T> (pila): entra y sale por UN solo lado (top); se
            saca el ÚLTIMO que entró (LIFO). push/pop/top/empty/size. Nada más.
          - std::queue<T> (cola): entra por UN lado (back), sale por el
            OTRO (front). FIFO. push/pop/front/back/empty/size.

        ¿Y para qué limitarse? Porque el MODELO es la API: el que lee
        "std::stack<int>" ya sabe el comportamiento sin leer tu código, y
        el compilador te impide borrar al medio "porque quedó cómodo". Es
        la misma filosofía de módulos previos: la restricción es diseño.

        Usos canónicos:
          - stack: deshacer/rehacer (undo), validación de paréntesis,
            DFS/ backtracking, sintaxis analizada.
          - queue: colas de eventos/procesos "en orden de llegada",
            BFS con productor-consumidor básico.

        La funda por defecto: stack y queue usan std::deque de fondo
        (porque un deque es O(1) en AMBOS bordes, justo lo que los dos
        modelos piden). Podés elegir otro fondo: std::stack<T,
        std::vector<T>> compila perfecto y en muchos casos anda
        más rápido (no hay recorrido: solo bordes).
   2. ¿QUÉ PASA POR DEBAJO?
        Los adaptadores NO SON CONTENEDORES: no tienen begin()/end(),
        no hay iteradores, no hay range-for. No los pasás a sort ni a
        find: los recorrés vaciándolos (pop repetidos) o usás el
        contenedor de fondo si realmente necesitás iterar (y ya estás
        pisando el diseño del adaptador: pensá si no querías deque).

        pop() NO devuelve el elemento: primero mirás con top()/front()
        y DESUÉS pop(). ¿Por qué? Porque cuando la copia (o el move)
        del elemento BALANEA (lanza), la pila habría perdido el
        elemento en vano... y el adaptador pretende ser seguro ante
        excepciones que salen de tu T.

        Estructura interna del deque (bonus): bloques fijos con un mapa
        central de punteros. Es por eso que puede hacer push_front y
        push_back en O(1) sin mover todos los datos, a diferencia del
        vector (solo O(1) amortizado al final).
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
        - top() en una pila VACÍA: comportamiento indefinido, sin aviso.
          Chequeá empty() antes (o hacé que el diseño lo excluya).
        - pop() esperando el elemento: "int x = s.pop();" ni compila.
        - Recorrer stack/queue con for y descubrir que no hay begin().
        - stack<int, std::vector<int>> para millones de elementos con
          push/pop alternado: el vector de fondo realoca al crecer
          (el deque no: por eso es el default).
        - "stack of unique_ptr": el push de un move-only requiere mover:
          es el patrón de composición del módulo 09 (no de la L10).
   4. PARA PROFUNDIZAR:
        https://en.cppreference.com/w/cpp/container/stack
        https://en.cppreference.com/w/cpp/container/queue
        https://en.cppreference.com/w/cpp/container/deque
   ============================================================================= */
#include <iostream>
#include <queue>
#include <stack>
#include <string>
#include <vector>

int main() {
    // --- 1. stack: LIFO con top/push/pop --------------------------------------------
    std::stack<std::string> deshacer;
    deshacer.push("escribi 'A'");
    deshacer.push("escribi 'B'");
    deshacer.push("borre 'A'");

    std::cout << "undo (del mas reciente al mas viejo):\n";
    while (!deshacer.empty()) {
        std::cout << "  " << deshacer.top() << '\n';
        deshacer.pop();
    }

    // --- 2. Detalle top/pop: el elemento sale en dos pasos ---------------------------
    std::stack<int> pila;
    pila.push(7);
    pila.push(3);
    int ultimo = pila.top();   // primero mirás
    pila.pop();                // después desapilás
    std::cout << "\ntop devolvio " << ultimo << " y la pila ahora: " << pila.top()
              << " (size " << pila.size() << ")\n";

    // --- 3. queue: FIFO con front/back -----------------------------------------------
    std::queue<std::string> mensajes;
    mensajes.push("hola");
    mensajes.push("tengo un problema");
    mensajes.push("listo, reimprimo desde tu lado");

    std::cout << "\ncola de mensajes (orden de llegada):\n";
    while (!mensajes.empty()) {
        std::cout << "  " << mensajes.front() << '\n';
        mensajes.pop();
    }

    // --- 4. La funda se puede elegir: stack sobre vector ------------------------------
    std::stack<int, std::vector<int>> pilaVec;
    pilaVec.push(1);
    pilaVec.push(2);
    std::cout << "\nstack sobre vector: top = " << pilaVec.top() << '\n';
    return 0;
}
