/* =============================================================================
   MÓDULO 10 - LECCIÓN 17: algoritmos ranges: proyecciones y ranges::sort
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Así como hay std::sort, en C++20 hay std::ranges::sort. Los
       algoritmos ranges hacen tres cosas nuevas:
         1. Aceptan el CONTENEDOR DIRECTO (ranges::sort(v)) en vez del
            par begin/end: menos ruido e imposible mandar por accidente
            iteradores de dos contenedores distintos (el error clásico
            de la firma clásica).
         2. Tienen el SENTINEL incluido: funciones como ranges::copy un
            solo argumento (ya lo viste en L14 con el view de sentinel).
         3. La PROYECCIÓN: un argumento extra de cada algoritmo ranges?
            No: el tercer argumento opcional es un callable (típicamente
            un puntero a miembro o lambda) que dice por DÓNDE mirar cada
            elemento antes de aplicar el criterio:
                std::ranges::sort(tabla, {}, &Entrada::puntaje);
            "<<{}, &Entrada::puntaje>" se lee: "usar el orden default (<),
            mirar cada Entrada a través del campo puntaje"; -> el sort
            queda ordenado por puntaje, SIN lambda comparing.
            Eso es la proyección a mano de la L13 integrada en la API: no hace falta lambda comparing: la función trae el unwrap.
       Y la proyección sirve igual para find/count/minmax:
           ranges::max_element(equipo, {}, &Fighter::hp)
           ranges::find_if(v, pred, &Fighter::nombre)   // pred sobre nombre
       El comparador y la proyección CONVIVEN: ranges::sort(tabla, comp,
       proyección): el comp recibe los valores YA proyectados.
       Y todo ranges::... acepta el RANGE a secas: un contenedor o una
       cadena de views (L14/L15): la misma API para datos y recetas.
   2. ¿QUÉ PASA POR DEBAJO?
       `&Entrada::puntaje` es un puntero a miembro con operator() INCLUIDO
       (al invocarlo con una Entrada devuelve el campo). Dentro,
       ranges::sort invoca la proyección en cada elemento ANTES de pasar
       el valor al comparador (via std::invoke: la misma maquinaria
       acepta puntero a miembro, getter o lambda). Lo único exigido es
       que lo proyectado sea comparable con el criterio elegido.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
        - Confundir los PARAMS de ranges::sort: ranges::sort(v, comp,
          proyeccion): el comp PRIMERO, la proyección DESPUÉS (opuesto
          el natural de la inversión; el default `{}` es menos::<:>).
        - Con un lambda deja de map: en puntos de::*; la lambda directo
          `[](const Entrada& e) { return e.puntaje; }` también es
      (y también sirve un getter con operator()).
        - Ordenar por UN campo con proyección; el multi-criterio sigue
          comp + std::tie (L08).
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/algorithm/ranges/sort
       https://en.cppreference.com/w/cpp/algorithm/ranges/find
       https://en.cppreference.com/w/cpp/utility/functional/invoke
   ============================================================================= */
#include <algorithm>
#include <functional>
#include <iostream>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>

struct Fighter {
    std::string nombre;
    int hp;
    int nivel;
};

int main() {
    std::vector<Fighter> roster{
        {"Quesito", 90, 3}, {"Mordel", 60, 7}, {"Turbo", 120, 2},
    };

    // --- 1. ranges::sort con proyección por campo ----------------------------
    std::ranges::sort(roster, {}, &Fighter::hp);            // por hp, < default
    std::cout << "por hp: ";
    for (const Fighter& f : roster)
        std::cout << f.nombre << '=' << f.hp << ' ';
    std::cout << '\n';

    // --- 2. proyección + comparador: por hp DESC con comp -------------------
    std::ranges::sort(roster, std::greater<int>{}, &Fighter::hp);
    std::cout << "por hp desc: ";
    for (const Fighter& f : roster)
        std::cout << f.nombre << '=' << f.hp << ' ';
    std::cout << '\n';

    // --- 3. el comp completo con std::tie ya funcionan igual (L08) ----------
    std::ranges::sort(roster,
        [](const Fighter& a, const Fighter& b) {
            return std::tie(a.nivel, a.nombre) <
                   std::tie(b.nivel, b.nombre);
        });
    std::cout << "nivel asc + nombre: ";
    for (const Fighter& f : roster)
        std::cout << f.nombre << ' ';
    std::cout << '\n';

    // --- 4. la proyección en OTROS algoritmos ranges -------------------------
    std::ranges::sort(roster, {}, &Fighter::nivel);         // de nuevo: nivel asc
    const auto mejorNivel = std::ranges::max_element(roster, {}, &Fighter::nivel);
    std::cout << "el mas entrenado es "
              << mejorNivel->nombre << " (nivel " << mejorNivel->nivel << ")\n";

    // find_if con proyección: pred sobre el CAMPO
    const auto busque = std::ranges::find_if(roster,
        [](const std::string& n) { return n == "Turbo"; },
        &Fighter::nombre);                  // el pred VE el nombre, no el Fighter
    std::cout << "find_if con proyeccion: " << (busque != roster.end()
        ? busque->nombre : "no esta") << '\n';

    return 0;
}
#include <tuple>   // (detalles: el tie ya lo usa; include arriba)
