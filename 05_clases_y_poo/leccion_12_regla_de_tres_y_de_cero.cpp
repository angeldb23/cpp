/* =============================================================================
   MÓDULO 05 - LECCIÓN 12: Regla de 0 y regla de 3
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Las "operaciones especiales" de una clase son cinco:
      1) constructor por defecto,
      2) destructor,
      3) constructor de copia,
      4) operator= de copia,
      5) constructor de movimiento y operator= de movimiento (módulo 08).

      REGLA DE CERO — la que deberías aplicar por defecto:
      Si el recurso está encapsulado en un objeto que ya sabe soltarse
      solo (std::string, std::vector), VOS NO DECLARÁS NADA. La clase no
      debe manejar recursos crudos: delega. Se llama regla de 0 porque
      declarás cero operaciones especiales.

          class Cofre {
              std::vector<int> monedas;    // sabe soltarse sola
              std::string nombre;          // idem
          public:
              // sin destructor, sin copias propias: TODO lo genera el
              // compilador, y es exactamente lo que querés
          };

      REGLA DE TRES — si tu clase administra UN recurso crudo por su
      cuenta (un int*, un FILE*), tiene que definir TRES en pack:
      1) destructor          → lo libera,
      2) constructor de copia → la copia es profunda (lección 10),
      3) operator= de copia  → idem en asignación (lección 11).
      Definir uno solo de los tres y no los otros deja inconsistencia:
      o violás el destructor, o violás la copia, o violás la asignación.

      ¿Por qué "si definís una" casi seguro hay que definir todas?
      Porque sí definiste un destructor, tu clase administra UN recurso,
      y la copia implícita del compilador entonces comparte el recurso
      de forma superficial (lección 10) → doble liberación al terminar.

   2. ¿QUÉ PASA POR DEBAJO?
      El compilador genera todas las operaciones especiales salvo que
      haya un motivo para no hacerlo:
      - Te definen el de copia miembro a miembro SI TODOS los miembros
        pueden copiarse (un miembro unique_ptr (08) o const ref hace que
        no se genere).
      - El destructor se genera SIEMPRE que sea posible (aunque el
        recurso crudo no se libera: el compilador no sabe nada de tu
        int*).
      Con regla de 0, la copia implícita copia a los miembros con SUS
      copias profundas: el costo de una asignación es lo que cuesten las
      partes, no más.

      Un detalle: si tu clase administra un recurso crudo y querés
      COMPARTIR entre objetos (dos "vistas" del mismo buffer), no uses
      copia superficial escondida: escribí semántica de propiedad
      explícita (quién libera). Eso llega con shared_ptr (módulo 07).

   3. ERRORES COMUNES
      - Clase con int* + destructor pero copia implícita: doble-free.
      - Clase con copia pero sin destructor (la memoria queda viva
        cuando el último objeto muere: fuga).
      - Aplicar la regla de 3 a tipos sin recursos: volumen de código
        al pedo y más chance de bugs.
      - Olvidar operator= cuando tenés ctor de copia: la copia funciona
        en `a = b` pero queda superficial.

   4. PARA PROFUNDIZAR
      C++ Core Guidelines: C.20, C.21, C.22 (the Rule of Zero, 3, 5)
      https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines# Rcdestructo-destructor
   ============================================================================= */
#include <iostream>
#include <vector>

// =========================================================================
// REGLA DE CERO: no administrás recursos crudos y no declarás nada.
// =========================================================================
class Cofre {
public:
    void agregar(int moneda) { m_monedas.push_back(moneda); }
    int total() const { return m_monedas.size(); }
    // NO hay destructor, NO hay constructor de copia, NO hay operator=.
    // El compilador los genera y son correctos porque std::vector
    // administra la memoria por nosotros.
private:
    std::vector<int> m_monedas;
};

// =========================================================================
// REGLA DE TRES: administramos un recurso crudo y definimos los tres.
// =========================================================================
class Espada {
public:
    explicit Espada(int tamano)
        : m_detalles(new int[tamano] {}),
          m_tamano(tamano) {
        std::cout << "  ctor: espada con " << m_tamano << " ranuras\n";
    }

    // 2/3: constructor de copia (profunda).
    Espada(const Espada& otra)
        : m_detalles(new int[otra.m_tamano] {}),
          m_tamano(otra.m_tamano) {
        for (int i = 0; i < m_tamano; ++i) {
            m_detalles[i] = otra.m_detalles[i];
        }
        std::cout << "  ctor copia (profunda)\n";
    }

    // 3/3: asignación de copia (profunda, sin auto-asignación).
    Espada& operator=(const Espada& otra) {
        if (this == &otra) return *this;
        int* nuevos = new int[otra.m_tamano] {};   // pedimos ANTES
        for (int i = 0; i < otra.m_tamano; ++i) {
            nuevos[i] = otra.m_detalles[i];
        }
        delete[] m_detalles;      // liberamos lo propio DESPUÉS
        m_detalles = nuevos;
        m_tamano = otra.m_tamano;
        std::cout << "  operator= (profunda)\n";
        return *this;
    }

    // 1/3: destructor.
    ~Espada() {
        delete[] m_detalles;
        std::cout << "  dtor: liberamos la espada\n";
    }

    void marcar(int ranura, int valor) { m_detalles[ranura] = valor; }

private:
    int* m_detalles;
    int m_tamano;
};

int main() {
    std::cout << "=== 1. REGLA DE CERO: copiar, pasar y soltar SON gratis ===\n";
    Cofre cofre;
    cofre.agregar(25);
    Cofre duplicado = cofre;        // ctor de copia implícito
    duplicado.agregar(50);
    std::cout << "  cofre: " << cofre.total() << " monedas, duplicado: "
              << duplicado.total() << " (independientes)\n";

    std::cout << "\n=== 2. REGLA DE TRES: un recurso crudo vigilado ===\n";
    Espada runica(3);
    runica.marcar(0, 100);
    Espada gemela = runica;         // copia profunda
    gemela.marcar(0, -1);
    Espada rival(2);
    rival = gemela;                 // asignación profunda
    gemela = gemela;                // auto-asignación (sin sorpresas)
    std::cout << "  (todas las rutas de vida cubiertas: dtor + copia + "
                 "operator=)\n";
    return 0;
}
