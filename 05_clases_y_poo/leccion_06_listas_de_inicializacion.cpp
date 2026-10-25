/* =============================================================================
   MÓDULO 05 - LECCIÓN 06: Listas de inicialización y orden real de inicialización
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      La lista de inicialización de miembros es la lista que va después de
      los `:` del constructor:

          Jugador(const std::string& nombre, int vida)
              : m_nombre(nombre), m_vida(vida)   // lista de inicialización
          {
              // cuerpo del constructor (arranca DESPUÉS de esto)
          }

      Regla de oro: TODO atributo se inicializa acá cuando se puede; el
      cuerpo ya es "asignar sobre un objeto ya construido". Diferencia:
      
      // En la lista: m_nombre se CONSTRUYE directamente con "Jorge".
      Jugador(std::string n) : m_nombre(n) {}
      
      // En el cuerpo: m_nombre se construye VACÍO y luego se le asigna.
      Jugador(std::string n) { m_nombre = n; }   // costo extra, mismo destino

      Para int/double no se nota casi nunca, pero sí:
      - referencias (no se pueden asignar, solo inicializar),
      - miembros const,
      - objetos pesados (std::string con datos largos, contenedores).

      Reglas del ORDEN: sin importar qué orden uses en la lista, los
      atributos se inicializan en el ORDEN DE DECLARACIÓN de la clase.
      Si escribís `: m_vida(v), m_nombre(nombre)` pero en la clase m_nombre
      está antes que m_vida, el compilador lo hace en otro orden igual y
      gcc lo avisa (con -Wreorder).

      Delegar a constructores (lección 05): un constructor que delega
      NO usa lista de inicialización propia; delega y su cuerpo corre
      después del constructor delegado.

   2. ¿QUÉ PASA POR DEBAJO?
      Recorrido de una construcción: 1) reservar memoria para el objeto,
      2) hacer correr antes los constructores de los miembros (en orden
      de declaración), con los argumentos de la lista — o el constructor
      por defecto si no están listados, y 3) correr el cuerpo. Asignar en
      el cuerpo es "construyo vacío y reemplazo": para std::string con un
      texto largo significa cargar memoria dos veces.

      Cuando el objeto muere, la ejecución corre en la destrucción en
      espejo: primero el cuerpo del destructor, luego los destructores de
      los miembros en orden INVERSO al de declaración. Espejo exacto de
      la construcción (la lección 07 lo prueba).

   3. ERRORES COMUNES
      - Creer que el orden de la lista manda: manda la DECLARACIÓN.
      - Inicializar un miembro CON OTRO que se declara después: usás
        algo que todavía no se inicializó (valores indeterminados).
      - Inicializar referencias o const en el cuerpo: no compila.
          class Pin { const int id; Pin(int i) : id(i) {} };  // la única vía
      - Escribir `Jugador p();` creyendo que llama al constructor por
        defecto: es una DECLARACIÓN DE FUNCIÓN (most vexing parse).
      - Heredar la lista: `: Base(args)` en una lista es otra cosa
        (herencia, módulo 06).

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/initializer_list
      https://en.cppreference.com/w/cpp/language/constructor#Initialization
   ============================================================================= */
#include <iostream>
#include <string>

// =========================================================================
// El orden de la DECLARACIÓN manda sobre el orden de la lista.
// =========================================================================
class Cuartel {
public:
    Cuartel(int alargos, int aliados)
        : m_lejanos(aliados), m_cercanos(alargos)  // lista al revés
    {
        std::cout << "  cuerpo del ctor\n";
    }

    const std::string& describir() const { return m_resumen; }

    // ojo: demostramos el problema antes de la solución
    void reporte() const {
        std::cout << "  lejanos: " << m_lejanos
                  << ", cercanos: " << m_cercanos << "\n";
    }

private:
    // m_cercanos se DECLARA primero y se inicializa primero (aunque en la
    // lista figura al final): gcc avisa con -Wreorder si no coincide.
    int m_cercanos;
    int m_lejanos;
    const std::string m_resumen = "Cuartel central";  // const: init en declaración
};

// =========================================================================
// Miembro const y REFERENCIA: sólo se inicializan en la lista.
// =========================================================================
class PinEquipo {
public:
    PinEquipo(int id, int& contadorGlobal)
        : m_id(id), m_contador(contadorGlobal)   // ref: obligatorio en lista
    {
        ++m_contador;
        std::cout << "  Pin " << m_id << " creado\n";
    }

    int id() const { return m_id; }

private:
    const int m_id;       // const: la única forma de darle valor es la lista
    int& m_contador;      // referencia externa: idem, se liga en la lista
};

int main() {
    std::cout << "=== 1. EL ORDEN DE LA DECLARACIÓN MANDA ===\n";
    Cuartel c(4, 9);
    c.reporte();
    std::cout << "(la lista dice 'lejanos, cercanos' pero se inicializó\n"
                 "'cercanos' primero: la declaración se impone;\n"
                 " gcc avisara con -Wreorder)\n\n";

    std::cout << "=== 2. CONST Y REFERENCIAS CON LISTA ===\n";
    int totalPins = 0;
    PinEquipo p1(10, totalPins);
    PinEquipo p2(11, totalPins);
    std::cout << "  pins creados: " << totalPins << "\n";
    std::cout << "  (m_id es const y m_contador es referencia: solamente la\n"
                 "lista de inicialización puede darle valor a esos miembros)\n";
    return 0;
}
