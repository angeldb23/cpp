/* =============================================================================
   MÓDULO 08 - LECCIÓN 01: Valor-categorías: lvalues y rvalues
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Antes de mover nada tenemos que responder una pregunta más básica:
      ¿QUÉ es exactamente aquello que estamos moviendo? En C++ cada
      EXPRESIÓN (no cada variable: cada expresión) tiene un tipo Y una
      categoría de valor. Las dos grandes son:

        - lvalue ("locator value"): tiene DIRECCIÓN. "Va a seguir
          existiendo después de esta línea". `x`, `v[0]`, `*p`,
          `puntaje`... Podés preguntarle "¿dónde vivís?" (con `&`) y
          contesta.
        - rvalue ("read value"): es un VALOR que está por consumirse.
          `42`, `x + y`, `nombre + "!"`, el temporal que devuelve una
          función por valor... No tiene identidad ni permanencia: va a
          desaparecer al terminar la sentencia.

      ¿Importa tanto así? MUCHO, y es la base de TODO este módulo:
        - Copiar un lvalue implica copiar su contenido (caro: n datos).
        - Un rvalue ya está muriendo de todas formas: agarrar sus
          internos y "robarlos" es O(1), y no rompe nada, porque nadie
          va a volver a usarlos. Nadie podría: no tienen identidad.

      Reglas prácticas para clasificar (sin teoría):
        1) ¿Es una variable, un miembro, v[i], *p, un resultado de
           referencia? -> lvalue.
        2) ¿Es un literal (`42`, `"hola"`, `3.14f`)? -> rvalue: no hay
           objeto, hay un valor.
        3) ¿Es una operación que CALCULA un resultado nuevo (`x + y`,
           `a && b`, `s1 + s2`)? -> rvalue: el valor es temporal.
        4) ¿Es una llamada que devuelve por valor? -> rvalue: el
           temporal retornado.
        5) Si dudás: probá `&expresion`. Compila -> lvalue. No
           compila -> rvalue. Test infalible para principiantes.

      Y una advertencia para deshacer un mal hábito: rvalue NO significa
      "solo lectura". Sobre un temporal podés llamar métodos que mutan:
        std::string("abc").clear();   // legal: muta un temporal
      "rvalue" significa "sin identidad ni permanencia", no "constante".

    2. ¿QUÉ PASA POR DEBAJO?
      La categoría de valor es una propiedad de la EXPRESIÓN, no del
      TIPO: `int x;` y `int f();` califican ambos como `int`, pero `x`
      es lvalue y `f()` es rvalue. El tipo no alcanza: el compilador
      lleva la categoría durante el análisis del código y la usa para
      DOS cosas, exactamente las que te llevan este módulo:
        1. elegir sobrecarga (¿`const T&` o `T&&`?: L02), y
        2. decidir que un rvalue es "movible" (L03 en adelante).
      A nivel binario la categoría ya no existe: en el código final un
      lvalue se materializa en una dirección (memoria) y un rvalue
      (típicamente) en un registro o un temporal en el stack. Por eso
      mover un rvalue es la jugada perfecta: "quitarle" el buffer a un
      temporal es EXACTAMENTE lo mismo en machine code que renombrar.

      Nota: C++ además parte las categorías finas (xvalue, prvalue, y
      las familias glvalue/rvalue). Para trabajar damos con las dos
      grandes; los detalles finos entran con las referencias rvalue en
      la L02, con el vocabulario justo que NOS SIRVE PARA mover.

    3. ERRORES COMUNES
      - Pensar lvalue = "está a la izquierda del =" y rvalue = "a la
        derecha". FALSA coincidencia del nombre: en `x = y;` los dos son
        lvalue, en `v[0] = 42;` hay lvalue a la izquierda y rvalue a la
        derecha, y en `x = v[0];` hay lvalue a la DERECHA. La regla
        confiable sigue siendo la 5: preguntarle con `&`.
      - Asumir que un temporal no puede mutar: puede (métodos sobre
        temporales). Lo que no puede es tener identidad y asignarse.
      - Clasificar por intuición en vez de las 5 reglas: siempre anda
        la 5, probá `&`.

    4. DEMO: clasificar en vivo con `&` y con el comportamiento
   ============================================================================
 */
#include <iostream>
#include <string>
#include <vector>

int suma(int a, int b) { return a + b; }                       // rvalue
std::string concatenar(const std::string& a, const std::string& b) {
    return a + b;                                              // rvalue
}

int main() {
    std::vector<int> notas = {7, 8, 9};
    int x = 10;
    int y = 3;

    std::cout << "== Clasificación en vivo ==\n";

    // Regla 1: variable -> lvalue
    int* px = &x;                    // compila: x tiene dirección
    std::cout << "  x            -> lvalue  (direccion: " << px << ")\n";

    // Regla 1: elemento de vector -> lvalue
    notas[1] = 99;                   // hay una casilla REAL asignable
    int* pi = &notas[1];
    std::cout << "  notas[1]     -> lvalue  (direccion: " << pi
              << ", valor: " << notas[1] << ")\n";

    // Regla 2: literal -> rvalue
    // int* pq = &42;               // NO compila: 42 es rvalue (quitá el
    std::cout << "  42           -> rvalue  (no hay objeto: valor literal)\n";
    //                               comentario de arriba para ver el error)

    // Regla 3: operación -> rvalue
    // int* pr = &(x + y);          // NO compila: x + y es un temporal
    std::cout << "  x + y        -> rvalue  (=" << x + y << ")\n";

    // Regla 4: llamada que devuelve por valor -> rvalue
    std::cout << "  concatenar() -> rvalue  (= \"" << concatenar("ab", "cd") << "\")\n";

    // Un temporal igual puede llamarse a métodos (mata el mito "solo lectura"):
    concatenar("hola", " mundo").append("!");
    std::cout << "  temporal.append() en un temporal: legal (va a morir igual)\n";

    // La izquierda también puede ser lvalue:
    x += suma(1, 1);                 // x: lvalue a la izquierda
    std::cout << "  x += suma(1,1) -> x queda en " << x << " (lvalue + rvalue)\n";
    return 0;
}
