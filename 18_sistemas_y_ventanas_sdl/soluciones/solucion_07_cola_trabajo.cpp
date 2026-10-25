/* =============================================================================
   MÓDULO 18 - SOLUCIÓN 07: Cola thread-safe de trabajo (cargador de assets)
   =============================================================================
   MISMO ENUNCIADO que ejercicios/ejercicio_07_cola_trabajo.cpp.
   SOLUCIÓN COMENTADA: qué hace cada pieza y por qué.

   PUNTOS CLAVE:
   - cv_.wait(lk, predicado): la forma DE LA CASA del módulo 14: espera
     "mientras la cola esté vacía y no cerrada"; el predicado re-chequea
     el estado tras cada despertar (no hay señales perdidas).
   - cerrar() hace notify_all: TODOS los consumidores despiertan; los que
     todavía tienen trabajo por drenar lo terminan; los que ya no tienen
     qué sacar reciben false y terminan (el flag compartido reemplaza
     al "sentinel" de la L10: más limpio para N consumidores).
   - procesarCola: N consumidores + join: cada tarea se procesa EXACTAMENTE
     una vez por la propia cola (FIFO con lock del deque); el contador
     compartido queda protegido con su propio mutex.
   - cerrada() es const: mutex mutable (lección 05 del módulo 05).
   - Cola "spawn" sin hilos separados: como lección 10: thread de carga
     NUNCA toca el renderer: acá no están ni el renderer ni SDL en el
     ejercício (lógica pura), pero el patrón es idéntico.
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

// ============================ SOLUCIÓN ======================================
class ColaTrabajo {
public:
    void push(Tarea t) {
        { std::lock_guard lk(m_); cola_.push_back(std::move(t)); }
        cv_.notify_one();   // solo uno: basta con que despierte 1 consumidor
    }

    bool pop(Tarea& out) {
        std::unique_lock lk(m_);
        // espera con predicado: despertar SOLO si hay trabajo o cierre
        cv_.wait(lk, [this] { return cerrada_ || !cola_.empty(); });
        if (cola_.empty()) return false;    // cerrada y vacía: fin del mundo
        out = std::move(cola_.front());
        cola_.pop_front();
        return true;                        // el drenaje continúa si hay más
    }

    void cerrar() {
        { std::lock_guard lk(m_); cerrada_ = true; }
        cv_.notify_all();                   // TODOS despiertan, ven cerrada_
    }

    bool cerrada() const {
        std::lock_guard lk(m_);
        return cerrada_;
    }

private:
    std::deque<Tarea> cola_;
    mutable std::mutex m_;
    std::condition_variable cv_;
    bool cerrada_ = false;
};

class ContadorProceso {
public:
    void procesa(const Tarea& t) {
        std::lock_guard lk(mtx_);           // el log es compartido por hilos
        idVistos_.push_back(t.id);
        if (t.pesada) ++pesadas_;
        ++procesadas_;
    }
    int procesadas() const { std::lock_guard lk(mtx_); return procesadas_; }
    int pesadas() const { std::lock_guard lk(mtx_); return pesadas_; }
    std::vector<int> idVistos() const {
        std::lock_guard lk(mtx_);
        return idVistos_;
    }

private:
    mutable std::mutex mtx_;
    std::vector<int> idVistos_;
    int procesadas_ = 0;
    int pesadas_ = 0;
};

// N consumidores con join; cada tarea se procesa UNA vez
int procesarCola(ColaTrabajo& cola, ContadorProceso& log, int hilos) {
    std::vector<std::thread> threads;
    std::atomic<int> procesadasPorHilos{0};     // lo que ESTOS hilos procesaron
    for (int i = 0; i < hilos; ++i) {
        threads.emplace_back([&] {
            Tarea t;
            while (cola.pop(t)) {               // "drain until cerrada y vacía"
                log.procesa(t);
                ++procesadasPorHilos;
            }
        });
    }
    for (auto& th : threads) th.join();         // cierre ordenado del producto
    return procesadasPorHilos.load();
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

int main() {
    return RUN_ALL_TESTS();
}
