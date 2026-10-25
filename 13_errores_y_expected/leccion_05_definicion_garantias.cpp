/* =============================================================================
   MÓDULO 13 - LECCIÓN 05: las garantías de excepción: básica, fuerte y no-throw
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Hasta acá hablamos del CANAL (cómo se avisa el error). Falta la
       otra mitad: qué le pasa al ESTADO de tu objeto cuando algo lance
       en el medio de la operación. La clave es la invariante (módulo
       05): tu BarraDeVida no puede quedar con valores imposibles y tu
       contenedor no puede quedar con la mitad de los elementos. Las
       garantías de Abrahams lo ordenan en una escalera de tres niveles:

         NO-THROW: la operación no lanza nada y cumple su efecto.
           Es la garantía de las piezas de base: swap de tu clase, el
           move noexcept del módulo 08, escribir un int. El resto de la
           escalera SE CONSTRUYE con piezas no-throw.
         FUERTE: si la operación lanza, el estado queda COMO ANTES de
           la llamada: o sale completa o no pasó nada (transacción).
         BÁSICA: si la operación lanza, el estado queda VÁLIDO (las
           invariantes se respetan) aunque quizá CAMBIÓ: puede quedar
           con otro contenido, pero sigue siendo objeto legal y
           utilizable (y destruible: sin fugas).

       Orden de la regla: pedí lo mínimo que resuelve tu caso. Lo usual
       es básica para las operaciones comunes (es lo que suelo dar una
       STL: push_back sin realloc exitosa es fuerte, pero hay llamadas
       solo básica). Y las piezas no-throw baratas hacen que el resto
       pueda prometer fuerte: eso es la L06 (copy-and-swap).

       SIEMPRE documentá la garantía de cada operación pública (una
       línea). Con eso el que usa la clase sabe si puede retray el
       estado después de una falla o si el objeto quedó inutilizable.
    2. ¿QUÉ PASA POR DEBAJO?
        Cuando una excepción atraviesa un frame, la parte "restante" de
        la función NO se ejecuta; los locales RAII se destruyen en orden
        inverso (unwinding). El resto del estado (lo ya modificado, los
        recursos recién adquiridos que no eran RAII) QUEDA como quedó:
        no hay rollback automático. Por eso: si la operación modifica
        dos cosas y lanza entre ellas, el objeto queda a medias, y la
        garantía depende de cómo ordenaste los pasos:
          (a) ejecutar primero TODO lo que puede lanzar (reservar la
              memoria, validar la entrada, preparar el nuevo estado) y
              recién al final, con piezas no-throw, aplicar los cambios.
        Con ese orden, una operación alcanza la garantía FUERTE sin
        depender de try/catch: el trabajo en cada paso intermedio no
        toca el estado visible del objeto. (b) Si algo de esos pasos
        preliminares falla, no tomamos nada; el objeto nunca entró al
        estado a medias. Así es la selección de piezas: el patrón de
        "copio fuera, aplico el paso no-throw" de la L06.
    3. BUENAS PRÁCTICAS:
        - Ordenar el cuerpo: peligro primero, efectos visibles al final.
        - No confundir no-throw con "que no haya errores": una noexcept
          puede fallar y reportarlo por código (L01): promete que el
          error no sale como excepción.
        - RAII (módulo 07) para cada recurso: con eso la garantía
          básica casi sal sola, y hay escasa necesidad de try/catch
          ad-hoc para "destruir lo que quedó a medias".
        - Garantía fuerte no es gratis (copia trabajo adicional): si el
          caso no la usa, no la prometas.
        - La STL es línea de estudio: swap, y cada operación documentada
          (vector::push_back fuerte en la reubicación SI y SOLO SI el
          move de T es noexcept, recuedo del módulo 08).
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace Leccion05 {

// Faltaste el "recuro que puede fallar": una reserva que simula fail.
int* reservar(std::size_t n) {
    if (n > 1024) throw std::bad_alloc();       // el recurso dice no
    return new int[n];                          // (no hay stock: no se puede
}                                               //  arreglar ADENTRO)

// Una clase con invariante: "suma guarda la suma exacta de agregados".
struct PilaConSuma {
    int suma = 0;
    std::size_t elementos = 0;

    // Antes de pensar en la garantía, mirá el ORDEN del trabajo:
    // los recursos y las actualizaciones NO se mezclan al azar.
    void agregar(int valor) {
        // Cada paso puede lanzar (reservar); la sub de invariantes de
        // "estructura su ROW que fiel suma == valor*elementos" se GUARDAN
        // si se actualiza al FINAL, después de lo que puede lanzar:
        (void)reservar(static_cast<std::size_t>(valor));  // puede lanzar
        ++elementos;                  // con la memoria asegurada, avanzar
        suma += valor;                // último: nada encima puede lanzar
    }
};

// La garantía de la LISTA: "de excepción" simple, un caso común con
// asignación de string: la garantía BÁSICA de la asignación por defecto:
struct GestorLinea {
    std::string texto;

    // Garantía básica: si la asignación lanza, el texto QUEDA igual (la
    // asignación de std::string es fuerte: si falla, no toca el destino).
    // Garantía fuerte manual: copio fuera, y un SWAP no-throw "sube":
    void setLineaFuerte(const std::string& nueva) {
        std::string copia = nueva;    // puede lanzar: nada se toca aquí
        std::swap(texto, copia);      // no-throw: paso final, atómico a la vista
    }
};

} // namespace Leccion05

int main() {
    std::cout << "== no-throw: swap de infraestructura ==\n";
    int x = 1, y = 2;
    std::swap(x, y);
    std::cout << "swap de ints (no-throw): x = " << x << ", y = " << y << '\n';

    std::cout << "== garantía básica: el orden del trabajo ==\n";
    Leccion05::PilaConSuma pila;
    pila.agregar(4);
    pila.agregar(6);
    std::cout << "suma = " << pila.suma << ", elementos = " << pila.elementos << '\n';
    std::cout << "un agregar con valor grande lanza bad_alloc SIN romper nada:\n";
    try {
        pila.agregar(5000);
    } catch (const std::bad_alloc&) {
        std::cout << "bad_alloc atrapado; estado: suma = " << pila.suma
                  << ", elementos = " << pila.elementos << " (aún legal)\n";
    }

    std::cout << "== garantía fuerte: construyo fuera y hago swap ==\n";
    Leccion05::GestorLinea g;
    g.texto = "original";
    g.setLineaFuerte("nueva");
    std::cout << "texto = \"" << g.texto << "\"\n";
    return 0;
}
