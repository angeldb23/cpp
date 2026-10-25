/* =============================================================================
   MÓDULO 06 - LECCIÓN 13: Object slicing y pasar polimórficos por valor
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      El SLICING (rebanado) es la trampa polimórfica más común y silenciosa:
      pasa cuando copias un objeto derivado a una variable de tipo BASE por
      VALOR.

      La copia de un valor construye un objeto nuevo del tipo de la
      VARIABLE: `Enemigo e = dragon;` crea «un Enemigo», y esa nueva
      entidad SOLO dado qué: dibuja los campos de Enemigo (la parte base).
      Los atributos del Dragon y su vtable se pierden: el objeto quedó
      "rebanado": solo la rodaja base del pastel llegó.

      Consecuencia en cadena:
      - El tipo estático y el dinámico son el mismo (Enemigo) => las
        llamadas virtuales corren la versión de la base. El polimorfismo
        desaparece COMPLETO, no parcialmente.

      Lo que evita el slicing: ¿cómo pasar objetos polimórficos?
      a) POR REFERENCIA (const o no): el comportamiento va al objeto real.
      b) POR PUNTERO: mismo efecto, con la opción de "nada" = nullptr.
      c) En colecciones: contenedor de punteros (o en módulo 07, de
         unique_ptr). NUNCA contenedor de Base por valor si vas a
         mezclar derivadas: cada push rebana al hijo.

      La regla práctica: si la función "habla con" la interfaz base y
      espera polimorfismo, firma (const Base&). Si está pensada para un
      valor concreto (posible copia): firma por valor, y es HONESTO: lo
      que recibe es un corte, no el hijo disfrazado.

   2. ¿QUÉ PASA POR DEBAJO?
      `Base b = derived;` compila al CONSTRUCTOR DE COPIA de Base
      (módulo 05): el que se genera toma `const Base&`, y por parámetro
      implícito acepta una derivada (upcast referencia). Esa copia lee
      solo la parte base y construye SOLO la parte base: es el mismo
      mecanismo que en el módulo 05, pero aplicado a jerarquías se
      convierte en un bug del juego.

      La vtable del nuevo objeto apunta a la de Base: el deriva vptr
      apunta al tipo DUAL: la última copiera b llega por constructor de
      copia de la base, entonces el tipo dinámico es, de nuevo, Base.

   3. ERRORES COMUNES
      - `void inicializar(Enemigo e) { e.atacar(); }` sobre una derivada:
        si atacar no fuera virtual, da la versión de la base SIEMPRE.
      - `std::vector<Enemigo>` con algunos cada push de una derivada:
        rebana TODOS al mismo koka de Enemigo.
      - Devolver la derivada como base por VALOR al copiar al hijo: el
        fragmento a lo largo del gesto del compilador silencioso. Se
        ve en el int genérico con el polimorfismo ligado para el muestra.
      - `auto` HACIENDO el slicing: `auto copia = *dragonPtr;` copia el
        objeto PERO ya que el static es Base*: rebanada inevitable.
   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/copy_constructor
      https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c67 (polymorphism shapes)
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// 1. La jerarquía: base y una derivada con estado extra.
// ---------------------------------------------------------------------------
class Enemigo {
protected:
    std::string nombre;
    int vida = 0;

public:
    Enemigo(std::string n, int v) : nombre(std::move(n)), vida(v) {}
    virtual ~Enemigo() = default;

    virtual int danioDeAtaque() const { return 1; }   // base: débil
    void mostrar() const {
        std::cout << "  " << nombre << " (vida " << vida
                  << ", danio " << danioDeAtaque() << ")\n";
    }
};

class Boss : public Enemigo {
    int furia = 3;

public:
    Boss() : Enemigo("boss", 500), furia(3) {}
    int danioDeAtaque() const override { return furia * 20; }
    int getFuria() const { return furia; }
    void molestar() { ++furia; }
};

// ---------------------------------------------------------------------------
// 2. El experimento: tres formas de pasarlo.
// ---------------------------------------------------------------------------
void porValor(Enemigo e) {
    std::cout << "  por valor:  ";
    e.mostrar();     // slicing: danio sera el de Enemigo SIEMPRE
}

void porReferencia(const Enemigo& e) {
    std::cout << "  por ref:    ";
    e.mostrar();     // polimorfismo intacto
}

int main() {
    Boss boss;
    boss.molestar();     // furia=4 -> danio 80

    std::cout << "=== 1. EL BOSS REAL ===\n";
    boss.mostrar();

    std::cout << "\n=== 2. POR VALOR (¡ slicing!): se corta la parte boss ===\n";
    porValor(boss);      // muestra vida 500 pero danio 1: el boss llego cortado

    std::cout << "\n=== 3. POR REFERENCIA: el polimorfismo viaja ===\n";
    porReferencia(boss); // danio 80

    std::cout << "\n=== 4. EN UN VECTOR (¡el mas trampa!) ===\n";
    {
        std::vector<Enemigo> coleccionRota;
        coleccionRota.push_back(boss);   // push de derivada: rebanada aqui
        std::cout << "  vector<Enemigo>:";
        for (const auto& e : coleccionRota) e.mostrar();
        // El danio abc de la base; el boss nunca estubo ahi.
        // El danio es el de la base: el boss nunca llego entero al vector.
    }
    {
        std::vector<Enemigo*> coleccionOK = {&boss};
        std::cout << "  vector<Enemigo*>:";
        for (const Enemigo* e : coleccionOK) e->mostrar();
        // El danio es el del boss: el polimorfismo viajo por puntero.
    }
    return 0;
}
