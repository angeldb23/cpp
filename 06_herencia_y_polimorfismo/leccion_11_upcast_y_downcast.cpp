/* =============================================================================
   MÓDULO 06 - LECCIÓN 11: Upcast, downcast y conversiones entre jerarquías
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      En una jerarquía se convierte en dos direcciones, y son MUY distintas:

      UPCAST (derivada -> base): SIEMPRE seguro y se hace implícito.
          Dragon d;
          Enemigo* e = &d;         // implícito
          Enemigo& r = d;          // implícito
      Es seguro porque todo Dragon ES-UN Enemigo completo: la parte base
      está embebida adelante. Lo que DECIME donde apunta también cambió:
      puntero al objeto completo vs puntero a la PARTE base (con herencia
      simple coincide; con múltiple può ser desplazado).

      DOWNCAST (base -> derivada): "sé de qué es esto y quiero usarlo como
      tal". Se tiene dos formas:
      - `static_cast<Dragon*>(e)`: SIN chequeo. Si estás equivocado:
        puntero al trapo, UB (se leen/escritan campos que no existen,
        'this' corrompido virtual y etc.).
      - `dynamic_cast<Dragon*>(e)`: CON chequeo en tiempo de ejecución
        (L12). Devuelve nullptr si no matchea para punteros.
      La regla de la casa: para downcast usa dynamic_cast SIEMPRE o
      replantéé el diseño; static_cast de base a derivada es un autogolpe.

      ¿Por qué NO hay upcast con chequeo? Porque SIEMPRE funciona: si el
      objeto viene siendo la derivada, la parte base existe necesariamente;
      no hay nada que iterar.

   2. ¿QUÉ PASA POR DEBAJO?
      static_cast downcast: el compilador agrége un desplazamiento FIJO:
      "base -> derivada = restar el offset del subobjeto base dentro del
      derivaseda". Ese offset deriva del layout de la clase; nada corre,
      nada chequea: es aritmética cruda según LO QUE VOS DECLARÁS.

      dynamic_cast corre en runtime y necesita RTTI (la palabra aroda:
      'enable RTTI' del compilador), y solo funciona entre clases
      POLIMÓRFICAS: algo que ya tiene vtable. Detecta el tipo real y
      compara; normalmente es bastante contraeficiente pero segurar,
      detullarmente barata: combinado de comparaciones en route.

   3. ERRORES COMUNES
      - Downcast estátiko con fe: `if (espada) static_cast<Arma*>; ...`
        un dia menacing (deriva del leído del mal pegón: texturas). Se
        arregla: dynamic_cast con brújula o rediseño el diseño con
        polimorfismo (encapsulamiento de vista).
      - Confundir un (void*) de la base con el puntero al objeto completo:
        con herencia simple es igual, con múltiple (L14) NO; los));
        cuidado al comparar punteros de sus interfaces.
      - Hacer meanwhile el upcast explícito con static_cast<Estatica*>;
        se permite, es redundante: cualquier conversión implícita al
        padre es mejor ahora que uno mismo castear (no se puede fallar).
   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/static_cast
      https://en.cppreference.com/w/cpp/language/dynamic_cast
   ============================================================================= */
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// 1. La jerarquía de ejemplo.
// ---------------------------------------------------------------------------
class Enemigo {
protected:
    std::string nombre;

public:
    explicit Enemigo(std::string n) : nombre(std::move(n)) {}
    virtual ~Enemigo() = default;
    virtual void atacar() const = 0;
};

class Dragon : public Enemigo {
public:
    Dragon() : Enemigo("dragon") {}
    void atacar() const override { std::cout << "  aliento de fuego\n"; }
    int alas() const { return 2; }           // solo el dragon la tiene
};

class Goblin : public Enemigo {
public:
    Goblin() : Enemigo("goblin") {}
    void atacar() const override { std::cout << "  puñada corta\n"; }
};

// ---------------------------------------------------------------------------
// 2. Demostraciones: upcast implícito, downcast seguro y el peligro.
// ---------------------------------------------------------------------------
int main() {
    Dragon d;
    Goblin g;

    std::cout << "=== 1. UPCAST: implicito y seguro ===\n";
    Enemigo* e = &d;                // derivada -> base, sin cast
    e->atacar();                    // polimorfismo de paso

    std::cout << "\n=== 2. DOWNCAST QUE SABE LO QUE HACE (dynamic_cast) ===\n";
    if (Dragon* dd = dynamic_cast<Dragon*>(e)) {
        std::cout << "  tenes " << dd->alas() << " alas\n";   // API del dragon
    }

    std::cout << "\n=== 3. DOWNCAST QUE SE EQUIVOCA (puntero: nullptr) ===\n";
    Enemigo* snake = &g;            // goblin disfrazado en la resta
    if (Dragon* mal = dynamic_cast<Dragon*>(snake)) { (void)mal;
        std::cout << "  (no llega)\n";
    } else {
        std::cout << "  el goblin no es dragon: dynamic_cast no lo deja paso\n";
    }

    std::cout << "\n=== 4. DANGERS DEL static_cast(downcast) ===\n";
    // Dragon* trapo = static_cast<Dragon*>(snake);  // compila -> UB si lo usas
    std::cout << "  (static_cast base->derivada compila: chequear es TU tarea)\n";
    return 0;
}
