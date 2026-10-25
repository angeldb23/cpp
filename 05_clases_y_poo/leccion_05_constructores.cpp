/* =============================================================================
   MÓDULO 05 - LECCIÓN 05: Constructores: por defecto y parametrizados
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Un constructor es el método que se ejecuta automáticamente cuando
      nace un objeto. Su trabajo es dejar el objeto EN UN ESTADO VÁLIDO.
      Características:
      - Tiene el nombre de la clase y NO declara tipo de retorno.
      - Puede haber varios (sobrecarga), como cualquier función.
      - Si no declarás ninguno, el compilador te da el constructor por
        defecto (el que se llama sin argumentos).

          class Enemigo {
          public:
              Enemigo();                  // por defecto (sin argumentos)
              Enemigo(const std::string& nombre, int vida);  // parametrizado
          };

      Regla importante: si declarás ALGÚN constructor parametrizado, el
      compilador deja de generar el por defecto. Si querés ambos, declará
      los dos (o usa `Enemigo() = default;` — lección 15).

      Constructor delegante: un constructor puede llamar a otro para no
      repetir la lógica de inicialización:

          Enemigo(const std::string& nombre)
              : Enemigo(nombre, 100) {}    // delega en el parametrizado

      ¿Cuándo se llama cada uno?
          Enemigo e;              // por defecto
          Enemigo f("Goblin", 5); // parametrizado
          Enemigo ejercito[3];    // un arreglo: tres llamadas por defecto

   2. ¿QUÉ PASA POR DEBAJO?
      "Nace un objeto" tiene tres pasos donde actúan los constructores:
      1) Se reserva memoria (stack para locales, o como decidas vos).
      2) Los constructores de los ATRIBUTOS corren primero, uno por uno,
         en el orden en que están declarados en la clase.
      3) Corre el cuerpo del constructor de la clase.
      Por eso inicializar atributos en la lista de inicialización (lección
      06) no es solo más rápido: evita que un atributo se construya vacío
      sólo para reasignarlo dentro del cuerpo.

      Cuando un objeto muere (salir del scope, delete) se invoca el
      destructor: nacimiento y muerte están balanceados SIEMPRE, lo
      escribas vos o no. Esa garantía es la base del RAII (lección 07).

   3. ERRORES COMUNES
      - `Enemigo e();` no crea un objeto: es la declaración de una función
        que devuelve Enemigo ("most vexing parse"). Se escribe `Enemigo e;`
        o, en C++ moderno, `Enemigo e{};`.
      - Esperar que el constructor por defecto exista cuando declaraste
        otro constructor: ya no existe.
      - Confundir la inicialización con asignación. `Enemigo e("x", 5);`
        CONSTRUYE directamente; `e = Enemigo("x", 5);` construye un
        temporal y lo ASIGNA (copia/moves: módulo 08). Distinto costo,
        distinta semántica.
      - Dejar atributos sin inicializar: int m_vida; vale basura.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/constructor
      https://en.cppreference.com/w/cpp/language/default_initialization
   ============================================================================= */
#include <iostream>
#include <string>

class Enemigo {
public:
    // Constructor por defecto: nombrado explícitamente (buena práctica).
    Enemigo() : m_nombre("desconocido"), m_vida(50) {
        std::cout << "  [ctor por defecto] nace '" << m_nombre << "'\n";
    }

    // Constructor parametrizado y const-reference para strings (módulo 02).
    Enemigo(const std::string& nombre, int vida)
        : m_nombre(nombre), m_vida(vida) {
        std::cout << "  [ctor parametrizado] nace '" << m_nombre
                  << "' con " << m_vida << " HP\n";
    }

    // Constructor delegante: reutiliza el anterior.
    Enemigo(const std::string& nombre) : Enemigo(nombre, 100) {
        std::cout << "  [ctor delegante] uso vida estándar para '"
                  << m_nombre << "'\n";
    }

    void mostrar() const {
        std::cout << "  " << m_nombre << " (" << m_vida << " HP)\n";
    }

private:
    std::string m_nombre;
    int m_vida;
};

// Un constructor por defecto en un struct con miembros con valor inicial.
struct Ficha {
    int fila = 0;          // inicializador default de miembro (C++11)
    int columna = 0;
    std::string etiqueta = "vacia";
};

int main() {
    std::cout << "=== 1. CONSTRUCTOR POR DEFECTO ===\n";
    Enemigo anonimo;
    anonimo.mostrar();

    std::cout << "\n=== 2. CONSTRUCTOR PARAMETRIZADO ===\n";
    Enemigo goblin("Goblin", 30);
    Enemigo boss("DRAGON", 500);

    std::cout << "\n=== 3. CONSTRUCTOR DELEGANTE ===\n";
    Enemigo caballero("Caballero");      // deja la vida estándar
    caballero.mostrar();

    std::cout << "\n=== 4. POD CON INICIALIZADORES DE MIEMBRO ===\n";
    Ficha ficha;        // fiel a los valores por defecto de cada atributo
    std::cout << "  ficha: " << ficha.etiqueta << " en ("
              << ficha.fila << ", " << ficha.columna << ")\n";
    return 0;
}
