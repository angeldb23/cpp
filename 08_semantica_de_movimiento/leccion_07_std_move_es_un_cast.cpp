/* =============================================================================
   MÓDULO 08 - LECCIÓN 07: std::move NO mueve: es un cast
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      La frase que rompe el hielo de todos: std::move NO mueve nada.
      Ni una instrucción. `std::move(x)` es, literalmente, un CAST:
          static_cast<std::remove_reference_t<T>&&>(x)
      Su único trabajo es cambiar la CATEGORÍA de la expresión x (de
      lvalue a rvalue). ¿Quién "roba" los internos? El CONSTRUCTOR o
      la ASIGNACIÓN de movimiento del tipo de destino, y SOLO SI
      existen.

      Eso tiene dos consecuencias importantes:
        - Si el tipo no tiene movimiento definido (solo copia), el
          rvalue resultante RECAE en el constructor de copia: el move
          es inútil, solo confunde al lector.
        - Si el origen es const, el cast produce `const T&&`, y no
          hay constructor de movimiento para const (para mover habría
          que vaciar el origen, y const no se puede vaciar). De
          nuevo: copia silenciosa, cero mejora.

      SEGUNDA mitad del titular: el origen queda "válido pero
      indeterminado". En la práctica:
        - SÍ puede destruirse: su destructor corre igual y sin error
          (es justamente el invariante que hicimos cumplir en la L05
          al vaciar el origen).
        - SÍ puede asignársele un valor nuevo y reusarse: es normal
          y seguro.
        - NO se puede confiar en su contenido. Un string movido USUAL
          mente queda vacío (si era largo, robaron su buffer), pero un
          string CORTO (SSO del módulo 03) puede quedarse con el
          texto adentro: el estándar no garantiza el estado exacto.
        - Para std::vector hay garantía de "vacío" (size 0); para
          tipos propios, la garantía es lo que tu constructor de
          movimiento decida (la nuestra: vacío, L05).

      Mindset: "moved-from" significa "casi apagado". El objeto
      existe, es limpio de fugas, PERO
      hay que asegurar que le escribiste algo antes de leerlo. La
      convención que se usa en el código de producción: después de
      `auto nueva = std::move(vieja);` nadie toca vieja hasta
      asignarle algo.

    2. ¿QUÉ PASA POR DEBAJO?
      El cast completo es
          static_cast<std::remove_reference_t<T>&&>(x)
      El `remove_reference` existe por las reglas de deducción de
      plantillas (módulo 09): `std::move` toma `T&&` y si le pasás un
      lvalue de tipo T existe el riesgo de que deduzca T = T& (una
      referencia dentro de otra: reference collapsing). El
      remove_reference "desarrolla" esa referencia y arma el `&&`
      limpio. A nivel
      machine code: CERO instrucciones. La categoría de valor es
      metadato del compilador, no un objeto en memoria. Por eso lo
      llaman "el cast más elegante de C++": todo en el nombre.

      Otra consecuencia del sistema de tipos: `const T a;
      std::move(a)` produce `const T&&`; las sobrecargas de
      movimiento (`T&&`) no se ligan a const y CAEN al constructor
      `const T&` (copia). El compilador no avisa. Los linters te lo
      marcan: no muevas const.

    3. ERRORES COMUNES
      - `std::move(unConstString)` esperando ahorrar una copia: NO
        la ahorra; copia igual y el código MIENTE al lector (dice
        "move" y no mueve). Quitá el move (o el const).
      - Releer un moved-from y que "funcione": el bug más desagradable
        del módulo. Con strings cortos el dato queda adentro, el test
        pasa, y el bug aparece cuando el string deja de ser corto
        (cambió el dato de entrada o el compilador). Regla: a un
        moved-from se le asigna, se destruye, o nada.
      - Devolver `std::move(local)`: no mueve "más"; NIEGA el RVO
        que el compilador podía aplicar gratis (L09, con medición).

    4. DEMO: el cast, la trampa const y el estado moved-from medidos
   ============================================================================
 */
#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

int main() {
    std::cout << "== A) move de vector: misma memoria, origen vacío ==\n";
    std::vector<int> a{1, 2, 3, 4, 5};
    int* const datosDeA = a.data();                // ANTES del move: donde viven
    std::vector<int> b = std::move(a);             // cast + (luego) move-ctor
    std::cout << "  b.size=" << b.size() << ", b usa la memoria de a: "
              << std::boolalpha << (b.data() == datosDeA) << "\n";
    std::cout << "  a.size=" << a.size()
              << " (moved-from con garantia estandar: vacio)\n";

    std::cout << "\n== B) la trampa const: std::move sobre const COPIA ==\n";
    const std::string lento(1000, 'x');            // const: nadie puede vaciarlo
    std::string lento2 = std::move(lento);         // compila... y COPIA igual
    std::cout << "  lento.size=" << lento.size() << " (no se vacio: era const)\n";
    std::cout << "  lento2.size=" << lento2.size()
              << " (la copia O(n) SI ocurrio)\n";

    std::cout << "\n== C) moved-from: vacio USUAL, no garantizado ==\n";
    std::string grande("unaPalabraDeCaracteresMuyAncho");  // largo: NO SSO
    std::string viajero = std::move(grande);       // roba el buffer
    std::cout << "  tras mover un string LARGO, grande queda size="
              << grande.size() << " (tipico: vacio)\n";
    std::string cortito = "hola";                  // corto: en el SSO
    std::string viajero2 = std::move(cortito);     // puede NO vaciarse
    std::cout << "  tras mover un string CORTO (SSO), cortito dice: \""
              << cortito << "\" <- estado indeterminado; no confies en el\n";

    std::cout << "\n== D) moved-from: reasignar y seguir ==\n";
    grande = "nueva vida";                         // seguro: al objeto se le escribe
    a.push_back(99);                               // seguro: al vector se le escribe
    std::cout << "  grande=\"" << grande << "\", a[0]=" << a[0] << "\n";
    return 0;
}
