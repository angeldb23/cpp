/* =============================================================================
   MÓDULO 08 - LECCIÓN 06: Asignación de movimiento, noexcept y regla de 5
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Al constructor de movimiento de la L05 le falta su hermano: la
      ASIGNACIÓN de movimiento, la que se usa cuando el destino YA
      EXISTE:

          destino = std::move(origen);

      Tiene tres diferencias con el constructor:

        1. El destino YA TIENE un recurso. Antes de robar el del
           origen hay que LIBERAR el propio. Si sobreescribís el
           puntero sin hacer delete, fuga directa.
        2. AUTOASIGNACIÓN: `a = std::move(a)` es raro pero posible
           (por accidente, con referencias o alias). No se puede
           hacer delete del recurso propio y después robar el del
           origen SI ES EL MISMO: quedaría apuntando a memoria
           liberada. La forma manual usa el chequeo `this != &otro`;
           la forma con swap lo hace innecesario (abajo).
        3. Devuelve `T&` (`return *this;`) para permitir cadenas
           `a = b = std::move(c);`. Igual que su hermana, la
           asignación de copia del módulo 05.

      noexcept. Acá aparece en su rol más práctico del módulo. La
      firma `T(T&&) noexcept` es una PROMESA COMPILADA: este
      movimiento no lanza excepciones. ¿Por qué importa tanto? Por el
      guardián invisible: std::vector. Cuando un vector crece y
      realoca, transporta sus elementos SIEMPRE que el movimiento sea
      noexcept; si no lo es, CAUTELA: copia (porque si un movimiento
      fallara a mitad de la realocación, el vector quedaría a medio
      robar y ahí sí no hay vuelta atrás). Con noexcept el "move en la
      realocación" está garantizado; sin él, todo tu trabajo de
      escribir un constructor de movimiento eficiente puede caer a
      copias silenciosas. La L10 lo mide con contadores.

      Con esto la REGLA DE 5 queda completa. Las cinco operaciones:

          ~T()                          // 1 destructor
          T(const T&)                   // 2 constructor de copia
          T& operator=(const T&)        // 3 asignación de copia
          T(T&&) noexcept               // 4 constructor de movimiento
          T& operator=(T&&) noexcept    // 5 asignación de movimiento

      Y la forma corta de producción: si tu clase tiene un `swap`
      barato (intercambiar dos palabras), la asignación de movimiento
      se reduce a:
          T& operator=(T&& otro) noexcept {
              swap(otro);   // el recurso nuevo pasa a this; el viejo
                            // muere con `otro` al salir de la función
              return *this;
          }
      Elegante porque el recurso VIEJO del destino ni siquiera se
      libera manualmente: queda "guardado" dentro de `otro`, y el
      destructor del otro lo libera. Y es segura ante autoasignación:
      intercambiar algo consigo mismo no rompe nada.

    2. ¿QUÉ PASA POR DEBAJO?
      `noexcept` no es un comentario: es parte del sistema de tipos. El
      compilador lo guarda en la firma y `noexcept(expresion)` (el
      operador, con esta sintaxis exacta) pregunta en tiempo de
      compilación si una expresión está declarada sin lanzar. El
      vector (y otras partes de la STL) consultan ese resultado con
      traits: la pregunta baja a `std::move_if_noexcept`. Para
      nuestro alcance: declará noexcept SIEMPRE en movimientos y
      destructores; las librerías dependen de esa promesa.

    3. ERRORES COMUNES
      - Olvidar el noexcept: el move "funciona" en tu código de prueba
        pero los contenedores copian en silencio cuando crecen. El bug
        más largo de encontrar: todo pasa, y pierde rendimiento
        invisiblemente. (Detectable midiendo: L10.)
      - Asignación de movimiento SIN liberar el recurso propio ANTES
        del robo: fuga en cada asignación.
      - Asignación de movimiento sin chequear autoasignación (en
        la versión manual): `delete[] datos_; datos_ = otro.datos_;`
        con otro == this deja el objeto apuntando a memoria liberada.
        El guard `if (this != &otro) { ... }` lo resuelve.

    4. DEMO: asignación de movimiento a mano + noexcept a la vista
   ============================================================================
 */
#include <cstddef>
#include <iostream>
#include <utility>

inline int allocationsEnVivo = 0;

class MuestrasAudio {
public:
    explicit MuestrasAudio(std::size_t n)
        : datos_(new float[n]{}), tamanio_(n) {
        ++allocationsEnVivo;
    }

    MuestrasAudio(const MuestrasAudio& otro)
        : datos_(new float[otro.tamanio_]), tamanio_(otro.tamanio_) {
        ++allocationsEnVivo;
        for (std::size_t i = 0; i < tamanio_; ++i) datos_[i] = otro.datos_[i];
    }

    MuestrasAudio(MuestrasAudio&& otro) noexcept          // L05: ctor move
        : datos_(otro.datos_), tamanio_(otro.tamanio_) {
        otro.datos_   = nullptr;
        otro.tamanio_ = 0;
    }

    ~MuestrasAudio() { delete[] datos_; }

    // LA ESTRELLA: asignación de movimiento (regla de 5, op. nº 5)
    MuestrasAudio& operator=(MuestrasAudio&& otro) noexcept {
        if (this != &otro) {                 // guard contra autoasignación
            std::cout << "  [asignacion move] libero lo propio y robo\n";
            delete[] datos_;                 // 1) libero el recurso propio
            datos_   = otro.datos_;          // 2) robo las palabras
            tamanio_ = otro.tamanio_;
            otro.datos_   = nullptr;         // 3) origen vacío (L05)
            otro.tamanio_ = 0;
        }
        return *this;                        // 4) habilita a = b = std::move(c)
    }

    std::size_t tamanio() const { return tamanio_; }
    float muestra(std::size_t i) const { return datos_[i]; }
    void guardo(std::size_t i, float v) { if (i < tamanio_) datos_[i] = v; }

private:
    float*      datos_;
    std::size_t tamanio_;
};

int main() {
    MuestrasAudio destino(2);                // el destino YA tiene un buffer
    destino.guardo(0, -1.f);
    std::cout << "allocaciones hasta acá: " << allocationsEnVivo << "\n";

    MuestrasAudio origen(5);
    origen.guardo(0, 0.75f);
    std::cout << "allocaciones con el origen: " << allocationsEnVivo << "\n";

    std::cout << "\n-- destino = std::move(origen) --\n";
    destino = std::move(origen);
    std::cout << "  destino[0]=" << destino.muestra(0)
              << ", tamanio " << destino.tamanio() << "\n";
    std::cout << "  origen queda vacio: size=" << origen.tamanio() << "\n";
    std::cout << "allocaciones despues: " << allocationsEnVivo
              << " (no crecio: se movio, no se copio)\n";

    std::cout << "\n-- autoasignacion por accidente (alias) --\n";
    MuestrasAudio& alias = destino;
    destino = std::move(alias);              // this == &otro: el guard corta
    std::cout << "  tras autoasignarse: size=" << destino.tamanio()
              << ", muestra[0]=" << destino.muestra(0) << " (intacto)\n";
    return 0;
}
