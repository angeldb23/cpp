/* =============================================================================
   MÓDULO 09 - LECCIÓN 09: Elegir: sobrecargas por categoría de valor
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
      Con referencia universal + std::forward (lecciones 06 a 08) ya sabés
      reenviar sin perder nada. Pero no siempre conviene la referencia
      universal: el forwarding ref captura TODO y esconde qué acepta tu
      función. La alternativa es OTORGAR la decisión con sobrecargas
      explícitas por categoría de valor:

        void fusionar(const Palabra& a, const Palabra& b);  // copia seguro
        void fusionar(Palabra&& a, Palabra&& b);            // se puede mover

      Qué une cada versión:
        - const T&   advierte: "lo miro, NO lo tomo": por naturaleza safe
          para lvalues; el que llama puede pasarle rvalues igual (y se
          copian adentro).
        - T&&  remueve la duda para rvalues: "podés absorberme".
      Con esto el QUE LLAMA decide en cada sitio de llamada con qué firma
      invoca, y el compilador elije la mejor sobrecarga (la sobrecarga con
      T&& es más específica para rvalues, la const T& gana en lvalues).

      La herramienta para eso en clases: funciones miembro con
      ref-qualifiers. Puedes poner el mismo calificativo a la instancia de
      `this`:
            std::string datos() const&;   // sobre un objeto vivo: devolver
                                          // una copia segura
            std::string datos() &&;       // el objeto ya no se usa más
                                          // (vino temporario): devolver
                                          // el interno movido
      La versión && sobre un objeto que se destruye evita copiar lo que
      igual se destruiría: el que llama se lleva el dato directo.

      Cuando ELEGIS una u otra:
        - Referencia universal (T&&): para templates "pase lo que pase",
          que delegan (fábricas, wrappers) o cuando el TIPO concreto no
          importa y la semántica de todos los tipos es la misma.
        - Sobrecargas por categoría: cuando el comportamiento depende de
          CÓMO se llamó (mover si se mueve, copiar si se copia) y querés
          un contrato visible, por ejemplo en una clase como Palabra
          (como los move-assignment del módulo 08, que trabajaban por
          sobrecargas, no por forwarding).

   2. ¿QUÉ PASA POR DEBAJO?
   En la resolución de sobrecargas, un argumento rvalue puede ligar a
      ambas (const T& existe desde hace sempre; T&& es la especial para
      rvalues) y el compilador prefiere la que coincide con su categoría:
      C++11 introdujo esa preferencia justamente para que el movimiento
      gane cuando corresponde. Con los ref-qualifiers, el compilador usa
      la categoría de la expresión sobre la que se llama al método.

   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
      - Sobre el STEP class con && pipeline con lifetimes: vale abandonar
        el objeto después de && (el peligro de "moved-from").
      - Si tenés SOLO T&& y no const T&, los lvalues no compilan: es una
        decisión, no un error (o preferí la referencia universal si
        podés aceptar ambos).
      - En los métodos &&: detrás de esa firma asumís que el objeto ya
        no se va a usar más; reservá & y const& para los vivos.
      - Cuando el comportamiento depende del TIPO y no de la categoría,
        volvé al template con concepts (lección 03) en lugar de una
        familia nueva de sobrecargas.

   4. PARA PROFUNDIZAR:
      https://en.cppreference.com/w/cpp/language/member_functions (ref-qualifiers)
      https://en.cppreference.com/w/cpp/language/overload_resolution
      C++ Core Guidelines: F.18 ("For 'will-move-from' parameters, pass
      by X&&") y F.24 ("Don't use forwarding references as an
      overloaded pair selector").
   ============================================================================= */
#include <iostream>
#include <string>
#include <utility>
#include <vector>

// Un tipo rastreable para VER qué sobrecarga ganó:
class Palabra {
public:
    std::string texto;

    explicit Palabra(std::string t) : texto(std::move(t)) {}
    Palabra(const Palabra&) = default;      // copia simple, sin rastreo
    Palabra(Palabra&&) = default;           // movimiento disponible
};

// --- 1. UNA sobrecarga para lvalues (const&) y otra para rvalues (&&) ---
void fusionar(const Palabra& a, const Palabra& b) {     // lvalues: mirar, no tomar
    std::cout << "  fusionar(const&): uso las dos SIN moverlas\n";
    std::cout << "  -> \"" << a.texto << " " << b.texto << "\" (copia si hace falta)\n";
}

void fusionar(Palabra&& a, Palabra&& b) {               // rvalues: derecho a mover
    std::cout << "  fusionar(&&): puedo absorber las dos\n";
    const std::string unida = a.texto + " " + b.texto;
    std::cout << "  -> \"" << unida << "\" (nada copió de más)\n";
}

// El que llama elige con la CATEGORIA de lo que pasa:
Palabra fabricarPalabra(const char* t) { return Palabra{t}; }

// --- 2. Lo mismo en una COVER de funciones miembro: ref-qualifiers ---
class ConstructorDeFrase {
public:
    explicit ConstructorDeFrase(std::string base) : base_(std::move(base)) {}

    // Sobre un objeto VIVO (&): no conviene saquearlo, devolvemos una copia.
    std::string frase() const& { return base_ + " (firma de lvalue)"; }

    // sobre un objeto QUE SE DESTRUYE (&&): devolvemos su interno por move.
    std::string frase() && { return std::move(base_) + " (firma de rvalue)"; }

private:
    std::string base_;
};

int main() {
    std::cout << "=== 1. SOBRECARGAS LIBRES: const& vs && ===\n";
    Palabra l1("mar"), l2("luna");
    fusionar(l1, l2);                       // dos lvalues -> const&
    fusionar(fabricarPalabra("sol"), fabricarPalabra("cielo"));  // dos rvalues -> &&

    std::cout << "\n=== 2. EL COMPILADOR DECIDE POR CATEGORIA ===\n";
    fusionar(l1, fabricarPalabra("piedra"));  // mezcla -> const& (el && exige DOS rvalues)
    std::cout << "  (con un lvalue en el medio, la sobrecarga && no aplica)\n";

    std::cout << "\n=== 3. REF-QUALIFIERS EN FUNCIONES MIEMBRO ===\n";
    ConstructorDeFrase vivo {"hola"};
    std::cout << "  lvalue:  " << vivo.frase() << "\n";
    std::cout << "  rvalue:  " << ConstructorDeFrase {"temp"}.frase() << "\n";

    std::cout << "\n=== 4. ¿CUÁNDO USO CADA ESTRATEGIA? ===\n";
    std::cout << "  template delegante (fábrica, wrapper) -> T&& + forward\n";
    std::cout << "  semántica distinta según llamada    -> const& / && como arriba\n";
    std::cout << "  clase con internos movibles          -> ref-qualifiers en el acceso\n";
    return 0;
}
