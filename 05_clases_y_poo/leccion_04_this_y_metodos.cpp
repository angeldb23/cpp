/* =============================================================================
   MÓDULO 05 - LECCIÓN 04: this, métodos y la clase como módulo
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Cuando escribís `c.depositar(100)`, ¿cómo sabe `depositar` a qué
      cuenta sumarle el dinero? La respuesta es `this`: en cada llamada a
      un método, el objeto en el que se llamó se pasa invisiblemente como
      primer argumento.

          c.depositar(100);      // ~ Cuenta_depositar(&c, 100)

      Dentro del método:
      - `saldo` resolve a `this->m_saldo`: los miembros se buscan
        automáticamente en el objeto actual.
      - `this` es un puntero: `Cuenta* const` (en métodos normales) o
        `const Cuenta* const` (en métodos const — lección 09).
      - Si un método recibe otro parámetro del MISMO tipo, necesitás
        `this` o un prefijo para distinguirlos:

          void Cuenta::fusionar(Cuenta otra) {
              otra.depositar(this->m_saldo);   // this-> explícito
          }

      - Un caso clásico donde `this` se devuelve: cadenar métodos.

          Cuenta& Cuenta::sumar(double m) { saldo += m; return *this; }
          c.sumar(10).sumar(20).sumar(30);   // encadenado

      Nota de estilo: el sufijo m_ en los atributos (m_saldo) o el campo
      titular_ son líneas que eligen los equipos para ver "esto es estado
      del objeto, no un parámetro" sin pensar. El curso usa el prefijo m_.

   2. ¿QUÉ PASA POR DEBAJO?
      Un método no es gratis por objeto: el compilador reescribe
      `void Cuenta::depositar(double)` como una función normal
      `void Cuenta_depositar(Cuenta* this, double)` y reescribe cada
      llamada `c.depositar(100)` como `Cuenta_depositar(&c, 100)`. Por
      eso, después de compilar, no existe tal "método": existen funciones
      que toman un primer argumento oculto.

      `*this` es el objeto apuntado por this. `return *this;` en un
      método que declara devolver `Cuenta&` devuelve una referencia al
      objeto actual (no una copia). `this` se copia si llamás por
      referencia a un método const: el puntero cambia de tipo a
      `const Cuenta*`.

   3. ERRORES COMUNES
      - Confundir `this` con el objeto: `this` es un PUNTERO, el objeto
        es `*this`.
      - Devolver `&otro_objeto` donde se pidió `Cuenta&`: devuelve
        referencia a algo que no era this. Puede colgar si ese objeto era
        local.
      - Usar el nombre del parámetro igual al del atributo y esperar que
        gane el atributo: gana el parámetro (scope local primero):

          void setSaldo(double saldo) { saldo = saldo; }   //  no cambia nada
          void setSaldo(double s)     { m_saldo = s; }     //  bien
          // o, con el prefijo m_, no chocan nunca.

      - Llamar a un método sobre un objeto temporal y devolver referencia
        a ese temporal: la referencia muere con la línea.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/this
   ============================================================================= */
#include <iostream>

class Textura {
public:
    // Cadena de configuración: cada set devuelve *this por referencia.
    Textura& setNombre(const char* nombre) {
        m_nombre = nombre;
        return *this;
    }
    Textura& setTamano(int ancho, int alto) {
        m_ancho = ancho;
        m_alto = alto;
        return *this;
    }
    Textura& setCanalAlpha(bool enable) {
        m_conAlpha = enable;
        return *this;
    }

    int memoria() const { return m_ancho * m_alto * 4; }

    void reporte() const {
        std::cout << "  '" << m_nombre << "' " << m_ancho << "x" << m_alto
                  << " (alpha: " << (m_conAlpha ? "si" : "no")
                  << "), ~" << memoria() << " bytes\n";
    }

    // "Fusión": recibe otra Textura del mismo tipo y usa this para
    // distinguir sus atributos de los del parámetro.
    void absorber(const Textura& otra) {
        m_ancho = (m_ancho >= otra.m_ancho) ? m_ancho : otra.m_ancho;
        m_alto  = (m_alto >= otra.m_alto) ? m_alto : otra.m_alto;
        if (otra.m_conAlpha) m_conAlpha = true;
    }

private:
    const char* m_nombre = "sin nombre";
    int m_ancho = 0;
    int m_alto = 0;
    bool m_conAlpha = false;
};

int main() {
    std::cout << "=== 1. CADENAR MÉTODOS: cada set devuelve *this ===\n";
    Textura suelo;
    suelo.setNombre("suelo.png").setTamano(64, 64).setCanalAlpha(false);
    suelo.reporte();

    std::cout << "\n=== 2. this-> para distinguir miembros de parámetros ===\n";
    Textura fondo;
    fondo.setNombre("fondo.png").setTamano(512, 512).setCanalAlpha(true);
    Textura sprite;
    sprite.setNombre("sprite.png").setTamano(32, 32);
    sprite.absorber(fondo);     // sprite queda con el tamaño del más grande
    sprite.reporte();

    std::cout << "\n=== 3. *this vs this ===\n";
    std::cout << "sizeof(Textura) = " << sizeof(Textura)
              << ", sizeof una Textura* = " << sizeof(&suelo) << "\n";
    std::cout <<("(el puntero pesa 8 bytes en x86-64; el objeto pesa la suma\n"
                 "de sus atributos + relleno)\n");
    return 0;
}
