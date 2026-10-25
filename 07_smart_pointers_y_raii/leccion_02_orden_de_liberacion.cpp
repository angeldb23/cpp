/* =============================================================================
   MÓDULO 07 - LECCIÓN 02: Orden de liberación: locales en reversa y miembros
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      RAII rinde cuando sabés EXACTAMENTE en qué orden se destruye todo. Dos
      reglas, ambas automáticas:

      a) Dentro de una función, los locales se destruyen en orden INVERSO al
         de su construcción. Último creado, primero muerto.

      b) Dentro de un objeto compuesto:
         - los MIEMBROS se destruyen en orden inverso al de su DECLARACIÓN
           en la clase (la lista de inicialización NO manda acá);
         - el CUERPO del destructor corre ANTES de que se destruyan los
           miembros: por eso dentro del cuerpo todavía podés usar recursos.

      De (a) y (b) salen dos patrones de diseño directos:

        1) Composición con miembros RAII: una clase que "tiene" recursos los
           declara como miembros RAII y NO escribe destructor: regla de 0
           (módulo 05). El destructor generado por el compilador llama al
           destructor de cada miembro en reversa, gratis.

        2) Si un miembro B usa a otro miembro A en el destructor (por
           ejemplo, liberar con una API que A administra), B se declara
           ANTES que A en la clase: como la destrucción es en reversa, A
           muere al final, con lo que B todavía puede usarlo al destruirse.

      c) Espejo perfecto: los miembros se CONSTRUYEN en orden de declaración
         y se DESTRUYEN en reversa. El mismo orden, espejado. Y lo mismo
         hacía la herencia en el módulo 06: base primero (construcción) /
         base al final (destrucción).

   2. ¿QUÉ PASA POR DEBAJO?
      La destrucción de un objeto compuesto NO es una tabla en runtime: el
      compilador "hornea" las llamadas dentro del destructor generado:
        ~Mundo -> cuerpo que escribiste -> ~shaders_ -> ~buffer_ -> ~texturas_
      Cada destructor sabe estáticameNTE qué miembros destruir, así que son
      llamadas directas, en línea, sin costo. Por eso RAII por composición es
      tan barato como escribir la limpieza a mano, pero imposible de olvidar.

   3. ERRORES COMUNES
      - Creer que la LISTA DE INICIALIZACIÓN define el orden de destrucción:
        no define ni siquiera el orden de construcción (eso es el orden de
        declaración; módulo 05). Declaración manda en ambos sentidos.
      - Depender en un destructor de un miembro declarado DESPUÉS: al
        destruirse, ese otro ya murió. Es un bug silencioso que solo explota
        si el recurso depende del otro.
      - Creer que el cuerpo del destructor corre DESPUÉS de liberar miembros:
        corre primero. El "ya me fui" de un dtor con cuerpo sigue con acceso
        a sus miembros.
      - Recursos con dependencia CRUZADA (A usa B y B usa A para liberarse):
        no hay orden de declaración que salve un ciclo; es diseño roto.

   4. DEMO: el orden impreso paso a paso
 */
#include <cstddef>
#include <iostream>

// Wrapper RAII con nombre para ver el orden en la consola.
class Pieza {
private:
    const char* nombre_;

public:
    explicit Pieza(const char* nombre) : nombre_(nombre) {
        std::cout << "  [ctor] " << nombre_ << "\n";
    }
    ~Pieza() { std::cout << "  [dtor] " << nombre_ << "\n"; }
    Pieza(const Pieza&)            = delete;
    Pieza& operator=(const Pieza&) = delete;
};

// ---------------------------------------------------------------- demo 1
class Mundo {
    // Orden de DECLARACIÓN = orden de construcción. Destrucción: reversa.
    Pieza texturas_ {"miembro texturas_ (declarado 1ro)"};   // muere ÚLTIMO
    Pieza buffer_   {"miembro buffer_   (declarado 2do)"};
    Pieza shaders_  {"miembro shaders_  (declarado 3ro)"};   // muere PRIMERO

public:
    Mundo() {
        std::cout << "  [ctor] Mundo: cuerpo (después de los 3 miembros)\n";
    }

    ~Mundo() {
        // El cuerpo corre PRIMERO: los miembros siguen vivos acá.
        std::cout << "  [dtor] Mundo: cuerpo (aún puedo usar mis miembros)\n";
        std::cout << "  [dtor] Mundo: ahora sí, mueren los miembros (reversa)\n";
    }
};

// ---------------------------------------------------------------- demo 2
// Clase compuesta SOLO de miembros RAII: regla de 0, cero delete en el
// archivo entero y cero destructor escrito a mano. Las demos 1 y 2 ya
// mostraron el patrón; esta clase existe solo para nombrarlo.
class DemoRegla0 {};

int main() {
    std::cout << "=== 1. LOCALES EN REVERSA ===\n";
    {
        Pieza a {"local A (creado 1ro)"};
        Pieza b {"local B (creado 2do)"};
        Pieza c {"local C (creado 3ro)"};
    }   // imprime: dtor C, dtor B, dtor A

    std::cout << "\n=== 2. MIEMBROS EN REVERSA + CUERPO PRIMERO ===\n";
    { Mundo m; }    // cuerpo dtor Mundo -> shaders_ -> buffer_ -> texturas_

    std::cout << "\n=== 3. ÁMBITOS ANIDADOS: CADA LLAVE ES UNA CAPA ===\n";
    {
        Pieza nivel1 {"nivel1"};
        {
            Pieza nivel2 {"nivel2"};
            std::cout << "  (trabajando con nivel2...)\n";
        }               // nivel2 muere acá
        std::cout << "  (nivel1 sigue vivo)\n";
    }                   // nivel1 muere recién acá

    std::cout << "\n=== 4. JUNTO A LA HERENCIA (módulo 06) ===\n";
    std::cout << "  construcción: base -> miembros (declaración) -> cuerpo\n";
    std::cout << "  destrucción : cuerpo -> miembros (reversa) -> base\n";
    std::cout << "  El mismo orden, espejado.\n";
    return 0;
}
