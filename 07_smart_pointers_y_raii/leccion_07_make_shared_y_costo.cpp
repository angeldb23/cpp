/* =============================================================================
   MÓDULO 07 - LECCIÓN 07: make_shared: una sola allocación y por qué importa
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Crear un shared_ptr tiene DOS formas posibles y NO son equivalentes:

        A) std::shared_ptr<T> p(new T(args...));
        B) std::shared_ptr<T> p = std::make_shared<T>(args...);

      a) Forma A: DOS allocaciones de heap separadas.
         1) `new T` reserva el objeto T.
         2) el constructor de shared_ptr reserva el bloque de control por
            afuera (contadores + destructor).
         Dos viajes al allocator, dos regiones de memoria distintas: el objeto
         y su bloque quedan DESALINEADOS en el heap (probablemente en páginas
         diferentes del caché).

      b) Forma B: UNA sola allocación.
         make_shared reserva un solo bloque: [ Padding ][ Contadores ][ T ].
         El objeto y el bloque de control viven JUNTOS, contiguos. Ventajas:
           - una llamada al allocator en vez de dos;
           - el contador está "caliente" en la misma cacheline: copiar o
             destruir un shared_ptr toca memoria cercana al objeto;
           - seguro en orden de evaluación: make_unique/make_shared reciben
             los args y hacen el new ADENTRO: no puede existir el error
             `f(std::shared_ptr<T>(new T), g())` en el que g() lanza ANTES de
             que el shared_ptr adopte el new: fuga garantizada (módulo 04).

      c) El costo que SI se paga: sizeof(shared_ptr<T>) = 2 punteros (16B):
         objeto + bloque. Copiar cuesta un incremento atómico del contador.
         make_shared reduce a la mitad las allocaciones, no la indirección:
         seguimos pagando el ControlBlock* por cada dueño.

      d) La trampa clásica de make_shared (+ weak_ptr): como bloque y objeto
         comparten la MISMA allocación, la memoria del objeto no puede
         liberarse hasta que NI los dueños fuertes NI los DÉBILES (weak_ptr,
         L08) terminen. Un weak_ptr pendiente mantiene "viva" (alquilada) la
         memoria del objeto (sin llamar a su dtor). Para objetos de juego
         muy grandes con muchos observadores débiles, shared_ptr<T>(new T)
         (forma A) libera el objeto antes: es el único argumento técnico a
         favor de A. Casi nunca importa; lo anotamos.
         (El detalle de los contadores débiles se ve completo en la L08-L09:
         acá solo se nombra.)

      e) Otras fábricas de la familia STL:
         - make_unique (C++14: L04) y make_shared (C++11).
         - C++20 agrega make_unique_for_overwrite / make_shared_for_overwrite:
           reservan memoria SIN inicializar, útiles para buffers grandes
           donde no hace falta construir cada byte.
         Regla del curso: SIEMPRE la fábrica; nunca `shared_ptr(new T)`.

   2. ¿QUÉ PASA POR DEBAJO?
      El bloque de control (con make_shared o no) tiene, en esencia, DOS
      contadores atómicos:
        - contador FUERTE: cuántos dueños vivos hay.
        - contador DÉBIL: cuántos observadores (weak_ptr, L08) quedan.
      Las reglas de vida del bloque:
        - strong llega a 0  ->  corre el destructor del objeto.
        - strong y weak en 0  ->  se libera el bloque entero (free).
      El detalle cambia con make_shared: porque objeto y bloque comparten
      UNA sola allocación, no se puede liberar "la parte del objeto" sin
      liberar el bloque: por eso, con make_shared, la MEMORIA del objeto
      vive hasta que también los débiles terminan (aunque el dtor del
      objeto ya corrió). Con la forma A, el bloque y el objeto son
      allocaciones independientes y el objeto queda liberado antes.

      El incremento/decremento es atomic_fetch_add/sub (instructiones 'lock'
      en x86). No es el fin del mundo, es solo que no es gratis: si un
      cargador copia 100k shared_ptr, ese trabajo existe y se paga.

   3. ERRORES COMUNES
      - new T a mano "por costumbre": doble allocación SIN motivo y con el
        riesgo de fuga del caso g().
      - ESPERAR el mismo layout con ambos caminos: A es más sencillo (dos
        piezas independientes); B es más inodoro (una).
      - Olvidar que con B la memoria vive mientras haya weak: grande e
        inmutable, cacheado... se nota (se retoma en la L08-L09).
      - Creer que make_shared evita la COPIA pesada: no, evita la allocación
        extra. La copia sigue haciendo el incremento atómico.

   4. DEMO: usar make_shared y ver que todo vive y muere en orden
 */
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Item {

public:
    explicit Item(std::string n) : nombre_(std::move(n)) {
        std::cout << "    [ctor ] " << nombre_ << "\n";
    }
    ~Item() { std::cout << "    [dtor ] " << nombre_ << "\n"; }
    const std::string& nombre() const { return nombre_; }

private:
    std::string nombre_;
};

int main() {
    std::cout << "=== 1. MAKE_SHARED: CREACION DIRECTA ===\n";
    {
        auto nada = std::make_shared<Item>("moneda.png");
        std::cout << "    use_count: " << nada.use_count() << "\n";
        std::cout << "    sizeof(shared_ptr<Item>) = " << sizeof(nada)
                  << " B (2 punteros: obj + control block)\n";
        std::cout << "    sizeof(unique_ptr<Item>)  = " << sizeof(std::unique_ptr<Item>)
                  << " B (1 solo puntero)\n";
    }   // dtor en 0: bloque + objeto entran juntos a la misma liberación.

    std::cout << "\n=== 2. UN SOLO NEW PARA EL PAR OBJETO+BLOQUE ===\n";
    std::cout << "  (esto es invisible a la vista, pero es lo que gana\n";
    std::cout << "   make_shared: objeto y contadores en UNA allocación,)\n";
    std::cout << "   contiguos: lugar caliente en el caché. Ver L08: el dtor\n";
    std::cout << "   del objeto corre con strong==0 aunque el bloque siga\n";
    std::cout << "   vivo si un weak_ptr/Observable queda pendiente.\n\n";

    std::cout << "=== 3. COPIA BARATA DE CONTADOR, NO DEL OBJETO ===\n";
    {
        auto inventario = std::make_shared<Item>("gema.png");
        std::vector<std::shared_ptr<Item>> slots;
        slots.push_back(inventario);            // 3 vecinos del mismo dueño
        slots.push_back(inventario);
        slots.push_back(inventario);
        std::cout << "    use_count: " << inventario.use_count() << " (4: main + 3 slots)\n";
    }   // todos los dueños mueren con el scope: [dtor] una sola vez.

    std::cout << "\n=== 4. RESUMEN ===\n";
    std::cout << "  formA: shared_ptr<T> p(new T(...)) : 2 allocs, riesgo g()";
    std::cout << "\n  formB: auto p = make_shared<T>(...)  : 1 alloc, seguro";
    std::cout << "\n  regla del curso: SIEMPRE la fábrica.\n";
    return 0;
}
