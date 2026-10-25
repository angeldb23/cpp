/* =============================================================================
   MÓDULO 18 - EJERCICIO 07: Cola thread-safe de trabajo (cargador de assets)
   =============================================================================
   DIFICULTAD: Media-Alta
   APOYO: Lección 10 (threads/handoff), Módulos 14 (condition_variable),
          Módulo 16 (states del cargador de recursos)

   CONSIGNA:
   El patrón real: un thread de fondo carga assets mientras el main loop
   desencadena el render (lección 10: el hilo de fondo NUNCA toca el
   renderer). Completá la COLA TRABAJO + el gestor del CICLO:
     1. ColaTrabajo::push(tarea): el productor HACE push y notifica.
     2. bool ColaTrabajo::pop(tarea&): bloquea con condition_variable
        mientras esté vacía y no cerrada; false SOLO cuando la cola se
        cerró y quedó vacía (el consumidores quizás quede esperando).
     3. void ColaTrabajo::cerrar(): atiende a TODOS los esperando (notify
        ALL) y marca la cola cerrada: los pops posteriores drenan lo que
        queda y devuelven false al terminar el dren.
     4. int Cargador::procesar(..., int hilos): lanza `hilos` threads
        consumidores que sacan tareas con pop y procesan SIN orden
        garantizado entre hilos, PERO cada tarea procesa exactamente
        UNA VEZ (índice: de un contador atomic/guard compartido por
        los consumidores? NO: cada tarea ES única por construcción de la
        cola FIFO).
     5. La cola no espera activa (busy wait): cv.wait con predicado.

   REGLAS TESTEABLES (los tests son deterministas: UN hilo productor y
   verificamos POR CONTADORES, no por timing):
   - drena TODO (el count de procesadas == el count de encoladas)
   - ninguna tarea se procesa 2 veces
   - cerrar() con la cola VACÍA y consumidores esperando: levanta
     TODOS (sin deadlock, sin colgarse).
   - pop en cola CERRADA y vacía: false inmediato (no block).

   APOYO REAL: el modelo ArchivoTarea de la L10 (SDL_PushEvent handoff).

   TESTS (no tocar). COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -pthread ejercicio_07_cola_trabajo.cpp -o ej && ./ej
   ============================================================================= */
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <deque>
#include <thread>
#include <vector>
#include "../../include/test_framework.hpp"

// ============================ MODELO (no tocar) ==============================
struct Tarea {
    int  id = 0;                // id única de asset
    bool pesada = false;        // "textura grande" (time bomb en simulación)
};

// ============================ TU CÓDIGO AQUÍ ================================
class ColaTrabajo {
public:
    void push(Tarea t) {
        // TODO 1: lock + queue + notify (solo uno)
        (void)t;
    }

    bool pop(Tarea& out) {
        // TODO 2: wait con predicado (vacía && !cerrada) + drenar FIFO
        (void)out;
        return false;
    }

    void cerrar() {
        // TODO 3: marca cerrada + notify_all
    }

    bool cerrada() const { return false; }
};

class ContadorProceso {
public:
    void procesa(const Tarea& t) {
        idVistos_.push_back(t.id);      // historial (test lo inspecciona)
        if (t.pesada) ++pesadas_;
        ++procesadas_;
    }
    int procesadas() const { return procesadas_; }
    int pesadas() const { return pesadas_; }
    const std::vector<int>& idVistos() const { return idVistos_; }

private:
    std::mutex mtx_;        // el contador es compartido por hilos
    std::vector<int> idVistos_;
    int procesadas_ = 0;
    int pesadas_ = 0;
};

// TODO 4: procesarCola lanza `hilos` consumidores que drenan la cola
int procesarCola(ColaTrabajo& cola, ContadorProceso& log, int hilos) {
    (void)cola; (void)log; (void)hilos;
    return 0;   // devuelve PROCESSAS (para facilitar asserts dos vías)
}

// ============================ TESTS (no tocar) ==============================
TEST_CASE(ProductorConsumidorDrenaTodo) {
    ColaTrabajo cola;
    ContadorProceso log;
    std::thread productor([&] {
        for (int i = 0; i < 100; ++i) cola.push(Tarea{i, i % 10 == 0});
        cola.cerrar();                          // el productor termina la cola
    });
    int n = procesarCola(cola, log, 3);         // 3 consumidores
    productor.join();
    ASSERT_EQ(n, 100);
    ASSERT_EQ(log.procesadas(), 100);           // sin sobrantes
    // cada id EXACTAMENTE una vez: orden multiplicativo en total
    std::vector<int> replica(100, 0);
    for (int id : log.idVistos()) ++replica[id];
    for (int i = 0; i < 100; ++i) ASSERT_EQ(replica[i], 1);
    ASSERT_TRUE(cola.cerrada());
}

TEST_CASE(CerrarColaVaciaDespiertaATodos) {
    ColaTrabajo cola;
    ContadorProceso log;
    std::vector<std::thread> hilos;
    for (int i = 0; i < 3; ++i)
        hilos.emplace_back([&] { Tarea t; (void)cola.pop(t); });
    cola.cerrar();                              // todos despiertan: pop false
    for (auto& th : hilos) th.join();
    ASSERT_EQ(log.procesadas(), 0);
    ColaTrabajo c2;
    c2.cerrar();
    Tarea t;
    ASSERT_FALSE(c2.pop(t));                    // pop inmediato: false
}

TEST_CASE(PopConPesadasEnElDrenaje) {
    ColaTrabajo cola;
    ContadorProceso log;
    cola.push(Tarea{1, true});
    cola.push(Tarea{2, false});
    cola.cerrar();
    int n = procesarCola(cola, log, 2);
    ASSERT_EQ(n, 2);
    ASSERT_EQ(log.pesadas(), 1);
}

int main() { return RUN_ALL_TESTS(); }
