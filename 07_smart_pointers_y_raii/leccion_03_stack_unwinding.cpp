/* =============================================================================
   MÓDULO 07 - LECCIÓN 03: Stack unwinding: RAII cuando una excepción escapa
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      El escenario que rompe a "new/delete a mano" y en el que RAII brilla:
      una EXCEPCIÓN que cruza el medio de la función.

      Cuando una excepción se propaga, el compilador desarma el stack FRAME
      de cada función por la que pasa, y EN CADA NIVEL llama a los
      destructores de los objetos locales. Eso se llama STACK UNWINDING
      ("desenrollado del stack"). El resultado:

        - Cada objeto RAII local libera su recurso AUNQUE la función "retorne
          por excepción".
        - El orden es el de la L02: reversa de construcción.

      a) Por qué importa: con este estilo:
            auto* b = new BufferMemoria(1024);
            if (!validar()) throw std::runtime_error("nivel roto");
            delete b;           // <- NUNCA LLEGA
         el `delete` queda en un camino que la excepción salta. La memoria
         queda fugada. En cambio con el objeto local RAII, la destrucción no
         depende del camino de salida: siempre corre.

      b) Qué NO se destruye durante el unwinding: los objetos a los que no se
         les terminó de llamar el constructor. Si un constructor lanza a
         mitad de camino, los MIEMBROS ya construidos se destruyen (son
         objetos completos), pero el objeto "global" nunca existió y no hay
         destructor suyo. ES LA RAZÓN por la que cada miembro RAII va a
         salvarse aunque otro miembro falle durante la construcción.

      c) Atrapa SIEMPRE por valor o por const& (módulo 04 ya lo comentó):
         `catch (const std::exception& e)`. Y note que la excepción atraviesa
         funciones AQÍ en este nivel: cada frame deja su aporte, imprimimos
         para verlo.

      d) La moraleja de diseño: con RAII, no escribís try/catch para limpiar.
         El try/catch queda para interiores de nivel de estrategia (módulo
         13): NO para reproducir la limpieza al vuelo.

   2. ¿QUÉ PASA POR DEBAJO?
      Cada función con objetos que necesitan destrucción tiene código de
      "limpieza" adjunto (realmente generado por el compilador). Cuando la
      excepción entra en un frame, se ejecuta el personal de limpieza de ese
      frame, y stack se desarma frame por frame hasta que alguien captura.
      Si NADIE captura, `std::terminate` mata el programa (pero los dtors
      locales ya corrieron).

      Microgolf: no lances excepciones en bucles de juego caliente-frame: la
      indirección de unwinding tiene el costo solo cuando LUEGO ocurre, pero
      el diseño con RAII garantiza el escape correcto ahí también.

   3. ERRORES COMUNES
      - try/catch SOLO para liberar: si el único contenido del catch es
        `delete`, tu código quiere RAII, no manejo de errores.
      - Lanzar desde un destructor: si durante el unwinding ya se está
        propagando otra excepción, lanzar de nuevo = std::terminate. Los
        dtors de recursos no lanzan (C++ moderno: destructores son noexcept
        implícitos).
      - Capturar por valor a una jerarquía: `catch (std::exception e)` SLICEA
        (módulo 06).
      - Asumir que el compilador "va a liberar lo reservado a mano": no lo
        hace. Un `new` crudo durante unwinding queda fugado, PERIODO. Único
        arreglo: que ese `new` esté dentro de un objeto RAII.

   4. DEMO: lanzar a mitad de camino y ver quién se limpia
 */
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>

// Recurso RAII con nombre que registra su ciclo de vida.
class Recurso {
private:
    const char* nombre_;

public:
    explicit Recurso(const char* n) : nombre_(n) {
        std::cout << "    [ctor  ] " << nombre_ << "\n";
    }
    ~Recurso() {
        std::cout << "    [dtor  ] " << nombre_ << "\n";
    }
    Recurso(const Recurso&)            = delete;
    Recurso& operator=(const Recurso&) = delete;
};

// Capa 3: acá se dispara el error DESPUÉS de adquirir recursos.
void cargarTexturas() {
    std::cout << "  cargarTexturas(): entro\n";
    Recurso sprites {"sprites (local de cargarTexturas)"};
    if (true) {   // simulamos falla (en un juego real: dispositivo sin VRAM)
        throw std::runtime_error("textura corrupta");
    }
    std::cout << "  cargarTexturas(): esto NUNCA se imprime\n";
}

// Capa 2: NO captura, pero sus locales igual se limpian al pasar el malón.
void construirNivel() {
    std::cout << "  construirNivel(): entro\n";
    Recurso tiles {"tiles (local de construirNivel)"};
    cargarTexturas();             // la excepción CRUZA esta función
    std::cout << "  construirNivel(): nunca llega\n";
}

// Capa 1: atrapa y reporta. Veinte frames arriba ya limpiaron.
int main() {
    std::cout << "=== 1. UNWINDING: LA EXCEPCION CRUZA DOS FUNCIONES ===\n";
    try {
        construirNivel();
    } catch (const std::exception& e) {          // por const&: sin copias
        std::cout << "  main(): capture -> \"" << e.what() << "\"\n";
        std::cout << "  main(): y ANTES de llegar aca ya corrieron los dtors:\n";
        std::cout << "          sprites y tiles se liberaron solos.\n";
    }

    std::cout << "\n=== 2. CONTRASTE: new CRUDO FUGA EN EL MISMO CASO ===\n";
    try {
        Recurso seguro {"b (RAII)"};
        auto* fugado = new int[100];   // SIMULACION de new crudo
        (void)fugado;
        throw std::runtime_error("segundo error");
        // delete[] fugado;  <- nunca corre: la fuga es real (ver nota abajo)
    } catch (const std::exception&) {
        std::cout << "  capture: 'b' se destruyo por RAII...\n";
        std::cout << "  ...pero 'fugado' (new crudo) quedo FUGADO.\n";
        std::cout << "  (Nota: fuga pedagogica y a proposito: con ASan\n";
        std::cout << "   el binario reporta 400 bytes leaked en esta demo.)\n";
    }

    std::cout << "\n=== 3. LA SOLUCION: ESA MEMORIA DENTRO DE RAII ===\n";
    std::cout << "  Si 'fugado' hubiera sido un BufferMemoria por valor,\n";
    std::cout << "  su dtor habria corrido en la misma pasada de unwinding.\n";
    std::cout << "  ESE es exactamente el trabajo de unique_ptr (L04).\n";
    return 0;
}
