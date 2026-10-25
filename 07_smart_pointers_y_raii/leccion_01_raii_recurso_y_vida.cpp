/* =============================================================================
   MÓDULO 07 - LECCIÓN 01: RAII: el recurso se ata a la vida del objeto
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      RAII = "Resource Acquisition Is Initialization": la adquisición de un
      recurso ES la inicialización de un objeto. Parece un trabalenguas pero
      la idea es una sola:

        - El recurso (memoria, archivo, textura de GPU, socket, lock) se
          ADQUIERE en el CONSTRUCTOR.
        - El recurso se LIBERA en el DESTRUCTOR.
        - El destructor corre solo cuando el objeto sale de ámbito.

      Con eso, "olvidarse de liberar" deja de ser posible: nadie escribe
      delete a mano, porque el objeto dueño lo hace al morir. Esto es lo que
      permite a C++ no necesitar recolector de basura: la limpieza no la hace
      un "recogedor" que aparece cada tanto, la hace el compilador insertando
      destructor-calls precisos, deterministas y gratis.

      a) Duración estática vs dinámica: una variable local tiene duración
         determinada por el ámbito (scope). La memoria que `new` reserva tiene
         duración arbitraria: vive hasta que alguien la libera. RAII consiste
         en PONER el recurso dinámico DENTRO de un objeto de duración
         determinada: el heap queda al servicio del stack.

      b) El invariante central: un objeto RAII bien construido SIEMPRE tiene
         su recurso. Un BufferMemoria recién construido ya tiene sus bytes
         reservados; no existe el estado "tengo puntero pero todavía no
         reservé". recordá de la POO I: la clase protege invariantes; acá el
         invariante es "recurso válido o no existe".

      c) El caso de la excepción (preview de la L03): si entre la adquisición
         y el fin del ámbito algo lanza una excepción, los destructores de los
         objetos locales corren igual. Con new/delete a mano, esa línea de
         escape es exactamente donde aparecen las fugas.

   2. ¿QUÉ PASA POR DEBAJO?
      No hay magia: RAII usa EXACTAMENTE los destructores del módulo 05. La
      secuencia del compilador es mecánica:
        1. `BufferMemoria b(1024);` -> llama al constructor.
        2. Fin del scope -> llama `~BufferMemoria()` automáticamente.
        3. Recién DESPUÉS se destruye el objeto (la memoria del objeto en sí
           era del stack, nunca hubo fuga de "objeto", solo del recurso).

      Cero costo: el destructor en línea es dos instrucciones (una llamada a
      free/devuelta de memoria). Un recolector de basura necesita rastrear
      punteros en Runtime; RAII no. Los juegos lo agradecen: sin pausas
      sorpresa de GC a mitad del frame.

      BufferMemoria juega dos papeles: es un wrapper RAII (será nuestra base
      para entender unique_ptr en la L04) y es un RECORDATORIO: esto que vas
      a escribir a mano acá, la STL ya lo trae resuelto y probado.

   3. ERRORES COMUNES
      - Adquirir un segundo recurso a mitad del constructor: si la primera
        adquisición está en un MIEMBRO RAII y falta que aprendamos a
        completarla cuando algo lanza (se ve en la L03), mejor un recurso por
        objeto o recursos ya encapsulados como miembros.
      - Hacer el recurso público exponiendo el puntero crudo: si cualquiera
        puede hacer `delete datos`, el invariante muere. El recurso vive
        `private` y se exponen operaciones seguras.
      - Usar el objeto después de que termine su ámbito: el destructor corrió
        SIEMPRE al salir del bloque, no "al final de main". Un puntero crudo
        guardado hacia afuera queda colgante (eso se retoma en la L10 y L11).

   4. DEMO: RAII a mano, contando cada liberación
 */
#include <cstddef>
#include <iostream>

// Wrapper RAII de un arreglo dinámico: constructor adquiere, destructor libera.
class BufferMemoria {
private:
    int*    datos_;
    std::size_t tamanio_;

public:
    explicit BufferMemoria(std::size_t tamanio)
        : datos_(new int[tamanio]{}), tamanio_(tamanio) {
        std::cout << "  [ctor] reservo " << tamanio_ * sizeof(int) << " bytes\n";
    }

    ~BufferMemoria() {
        std::cout << "  [dtor] libero " << tamanio_ * sizeof(int) << " bytes\n";
        delete[] datos_;
    }

    // Regla 0 (módulo 05): no copio el puntero crudo; el wrapper no se copia.
    // (unique_ptr resuelve esto con propiedad exclusiva: L04.)
    BufferMemoria(const BufferMemoria&)            = delete;
    BufferMemoria& operator=(const BufferMemoria&) = delete;

    // Operaciones seguras: el recurso sale validado, nunca el puntero crudo.
    void set(std::size_t i, int valor) {
        if (i < tamanio_) datos_[i] = valor;
    }
    int get(std::size_t i) const {
        return (i < tamanio_) ? datos_[i] : -1;
    }
    std::size_t tamanio() const { return tamanio_; }
};

// Un "archivo" simulado para mostrar que RAII no es solo memoria: cualquier
// recurso con adquisición/liberación se encapsula igual. (La versión real
// con deleter propio viene en la L12.)
struct Registro {
    explicit Registro(const char*) { std::cout << "  [registro] abro log\n"; }
    ~Registro() { std::cout << "  [registro] cierro log\n"; }
};

void cargarNivel(const char* nombre) {
    std::cout << "-> entro a cargarNivel(" << nombre << ")\n";
    BufferMemoria tiles(8);         // recurso: memoria
    Registro  log(nombre);          // recurso: "archivo"
    tiles.set(0, 42);
    std::cout << "  [uso]     tiles.get(0) = " << tiles.get(0)
              << ", tiles.tamanio() = " << tiles.tamanio() << "\n";
    // Al cerrar esta llave destruyen en orden INVERSO: primero log, después
    // tiles. El '->' imprime el orden exacto: L02 entra a fondo en esto.
}   // <- LOS DOS se liberan acá, sin una sola línea de "limpiar".

int main() {
    std::cout << "=== 1. RAII BASICO: RECURSO ATADO AL AMBITO ===\n";
    {
        BufferMemoria b(16);        // adquisición en construcción
        b.set(3, 7);
        std::cout << "  [uso]     b.get(3) = " << b.get(3) << "\n";
    }                               // liberación en destrucción, automática

    std::cout << "\n=== 2. VARIOS RECURSOS EN UNA FUNCION ===\n";
    cargarNivel("level_01.bin");

    std::cout << "\n=== 3. EL PUNTO CLAVE: NADIE ESCRIBIO delete ===\n";
    std::cout << "  Todo se libero al salir de cada bloque. Cero fugas,\n";
    std::cout << "  cero lineas de limpieza, y corre igual de rapido que C.\n";
    return 0;
}
