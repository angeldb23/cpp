/* =============================================================================
   MÓDULO 06 - LECCIÓN 08: Destrucción polimórfica en contenedores
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      La lección 07 arregló el delete por un solo puntero. Pero en un juego
      los objetos polimórficos viven en COLECCIONES: un vector de enemigos,
      una lista de widgets. Con punteros crudos hay que responder dos
      preguntas: quién DESTRUYE cada elemento y CUÁNDO.

      a) El patrón correcto con lo que sabemos hoy:
             std::vector<Enemigo*> mapa;
             // ... se agregan y se usan ...
             for (Enemigo* e : mapa) delete e;   // cada uno, por la base virtual
             mapa.clear();                       // opcional: por cero

      b) ¿Por qué es peligroso igual? Porque YA NO SÉ, mirando el vector,
         si el dueño de los punteros es él o alguien más:
           - Si otro código también tiene esos punteros, y ambos hacen
             delete: doble liberación (UB).
           - Si el vector tiene copias del MISMO puntero, dos delete: UB.
           - Si se puede exceder el punto de destrucción: fuga.
         Ese problema no es de la herencia: es de la propiedad de la
         memoria. Se resuelve con RAII y smart pointers (módulo 07); acá
         aprendemos a RECONOCER el problema para no normalizarlo.

      c) La regla de uso del curso para módulos previos a 07:
           - La clase dueña que guarda objetos polimórficos crudos es
             el DUEÑO: crea con `new` y destruye en su destructor o en el
             punto lógico de fin de vida (aunque sea feo, es explicito).
           - Nadie más toca el delete de esos mismos punteros.
           - Todo `new` tiene UN `delete` responsable (regla del módulo 04).

      d) Lo que ya podés hacer mejor incluso sin smart pointers:
         Si la vida de la colección termina con el objeto dueño y el dueño
         vive en el stack, los deletes en el destructor de la clase dueño
         son el paso natural RAII: método Dtor que limpia.
         (En el módulo 05 viste RAII con recursos propios; acá se aplica a
         objetos polimórficos.)

   2. ¿QUÉ PASA POR DEBAJO?
      delete por la base virtual llama al destructor del tipo dinámico y
      libera el bloque del tamaño CORRECTO (la base virtual también instala
      el "deleting dtor" que pasa el tamaño real). El vector solo guarda
      punteros: su clear NO toca la vida de lo que se apunta.

      El clear borra los punteros (la "dirección"), no los objetos. Si
      después de clear aún haces delete de esos punteros crudos guardados
      en otra parte, vuelve el doble liberé.

   3. ERRORES COMUNES
      - `std::vector<Enemigo>` con clases derivadas por valor: cada push
        SE CORTA la derivada (slicing, L13) y el polimorfismo no existe más.
      - Limpiar sin dueño claro: llamar delete desde dos lugares.
      - Hacer `mapa.clear()` y CREER que destruye los objetos.
      - delete con el base destructor no virtual: recuerda
        `virtual ~Enemigo() = default;` con casi ninguna excepción.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/container/vector (semántica de,size)
      https://en.cppreference.com/w/cpp/language/raii  (para el módulo 07)
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// 1. Jerarquía mínima, con destructor virtual y contando recursos.
// ---------------------------------------------------------------------------
class Enemigo {
protected:
    std::string nombre;

public:
    explicit Enemigo(std::string n) : nombre(std::move(n)) {}
    virtual ~Enemigo() {
        std::cout << "  -" << nombre << "\n";
    }
    virtual void atacar() const = 0;
};

class Goblin : public Enemigo {
public:
    Goblin() : Enemigo("goblin") {}
    void atacar() const override { std::cout << "  golpe de goblin\n"; }
};

class BossDragon : public Enemigo {
public:
    BossDragon() : Enemigo("boss dragon") {}
    void atacar() const override { std::cout << "  aliento del boss\n"; }
};

// ---------------------------------------------------------------------------
// 2. La clase dueña: todo `new` de la colección se limpia EN SU DESTRUCTOR.
// ---------------------------------------------------------------------------
class Escena {
    std::vector<Enemigo*> enemigos;      // dueña de los punteros

public:
    void agregar(Enemigo* e) { enemigos.push_back(e); }

    void ciclo() const {
        for (const Enemigo* e : enemigos) e->atacar();
    }

    ~Escena() {
        std::cout << "  ~Escena: liberando " << enemigos.size() << " entidades\n";
        for (Enemigo* e : enemigos) delete e;
    }
};

// ---------------------------------------------------------------------------
// 3. Demostración.
// ---------------------------------------------------------------------------
int main() {
    std::cout << "=== 1. ESCENA QUE ES DUEÑA DE SUS ENTIDADES ===\n";
    {
        Escena escena;
        escena.agregar(new Goblin{});
        escena.agregar(new BossDragon{});
        escena.ciclo();
    } // sale del bloque: ~Escena libera items al salir (RAII del dueño)

    std::cout << "\n=== 2. LO QUE EVITA TRAMPAS: DUEÑO ÚNICO POR PUNTERO ===\n";
    std::cout << "  (sin RAII esto es manual: una base, un delete, en un lugar)\n";
    return 0;
}
