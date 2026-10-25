/* =============================================================================
   MÓDULO 10 - LECCIÓN 04: preguntas all_of/any_of/none_of y acciones con for_each
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Para las preguntas "¿todos?", "¿alguno?" y "¿ninguno?" sobre una
       secuencia hay tres algoritmos que son sus nombres:
         - `std::all_of(prin, fin, pred)`: true si cumple TODOS.
         - `std::any_of(...)`: true si cumple AL MENOS UNO.
         - `std::none_of(...)`: true si NO cumple NINGUNO.
       Dato que sorprende la primera vez: sobre un rango VACÍO, all_of y
       none_of devuelven true y any_of devuelve false. No es un bug: es la
       convención matemática (el "para todo" del conjunto vacío no tiene
       contraejemplo). Si en tu juego "todo el equipo está vivo" con un equipo
       vacío es raro, la validación del vacío es TUYA: decidí si chequeás
       antes tamaño o si la convención te sirve.

       `std::for_each(prin, fin, accion)`: aplica una acción a cada elemento.
       Con un lambda que captura por REFERENCIA podés acumular estado (sumar,
       contar, guardar el máximo) sin crear contenedores intermedios ni
       bucles con bandera. Y tiene un detalle que lo distingue del range-for:
       DEVUELVE el functor que recibió. Eso sirve cuando la acción venía por
       valor (un functor con estado interno): el retorno te devuelve ese
       estado acumulado, como muestra el ejemplo 3 de abajo.

       Continuidad: los algoritmos toman cualquier callable y, como son
       templates (módulo 09), el compilador conoce el tipo concreto del
       predicado y lo puede meter inline: no hay costo extra respecto de un
       bucle a mano.
   2. ¿QUÉ PASA POR DEBAJO?
       all/any/none tienen salida temprana: any_of corta apenas encuentra uno
       que cumple; all_of/none_of cortan apenas ven un contraejemplo. Peor
       caso igual O(n), pero dejan de mirar antes. `for_each` NO corta:
       siempre recorre todo lo que le diste. Si tu lógica necesita salir
       apenas algo pasa, no es for_each: es find_if (L03) o un bucle a mano.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - Escribir un range-for con un bool y un break para responder
         "¿alguno...?": eso ya existe y se llama any_of (y el nombre dice lo
         que hace).
       - Capturar por referencia de un SCOPE que muere antes que la lambda: si
         el for_each guarda/entrega el callable y vos seguís usándolo después,
         con capturas [ & ] de variables locales es colgante (la L12 ve la
         trampa a fondo).
       - Capturá solo lo que usás ([&suma]) en vez de [&] a ciegas: el lector
         ve exactamente qué cambia.
       - Un lambda vacío `[](const Enemigo&) {}` no avisa nada: cada predicado
         debería devolver algo; en all/any/none el bool es la respuesta.
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/algorithm/all_any_none_of
       https://en.cppreference.com/w/cpp/algorithm/for_each
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

struct Enemigo {
    std::string nombre;
    int hp;
    bool envenenado;
};

int main() {
    std::vector<Enemigo> oleada{
        {"Blob", 30, false}, {"Vespa", 45, true}, {"Murk", 10, true},
    };

    // --- 1. all_of / any_of / none_of ----------------------------------------
    const bool todosVivos = std::all_of(oleada.begin(), oleada.end(),
        [](const Enemigo& e) { return e.hp > 0; });
    std::cout << "todos vivos? " << (todosVivos ? "si" : "no") << '\n';

    if (std::any_of(oleada.begin(), oleada.end(),
            [](const Enemigo& e) { return e.envenenado; })) {
        std::cout << "hay envenenado: activo antitoxina\n";
    }

    if (std::none_of(oleada.begin(), oleada.end(),
            [](const Enemigo& e) { return e.hp < 15; })) {
        std::cout << "ninguno casi muerto\n";
    }

    // la convención con el rango vacío:
    std::vector<Enemigo> nadie;
    std::cout << "vacio -> all_of: " << std::all_of(nadie.begin(), nadie.end(),
                      [](const Enemigo& e) { return e.hp > 0; })
              << " | any_of: " << std::any_of(nadie.begin(), nadie.end(),
                      [](const Enemigo& e) { return e.hp > 0; }) << '\n';

    // --- 2. for_each con estado por referencia -------------------------------
    // Las variables capturadas por referencia quedan ACTUALIZADAS al salir.
    int suma_hp = 0;
    int mayor_hp = 0;
    std::for_each(oleada.begin(), oleada.end(), [&](const Enemigo& e) {
        suma_hp += e.hp;
        if (e.hp > mayor_hp) mayor_hp = e.hp;
    });
    std::cout << "hp total: " << suma_hp << " | mayor: " << mayor_hp << '\n';

    // --- 3. for_each devuelve el functor: la variante por valor --------------
    // Si la acción venía POR VALOR con estado interno, for_each devuelve ese
    // functor (movido) con el estado ya acumulado:
    struct ContadorVeneno {
        int n = 0;
        void operator()(const Enemigo& e) { if (e.envenenado) ++n; }
    };
    const auto contador = std::for_each(oleada.begin(), oleada.end(),
                                        ContadorVeneno{});
    std::cout << "envenenados: " << contador.n << '\n';

    return 0;
}
