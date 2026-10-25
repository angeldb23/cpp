/* =============================================================================
   MÓDULO 14 - EJERCICIO 06: Thread pool básico (L12)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (tema 6 de TEMAS.md: pool de hilos y cola de tareas):
     Implementar un ThreadPool simple con:
     1. Constructor: n hilos workers (default hardware_concurrency)
     2. submit(F&&) -> future<Ret>: encola tarea, retorna future
     3. shutdown(): marca stop, despierta workers, join
     4. Workers usan jthread con stop_token
     5. Cola thread-safe con mutex + condition_variable

     Test: enviar 100 tareas que duermen 1ms y retornan i*2, verificar suma

    COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../../include ejercicio_06_threadpool.cpp -o ej06 && ./ej06
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

class ThreadPool {
    std::vector<std::jthread> workers_;
    std::queue<std::packaged_task<void()>> tasks_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};

public:
    explicit ThreadPool([[maybe_unused]] size_t n = std::thread::hardware_concurrency()) {
        // TU CÓDIGO: crear n workers con jthread
        // Para que los tests fallen limpiamente, no crear workers reales
    }

    ~ThreadPool() { shutdown(); }

    template <typename F>
    auto submit(F&& f) -> std::future<std::invoke_result_t<F>> {
        // TU CÓDIGO: crear packaged_task, obtener future, encolar, notify_one
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
        // TU CÓDIGO: stop_=true, notify_all, workers hacen join auto (jthread)
        stop_ = true;
        cv_.notify_all();
    }

private:
    void worker_loop(std::stop_token st) {
        // TU CÓDIGO: while !stop, wait for task, execute
        while (!st.stop_requested() && !stop_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
};

// ============= TESTS =============
TEST_CASE(ThreadPoolBasico) {
    ThreadPool pool(2);
    auto f = pool.submit([] { return 42; });
    ASSERT_EQ(f.get(), 42);
}

TEST_CASE(ThreadPoolMultiplesTareas) {
    ThreadPool pool(4);
    std::vector<std::future<int>> futuros;
    for (int i = 0; i < 100; ++i) {
        futuros.push_back(pool.submit([i] {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return i * 2;
        }));
    }
    long long suma = 0;
    for (auto& f : futuros) suma += f.get();
    ASSERT_EQ(suma, 9900);  // 2 * (0+1+...+99) = 2*4950 = 9900
}

TEST_CASE(ShutdownEsperaTareas) {
    ThreadPool pool(2);
    auto f = pool.submit([] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return 1;
    });
    // shutdown espera a que termine
    // (destructor llama shutdown)
    ASSERT_EQ(f.get(), 1);
}

int main() {
    return RUN_ALL_TESTS();
}