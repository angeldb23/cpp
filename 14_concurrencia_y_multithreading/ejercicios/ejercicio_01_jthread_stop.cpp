/* =============================================================================
   MÓDULO 14 - EJERCICIO 01: jthread y stop_token básico (L01-L02)
   =============================================================================
   DIFICULTAD: Básico
   CONSIGNA (tema 1 de TEMAS.md: std::thread, join/detach, jthread y stop_token):
     Implementá un sistema de "workers" que procesen tareas de una cola usando
     std::jthread con stop_token para cancelación cooperativa.

     1. worker_loop(st, cola, resultados, id):
        - Mientras !st.stop_requested():
          - Tomar tarea de la cola (proteger con mutex + condition_variable)
          - Si cola vacía y stop pedido -> salir
          - Ejecutar tarea (int -> int), guardar resultado en vector resultados[id]
        - Usar wait con predicado que chequee !cola.empty() || st.stop_requested()

     2. main():
        - Crear cola de tareas (queue<int>) y vector resultados
        - Lanzar 3 workers con jthread
        - Encolar 20 tareas (valores 1..20)
        - Esperar 100ms, pedir stop a todos
        - Verificar que workers terminaron y resultados parciales son correctos

    COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../../include ejercicio_01_jthread_stop.cpp -o ej01 && ./ej01
    ============================================================================= */
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

// TODO 1: worker_loop - procesar tareas de la cola hasta stop_requested
void worker_loop([[maybe_unused]] std::stop_token st,
                 [[maybe_unused]] std::queue<std::function<int(int)>>& cola,
                 [[maybe_unused]] std::mutex& mtx_cola,
                 [[maybe_unused]] std::condition_variable& cv,
                 [[maybe_unused]] std::vector<int>& resultados,
                 [[maybe_unused]] int id) {
    // TU CÓDIGO AQUÍ
    // Pista: while (!st.stop_requested()) { unique_lock lk(mtx); cv.wait(lk, pred); ... }
}

// TODO 2: enqueue_task - agregar tarea a la cola de forma thread-safe
void enqueue_task([[maybe_unused]] std::queue<std::function<int(int)>>& cola,
                  [[maybe_unused]] std::mutex& mtx,
                  [[maybe_unused]] std::condition_variable& cv,
                  [[maybe_unused]] std::function<int(int)>&& tarea) {
    // TU CÓDIGO AQUÍ
}

// ============= TESTS =============
TEST_CASE(WorkersProcesanTareas) {
    std::queue<std::function<int(int)>> cola;
    std::mutex mtx_cola;
    std::condition_variable cv;
    std::vector<int> resultados(3, 0);
    std::atomic<int> tareas_procesadas{0};

    std::jthread w1([&](std::stop_token st) { worker_loop(st, cola, mtx_cola, cv, resultados, 0); });
    std::jthread w2([&](std::stop_token st) { worker_loop(st, cola, mtx_cola, cv, resultados, 1); });
    std::jthread w3([&](std::stop_token st) { worker_loop(st, cola, mtx_cola, cv, resultados, 2); });

    // Encolar tareas: cada tarea duerme 5ms y retorna x*2
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
    // Suma esperada: 2*(1+2+...+15) = 2*120 = 240
    ASSERT_EQ(suma, 240);
}

TEST_CASE(StopCancelaWorkers) {
    std::queue<std::function<int(int)>> cola;
    std::mutex mtx_cola;
    std::condition_variable cv;
    std::vector<int> resultados(2, 0);

    std::jthread w1([&](std::stop_token st) { worker_loop(st, cola, mtx_cola, cv, resultados, 0); });
    std::jthread w2([&](std::stop_token st) { worker_loop(st, cola, mtx_cola, cv, resultados, 1); });

    // No encolar nada, pedir stop inmediato
    w1.request_stop(); w2.request_stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Workers deben haber terminado sin crashear
    ASSERT_TRUE(true);  // Si llegamos aquí, OK
}

int main() {
    return RUN_ALL_TESTS();
}