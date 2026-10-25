/* =============================================================================
   MÓDULO 14 - EJERCICIO 07: Thread pool con prioridades (L12)
   =============================================================================
   DIFICULTAD: Avanzado
   CONSIGNA (tema 6: pool de hilos y cola de tareas):
     Extender ThreadPool con prioridades:
     1. PriorityThreadPool: usa priority_queue (min-heap: menor prioridad = más urgente)
     2. submit(prioridad, F&&) -> future<Ret>
     3. Workers roban tareas de mayor prioridad primero

     Test: encolar 10 tareas prioridad 10 (lentas) + 1 tarea prioridad 1 (rápida),
           verificar que la prioridad 1 se ejecuta ANTES

    COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../../include ejercicio_07_priority_pool.cpp -o ej07 && ./ej07
    ============================================================================= */
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <atomic>
#include <chrono>
#include "../../include/test_framework.hpp"

class PriorityThreadPool {
    struct Task {
        int priority;
        std::packaged_task<void()> task;
        bool operator>(const Task& other) const { return priority > other.priority; }
    };

    std::vector<std::jthread> workers_;
    std::priority_queue<Task, std::vector<Task>, std::greater<>> pq_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};

public:
    explicit PriorityThreadPool([[maybe_unused]] size_t n = std::thread::hardware_concurrency()) {
        // TU CÓDIGO: crear workers
        // Para que los tests fallen limpiamente, no crear workers reales
    }

    ~PriorityThreadPool() { shutdown(); }

    template <typename F>
    auto submit([[maybe_unused]] int priority, F&& f) -> std::future<std::invoke_result_t<F>> {
        // TU CÓDIGO: empaquetar en packaged_task, push a pq_ con prioridad, notify_one
        // Devuelve future con valor por defecto (incorrecto) para que fallen aserciones
        using Ret = std::invoke_result_t<F>;
        auto task = std::make_shared<std::packaged_task<Ret()>>(
            []() -> Ret { return Ret{}; }
        );
        std::future<Ret> fut = task->get_future();
        std::thread([task]() { (*task)(); }).detach();
        return fut;
    }

    void shutdown() {
        // TU CÓDIGO
        stop_ = true;
        cv_.notify_all();
    }

private:
    void worker_loop(std::stop_token st) {
        // TU CÓDIGO
        while (!st.stop_requested() && !stop_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
};

// ============= TESTS =============
TEST_CASE(PriorityPoolBasico) {
    PriorityThreadPool pool(2);
    auto f = pool.submit(5, [] { return 42; });
    ASSERT_EQ(f.get(), 42);
}

TEST_CASE(PrioridadAltaSeEjecutaPrimero) {
    PriorityThreadPool pool(2);
    std::atomic<int> orden{0};
    std::vector<int> secuencia;

    // 10 tareas BAJA prioridad (10) - lentas
    std::vector<std::future<void>> bajas;
    for (int i = 0; i < 10; ++i) {
        bajas.push_back(pool.submit(10, [&, i] {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            secuencia.push_back(i);  // 0..9
        }));
    }

    // 1 tarea ALTA prioridad (1) - rápida
    auto alta = pool.submit(1, [&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        secuencia.push_back(99);  // marca de alta prioridad
    });

    alta.wait();  // esperar a que termine la alta

    // La alta prioridad (99) debería aparecer ANTES que la mayoría de las bajas
    auto it = std::find(secuencia.begin(), secuencia.end(), 99);
    ASSERT_TRUE(it != secuencia.end());
    // Verificar que 99 está en la primera mitad aprox
    ASSERT_TRUE(std::distance(secuencia.begin(), it) < 5);
}

int main() {
    return RUN_ALL_TESTS();
}