/* =============================================================================
   MÓDULO 06 - LECCIÓN 07: Destructor virtual y destrucción polimórfica
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Regla de oro: si una clase va a ser la BASE de una jerarquía y vas
      a destruir objetos por un puntero a la base, su destructor DEBE ser
      virtual. Sin excepción conocida. La regla para clases base de juego: siempre.

      ¿Qué rompe no hacerlo?
          Enemigo* e = new Dragon;
          delete e;                  // sin virtual: COMPORTAMIENTO INDEFINIDO
      delete llama al destructor del tipo ESTÁTICO (Enemigo::~Enemigo).
      La parte Dragon (sus atributos, otros recursos que contenga) NUNCA
      se destruye: fuga de la parte derivada, y con clases que guardan
      memoria, archivos o texturas: recursos hechos pedazos. Con sanitizers
      (ASan) se ve como fuga y como "destructor no virtual eliminado vía
      base".

      ¿Por qué `virtual` lo arregla?
      Porque el destructor "es" el método virtual polimórfico de fábrica:
      el delete mira el vptr del objeto real y entra por la versión de
      Dragon, que a su vez (automáticamente) DISPARA la destrucción desde
      el orden de debajo de ese objeto: Dragon::~Dragon, miembros, y LUEGO
      Enemigo::~Enemigo. La L08 verá esto en contenedores; la L02 mostró
      el orden de construcción y destrucción.

      ¿Cuándo NO hace falta destructor virtual?
      Si nunca vas a destruir por la base (pasá siempre por valor o por
      puntero de la derivada, o no existe delete en su interior). Es el caso
      de prototipos rápidos. La regla para clases base de juego es sí o sí.

   2. ¿QUÉ PASA POR DEBAJO?
      Un destructor virtual ocupa una entrada adicional en la vtable y
      también la "deleting destructor" (D2) que compila `delete p;` con el
      tamaño correcto del objeto real; por eso el compilador pone la
      operación de desasignación apuntando al tipo dinámico.

      Cuidado: el destructor virtual no protege de otros usos del objeto.
      Si asignás a un base por valor, se destruye igualmente mal por
      slicing (L13); el virtual solo protege el `delete` polimórfico.

   3. ERRORES COMUNES
      - Destructor virtual en todas las clases SIN herencia: overkill;
        solo lo necesitan base polimórficas o clases con liberaciones
        especiales.
      - Pensar que un destructor protegido es un reemplazo del virtual:
        es un patrón para interfaces SALTIBALES heredados por terceros y
        con propiedad gestionada a la derecha (va un poco más allá de esta
        lección; en el módulo 07, con smart pointers, vuelve).
      - Confundir "la parte base se destruye" con "todo se libera": solo
        se ejecutan los destructores de la parte que pertenecía al objeto
        real, en orden inverso de construcción.
   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/virtual
      https://en.cppreference.com/w/cpp/language/destructor
      https://en.cppreference.com/w/cpp/language/delete  (operator delete)
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// 1. Base decorada con destructor virtual (y contador de destrucciones).
// ---------------------------------------------------------------------------
class Recurso {
    std::string etiqueta;

public:
    explicit Recurso(std::string et) : etiqueta(std::move(et)) {
        std::cout << "  +Recurso(" << etiqueta << ")\n";
    }
    virtual ~Recurso() {
        std::cout << "  -Recurso(" << etiqueta << ")\n";
    }
};

class TexelBase : public Recurso {
public:
    TexelBase() : Recurso("motor-base") {}
    virtual ~TexelBase() = default;      // con virtual y default: la casa
};

class Textura : public TexelBase {
    // (simulamos un recurso pesado con un miembro propiamente dicho)
    std::vector<int>Buffer = {1, 2, 3, 4};

public:
    ~Textura() override {
        std::cout << "  ~Textura (liberando frames-gpu)\n";
    }
};

// ---------------------------------------------------------------------------
// 2. La demo en tres pasos: correcto, el error y el orden inverso.
// ---------------------------------------------------------------------------
int main() {
    std::cout << "=== 1. DTOR VIRTUAL: delete POR LA BASE FUNCIONA ===\n";
    {
        Recurso* r = new Textura{};      // tipo dinamico: Textura
        delete r;                        // entra por Textura baja el orden
    }

    std::cout << "\n=== 2. SIN VIRTUAL SERIA UB (explicado, NO ejecutado) ===\n";
    std::cout << "  (formato base sin virtual: la parte Textura NUNCA "
                 "se destruye; fuga y recursos de GPU sueltos)\n";

    std::cout << "\n=== 3. ORDEN INVERSO: objetos vivos al mismo tiempo ===\n";
    {
        std::cout << "-- abre: A, B\n";
        Recurso a{"A"};
        Recurso b{"B"};
        std::cout << "-- cierra: B, A (al reves de como se crearon)\n";
    }
    std::cout << "  (los dtors corren en orden inverso de construcción)\n";
    return 0;
}
