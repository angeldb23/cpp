/* =============================================================================
   MÓDULO 06 - LECCIÓN 02: Orden de construcción y destrucción con herencia
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Cuando creás un objeto derivado no se construye "un Mago": se construye
      un objeto que contiene una parte Entidad y una parte Mago. El orden es
      estricto y no negociable:

      CONSTRUCCIÓN (de adentro hacia afuera... pero empezando por la base):
        1. Constructor(es) de SUBCLASE(s) base(s), en el orden de la lista
           `class Mago : public A, private B` (el orden de la lista de
           herencia, NO el orden de la lista de inicialización).
        2. Miembros de datos de la derivada, en el orden de DECLARACIÓN
           (lo mismo que viste en el módulo 05).
        3. Cuerpo del constructor de la derivada.

      DESTRUCCIÓN: exactamente al revés.
        1. Cuerpo del destructor de la derivada.
        2. Miembros de datos de la derivada.
        3. Destructor de la(s) base(s).

      Reglas prácticas:
      a) Si la base no tiene constructor por defecto, la derivada DEBE
         llamarla explícitamente en su lista de inicialización.
      b) Si el destructor de la derivada necesita datos de la parte base,
         todo funciona: al llegar al destructor de la base, la derivada ya
         se destruyó COMPLETA (sus miembros ya no existen).
      c) Nunca dependas del orden para inicializar con datos del hijo:
         cuando corre el ctor de Entidad, el Mago todavía no existe.

   2. ¿QUÉ PASA POR DEBAJO?
      El compilador inyecta llamadas: el constructor de la derivada empieza
      por (si no lo hacés vos) el constructor por defecto de cada base. Si
      la herencia es virtual (L15), la base virtual la construye SIEMPRE la
      clase más derivada. La destrucción respeta el orden inverso por
      diseño: no podés destruir la base de algo que todavía vive.

      El ejemplo imprime cada paso para que veas el orden real, y también
      qué pasa si el ctor de la derivada llama a un método del padre: el
      método corre sobre una parte derivada INCOMPLETA (nombre ya existe,
      dificultad todavía no).

   3. ERRORES COMUNES
      - Creer que el orden de la lista de inicialización define el orden:
        el orden es el de declaración de membresía y herencia. La lista
        solo DIQUÉ valores; no el orden.
      - Llamar a métodos de la derivada (o virtuals) desde la base: si
        Entidad llama a un método virtual, se dispara la versión de la
        base porque la parte derivada no está construida todavía.
      - Depender de la base para inicializar miembros del hijo: por eso
        los valores se pasan por parámetros, no se espián atributos.
      - Doble Inicialización accidental: el ctor de la base hace algo y el
        cuerpo del ctor de la derivada lo repite.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/constructor (Initialization Order)
      https://en.cppreference.com/w/cpp/language/destructor
   ============================================================================= */
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// 1. La base imprime en su ctor y dtor para ver el orden real.
// ---------------------------------------------------------------------------
class Entidad {
protected:
    std::string nombre;

public:
    explicit Entidad(const std::string& nombre_) : nombre(nombre_) {
        std::cout << "  1. ctor Entidad(" << nombre << ")\n";
    }
    virtual ~Entidad() {
        std::cout << "  3. dtor Entidad(" << nombre << ")\n";
    }
    void mostrarBase() const {
        std::cout << "  Entidad::mostrarBase -> nombre: " << nombre << "\n";
    }
};

// ---------------------------------------------------------------------------
// 2. La derivada: primero la base, luego sus miembros (en orden de declaración).
// ---------------------------------------------------------------------------
class Mago : public Entidad {
    int mana = 0;                     // miembro 1 de Mago
    int hechizosConocidos = 0;        // miembro 2 de Mago

public:
    Mago(const std::string& nombre_, int mana_, int hechizos_)
        : Entidad(nombre_),           // SIEMPRE primero: la base
          mana(mana_),                // miembros en orden de declaración
          hechizosConocidos(hechizos_) {
        std::cout << "  2. ctor Mago (mana=" << mana
                  << ", hechizos=" << hechizosConocidos << ")\n";
        // Trampa clásica: llamar a virtuals acá (L04) resuelve la versión
        // de la base. Moo-estricto: esta parte derivada aún está incompleta.
        mostrarBase();                // ok: método no virtual de la base
        // Aviso: usar `mana` acá está bien, pero el compilador ya construyó
        // la base ANTES de llegar a esta línea: no hay nada más que hacer.
    }
    ~Mago() {
        // Al llegar acá, `mana` y `hechizosConocidos` ya se destruyeron.
        std::cout << "  2'. dtor Mago\n";
    }
};

// ---------------------------------------------------------------------------
// 3. Base SIN constructor por defecto: el hijo OBLIGA a llamarla.
// ---------------------------------------------------------------------------
class SinCtorVacio {
public:
    explicit SinCtorVacio(int x) { std::cout << "  ctor SinCtorVacio(" << x << ")\n"; }
};

class Hijo : public SinCtorVacio {
public:
    // Sin la llamada explícita abajo, Hijo no compila: no hay ctor default.
    Hijo() : SinCtorVacio(42) {}
};

int main() {
    std::cout << "=== 1. ORDEN DE CONSTRUCCION ===\n";
    {
        Mago gandalf("Gandalf", 50, 3);
    }
    std::cout << "\n=== 2. ORDEN DE DESTRUCCION (al salir del bloque) ===\n";
    std::cout << "  (ver el bloque anterior: dtor Mago, dtor Entidad)\n";

    std::cout << "\n=== 3. BASE SIN CTOR DEFAULT ===\n";
    {
        Hijo h;
    }
    std::cout << "\n=== 4. RECORDAR: dtor va al revés de ctor ===\n";
    return 0;
}
