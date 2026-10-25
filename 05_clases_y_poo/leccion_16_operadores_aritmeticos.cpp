/* =============================================================================
   MÓDULO 05 - LECCIÓN 16: Sobrecarga de operadores aritméticos (+, +=)
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Los operadores son funciones con nombre raro: `operator+`,
      `operator+=`, `operator==`... Podés darles un significado a tus
      tipos, para que un Vector2 se use como un número "de física":
      nada de `suma(v, w)` a mano.

      DOS formas de escribirlos:
      a) MÉTODO miembro: el receptor izquierdo es un objeto de la clase.
          class Vector2 {
              Vector2& operator+=(const Vector2& otro) {   // muta a *this
                  m_x += otro.m_x;
                  m_y += otro.m_y;
                  return *this;     // encadenable e idiomático para +=
              }
          };

      b) FUNCIÓN LIBRE (a veces friend): cuando el izquierdo NO es de la
         clase, o para mantener simetría.
          Vector2 operator+(const Vector2& a, const Vector2& b) {
              return Vector2{a.x + b.x, a.y + b.y};
          }

      Patrón clásico y poco texto repetido: `+=` como miembro (muta) y
      `+` como función libre en términos de `+=`:
          Vector2 operator+(Vector2 copia, const Vector2& otro) {
              copia += otro;      // reutiliza: una sola lógica de suma
              return copia;       // la guarda por valor: podemos mutarla
          }
      Regla: los operadores con efecto combinado (+=, *=, []) suelen ser
      métodos; los simétricos (+, ==) funciones libres.

      Lo que NO se debe sobrecargar: operadores con semántica de puntero
      (&, &&, ,) a menos que precisamente estés en un proceso de punteros
      inteligentes (módulo 07). Y nunca sorprender: `+` debe parecer
      suma, no "perder el archivo".

   2. ¿QUÉ PASA POR DEBAJO?
      `a + b` se convierte en una llamada de función: si `+` es un
      método, `a.operator+(b)`; si es libre, `operator+(a, b)`. El
      compilador no inventa según la semántica matemática: 100% lo que
      escribiste vos. Por eso el cuerpo decide solo.

      Un operador binario miembro recibe UN argumento: el izquierdo es
      this. Un operador binario libre recibe DOS. Un unario (como el
      `-` de inverso) recibe cero si es miembro: el objeto ES el único
      operando.

      Las versiones sin efecto colateral (a + b) se implementan
      recibiendo la copia por valor (Vector2 copia = a), mutando la
      copia y devolviéndola por valor. Es la versión legible; el
      compilador la optimiza (módulo 08: copy elision).

   3. ERRORES COMUNES
      - Devolver referencia a un local:
          Vector2& operator+(const Vector2& a, const Vector2& b) {
              Vector2 r = ...; return r;   // ¡colgante! r muere en la línea
          }
        Se devuelve POR VALOR (la optimización se ocupa).
      - operator+= que no devuelva *this: se pierde el encadenado
        (a += b += c) y el retorno es residuo de usos futuros.
      - Sobrecargar operadores con efecto inesperado (usar `*` para
        "multiplicar trazo pool" cuando significa cross product, etc).
      - Abusar: si el tipo no tiene una analogía obvia ("cañón + cañón
        ¿misiles?"), no lo sobrecargues: texto libre es más claro.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/operators
      C++ Core Guidelines: C.83, C.84 (a binary operator debe ser una
      función free y symmetric).
   ============================================================================= */
#include <iostream>

class Vector2 {
public:
    Vector2(float x = 0.0f, float y = 0.0f) : m_x(x), m_y(y) {}

    // += como método: muta el receptor y devuelve *this.
    Vector2& operator+=(const Vector2& otro) {
        m_x += otro.m_x;
        m_y += otro.m_y;
        return *this;
    }

    // -= y *=: métodos hermanos, con la misma lógica.
    Vector2& operator-=(const Vector2& otro) {
        m_x -= otro.m_x;
        m_y -= otro.m_y;
        return *this;
    }

    // Operador UNARIO (un solo operando): puede ser método sin argumentos.
    Vector2 operator-() const {          // -v: inverso, NO muta a v
        return Vector2(-m_x, -m_y);
    }

    float x() const { return m_x; }
    float y() const { return m_y; }

private:
    float m_x, m_y;
};

// Función libre en términos de +=: una sola lógica de suma.
inline Vector2 operator+(Vector2 a, const Vector2& b) {
    a += b;
    return a;
}

inline Vector2 operator-(Vector2 a, const Vector2& b) {
    a -= b;
    return a;
}

void mostrar(const char* etiqueta, const Vector2& v) {
    std::cout << "  " << etiqueta << " = (" << v.x() << ", " << v.y() << ")\n";
}

int main() {
    std::cout << "=== 1. += COMO MÉTODO (MUTA Y ENCADENA) ===\n";
    Vector2 player(10.0f, 20.0f);
    Vector2 viento(2.0f, -1.0f);
    player += viento;
    player += viento;                // en la práctica: suma fuerza a fuerza
    mostrar("player", player);
    mostrar("viento", viento);

    std::cout << "\n=== 2. + COMO FUNCIÓN LIBRE (SIN EFECTO COLATERAL) ===\n";
    Vector2 a(1, 2), b(3, 4);
    Vector2 c = a + b;
    mostrar("a", a);
    mostrar("b", b);
    mostrar("a + b", c);
    std::cout << "  (a y b quedaron intactos: + no muta a nadie)\n";

    std::cout << "\n=== 3. MENOS UNARIO (MÉTODO SIN ARGUMENTOS) ===\n";
    Vector2 retro = -c;              // unario: un operando, c no cambia
    mostrar("-c", retro);
    mostrar("c", c);
    std::cout << "  (el - unario es método porque el receptor IS el objeto)\n";
    return 0;
}
