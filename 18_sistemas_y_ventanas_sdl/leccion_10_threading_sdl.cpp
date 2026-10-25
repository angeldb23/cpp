/* =============================================================================
   MÓDULO 18 - LECCIÓN 10: Threading SDL: threads, mutex/cond y handoff de
   eventos
   =============================================================================
   TEMA 7 de TEMAS.md: Threading SDL: SDL_CreateThread, SDL_WaitThread,
   mutex/condvar y colas thread-safe (1 lección).

   CORRER A MANO (sin ventana: spawn de threads reales)
     g++ -std=c++20 -Wall -Wextra leccion_10_threading_sdl.cpp \
         $(pkg-config --cflags --libs sdl3) -o leccion10 && ./leccion10

   ¿POR QUÉ SDL TIENE SUS PROPIAS PRIMITIVAS? (cuando ya conocés
   std::thread del módulo 14): portabilidad (Windows threads vs pthreads vs
   Emscripten), y porque los callbacks de audio/device corren EN threads de
   SDL. Regla del curso: DENTRO de tu juego podés usar std::thread +
   std::mutex (sirven igual); lo que SDL aporta:
     - SDL_CreateThread(fn, nombre, data): thread CON nombre (aparece en
       gdb/htop/debug dumps) y devolución de integridad limpia con
       SDL_WaitThread (USD el handle después) / SDL_DetachThread.
     - Con el nombre del thread, el profiler (perf --with-symcache,
       instrumentos gráficos) puede marcar los threads por nombre en los
       reportes, lo que ayuda a leer las gráficas de hilos (ver módulo 24).

   VERIFICADO EN CABECERAS:
     SDL3 (SDL_thread.h, SDL_mutex.h, 3.4.18):
       SDL_Thread* SDL_CreateThread(SDL_ThreadFunction fn, const char* name, void* data)
       void SDL_WaitThread(SDL_Thread*, int* status)   // SÍ consume el handle
       SDL_Condition* SDL_CreateCondition(void); void SDL_DestroyCondition(...)
       void SDL_SignalCondition(...); void SDL_BroadcastCondition(...)
       bool SDL_WaitConditionTimeout(cond, mutex, Uint32 ms) // false=timeout
       SDL_Mutex* SDL_CreateMutex(void); SDL_LockMutex / SDL_UnlockMutex
     SDL2 (SDL_thread.h / SDL_mutex.h, 2.32.74):
       SDL_Thread* SDL_CreateThread(fn, name, data)
       SDL_cond* SDL_CreateCond(void); int SDL_CondWait(SDL_cond*, SDL_mutex*)
       int SDL_CondWaitTimeout(cond, mutex, Uint32 ms); int SDL_CondSignal(...)
       SDL_mutex* SDL_CreateMutex(void);

   HANDOFF THREAD MAIN -> (el patrón más útil):
     el thread de fondo NO toca la pantalla ni el renderer (jamas!); produce
     "resultados" y los empuja al main loop con SDL_PushEvent(&e) usando un
     SDL_EVENT_USER+n o la cola thread-safe de abajo. El main loop los
     procesa en su silo con su timestep del módulo 16.
     (SDL2 tenía SDL_QueueUserEvent: removido en SDL3, es SDL_PushEvent.)
   ============================================================================= */

#include <condition_variable>
#include <deque>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <SDL3/SDL.h>

// -----------------------------------------------------------------------------
// 1. COLA THREAD-SAFE (la del ej 07: la que carga assets de fondo)
//    Nota: la versión STL (thread-safe con mutex+cv) la viste en el módulo 14;
//    la versión SDL reemplaza mutex/cv por SDL_Mutex/SDL_Condition: misma
//    idea, otra librería. Acá std:: para que compile y se testee sin SDL.
// -----------------------------------------------------------------------------
template <typename T>
class ColaThreadSafe {
public:
    void push(T v) {
        { std::lock_guard lk(m_);  cola_.push_back(std::move(v)); }
        cv_.notify_one();
    }
    // pop bloqueante; false si llegó la señal de "cerrar la cola"
    bool pop(T& out) {
        std::unique_lock lk(m_);
        cv_.wait(lk, [this]{ return cerrada_ || !cola_.empty(); });
        if (cola_.empty()) return false; // cerrada y vacía: chau
        out = std::move(cola_.front());
        cola_.pop_front();
        return true;
    }
    void cerrar() {
        { std::lock_guard lk(m_); cerrada_ = true; }
        cv_.notify_all(); // TODOS los consumidores despiertan y ven cerrada_
    }
private:
    std::deque<T> cola_;
    std::mutex m_;
    std::condition_variable cv_;
    bool cerrada_ = false;
};

