/* =============================================================================
   MÓDULO 10 - LECCIÓN 12: lambdas a fondo: capturas, mutable e init-capture
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       El módulo 04 te dio la sintaxis mínimo `[](params){...}`. Esta
       lección es la GUÍA de la lista de captura `[...]`: qué entra al cuerpo
       de la lambda y cómo.

       Modos de captura:
         - `[x]`: COPIA de x, hecha cuando la lambda SE CREA. Los cambios
           posteriores de la variable original no afectan la copia.
         - `[&x]`: REFERENCIA a la x. Al llamar usa la variable ACTUAL.
         - `[x, &y]` lista explícita; `[&]` (todo por referencia) y `[=]`
           (todo por copia) son la forma wholesale: usalas SOLO cuando la
           vida de la lambda es corta y visible (adentro de un for_each o
           un sort, sin guardarla).
         - `[this]` (o `[*this]`, C++17): adentro de un método captura el
           puntero this: los miembros se acceden directo. ATENCION: es
           puntero: si el objeto murió antes de la llamada, la lambda
           cuelga (igual que las trampas de referencias del módulo 04).
           `[*this]` en cambio copia el OBJETO entero.

       `mutable`: por defecto el operator() de una lambda es const: los
       cambios de las capturas POR VALOR no están permitidos. `mutable` le
       saca el const: la copia guardada PUEDE mutarse en cada llamada (y
       vive mientras viva el OBJETO lambda, no mientras dura la llamada).
       El contador `[cnt]() mutable { ++cnt; }`: cada llamada del MISMO
       objeto lambda incrementa SU copia guardada; si la lambda se copia,
       cada copia lleva su contador desde cero.

       Init-capture (C++14) `[nombre = expr]`: declara un NUEVO miembro con
       ese nombre, inicializado con la expr. Dos usos que valen oro:
         - MOVER: `[v = std::move(v)]` hace a la lambda ÚNICA dueña de los
           datos (un vector grande, un unique_ptr del módulo 07) sin copia;
           con eso la lambda puede GUARDARSE (por ejemplo en un
           std::function del módulo 09).
         - CALCULAR: `[ahora = std::chrono::steady_clock::now()]`,
           `[[cuantos = pool.size()]]`: el valor nace CON LA LAMBDA.
       Nota: la referencia a rvalue `[&v = std::move(v)]` también existe
       pero cae en territorio dangling si la lambda sobrevive al dueño.
   2. ¿QUÉ PASA POR DEBAJO?
       Una lambda es UNA CLASE anónima generada por el compilador: sus
       capturas son MIEMBROS de esa clase (por valor guarda la copia, por
       referencia guarda el puntero) y el cuerpo es el operator() de la
       clase (const por defecto; mutable lo saca). Por eso una lambda CON
       capturas ya no se convierte gratis en puntero a función (la sin
       captura sí, como mostró el módulo 04) y por eso capturar tiene un
       costo de tamaño (el objeto lambda pesa lo que sus capturas) y de
       copias (copiar una lambda CON capturas por valor copia esas capturas).
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - Devolver/se almacenar una lambda que captura por REFERENCIA la
  
       
       - Las capturas por valor guardadas MUTAN solo con mutable: sin
         mutable el operator() es const y el compilador rechaza el ++.
       - Guardar una lambda con [this] en un lugar que sobrevive al
         objeto: la trampa clásica de callbacks (la vas a reencontrar siempre
         que lees sobre callbacks de APIs).
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/language/lambda
       https://en.cppreference.com/w/cpp/language/lambda#Capture
   ============================================================================= */
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// --- 1. Los dos modos base: copia y referencia ------------------------------
int main() {
    int base = 10;
    int extra = 5;

    auto porValor = [base](int x) { return x + base; };       // copia de base
    auto porRef = [&extra](int x) { return x + extra; };      // ref de extra

    std::cout << "porValor(3) = " << porValor(3) << '\n';

    extra += 100;                    // la variable ACTUAL cambiando:
    std::cout << "porRef(3) = " << porRef(3) << " (uso el extra actual)\n";
    std::cout << "porValor(3) sigue = " << porValor(3)
              << " (la copia no cambia)\n";

    // --- 2. mutable: el contador DUEÑO de su copia ---------------------------
    auto contador = [n = 0]() mutable { return ++n; };   // init-capture + mut
    contador(); contador();
    std::cout << "contador propio: " << contador() << " llamadas\n";

    // CADA lambda tiene su estado: una copia empieza de cero:
    auto copia = contador;
    std::cout << "copia nueva arranca en " << copia() << '\n';

    // --- 3. init-capture con move: la lambda es LA DUEÑA ---------------------
    std::vector<std::string> cargas{"bomba", "trampa"};
    auto almacen = [guardadas = std::move(cargas)]() {
        return guardadas.size();
    };
    std::cout << "almacen guarda " << almacen() << " cargas\n";
    std::cout << "el vector original quedo vacio? " << cargas.empty() << '\n';

    // init-capture con unique_ptr (modulo 07): el puntero MIGRA a la lambda:
    auto ficha = std::make_unique<int>(42);
    auto tarea = [p = std::move(ficha)]() { return *p; };
    std::cout << "la lambda es la unica dueña de la ficha: " << tarea() << '\n';
    std::cout << "ficha original quedo nula? " << (ficha == nullptr) << '\n';

    // --- 4. capturar everything explicito vs wholesale ----------------------
    int a = 1, b = 2;
    auto completo = [&a, &b]() { return a + b; };    // lista explicita: se LEE
    auto deMayor = [&]() { return a + b; };          // wholesale: para call corto
    a = 100;
    std::cout << "explicito: " << completo() << " | el [&]: "
              << deMayor() << '\n';

    return 0;
}
