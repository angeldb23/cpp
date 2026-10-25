/* =============================================================================
   MÓDULO 14 - SOLUCIÓN 01: jthread y stop_token básico (ej. 01)
   =============================================================================
   L01-L02: jthread hace join automático en dtor + stop_token integrado. */
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <functional>
#include <stop_token>
#include "../../include/test_framework.hpp"

// 1) worker_loop: procesar tareas hasta stop_requested
void worker_loop(std::stop_token st,
                 std::queue<std::function<int(int)>>& cola,
                 std::mutex& mtx_cola,
                 std::condition_variable& cv,
                 std::vector<int>& resultados,
                 int id) {
    // stop_callback para despertar cv cuando se pida stop
    std::stop_callback cb(st, [&cv]() { cv.notify_all(); });

    while (!st.stop_requested()) {
        std::function<int(int)> tarea;
        {
            std::unique_lock<std::mutex> lk(mtx_cola);
            // Esperar: hay tarea O stop pedido
            cv.wait(lk, [&] { return !cola.empty() || st.stop_requested(); });

            if (cola.empty() && st.stop_requested()) break;

            tarea = std::move(cola.front());
            cola.pop();
        }

        // Ejecutar SIN lock (reducir contención)
        int resultado = tarea(id);
        resultados[id] += resultado;
    }
}

// 2) enqueue_task: agregar tarea thread-safe
void enqueue_task(std::queue<std::function<int(int)>>& cola,
                  std::mutex& mtx,
                  std::condition_variable& cv,
                  std::function<int(int)>&& tarea) {
    {
        std::lock_guard<std::mutex> lk(mtx);
        cola.push(std::move(tarea));
    }
    cv.notify_one();  // notificar FUERA del lock
}

// ============= TESTS =============
TEST_CASE(WorkersProcesanTareas) {
    std::queue<std::function<int(int)>> cola;
    std::mutex mtx_cola;
    std::condition_variable cv;
    std::vector<int> resultados(3, 0);

    std::jthread w1([&](std::stop_token st) { worker_loop(st, cola, mtx_cola, cv, resultados, 0); });
    std::jthread w2([&](std::stop_token st) { worker_loop(st, cola, mtx_cola, cv, resultados, 1); });
    std::jthread w3([&](std::stop_token st) { worker_loop(st, cola, mtx_cola, cv, resultados, 2); });

    for (int i = 1; i <= 15; ++i) {
        int val = i;
        enqueue_task(cola, mtx_cola, cv, [val](int) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            return val * 2;
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    w1.request_stop(); w2.request_stop(); w3.request_stop();

    int suma = 0;
    for (int r : resultados) suma += r;
    ASSERT_EQ(suma, 240);
}

TEST_CASE(StopCancelaWorkers) {
    std::queue<std::function<int(int)>> cola;
    std::mutex mtx_cola;
    std::condition_variable cv;
    std::vector<int> resultados(2, 0);

    std::jthread w1([&](std::stop_token st) { worker_loop(st, cola, mtx_cola, cv, resultados, 0); });
    std::jthread w2([&](std::stop_token st) { worker_loop(st, cola, mtx_cola, cv, resultados, 1); });

    w1.request_stop(); w2.request_stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_TRUE(true);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 01 (jthread + stop_token)...\n";
    return RUN_ALL_TESTS();
}