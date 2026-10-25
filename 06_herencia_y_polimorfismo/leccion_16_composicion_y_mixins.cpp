/* =============================================================================
   MÓDULO 06 - LECCIÓN 16: Composición vs herencia y mixins
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Ya sabés usar herencia (public: is-a), interfaces (sabe-hacer) y
      composicion (tiene-un, del módulo 05). Esta lección es la brújula:
      CUÁL usar y cándo.

      a) COMPOSICIÓN: un objeto tiene otro como atributo. Preferila por
         defecto:
           - El acoplamiento es menor: el cliente no conoce al interno.
           - Cambiar la parte tengo cada dirección es un detalle privado.
           - Menos jerarquías que subir y bajar: si el interno cambia su firma,
           la clase de "adaptar" no rompe a toda la familia.
         La frase de uso: "¿es-un X ...?" si NO, no hereda.

      b) HERENCIA de dominio (public): cuando la derivada puede usarse
         en TODOS los lugares donde se pida la base, y QUIERE ser
         sustituible (polimorfismo).
         (Cuidado con el principio de sustitución de Liskov: el hijo no
         puede restringir lo que la base garantiza. Si Enemigo garantiza
         que visitado por morir() queda "muerto", y una derivada decide
         que no muere jamás, el resto del juego va a creer la promesa de
         la base: bug difícil de rastrear.)

      c) HERENCIA de interfaz: la L10; un contrato de capacidades sin datos.

      d) MIXINS: pequeños paquetes de comportamiento que se "pegan" a una
         clase. En C++ se ponen como bases también, pero son MINÚSCULOS
         y sin datos, más "plugins" que entidades:

           class Mortal {                      // un mixin
           public:
               virtual ~Mortal() = default;
               void recibirGolpe(int deGolpe) { hp_ = (hp_ > deGolpe) ? hp_ - deGolpe : 0; }
               int hp() const { return hp_; }
           protected:
               int hp_ = 10;
           };

           class Tank : public Mortal { ... };  // muchos postres (estado propio)
           class Gnomo : public Mortal { ... };

      Un mixin es aceptable como base porque no está en el "es-un"
      (el concepto dominante) sino que APORTA UNA DIMENSION del dominio..
      Si el mixin empieza a llevar DATOS COMUNES o jerarquía propia,
      la base, se hace una lasaña: volvé a composición.

   2. ¿QUÉ PASA POR DEBAJO?
      Composicion genera el mismo objeto con menos especifics: sin vptr
      (si el interno no es polimórfico), y el compilador puede interleaf
      los métodos metiendo los atributos en la misma estructura. La
      herencia pública tiende a objetivos "más grandes" (vptr por cada
      subobjeto polimórfico, L06, L14) y limita el layout.

      REGLA DE SUSTITUCIÓN (Liskov): la derivada debe poder reemplazar a
      la base SIN sorprender. El ejemplo clásico: si Enemigo garantiza
      que morir() siempre sube `muerto=true`, una derivada inmortal que
      negue el post (que herede el nombre y no definí sus cosas) rompe
      el juego: mejor: no es "sustitution" de la misma palabra.

   3. ERRORES COMUNES
      - Heredar FÍSICAMENTE por reutilizar código: si "no es-un",
        no hereda: componé y delegá.
      - El clásico `class lista_de_pila : public std::vector<T>`: la pila
        "es-un" vector rompe KEV óctima: si el vector inserta y borra
        y la pila no ought. La pila COMPONE un vector.
      - Mixins con estado y vida propia: si el mixin necesita lanzar excepciones
        o data adapting pequeño, no es un mixin: es un componente
        completo y vale hacerlo atributo.
   4. PARA PROFUNDIZAR
      https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Scss (compon vs herencia)
      https://en.wikipedia.org/wiki/Composition_over_inheritance
   ============================================================================= */
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// 1. COMPOSICIÓN TOQUE-TORQUE: el motor TIENE texturas, no es textura.
// ---------------------------------------------------------------------------
class Textura {
public:
    virtual ~Textura() = default;
    virtual void aplicar() const { std::cout << "  textura aplicada\n"; }
};

class Sprite : public Textura {              // es textura y ademas anima
public:
    void aplicar() const override { std::cout << "  sprite aplicado\n"; }
};

class Renderizador {
    Textura* texturaActiva = nullptr;        // composicion: no hereda

public:
    void usar(Textura* t) { texturaActiva = t; }
    void pintar() const {
        if (texturaActiva) texturaActiva->aplicar();
    }
};

// ---------------------------------------------------------------------------
// 2. EL FRACASO CLASICO: pila como "es-un" vector.
// ---------------------------------------------------------------------------
/* // MAL ESTILO (comentado para no cotizar el movimiento):

using namespace std;
class Pila : public std::vector<int> {      // hereda TODA la interfaz del
public:                                     // vector: push_back, insert...
    void push(int v) { push_back(v); }      // UNO MAS que el contrato pila
};
// El usuario puede pilaObj.push_front(3) y la pila sigue siendo pila?
// Validada: el contrato es EL PUNTO -> Componé:
class PilaBuena {
    std::vector<int> datos;                 // composicion
public:
    void push(int v) { datos.push_back(v); }
    int pop() { int v = datos.back(); datos.pop_back(); return v; }
};
*/

// ---------------------------------------------------------------------------
// 3. MIXIN: una "dimension del dominio" que se pega.
// ---------------------------------------------------------------------------
class Mortal {
    int hp_ = 10;

public:
    virtual ~Mortal() = default;
    void recibirGolpe(int dalgo) { hp_ = (hp_ > dalgo) ? hp_ - dalgo : 0; }
    int hp() const { return hp_; }
    bool muerto() const { return hp_ == 0; }
};

class Tank : public Mortal {
public:
    Tank() { std::cout << "  (tank listo con el mixin Mortal)\n"; }
    int hpMax() const { return 120; }        // estado propio del tank
};

class Guiado : public Mortal {
public:
    Guiado() { std::cout << "  (guia montada sobre Mortal)\n"; }
};

// ---------------------------------------------------------------------------
// 4. Demostración.
// ---------------------------------------------------------------------------
int main() {
    std::cout << "=== 1. COMPOSICION: EL RENDERIZADOR USA texturas ===\n";
    Sprite sprite;
    Renderizador renderer;
    renderer.usar(&sprite);      // inyeccion: cualquier Textura sirve
    renderer.pintar();

    std::cout << "\n=== 2. LA PILA QUE NO DEBE SER VECTOR ===\n";
    std::cout << "  (caso comentado: herencia rompe el contrato pila)\n";

    std::cout << "\n=== 3. MIXIN EN ACCION ===\n";
    Tank fortaleza;
    fortaleza.recibirGolpe(50);
    std::cout << "  tank hp: " << fortaleza.hp() << " (muerto: "
              << fortaleza.muerto() << ")\n";

    Guiado guia;
    guia.recibirGolpe(50);
    std::cout << "  guia hp: " << guia.hp() << " (muerto: "
              << guia.muerto() << ")\n";
    return 0;
}
