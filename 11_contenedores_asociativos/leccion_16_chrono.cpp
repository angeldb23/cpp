/* =============================================================================
   MÓDULO 11 - LECCIÓN 16: std::chrono: medir tiempos del juego
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
        Último tema del módulo: chrono es la biblioteca de TIEMPOS con
        tipos propios (siempre compilando "unidades", no ints):

            using namespace std::chrono;
            using namespace std::chrono_literals;

            auto lapso = 100ms;                      // duration<int, milli>
            auto espera = 2s + 500ms;                // compone y convierte
            duration<float> en_segundos = espera;    // 2.5

        El mensaje de inserción (pieces): el TIPO dura y es "la unidad":
        un microseconds no se desconfunde con un literal int GANANCIA
        (units-safety: lo que inquiries en el módulo 01 con constexpr).

        RELOJES (clock): three tipos canónicos:
          - steady_clock: el reloj del tiempo de máquina: no mueren
            (nunca retrocede ni salta por "afuera").
            LA conveniente para medir tiempos de juego o profiling.
          - system_clock: el reloj de "hora de la pared" (cambia con
            NTP o por el usuario): sirve para timestamps visibles,
            NO para deltas framesnera juego
          - high_resolution_clock: alias del reloj más preciso de la
            plataforma; en GCC es steady_clock (por eso no lo uses
            "porque es rápido": steady ya lo es).

        Measuring el patrón:
            auto t0 = steady_clock::now();
            hacerTrabajo();
            auto dt = steady_clock::now() - t0;   // duration
            long ms = duration_cast<milliseconds>(dt).count();

       (dt) es una duration: la compilacion es por unidad. duration_cast
        es EL cast explícito entre unidades (el primo del static_cast del
        módulo 01). count() te da el valor bruto en esa unidad.

        El otro tipo: time_point<Clock, Duration>: "un punto en la línea
        del reloj": el tipo de retorno de now(): restar dos time_point
        del mismo reloj da una duration.
   2. ¿QUÉ PASA POR DEBAJO?
        std::chrono modela el tiempo como VALOR + UNIDAD: la template
        duration<int, std::milli>; el ratio define la unidad (millis es
        1/1000). Las conversiones entre unidades tienen una regla
        simple: si son exactas, se permiten implícitas; si perderían
        el decimal, exigen duration_cast.

        El steady_clock es un reloj MONÓTONO: si medís el mismo
        intervalo dos veces, el resultado es coherente; el system_clock
        no (NTP o tu horario pueden saltarlo). Por eso toda medición
        "de máquina" va con steady_clock.
    3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
        - system_clock para timings de juego: el reloj de pared puede
          saltar (NTP, ajuste manual) y tu delta se muere: steady_clock.
        - duration_cast silencioso cuando la unidad no cierra exacto:
          es el cast EXPLÍCITO y corta el decimal: si contás en milis y el frame
          duró 0.9 ms, el dt da 0: entendé qué unidad pediste.
        - dt.count() devuelve el número bruto EN ESA unidad: solo tiene
          sentido junto al tipo: no confundas un long de micros con un
          double de segundos.
        - Este módulo no enseña threads: sleep_for se usa acá solo como
          "trabajo del frame"; el hilo formal es el módulo 14.
    4. PARA PROFUNDIZAR:
        https://en.cppreference.com/w/cpp/chrono/duration
        https://en.cppreference.com/w/cpp/chrono/steady_clock
        https://en.cppreference.com/w/cpp/chrono/time_point
   ============================================================================= */
#include <chrono>
#include <iostream>
#include <thread>

int main() {
    using namespace std::chrono;
    using namespace std::chrono_literals;

    // --- 1. Durations: unidades que se comparan y suman --------------------------------
    const auto espera = 2s + 500ms;          // compone a una duration comun
    const milliseconds en_ms = espera;       // conversion implicita a la unidad
    std::cout << "espera = " << en_ms.count() << " ms\n";

    const auto floatSegs = duration_cast<duration<float>>(1500ms);
    std::cout << "1500ms = " << floatSegs.count() << " seg\n";

    // --- 2. Medir con steady_clock: el patron de SIEMPRE -------------------------------
    const auto t0 = steady_clock::now();
    std::this_thread::sleep_for(150ms);      // "trabajo" del frame
    const auto dt = steady_clock::now() - t0;

    const long micros = duration_cast<microseconds>(dt).count();
    std::cout << "\ndelta: " << micros << " us | en millis: "
              << static_cast<double>(micros) / 1000.0 << " ms\n";

    // --- 3. system_clock: para la hora de pared, no para deltas de juego ----------------
    const auto wall = system_clock::now();
    const auto tic = duration_cast<milliseconds>(wall.time_since_epoch()).count();
    std::cout << "\nsystem_clock desde epoch: " << tic << " ms (hora de pared)\n";

    // --- 4. El "unidades con su propio tipo" otra vez: minutos también son durations ---
    const auto maratón = 26min + 800ms;
    std::cout << "maraton: " << duration_cast<milliseconds>(maratón).count()
              << " ms\n";
    return 0;
}
