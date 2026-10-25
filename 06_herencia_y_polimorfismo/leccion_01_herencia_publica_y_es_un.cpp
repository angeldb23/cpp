/* =============================================================================
   MÓDULO 06 - LECCIÓN 01: Herencia pública y la relación "es-un"
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      La herencia permite crear una clase nueva (derivada) a partir de una
      existente (base), reutilizando sus atributos y métodos.

          class Mago : public Entidad {
              // ...
          };

      Se lee: "Mago ES-UN Entidad". Esa lectura no es decoración: es la
      condición para usar `public`. Si no podés decir la frase sin mentir,
      la herencia es la herramienta equivocada (volvé al módulo 05: si un
      componente TIENE una cuerda, la cuerda es un atributo, no su padre).

      a) Qué hereda la clase derivada:
         - Todos los miembros de datos del padre (aunque no pueda verlos).
         - Todos los métodos del padre que no declare de nuevo.
         - El tipo: un Mago puede usarse donde se espera una Entidad (ya
           vamos a ver los detalles en la L11).

      b) Qué NO hereda:
         - Constructores y destructor: el Mago tiene su propio constructor
           (que llama al de Entidad); el destructor también es propio.
         - El operador de asignación por defecto de clases no se hereda
           tal cual: se genera nuevo en la derivada (copia todo, incluida
           la parte base).
         - Los `friend` del padre no son friends del hijo.

      c) Los tres especificadores de herencia:
         - `public`: los públicos y protegidos del padre quedan
           públicos y protegidos en la hija. Es el único para "es-un".
         - `protected`: todo lo heredado cae en protected (el resto del
           mundo ya no ve la interfaz del padre).
         - `private`: todo lo heredado se vuelve privado: la hija reutiliza
           la implementación, pero nadie más la ve (y una Mago ya no es
           "usable como Entidad" para el compilador). Se usa poco: casi
           siempre composición es más clara.

      d) Atributos `protected`: son como `private` pero la clase derivada
         los ve. Es la vía de escape del encapsulamiento para jerarquías:
         usalo con moderación, porque los invariantes dejan de estar
         protegidos por una sola clase (los hijos también pueden romperlos).

   2. ¿QUÉ PASA POR DEBAJO?
      Layout: el objeto Mago se construye con la parte Entidad EMPOTRADA
      adelante: [ Entidad: nombre, vida ][ Mago: mana ]. El compilador
      garantiza que la dirección del subobjeto Entidad esté al inicio, para
      que un Entidad* apuntando al Mago pueda leer esa parte sin sumas raras
      (con herencia simple; con herencia múltiple hay desplazamientos, L14).

      Método heredado: no se "copia el código". La llamada
      gandalf.mostrarInfo() compila a una llamada que recibe un puntero
      extra invisible al subobjeto Entidad: es exactamente el `this` del
      módulo 05, ahora apuntando a la parte base.

   3. ERRORES COMUNES
      - Heredar con `public` por default por inercia cuando la relación es
        "tiene un": un Casco TIENE un nivel de protección, no ES-UN nivel.
      - Hacer el padre de datos `protected` y permitir que cualquier hijo
        los reescriba sin invariante: si el padre garantiza que `vida > 0`,
        cada hijo que escribe `vida` directamente puede romperlo.
      - Creer que la hija hereda los constructores: si `Entidad` no tiene
        constructor por defecto, el `Mago` DEBE llamarlo desde su lista de
        inicialización, y si el Mago no declara ninguno, no se puede crear.
      - Intentar acceder a un miembro privado del padre desde la hija:
        `private` es privado también para los hijos. protected no significa
        "privado + hijos": significa "visibilidad para la cadena de clases".

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/derived_class
      https://en.cppreference.com/w/cpp/language/access
   ============================================================================= */
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// 1. Clase base: la parte común de todas las entidades del juego.
// ---------------------------------------------------------------------------
class Entidad {
protected:                       // visibles a la cadena de clases
    std::string nombre;
    int vida = 0;

public:
    Entidad(const std::string& nombre_, int vida_) : nombre(nombre_), vida(vida_) {
        std::cout << "  [ctor Entidad] \"" << nombre << "\"\n";
    }

    void mostrarInfo() const {          // método heredable: se reutiliza en Mago
        std::cout << "  nombre: " << nombre << " | vida: " << vida << "\n";
    }

    void restarVida(int cantidad) {     // mantiene el invariante vida >= 0
        vida = (vida >= cantidad) ? vida - cantidad : 0;
    }
};

// ---------------------------------------------------------------------------
// 2. Clase derivada con herencia PÚBLICA.
// ---------------------------------------------------------------------------
class Mago : public Entidad {
private:
    int mana = 0;

public:
    // El constructor de Mago LLAMA al de Entidad (no lo reemplaza):
    Mago(const std::string& nombre_, int vida_, int mana_)
        : Entidad(nombre_, vida_), mana(mana_) {   // base primero, luego miembros
        std::cout << "  [ctor Mago] mana inicial: " << mana << "\n";
    }

    void lanzarHechizo() {
        if (mana >= 10) {
            mana -= 10;
            restarVida(2);          // método heredado: usa el padre
            std::cout << "  " << nombre << " lanza el hechizo (mana: " << mana
                      << ", vida: " << vida << ")\n";
        } else {
            std::cout << "  " << nombre << " no tiene mana suficiente\n";
        }
    }

    int getMana() const { return mana; }
    int getVida() const { return vida; }   // el atributo `vida` es protected
};

// ---------------------------------------------------------------------------
// 3. Demostración: qué hereda y dónde vive cada dato en memoria.
// ---------------------------------------------------------------------------
int main() {
    std::cout << "=== 1. CONSTRUCCION: EL HIJO CONSTRUYE SU PARTE BASE ===\n";
    Mago gandalf("Gandalf", 90, 50);

    std::cout << "\n=== 2. METODOS HEREDADOS (salen del padre, gratis) ===\n";
    gandalf.mostrarInfo();

    std::cout << "\n=== 3. EL HIJO USA LO DEL PADRE ===\n";
    gandalf.lanzarHechizo();
    gandalf.lanzarHechizo();
    gandalf.restarVida(40);
    gandalf.mostrarInfo();

    std::cout << "\n=== 4. LAYOUT: EL PADRE VA EMPOTRADO ADELANTE ===\n";
    Entidad base("Base", 10);
    std::cout << "  sizeof(Entidad) = " << sizeof(base) << "\n";
    std::cout << "  sizeof(Mago)    = " << sizeof(gandalf)
              << "  (la parte Entidad mas el mana extra)\n";
    return 0;
}
