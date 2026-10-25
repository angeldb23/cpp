/* =============================================================================
   MÓDULO 06 - LECCIÓN 14: Herencia múltiple y ambigüedades
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Una clase puede heredar de MÁS de una base:

          class ArqueroVolador : public Enemigo, public Volador {};

      a) Orden de construcción: cada base se construye en el ORDEN EN QUE
         SE LISTA en el `class D : public A, public B` (no en el orden de
         la lista de inicialización, igual que en la L02). La destrucción
         en orden inverso.
      b) Cada base aporta un SUBOBJETO completo e independiente: los datos
         de Enemigo y los de Volador viven a la vez en ArqueroVolador.
      c) El upcast a cada base es válido, pero los punteros a cada base
         pueden apuntar a DIRECCIONES DISTINTAS dentro del objeto:
         solo la primera base coincide con el comienzo del objeto; las
         demás empiezan "más adentro". Comparar punteros crudos de bases
         distintas exige pensar eso.

      Ambiguüedades propias de heredar de varios lados:
      d) Nombre repetido: si dos bases tienen un método con el mismo
         nombre y la derivada lo usa sin calificar, el compilador no
         elige: error de ambigüedad. Hay que calificar:
         `Enemigo::atacar()` o `Volador::atacar()`, o overrides que decidan.
      e) Ocultamiento cruzado (L03): si la derivada define su propio
         `atacar()`, oculta el de TODAS sus bases con ese nombre. Si
         querés traer de nuevo el de una base: `using Enemigo::atacar;`.
      f) El caso límite: si una tercera clase hereda de ArqueroVolador,
         todo se acumula; y si la clase tiene DOS bases que a su vez
         comparten una base común, aparece el diamante (L15).

      ¿Cuándo es buena idea? En general, cuando las segundas bases son
      INTERFACES (L10): capacidades con pocos métodos y SIN datos. La
      herencia múltiple de familias de DATOS es lo complicado: eso es el
      tema de la L15.

   2. ¿QUÉ PASA POR DEBAJO?
      Layout del objeto (herencia múltiple SIN bases virtuales):
          [ Enemigo ][ Volador ][ datos de ArqueroVolador ]
      Cada base polimórfica lleva SU vtable, y el objeto completo suma
      los vptr necesarios (uno por subobjeto polimórfico). La conversión
      a la primera base es "dejar la dirección igual"; a la segunda,
      "saltar el offset del subobjeto". Con llamadas virtuales, el
      compilador inserta ajustes de `this` para que cada metodo vea el
      subobjeto que corresponde.

   3. ERRORES COMUNES
      - Heredar por comodidad para copiar código (composición suele ser la
        mejor herramienta): datos repetidos en varias bases es el síntoma.
      - Olvidar que cada base con delete necesita su destructor virtual:
        si podés borrar por Volador*, Volador también lleva dtor virtual.
      - Creer que la derivada ve los métodos PRIVADOS de las bases (nunca)
        o que las dos bases comparten subobjeto (eso puede ser, pero es
        herencia virtual: L15).
      - Bases repetidas accidentalmente (A abuela -> B y C heredan A y D
        hereda B y C): dos A => diamante (L15).

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/derived_class (Multiple inheritance)
   ============================================================================= */
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// 1. Dos bases: una de dominio (datos), una de capacidad (interfaz).
// ---------------------------------------------------------------------------
class Enemigo {
protected:
    std::string nombre;

public:
    explicit Enemigo(std::string n) : nombre(std::move(n)) {}
    virtual ~Enemigo() = default;
    virtual void atacar() const = 0;
    void presentarse() const { std::cout << "  nombre: " << nombre << "\n"; }
};

class Volador {
public:
    virtual ~Volador() = default;       // cada base polimorfica, su dtor virtual
    virtual void planear() const { std::cout << "  planeando\n"; }
};

// ---------------------------------------------------------------------------
// 2. Herencia múltiple: dos subobjetos en un solo objeto.
// ---------------------------------------------------------------------------
class ArqueroVolador : public Enemigo, public Volador {
public:
    ArqueroVolador() : Enemigo("arquero volador") {}
    void atacar() const override { std::cout << "  flecha desde arriba\n"; }

    // Con este override, `atacar` queda definida y no hay ambigüedad.
    // Si solo una base tuviera `atacar` no virtual, idem: no hay drama;
    // el drama aparece cuando DOS bases tienen EL MISMO nombre.
};

class ConsolaVoladora : public Volador {
public:
    void planear() const override { std::cout << "  planeo rapido\n"; }
};

// ---------------------------------------------------------------------------
// 3. Ambiguüedad clásica: dos bases con el mismo nombre de método.
// ---------------------------------------------------------------------------
class Magico {
public:
    void describir() const { std::cout << "  es magico\n"; }
};
class Fisico {
public:
    void describir() const { std::cout << "  es fisico\n"; }
};

class Elemental : public Magico, public Fisico {
public:
    // Sin desambiguar acá, `elemental.describir()` NO compila:
    // "member 'describir' found in multiple base classes".
    void describir() const {
        Magico::describir();            // la de Magico
        Fisico::describir();            // la de Fisico
    }
};

// ---------------------------------------------------------------------------
// 4. Demostración.
// ---------------------------------------------------------------------------
int main() {
    std::cout << "=== 1. UN OBJETO, DOS BASES ===\n";
    ArqueroVolador flechador;
    flechador.presentarse();            // viene de Enemigo
    flechador.planear();                // viene de Volador
    flechador.atacar();                 // la propia

    std::cout << "\n=== 2. UPCAST A CADA BASE ===\n";
    Enemigo* e = &flechador;            // ok: primera base
    Volador* v = &flechador;            // ok: segunda base
    e->atacar();
    v->planear();
    std::cout << "  (mismo objeto, dos punteros: "
              << (e != reinterpret_cast<void*>(v) ? "direcciones distintas" : "iguales")
              << " en general)\n";

    std::cout << "\n=== 3. DESAMBIGUIEDAD: LA DERIVADA DECIDE ===\n";
    Elemental anfibia;
    anfibia.describir();                // llama a ambas bases

    std::cout << "\n=== 4. AMBIGUEDAD DIRECTA: sin la derivada no compila ===\n";
    // anfibia.Fisico::describir();     // calificada: siempre se puede
    ConsolaVoladora cv;
    cv.planear();
    return 0;
}
