/* =============================================================================
   MÓDULO 13 - LECCIÓN 06: copy-and-swap: la receta de la garantía fuerte
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       L05 enseñó la receta: "prepará fuera, aplicá con piezas
       no-throw". La versión clásica para la asignación de una clase con
       recursos es copy-and-swap:

         Buffer& Buffer::operator=(Buffer otro) {  // por VALOR
             swap(otro);                            // intercambia (no-throw)
             return *this;                          // `otro` muere con lo viejo
         }

       Los tres pasos, con ojos de módulos anteriores:
         1. El parámetro llega POR VALOR: se copia (o se mueve) ANTES de
            entrar. Si esa construcción lanza, el operador ni arrancó:
            el objeto destino no se tocó. Ese UN paso ya paga la garan-
            tía fuerte (L05).

         2. El swap de clase intercambia los recursos del destino con
            la copia: es noexcept por diseño (L03: es la pieza que
            diseñaste vos sin allocaciones).
         3. Al salir, `otro` -que ahora tiene el recurso VIEJO del
            destino- se destruye: la liberación la hace el dtor (RAII,
            módulo 07), sin try/catch y sin borrar a mano.

       La garantía resultante: o terminó (nuevo estado con la garantía
       fuerte) o lanzó y el objeto quedó EXACTAMENTE como estaba.

       Regalo del diseño:
         - la asignación POR VALOR sirve también como asignación de
           MOVIMIENTO: si el argumento es un rvalue, el parámetro se
           CONSTRUYE con move (módulo 08): barato, y el resto es igual.
           Un solo operador para copia y movimiento. (El std::swap
           genérico NO se usa acá: para no recursar la propia firma.)
         - la copia y el destructor siguen su camino clásico (módulo
           05): la receta solo reemplaza el operator= por asignación.
         - la autoasignación (a = a) no rompe nada: el swap intercambia
           y listo (el clásico `delete propio; copiar` corrigió la
           identidad, pero en copy-and-swap es gratis).

    2. ¿QUÉ PASA POR DEBAJO?
        El mecanismo descansa en que swap NO LANCE: si el swap de la
        clase pide memoria, la receta perdió su valor (queda una falla
        débil: mitad en el destino, mitad en la copia). Por eso el
        swap se marca noexcept (L03) y un static_assert lo documenta.
        El costo del copy-and-swap: copia TODA la información del
        objeto aunque el destino tenga su buffer ya al tamaño: NO
        reutiliza la memoria del destino (libera y alocó otra vez).
        Para asignaciones repetidas en hot loop entre buffers del
        mismo tamaño, el operador clásico (con `if (this == &otro)`,
        delete, new, copiar) puede ser más rápido prometiendo solo
        la garantía BÁSICA. Copy-and-swap promete FUERTE y es
        corto/recto: elegir se puede solo después de saber qué se
        pierde. Documentá en la clase qué garantía entrega cada
        operación (publica: básica / fuerte / no-throw) y qué hizo el
        desempeño decidir: un comentario: la asignación con
        copy-and-swap = fuerte, confirmada dentro de swap noexcept.
    3. BUENAS PRÁCTICAS:
        - copy-and-swap para la asignación de clases con UN recurso de
          tamaño variable (buffer, string, vector): corto, correcto,
          fuerte.
        - No se usa el std::swap genérico adentro del operador (con la
          firma por valor se retorcería); usá el swap MIEMBRO, que es
          el que prometiste noexcept.
        - swap de la clase sí o sí noexcept y probado con static_assert:
          la receta pierde su valor si el swap lanza.
        - Para clases con recursos múltiples (dos buffers,
          textura+resultado), la garantía fuerte no sale gratis por
          copy-and-swap (deben copiarse TODOS los recursos: la L05
          aclaró el costo). Garantía fuerte no es gratis: copia
          trabajo.
        - Cuando el rendimiento neceite prioridad, el operador clásico
          con resize/reusar el buffer, promete BÁSICA, y es aceptable
          MIENTRAS esté documentado.
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace Leccion06 {

// Clase con un recurso crudo: la crítica de opciones de buffer.
class Buffer {
public:
    Buffer() = default;
    explicit Buffer(std::size_t n) : datos_(n ? new int[n]{} : nullptr), tam_(n) {}

    // El constructor de copía: el clásico del módulo 05 (copia profunda).
    Buffer(const Buffer& otro) : datos_(otro.tam_ ? new int[otro.tam_] : nullptr),
                                 tam_(otro.tam_) {
        for (std::size_t i = 0; i < tam_; ++i) datos_[i] = otro.datos_[i];
    }

    // Destructor: el RAII libera... y aquello empuón de la cuenta
    ~Buffer() { delete[] datos_; }

    // Move: el módulo 08 ( la receta del buffer crudo), noexcept (L03-04).
    Buffer(Buffer&& otro) noexcept : datos_(otro.datos_), tam_(otro.tam_) {
        otro.datos_ = nullptr;
        otro.tam_ = 0;
    }

    // swap de clase: la pieza no-throw de la receta (L03).
    void swap(Buffer& otro) noexcept {
        std::swap(datos_, otro.datos_);
        std::swap(tam_, otro.tam_);
    }

    // LA RECETA: asignación copy-and-swap (por VALOR).
    // Un solo operador sirve para copia (lvalue) y movimiento (rvalue):
    // la construcción del parámetro hace el trabajo caro ANTES de tocar
    // *this: si esa construcción lanza, el operador ni se entra.
    Buffer& operator=(Buffer otro) noexcept {
        swap(otro);                 // el swap MIEMBRO (no el std::swap
                                    // genérico: este se anida con la
                                    // firma por valor, acá se espera):
        return *this;               // al salir, `otro` (con el recurso
                                    // viejo) se destruye: RAII, sin try
    }

    std::size_t tam() const { return tam_; }
    int& operator[](std::size_t i) { return datos_[i]; }
    int operator[](std::size_t i) const { return datos_[i]; }

private:
    int* datos_ = nullptr;
    std::size_t tam_ = 0;
};

// helper que amistad la política de std::swap cuando copia-and-swap:
void swap(Buffer& a, Buffer& b) noexcept { a.swap(b); }

} // namespace Leccion06

int main() {
    std::cout << "== asignación copy-and-swap ==\n";
    Leccion06::Buffer a(3);
    a[0] = 1; a[1] = 2; a[2] = 3;
    Leccion06::Buffer b(2);
    b[0] = 40; b[1] = 41;
    a = b;                              // copia el b; el buffer viejo de `a` se libera cruz
    std::cout << "a.tam() = " << a.tam()
              << ", a[0] = " << a[0] << ", a[1] = " << a[1] << '\n';

    std::cout << "== mismas firmas que un movimiento ==\n";
    Leccion06::Buffer c(4);
    c[0] = 9;
    a = std::move(c);                   // mueve c a `a` (c queda vacío: módulo 08)
    std::cout << "a.tam() = " << a.tam() << ", a[0] = " << a[0]
              << "; c.tam() tras mover = " << c.tam() << '\n';

    std::cout << "== asignación a sí misma: no rompe nada ==\n";
    Leccion06::Buffer d(2);
    d[0] = 7;
    d = d;                              // copy-and-swap: seguro (el clásico borraría)
    std::cout << "tras d = d: tam = " << d.tam() << ", d[0] = " << d[0] << '\n';

    std::cout << "== swap de clase la promesa noexcept ==\n";
    static_assert(noexcept(std::declval<Leccion06::Buffer&>()
                           .swap(std::declval<Leccion06::Buffer&>())),
                  "el swap de Buffer NO lanza: la pieza clave de la receta");
    std::cout << "swap de Buffer: noexcept (la pieza de la receta)\n";
    return 0;
}
