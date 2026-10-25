/* =============================================================================
   MÓDULO 05 - LECCIÓN 10: Constructor de copia y copia superficial vs profunda
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Copiar es parte de la vida del lenguaje: pasar por valor, devolver
      por valor, `auto copia = otro;`. Cuando el tipo tiene recursos
      (memoria pedida con new, un archivo, un sprite), preguntarse QUÉ
      significa copiar es obligatorio.

      El constructor de copia:

          Bateria(const Bateria& otra);      // copia desde otro objeto

      Se llama en: `Bateria copia = original;` (con la variable recién
      nacida), al pasar por valor, y al devolver por valor (con matices
      que llegarán en el módulo 08 con copy elision).

      Asignación de copia es OTRA operación (lección 11): cuando la
      variable YA existe.

          Bateria copia = original;      // constructor de copia
          b = original;                  // operator= de copia

      Copia superficial (shallow): se copian los CAMPOS a secas. Copiar
      un puntero copia la dirección: ahora dos objetos apuntan a la MISMA
      memoria. Problemas:
      - Cambiar por uno se ve por el otro (no hay independencia).
      - Los dos destructores liberan la misma memoria: doble free.

      Copia profunda (deep): cada copia tiene su PROPIA copia del
      recurso: se pide memoria nueva en el constructor de copia y se
      copia el contenido.

      La regla: el que pide (o administra) un recurso crudo hace copia
      profunda. Los tipos que no administran recursos (Vector2 con dos
      float) pueden dejar que el compilador genere la copia miembro a
      miembro: eso ES lo que hace la copia implícita.

   2. ¿QUÉ PASA POR DEBAJO?
      La copia implícita del compilador es "copiar cada miembro con su
      propia copy-ctor". Para los tipos sin punteros es exactamente lo
      correcto y gratis de escribir. El problema aparece justo cuando el
      miembro es un RECURSO: el puntero copiado comparte el recurso
      original. Un cuestion de propiedad: quién borra qué cuando mueren
      los objetos.

      ¿Viste la ilusión de "dos buffers de 4 MB" que en realidad comparten
      la misma RAM? Eso es copia superficial: dos objetos, un solo dato.
      Y si ambos destruyen, ambos intentan liberar: undefined behavior.

   3. ERRORES COMUNES
      - Creer que la copia superficial es "más rápida": es correcta
        SOLAMENTE cuando el recurso tiene propiedad compartida real
        (contador de usos: llegan los shared_ptr en el módulo 07).
      - Declarar el ctor de copia por VALOR:
          Bateria(const Bateria otra);     // ERROR de compilación
        Tiene que recibir por referencia (copiar desde una copia por
        valor pede la copia infinita).
      - Cuerpo del constructor de copia que re-asigna en vez de
        inicializar: funciona, pero pierde la oportunidad de construir
        el recurso al tamaño correcto desde el inicio.
      - Copia profunda olvidando copiar TAMBIÉN los escalares
        (m_tamano, flags): queda un recurso del tamaño equivocado.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/copy_constructor
   ============================================================================= */
#include <iostream>

// =========================================================================
// Clase con recurso crudo: la copia superficial está MAL, la profunda OK.
// =========================================================================
class Buffer {
public:
    explicit Buffer(int tamano)
        : m_tamano(tamano), m_datos(new int[tamano] {}) {   // el recurso
        std::cout << "  ctor: recurso nuevo de " << m_tamano << "\n";
    }

    // Constructor de copia: PROFUNDA. Cada copia tiene SU recurso.
    Buffer(const Buffer& otra)
        : m_tamano(otra.m_tamano),
          m_datos(new int[otra.m_tamano]) {
        for (int i = 0; i < m_tamano; ++i) {
            m_datos[i] = otra.m_datos[i];   // copiamos el contenido
        }
        std::cout << "  ctor copia: recurso NUEVO de " << m_tamano << "\n";
    }

    ~Buffer() {
        std::cout << "  dtor: libera recurso de " << m_tamano << "\n";
        delete[] m_datos;   // el recurso es PROPIO, solo este lo libera
    }

    int& operator[](int i) { return m_datos[i]; }
    int const& operator[](int i) const { return m_datos[i]; }
    int tamano() const { return m_tamano; }

private:
    int m_tamano;
    int* m_datos;      // SIN copia profunda, compartiría o liberaría doble
};

// =========================================================================
// Copia implícita: los tipos sin recursos pueden dejarla como está.
// =========================================================================
struct Vector2 {
    float x = 0.0f, y = 0.0f;      // nada de punteros: copia implícita ideal
};

int main() {
    std::cout << "=== 1. COPIA PROFUNDA: DOS RECURSOS INDEPENDIENTES ===\n";
    Buffer original(3);
    original[0] = 10;
    original[1] = 20;
    original[2] = 30;
    Buffer copia = original;     // constructor de copia profunda
    copia[0] = 999;
    std::cout << "  original[0] = " << original[0]
              << "  (cambiar la copia NO modifica el original)\n";

    std::cout << "\n=== 2. LO QUE ROMPERÍA LA COPIA SUPERFICIAL ===\n";
    std::cout << "  (si m_datos se copiara a secas, original y copia\n"
                 " compartirían el mismo bloque; los dos destructores\n"
                 " liberarían DOS VECES la misma memoria: undefined behavior\n"
                 " por eso la copia profunda pide su propio bloque)\n";

    std::cout << "\n=== 3. TIPOS SIN RECURSOS: COPIA IMPLÍCITA ===\n";
    Vector2 a{1.5f, 2.5f};
    Vector2 b = a;
    b.x = 42.0f;
    std::cout << "  a.x = " << a.x << ", b.x = " << b.x << "\n";
    std::cout << "  (floats planos: la copia implícita ya es correcta)\n";
    return 0;
}
