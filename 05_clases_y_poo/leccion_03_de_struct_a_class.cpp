/* =============================================================================
   MÓDULO 05 - LECCIÓN 03: De struct a class: miembros, métodos y acceso
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Ya sabés usar `struct` como paquete de datos (módulo 01). Una `class`
      es el siguiente paso: un tipo que agrupa DATOS (atributos) y
      COMPORTAMIENTO (métodos) que opera sobre ellos.

      La única diferencia técnica entre struct y class en C++:
          struct → los miembros son PÚBLICOS por defecto.
          class  → los miembros son PRIVADOS por defecto.
      (Y si uno hereda: public por defecto en struct, private en class.
      La herencia llega en el módulo 06.)

      Convención de uso, no ley:
      - struct para datos simples que se pueden pasar por valor y tocar
        directo: Vector2, Color, Config.
      - class para tipos que protegen invariantes (el saldo no baja de 0;
        la vida no pasa de 100) usando acceso privado + métodos.

      Control de acceso:
          public:    lo que cualquier código puede usar (la interfaz).
          private:   solo lo puede tocar la propia clase (el estado).
          protected: como private, pero también para clases hijas (módulo 06).

      No hay límite de secciones: podés alternar private/public cuantas
      veces quieras, aunque por legibilidad conviene agrupar: una sección
      public arriba y una private abajo.

   2. ¿QUÉ PASA POR DEBAJO?
      El acceso privado es comprobado por el COMPILADOR, no por el
      hardware: los miembros están en la misma memoria, uno al lado del
      otro, con el mismo layout que si fueran públicos (un struct con los
      mismos campos ocupa lo mismo). "private" significa "el compilador
      rechaza este acceso desde afuera". Por ejemplo `c.saldo = -500;`
      no compila si saldo es privado.

      Los métodos no ocupan memoria por objeto: el compilador los convierte
      en funciones normales que reciben un puntero al objeto invisiblemente
      (ese puntero se llama `this` — la lección 04 lo abre). Un objeto
      Cuenta con un double pesa lo mismo que la variable `double` sola
      (con padding).

   3. ERRORES COMUNES
      - Hacer todo public "para que sea fácil": se pierde el encapsulamiento
        y cualquier código puede dejar el objeto en estado inválido.
      - Creer que private protege frente a accesos de bajo nivel: es una
        garantía a nivel de lenguaje, no criptografía.
      - Declarar en la clase y definir afuera con otro nombre del tipo:
        `void Cuenta::deposar(...)` no compila; el nombre tiene que
        coincidir exactamente.
      - Olvidar el `;` final de la definición de la clase: el error
        aparece luego, en otra línea, y desconcierta.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/class
      https://en.cppreference.com/w/cpp/language/access
   ============================================================================= */
#include <iostream>
#include <string>

// =========================================================================
// Un struct: datos públicos, acceso directo, sin invariantes que proteger.
// =========================================================================
struct Vector2 {
    float x = 0.0f;
    float y = 0.0f;

    // Un struct también puede tener métodos (todo lo de esta lección
    // aplica a struct y class por igual):
    float longitud() const { return x * x + y * y; }  // sin raíz: comparo al cuadrado
};

// =========================================================================
// Una class: datos privados y métodos públicos que mantienen invariantes.
// Invariante de esta clase: 0.0 <= m_saldo (nunca negativo).
// =========================================================================
class Cuenta {
public:
    // Interfaz pública: lo que el mundo puede hacer con una Cuenta.
    void depositar(double monto) {
        if (monto > 0.0) {
            m_saldo += monto;
        } else {
            std::cout << "  deposito rechazado (monto <= 0)\n";
        }
    }

    bool extraer(double monto) {
        if (monto <= 0.0 || monto > m_saldo) {
            std::cout << "  extraccion rechazada (saldo: " << m_saldo << ")\n";
            return false;
        }
        m_saldo -=monto;
        return true;
    }

    double saldo() const { return m_saldo; }  // getter: lectura sin escritura

private:
    // El estado vive acá, invisible desde afuera.
    double m_saldo = 0.0;
};

int main() {
    std::cout << "=== 1. STRUCT: DATOS PÚBLICOS ===\n";
    Vector2 pos{3.0f, 4.0f};
    pos.x += 1.0f;                      // acceso directo: es un struct
    std::cout << "pos = (" << pos.x << ", " << pos.y
              << "), longitud^2 = " << pos.longitud() << "\n";

    std::cout << "\n=== 2. CLASS: LA INTERFAZ ES LA ÚNICA PUERTA ===\n";
    Cuenta c;
    // c.m_saldo = -500.0;  // ERROR de compilación: m_saldo es privado
    c.depositar(1000.0);
    c.depositar(-50.0);   // el método decidió: no se aceptan montos negativos
    c.extraer(200.0);
    c.extraer(5000.0);    // el método protege el invariante
    std::cout << "saldo final: " << c.saldo() << "\n";
    std::cout << "(nadie pudo poner el saldo en -500: la única vía de entrada\n"
                 "es un método public que valida. Eso es encapsulamiento.)\n";
    return 0;
}
