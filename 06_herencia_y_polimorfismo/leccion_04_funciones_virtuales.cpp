/* =============================================================================
   MÓDULO 06 - LECCIÓN 04: Funciones virtuales y despacho dinámico
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Polimorfismo: usar muchos tipos concretos desde una sola interfaz base,
      y que CADA uno responda con SU comportamiento. Para eso existe `virtual`:

      Sin `virtual`, la llamada se resuelve por el TIPO DE LA VARIABLE / del
      PUNTERO (despacho estático). Con `virtual`, se resuelve por el TIPO REAL
      del objeto en tiempo de ejecución (despacho dinámico).

          Enemigo* e = new Dragon;
          e->atacar();          // sin virtual: versión de Enemigo
                                // con virtual: versión de Dragon

      Reglas:
      a) `virtual` se marca en la base (y el compilador lo hereda para
         todos los overridden; aún así marcalo vos con `override` en los
         hijos: se dixa en la L05).
      b) Lo que decide la versión llamada es el TIPO DINÁMICO del objeto:
         cómo se CREÓ, no a qué se lo asignó después.
      c) Solo sirve a través de puntero o referencia. Un objeto por valor
         SIEMPRE se comporta según su tipo estático (eso es el object
         slicing, L13).

      Tipos: el TIPO ESTÁTICO es el que aparece en el código (Enemigo*);
      el tipo DINÁMICO es el del objeto real al que apunta (Dragon).

   2. ¿QUÉ PASA POR DEBAJO?
      Cada clase con funcion virtual tiene una vtable (L06), y cada objeto
      lleva un vptr. La llamada `e->atacar()` mira el vptr del objeto real
      y salta a la entrada de `atacar` de SU clase. Eso implica:1 indirección
      extra por llamada (no es mágicamente gratis) y tamaño extra por objeto.

      Trampa clave: llamar un virtual dentro de un constructor o destructor
      de la base no llega a la versión del hijo: durante la construcción de
      Entidad, el vptr todavía apunta a la vtable de Entidad. Más detalle
      en la L07 y la L02.

   3. ERRORES COMUNES
      - Olvidar `virtual` en la base y esperar polimorfismo: sin él, la
        llamada sale por tipo estático y no hay que se rejuega nada.
      - Marcar virtual un destructor "porque sí": es necesario SOLO si
        vas a destruir desde un puntero a la base  (y ahí es OBLIGATORIO:
        L07). La regla práctica para clases base de juego: sí o sí.
      - Copiar objetos polimórficos a un vector de Base por valor: se
        perdía el hijo. Ahora lo natural es vector de punteros.
   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/virtual
      https://en.cppreference.com/w/cpp/language/function#Virtual_functions
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// 1. La base marca `virtual` en el método que va a ser polimórfico.
// ---------------------------------------------------------------------------
class Enemigo {
protected:
    std::string nombre;

public:
    explicit Enemigo(std::string nombre_) : nombre(std::move(nombre_)) {}
    virtual ~Enemigo() = default;      // destructor virtual: regla de la casa (L07)

    virtual void atacar() const {
        std::cout << "  " << nombre << " hace un ataque generico\n";
    }
    void presentarse() const {          // no virtual: mismo código para todos
        std::cout << "  Soy un " << nombre << "\n";
    }
};

// ---------------------------------------------------------------------------
// 2. Derivadas: cada una redefine `atacar`.
// ---------------------------------------------------------------------------
class Arquero : public Enemigo {
public:
    Arquero() : Enemigo("arquero elfo") {}
    void atacar() const override {
        std::cout << "  " << nombre << " dispara una flecha\n";
    }
};

class Dragon : public Enemigo {
public:
    Dragon() : Enemigo("dragon rojo") {}
    void atacar() const override {
        std::cout << "  " << nombre << " exhala una llamarada\n";
    }
};

// ---------------------------------------------------------------------------
// 3. Demostración: mismo puntero base, distinto comportamiento.
// ---------------------------------------------------------------------------
int main() {
    Arquero legolas;
    Dragon smaug;

    std::cout << "=== 1. SIN POLIMORFISMO (referencias directas) ===\n";
    legolas.presentarse();   // estático: siempre igual

    std::cout << "\n=== 2. CON POLIMORFISMO: el tipo REAL manda ===\n";
    std::vector<Enemigo*> mapa = {&legolas, &smaug};   // stack: sin deletes
    for (const Enemigo* e : mapa) {
        e->atacar();             // virtual: cada uno dice lo suyo
    }

    std::cout << "\n=== 3. STATICO vs DINAMICO con la MISMA variable ===\n";
    Enemigo* e = &legolas;
    e->atacar();                 // dinamico: arquero
    e = &smaug;
    e->atacar();                 // dinamico: dragon
    e->presentarse();            // estatico: Enemigo::presentarse SIEMPRE
    return 0;
}
