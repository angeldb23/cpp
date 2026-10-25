/* =============================================================================
   MÓDULO 05 - LECCIÓN 15: Operaciones especiales con = default y = delete
   =============================================================================
   CONTEXTO: sabés declarar las operaciones especiales a mano (lecciones
   05, 10, 11, 12, 13). C++11 introduce dos formas de PEDIRLE o
   QUITARLE estas operaciones al compilador.

   1. EXPLICACIÓN CONCEPTUAL
      `= default`: "generala como si no la hubiera tocado."
      Útil cuando declaraste ALGÚN constructor y además querés el
      por defecto (lección 05: al declarar uno parametrizado ya no se
      genera implícitamente):

          class Reloj {
          public:
              Reloj() = default;            // por defecto, generado
              Reloj(int hora);              // parametrizado propio
          };

      Sirve también para pedir copia/movimiento generados DESPUÉS de la
      declaración:
          Reloj(const Reloj&) = default;

      = delete: "esta operación NO existe."

          class Valor {
              // prohibir la copia (identidad: el objeto ES este valor)
              Valor(const Valor&) = delete;
              Valor& operator=(const Valor&) = delete;
          public:
              Valor(int v) : m_v(v) {}
          private:
              int m_v;
          };
          Valor a(1);
          Valor b = a;    // ERROR DE COMPILACIÓN: copia borrada

      Cuando no quieras que algo exista, delete es la forma correcta.
      Antes de C++11 se usaba el truco de declararla privada y no
      definirla: delete se lo dice al compilador y da mensajes de error
      claros.

      Usos típicos de delete:
      - Clases "de identidad única": copiar no tiene sentido.
      - Prohibir copia para recursos no copiables (la clase dueña de un
        mutex, un socket, un archivo: por eso std::mutex no se copia).
      - Borrar SOBRECARGAS peligrosas (no queremos dinero de "99.99"
        redondeado en silencio):
          Dinero(int);
          Dinero(double) = delete;   // "no acepto floats"

   2. ¿QUÉ PASA POR DEBAJO?
      Las funciones deleteadas se eligen en el overload y recién ahí
      se queja el compilador: "use of deleted function". No existe el
      símbolo en el binario: el error es al momento de escribir el
      código, no al linkar.

      `= default` deja que el compilador genere la operación con la
      MISMA semántica que tendría implícita (y a menudo trivial y
      constexpr si se puede). Es distinto de declarar un cuerpo vacío
      `Reloj() {}`:
      - `= default` conserva las propiedades especiales (trivial).
      - `Reloj() {}` es un constructor "escrito por vos": el tipo queda
        no-trivial y pierde propiedades.

   3. ERRORES COMUNES
      - `= delete` en el constructor por defecto y esperar objetos
        por defecto: se borró, no existe.
      - Olvidar que una clase con un miembro no-copiable (un mutex)
        ya pierde su copia implícita: el compilador la deja definida
        como borrada. Escribí `= delete` vos mismo para que quede
        explícito y con mensaje claro.
      - Creer que delete se hereda distinto (llegan detalles en 06).
      - Escribir `= delete` y además una implementación: son excluyentes.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/function#Deleted_functions
      https://en.cppreference.com/w/cpp/language/function#Explicitly-defaulted_functions
   ============================================================================= */
#include <iostream>

class Reloj {
public:
    Reloj() = default;                       // reactivado a mano
    explicit Reloj(int hora) : m_hora(hora) {}  // parametrizado propio

    void tic() { m_hora = (m_hora + 1) % 24; }
    int hora() const { return m_hora; }

private:
    int m_hora = 12;    // valor inicial de miembro: no requiere cuerpo del ctor
};

class SensorUnico {                       // tipo de "identidad": no copiable
public:
    explicit SensorUnico(const char* lugar) : m_lugar(lugar) {}

    // Copia prohibida explícitamente:
    SensorUnico(const SensorUnico&) = delete;
    SensorUnico& operator=(const SensorUnico&) = delete;

    void leer() { ++m_lecturas; }
    int lecturas() const { return m_lecturas; }

private:
    const char* m_lugar;
    int m_lecturas = 0;
};

class Dinero {
public:
    explicit Dinero(int pesos) : m_pesos(pesos) {}
    Dinero(double) = delete;      // prohibir construccion desde double
    int pesos() const { return m_pesos; }
private:
    int m_pesos;
};

int main() {
    std::cout << "=== 1. = default: DOS CTORS CONVIVENTES ===\n";
    Reloj porDefecto;               // el default existe otra vez
    Reloj movila(19);
    std::cout << "  porDefecto: " << porDefecto.hora()
              << ", movila: " << movila.hora() << "\n";
    movila.tic();
    std::cout << "  movila tras tic: " << movila.hora() << "\n";

    std::cout << "\n=== 2. = delete: SIN COPIA ===\n";
    SensorUnico puerta("puerta sur");
    puerta.leer();
    puerta.leer();
    std::cout << "  " << puerta.lecturas() << " lecturas\n";
    // SensorUnico duplicado = puerta;   // ERROR: dijo deleted
    std::cout << "  (SensorUnico no se puede copiar: descommentar da error)\n";

    std::cout << "\n=== 3. = delete SOBRECARGA: no sin doubles ===\n";
    Dinero importe(150);
    std::cout << "  importe: " << importe.pesos() << " pesos\n";
    // Dinero importeRaro(99.99);      // ERROR: ctor deleted
    std::cout << "  (la sobrecarga Dinero(double) está deleteada: el\n"
                 " error es claro y no se pierde centavos por silencio)\n";
    return 0;
}