// -----------------------------------------------------------------------------
// 2. SDL THREADS: productor que reporta progreso por eventos SDL
// -----------------------------------------------------------------------------
struct Progreso {
    Uint32 tipo = SDL_EVENT_USER;      // cada app tiene 0..SDL_GetNumEvents()-1
    int porcentaje = 0;
};

static int SDLCALL hiloCargador(void* data) {
    // Un thread de SDL ejecuta EXACTAMENTE la función que le pasaste: recibe
    // void* (nuestros datos) y retorna un int 'status' que SDL_WaitThread lee.
    auto* cola = static_cast<ColaThreadSafe<Progreso>*>(data);
    for (int p = 25; p <= 100; p += 25) {
        SDL_Delay(120);                     // "cargando" (disco en la vida real)
        cola->push(Progreso{SDL_EVENT_USER, p});
    }
    cola->push(Progreso{SDL_EVENT_USER, -1}); // SENTINEL: "no hay más datos"
    return 0; // status: lo notifica SDL_WaitThread del otro lado
}

static int demoThreads() {
    ColaThreadSafe<Progreso> cola;

    SDL_Thread* th = SDL_CreateThread(hiloCargador, "cargador-assets", &cola);
    if (th == nullptr) { std::cout << "thread " << SDL_GetError() << "\n"; return 1; }
    std::cout << "=== LECCIÓN 10: threads ===\nthread 'cargador-assets' lanzado\n";

    Progreso p;
    while (cola.pop(p)) {                    // bloquea hasta el próximo o cierre
        if (p.porcentaje < 0) {              // sentinel: fin del productor
            cola.cerrar();                   // desbloquea pop() para SIEMPRE
            std::cout << "sentinel recibido: cola cerrada\n";
            break;
        }
        std::cout << "progreso: " << p.porcentaje << "% (evento user)\n";
    }
    std::cout << "cola cerrada y drenada\n";

    int status = -1;
    SDL_WaitThread(th, &status);             // reúne el status y libera el handle
    std::cout << "hilo terminó con status " << status << " (limpio)\n";
    return 0;
}

/* -----------------------------------------------------------------------------
   3. REGLAS DE ORO (aprendidas del módulo 14, aplicadas con SDL)
     - El thread de AUDIO/device JAMÁS llama a funciones de
       renderer/ventana (por qué: no es thread-safe y bloqueás el device).
     - El thread de fondo solo NOSOTROS llega hasta Cola/estado compartido
       con mutex; la render pasa SIEMPRE por el main loop.
     - CADA SDL_Thread o SDL se juego con SDL_WaitThread/SDL_DetachThread:
       un thread sin "consumir" es un leak de scheduler (como un std::thread
       destruido sin join: trae std::terminate!).
     - Para meter datos al AudioStream desde otro thread: no hace falta
       lock extra, el stream interno ya es thread-safe (doc de SDL_audio).
     - Cuidado con las cuentas: bloquear un mutex dentro del callback de
       audio introduce jank audible; el análisis pesado va al main loop.
----------------------------------------------------------------------------- */

/* =============================================================================
   CÓMO PROBARLA A MANO: ./leccion10 NO abre ventana (no window); imprime
   el progreso con threads SDL reales (sacá el pid con ps -T y vas a ver el
   thread 'cargador-assets'). La versión std::thread es la base del ej 07
   (cola + productores/consumidores, con tests DETERMINISTAS).
   ============================================================================= */
int main() { return demoThreads(); }
