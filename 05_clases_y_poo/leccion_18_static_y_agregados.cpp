/* =============================================================================
   MÓDULO 05 - LECCIÓN 18: Miembros static, constantes de clase y agregados
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Hasta acá, cada atributo pertenece a UN objeto. Los miembros
      static pertenecen a la CLASE entera: hay uno solo por programa,
      compartido por todos los objetos.

      Miembro static (atributo):
          class Bala {
              static int m_vivas;    // declaración (uno por clase)
          };
          int Bala::m_vivas = 0;     // definición (en el .cpp, fuera)

      Método static:
          class Bala {
              static int vivas() { return m_vivas; }  // sin this
          };
      No recibe this: no puede tocar miembros "de instancia", pero sí
      miembros static. Se puede llamar sin objeto: Bala::vivas().

      Constantes de clase: constante compartida por todos los objetos.
          static constexpr int MAX_VIDA = 100;   // integrales: dentro sí
          static constexpr float GRAVEDAD = 9.8f; // C++17+: inline constexpr
      Regla práctica: los tipos integrales y float con valor constante se
      inicializan dentro de la clase; las constantes pesadas (por
      ejemplo un std::string) necesitan `inline static` o su definición
      separada en el .cpp.

      Agregados y designated initializers (C++20):
          struct Opciones {
              int  velocidad = 60;
              bool pantallaCompleta = false;
              int  volumen = 50;
          };
          Opciones o{ .velocidad = 120, .volumen = 30 };
      Nombrás cada campo, en el orden de declaración, con los que
      quieras. Para struct de configuración es menos propenso a
      equivocarse que las llaves posicionales `{120, 30}`.

   2. ¿QUÉ PASA POR DEBAJO?
      static member: UN solo símbolo en la memoria (no vive dentro de
      cada objeto). sizeof(Enemigo) no cambia si agregás un static int:
      es una variable global cuyo ámbito de acceso es la clase. También
      se puede usar sin crear ningún objeto: Enemigo::vivos().

      El método static se compila como una función sin el parámetro
      oculto this — por eso desde un método static no podés leer los
      atributos normales (no hay this).

      Agregados: estructuras sin constructores declarados y con públicas
      sus miembros: C++ hace la "inicialización agregada": cada elemento
      de las llaves {..} inicializa el miembro correspondiente, en
      orden de declaración, con los restantes tomando su valor por
      defecto. Designated initializers (.campo = valor) agregan
      legibilidad; el orden debe respetar la DECLARACIÓN.

   3. ERRORES COMUNES
      - Olvidar la definición fuera de la clase (int Bala::m_vivas = 0;)
        para static no-constexpr: "undefined reference" al usarlos.
      - Pensar que static de clase = static de archivo (lección 02):
        usan la misma palabra pero el miembro static de clase
        pertenece al TIPO, no al .cpp.
      - Llamar desde métodos static a atributos normales: error de
        compilación (no hay this).
      - Designated initializers fuera de orden (.volumen antes de
        .velocidad): no compila.
      - Usar aggregate init en una clase que declaró constructor:
        ya no es agregado; el {..} posiciónles van al constructor.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/static
      https://en.cppreference.com/w/cpp/language/aggregate_initialization
   ============================================================================= */
#include <iostream>
#include <string>

// =========================================================================
// Miembros static: un contador de instancias compartido por toda la clase.
// =========================================================================
class Enemigo {
public:
    explicit Enemigo(const char* nombre) : m_nombre(nombre) {
        ++m_vivos;
        std::cout << "  nace " << m_nombre << " (vivos: " << m_vivos << ")\n";
    }
    ~Enemigo() {
        --m_vivos;
        std::cout << "  muere " << m_nombre << " (vivos: " << m_vivos << ")\n";
    }

    // Constante de clase: propiedad del TIPO, no de un objeto.
    static constexpr int VIDAS = 1;
    // static constexpr double COOLDOWN = 0.7;  // C++17+: inline constexpr

    // Método static: consulta de clase, sin objeto.
    static int vivos() { return m_vivos; }

private:
    const char* m_nombre;
    static int m_vivos;      // 1) declaración dentro de la clase
};
int Enemigo::m_vivos = 0;    // 2) definición única, fuera

// =========================================================================
// Agregado con initializers designated (C++20): configuración clara.
// =========================================================================
struct Opciones {
    int  fps = 60;         // valores por defecto sensatos
    bool pantallaCompleta = false;
    int  volumen = 50;
    std::string perfil = "jugador";
};

// =========================================================================
// Agregado con constantes de clase y métodos static, típicos de librería.
// =========================================================================
struct Colisiones {
    static constexpr int CAPA_NINGUNA = 0;   // clases de capa: bitmask simple
    static constexpr int CAPA_PISO = 1;
    static constexpr int CAPA_JUEGOS = 2;

    static bool choca(int capaA, int capaB) {
        return capaA != CAPA_NINGUNA && capaB != CAPA_NINGUNA;
    }
};

int main() {
    std::cout << "=== 1. STATIC QUE CUENTA INSTANCIAS ===\n";
    {
        Enemigo a("goblin");
        Enemigo b("troll");
        std::cout << "  dentro del bloque: vivos() = " << Enemigo::vivos() << "\n";
    }
    std::cout << "  tras el bloque: vivos() = " << Enemigo::vivos() << "\n";

    std::cout << "\n=== 2. CONSTANTES DE CLASE ===\n";
    std::cout << "  Enemigo::VIDAS = " << Enemigo::VIDAS << "\n";
    std::cout << "  choca(PISO, PISO): "
              << Colisiones::choca(Colisiones::CAPA_PISO, Colisiones::CAPA_PISO)
              << "\n";

    std::cout << "\n=== 3. AGREGADO CON DESIGNATED INITIALIZERS ===\n";
    Opciones op{ .fps = 120, .volumen = 30 };     // el resto, default
    std::cout << "  fps: " << op.fps << ", pantalla: "
              << (op.pantallaCompleta ? "sí" : "no") << ", volumen: "
              << op.volumen << ", perfil: " << op.perfil << "\n";
    // Opciones roto{ .volumen = 30, .fps = 60 }; // ERROR: fuera de orden
    return 0;
}
