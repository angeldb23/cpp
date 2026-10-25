/* =============================================================================
   MÓDULO 06 - LECCIÓN 10: Interfaces puras y diseño por contrato
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Una INTERFAZ pura es una clase abstracta SIN datos:
      - Todos sus métodos son puros (o son utilidades con costo bajo).
      - No tiene atributos: promete QUÉ se puede pedir, no CÓMO ni CON QUÉ.
          class Dibujable {
          public:
              virtual ~Dibujable() = default;
              virtual void         dibujar() const = 0;
              virtual Rectangulo   caixa() const = 0;   // contrato: no se sale
          };

      Diferencia con la clase abstracta data-laden de la L09:
          |   abstracta base        |   interfaz pura              |
          | tiene estado (nombre..) | no tiene estado              |
          | código compartido       | casi sin código              |
          | "es-un" en familia      | "sabe-hacer" (capacidad)     |
      Una clase puede tener UNA base de datos y VARIAS interfaces: el
      abuelo es de datos (una familia de datos), las interfaces muchas.
      Eso se reserva para la L14 (herencia múltiple); acá basta la idea.

      DISEÑO POR CONTRATO (DbC): cada método fija condiciones:
      - PRECONDICIÓN: qué debe cumplirse para LLAMAR (ej: dir > 0).
      - POSTCONDICIÓN: qué garantiza al FINAL (ej: vida >= 0, y el puñal
        rota puesto si se infla).
      - INVARIANTE: qué es verdad siempre que el objeto exista (ej:
        vida entre 0 y maxVida). El encapsulamiento del módulo 05 es la
        mecánica para cumplir invars; la interfaz los DICE.

      En C++ no hay `contract` como en Eiffel: la práctica estándar es
      DOCUMENTAR las pre/post (comentario del header) y ENFORCEar con
      assert/debug o devolver estados inválidos. En interfaces puras, la
      firma ES la mitad del contrato; la otra mitad es documentación.

   2. ¿QUÉ PASA POR DEBAJO?
      Una interfaz pura con destructor virtual suele estar VACÍA en datos:
      su subobjeto ocupa solo el vptr. Por eso, componer clases con varias
      capacidades no infla objetos. Cada interfaz aporta una entrada
      distinta en la vtable (o tablas completas separadas con herencia
      múltiple, a ver).

      El costo real del "contrato" en juego: validar la precondición cada
      frame es caro. En build Debug muchos equipos validan con assert;
      en Release se quitan o se asumen. El diseño asume eso:
      validación fuerte en editores, asserts fuertes en
      runtime raro.

   3. ERRORES COMUNES
      - Ponerle datos a la interfaz "por si acaso": ya no es una interfaz.
      - Contratos implicitos que el llamado no conoce: precondiciones que
        no está por escrito y explota en runtime.
      - Confundir interfaz con clase base general: heredás de Enemigo
        porque ES-UN enemigo; heredás de Dibujable porque SABE dibujar.
        La relación "sabe" en C++ se modela como herencia de interfaz
        pero semánticamente es distinta.
   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/abstract_class
      https://en.wikipedia.org/wiki/Design_by_contract  (DbC)
      https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-int (interfaces)
   ============================================================================= */
#include <cassert>
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// 1. INTERFAZ PURA: sin datos, todas funciones virtuales puras.
// ---------------------------------------------------------------------------
class Dibujable {
public:
    virtual ~Dibujable() = default;
    virtual void dibujar() const = 0;                    // contrato: es visual
    virtual int  ordenZ() const = 0;                     // contrato: 0 = fondo
};

// ---------------------------------------------------------------------------
// 2. La base DEL DOMINIO (con datos) es otra clase.
// ---------------------------------------------------------------------------
class EntidadEscena {
protected:
    int x = 0, y = 0;

public:
    EntidadEscena(int x_, int y_) : x(x_), y(y_) {}
    virtual ~EntidadEscena() = default;

    void mover(int dx, int dy) { x += dx; y += dy; }     // invariante: no neg?
protected:
    int getX() const { return x; }
    int getY() const { return y; }
};

// ---------------------------------------------------------------------------
// 3. Una clase concreta tiene UNA data-base y VARIAS capacidades.
// ---------------------------------------------------------------------------
class Cofre : public EntidadEscena, public Dibujable {
    int orden = 1;

public:
    Cofre(int x_, int y_, int orden_) : EntidadEscena(x_, y_), orden(orden_) {}

    // Contratos de Dibujable:
    void dibujar() const override {
        std::cout << "  [cofre] dibujado en (" << getX() << ", " << getY()
                  << ") con z=" << orden << "\n";
    }
    int ordenZ() const override {
        assert(orden >= 0 && "postcondición: z negativa invalida la escena");
        return orden;
    }
};

// ---------------------------------------------------------------------------
// 4. DbC mini: pre/post en funciones concretas (método regular).
// ---------------------------------------------------------------------------
class Mochila {
    int monedas = 10;

public:
    // PRE: cantidad <= monedas.   POST: monedas >= 0 (invariante).
    bool quitar(int cantidad) {
        if (cantidad < 0 || cantidad > monedas) return false;  // pre rota
        monedas -= cantidad;
        assert(monedas >= 0);
        return true;
    }
    int getMonedas() const { return monedas; }
};

int main() {
    std::cout << "=== 1. CLASE CON DOBLE CAPACIDAD ===\n";
    Cofre cofre(3, 4, 1);
    cofre.mover(1, 0);         // de EntidadEscena
    cofre.dibujar();           // de Dibujable

    std::cout << "\n=== 2. LA INTERFAZ SE USA SIN SABER EL TIPO ===\n";
    Dibujable* d = &cofre;
    d->dibujar();
    std::cout << "  z de la interfaz: " << d->ordenZ() << "\n";

    std::cout << "\n=== 3. CONTRATOS: PRE/POST VISIBLES EN CODIGO ===\n";
    Mochila mochila;
    std::cout << "  quitar(5): " << (mochila.quitar(5) ? "ok" : "denegado")
              << " (queda=" << mochila.getMonedas() << ")\n";
    std::cout << "  quitar(99): " << (mochila.quitar(99) ? "ok" : "denegado")
              << " (la precondicion protege el invariante)\n";
    return 0;
}
