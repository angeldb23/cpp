/* =============================================================================
   MÓDULO 08 - LECCIÓN 05: El constructor de movimiento, escrito a mano
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Hasta acá movimos tipos de la STL, que ya traen su constructor de
      movimiento regalado. Ahora lo escribimos NOSOTROS para un tipo
      con memoria dinámica cruda (el clásico de los módulos 04 y 05),
      porque ahí se ve EXACTAMENTE qué hace y qué deja.

      El contrato completo de un constructor de movimiento:

          T(T&& otro) noexcept  { ... }

      ¿Qué tiene que hacer?
        1. Apropiarse de cada "recurso interno" del otro: el puntero,
           el tamaño. (Se copian, acá sí, PERO son pocas palabras, no
           el contenido.)
        2. DEJAR AL OTRO EN UN ESTADO SEGURO: el estado canónico es
           "vacío": puntero nullptr, tamaño 0. ¿POR QUÉ? El otro va a
           morir en algún momento y su DESTRUCTOR se ejecuta igual.
           Si dejamos el puntero robado tal cual, el destructor del
           ORIGEN hace delete del buffer que ahora le pertenece al
           DESTINO: doble liberación (módulo 04, "errores clásicos").
           Vaciar el origen es OBLIGATORIO: es la diferencia entre
           mover y robar a lo descuidado.
        3. Hacerlo noexcept: explicado a fondo en la L06; por ahora
           anotalo como "promesa de no lanzar".

      El tipo que muestra la demo (ver más abajo, clase MuestrasAudio)
      escribe las cinco operaciones: copia, movimiento, asignación de
      copia, asignación de movimiento y destructor. Es la REGLA DE 5
      COMPLETA: esta lección se centra en el constructor; la
      asignación va en la L06.

    2. ¿QUÉ PASA POR DEBAJO?
      Con BufferSenal (L03) movimos std::vector, que ya lo trae
      resuelto. Con crudos se ve el mecanismo pelado:

          BufferSenal(BufferSenal&& otro) noexcept
              : datos_(otro.datos_), tamanio_(otro.tamanio_) {
              otro.datos_  = nullptr;    <- el paso obligatorio
              otro.tamanio_ = 0;
          }

      Ojo: `otro.datos_ = nullptr` es una instrucción más del
      constructor de movimiento y NO se puede omitir. El "robo" son
      dos copias de palabras del objeto; la limpieza del otro es lo
      que evita el duelo de destructores. Nada de memoria se reserva:
      los n bytes no viajan ni una sola vez.

      Y mientras: mover de un objeto CONST no es posible. La limpieza
      (`otro.datos_ = nullptr`) necesita ESCRIBIR sobre el origen, y
      una referencia `T&&` solo se liga a no-const. Otra razón por la
      que std::move sobre const cae a copia (L03/L07).

    3. ERRORES COMUNES
      - Olvidar vaciar el origen -> doble delete cuando mueren los dos
        objetos. El bug favorito de este módulo: es silencio... hasta
        que ASan lo ataja (módulo 04).
      - Mover con ámbitos mezclados: `T(T&& otro) noexcept` SIEMPRE
        con `&&` (una `T(const&)` de movimiento no existe).
      - Mover DOS recursos y dejar el tercero: si la clase tiene dos
        buffers, hay que transferir los DOS y vaciar los DOS. Dejar
        uno sin vaciar es otra puerta para la doble liberación. La
        limpieza simétrica de todos los recursos es parte del
        contrato: repásalo recurso por recurso cuando lo escribas.

    4. DEMO: constructor movido a mano, contando allocations
   ============================================================================
 */
#include <cstddef>
#include <iostream>
#include <utility>

inline int allocationsEnVivo = 0;   // contador de new[] de esta demo

// Clase con buffer crudo dinámico: copiar es O(n), mover es O(1).
class MuestrasAudio {
public:
    explicit MuestrasAudio(std::size_t n)
        : datos_(new float[n]{}), tamanio_(n) {
        ++allocationsEnVivo;
        std::cout << "  [ctor]   reservo " << n << " floats\n";
    }

    // Constructor de COPIA: sí, se recorre TODO el arreglo.
    MuestrasAudio(const MuestrasAudio& otro)
        : datos_(new float[otro.tamanio_]), tamanio_(otro.tamanio_) {
        ++allocationsEnVivo;
        for (std::size_t i = 0; i < tamanio_; ++i) datos_[i] = otro.datos_[i];
        std::cout << "  [COPIA]  recorro " << tamanio_ << " floats\n";
    }

    // Constructor de MOVIMIENTO: cero recorridos, cero reservas.
    MuestrasAudio(MuestrasAudio&& otro) noexcept
        : datos_(otro.datos_), tamanio_(otro.tamanio_) {   // robo las palabras
        otro.datos_   = nullptr;    // ORIGEN VACIO: declara que no tiene nada
        otro.tamanio_ = 0;
        std::cout << "  [MOVE]   robo el puntero (0 floats recorridos)\n";
    }

    ~MuestrasAudio() { delete[] datos_; }

    std::size_t tamanio() const { return tamanio_; }
    float muestra(std::size_t i) const { return datos_[i]; }
    void guardo(std::size_t i, float v) { if (i < tamanio_) datos_[i] = v; }

private:
    float*      datos_;
    std::size_t tamanio_;
};

int main() {
    std::cout << "== COPIA: n reservas, n recorridos ==\n";
    MuestrasAudio pistas(4);
    pistas.guardo(0, 0.25f); pistas.guardo(1, 0.5f);
    MuestrasAudio copiaDePistas(pistas);   // O(n): otra reserva
    std::cout << "  copia[1]=" << copiaDePistas.muestra(1)
              << ", y 'pistas' sigue viva con " << pistas.tamanio() << " floats\n";
    std::cout << "  (allocations totales: " << allocationsEnVivo << ")\n";

    std::cout << "\n== MOVE: ninguna reserva, ningún recorrido ==\n";
    MuestrasAudio movidaDePistas(std::move(pistas));  // O(1)
    std::cout << "  movida[1]=" << movidaDePistas.muestra(1)
              << ", 'pistas' queda VACIO (size " << pistas.tamanio() << ")\n";
    std::cout << "  (allocations totales: " << allocationsEnVivo << ": no crecieron)\n";

    std::cout << "\n== y al salir del bloque se destruyen 3 objetos pero SOLO\n"
              << "   hay 2 buffers (uno se robo): ninguna doble liberacion. ==\n";
    return 0;
}
