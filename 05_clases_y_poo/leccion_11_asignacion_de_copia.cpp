/* =============================================================================
   MÓDULO 05 - LECCIÓN 11: Asignación de copia (operator=)
   =============================================================================
   CONTEXTO: ya sabés construir una copia (lección 10). La asignación es
   otra historia: el objeto receptor YA EXISTE y tiene un recurso por
   liberar antes de aceptar el nuevo.

   1. EXPLICACIÓN CONCEPTUAL
      El operator= de copia se llama cuando una variable existente recibe
      el valor de otra:

          Buffer b(4);
          Buffer c(8);
          c = b;        // operator= (c ya existía)

      Firma canónica:

          Buffer& operator=(const Buffer& otra) {
              if (this == &otra) return *this;   // 1. auto-asignación
              // 2. liberar el recurso propio viejo
              // 3. copiar el recurso y los datos desde otra
              return *this;                      // 4. devolver *this
          }

      Pasos fundamentales (y por qué):
      1) Auto-asignación (b = b): sin el chequeo, liberarías el recurso y
         después lo copiarías desde el (ya liberado): undefined behavior.
      2) El receptor ya tiene un recurso: hay que liberar ANTES de
         copiar, o vas a dejar tirada memoria.
      3) Copiar tamaño y contenido.
      4) `return *this;` es la convención que permite encadenar
         (a = b = c) y es así usualmente sin replantearte nada.

      OJO: constructor y asignación parecen iguales pero son momentos
      distintos de la vida:
          Buffer c = b;    // NACE una variable: constructor de copia
          c = b;           // YA EXISTE  : operator= de copia

   2. ¿QUÉ PASA POR DEBAJO?
      Con `c = b;` no hay "nuevo objeto": se llama la función operator=
      con this = &c y otra = b. La expresión devuelve `*this` (una
      referencia), que es lo que permite `a = (c = b)`. El compilador
      igual detiene definiciones absurdas (hacerla const hace que no
      puedas reasignar: no conviene).

      El patrón clásico "copy-and-swap" (se verá completo en el módulo
      13) escribe operator= así: se pasa el parámetro POR VALOR (se copia
      una vez), y al final se intercambia con el propio: corto,
      a pruebas de balas con la auto-asignación y con garantía fuerte.

   3. ERRORES COMUNES
      - No manejar auto-asignación: b = b libera y usa lo liberado.
      - Devolver void: se pierde el encadenamiento (a = b = c).
      - No liberar el recurso viejo: fuga de memoria.
      - Usar memcpy para tipos con destructor: copia bits y comparte
        recursos sin avisar (la lección 10 explica exactamente por qué).
      - Olvidar que los miembros const o referencias hacen imposible el
        operator= generado por el compilador (no se generan).

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/copy_assignment
      https://en.cppreference.com/w/cpp/language/operators
   ============================================================================= */
#include <iostream>

class Zona {
public:
    explicit Zona(const char* nombre)
        : m_nombre(nombre), m_danio(0) {
        std::cout << "  ctor: '" << m_nombre << "'\n";
    }

    // Asignación de copia: el receptor ya existe.
    Zona& operator=(const Zona& otra) {
        std::cout << "  operator= (de '" << m_nombre << "' a '"
                  << otra.m_nombre << "')\n";
        if (this == &otra) {                    // 1. auto-asignación
            std::cout << "  (auto-asignación: no cambió nada)\n";
            return *this;
        }
        m_danio = otra.m_danio;                 // copiar los datos
        // m_nombre no se copiaría si fuera la parte "identidad";
        // en este ejemplo predes copiarlo para mostrar la operación:
        m_nombre = otra.m_nombre;
        return *this;                            // 4. encadenable
    }

    Zona& danificar(int danio) {
        m_danio += danio;
        return *this;
    }

    void reporte() const {
        std::cout << "  '" << m_nombre << "' acumuló " << m_danio << "\n";
    }

private:
    const char* m_nombre;
    int m_danio;
};

int main() {
    std::cout << "=== 1. ASIGNACIÓN (RECEPTOR YA EXISTENTE) ===\n";
    Zona puente("puente");
    Zona aldea("aldea");
    puente.danificar(30).danificar(10).reporte();   // encadenado de métodos
    std::cout << "\n=== 2. operator= ===\n";
    aldea = puente;
    aldea.reporte();
    std::cout << "\n=== 3. AUTO-ASIGNACIÓN ===\n";
    aldea = aldea;
    aldea.reporte();
    std::cout << "\n=== 4. ENCADENADO: fortin = torre = aldea ===\n";
    Zona torre("torre");
    Zona fortin("fortin");
    fortin = torre = aldea;   // primero torre = aldea, después fortin = torre
    fortin.reporte();
    return 0;
}
