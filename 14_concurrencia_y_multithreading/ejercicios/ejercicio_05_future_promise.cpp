/* =============================================================================
   MÓDULO 14 - EJERCICIO 05: future, promise, async, packaged_task (L10-L11)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (tema 5 de TEMAS.md: future/promise/async, packaged_task):
     1. async_task(F&&): lanza tarea con std::async(std::launch::async), retorna future
     2. promise_task(F&&): crea promise, lanza hilo que cumple promise, retorna future
     3. packaged_task_queue: cola de std::packaged_task<void()> con worker thread
        - enqueue(F&&) -> future<Ret>
        - worker loop: pop -> ejecutar -> repetir
        - shutdown(): marca fin, worker termina
     3. shared_future_demo: múltiples hilos esperan mismo resultado
        - Crear promise, obtener future, convertir a shared_future
        - 3 hilos leen shared_future.get() -> todos ven mismo valor

    COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../../include ejercicio_05_future_promise.cpp -o ej05 && ./ej05
    ============================================================================= */
#include <iostream>
#include <thread>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>
#include "../../include/test_framework.hpp"

// TODO 1: async_task
template <typename F>
auto async_task(F&& f) -> std::future<std::invoke_result_t<F>> {
    // TU CÓDIGO: std::async(std::launch::async, ...)
    // Devuelve valor incorrecto (0) para que fallen las aserciones
    return std::async(std::launch::async, []() -> std::invoke_result_t<F> { return {}; });
}

// TODO 2: promise_task
template <typename F>
auto promise_task(F&& f) -> std::future<std::invoke_result_t<F>> {
    // TU CÓDIGO: promise + thread que set_value
    // Devuelve valor por defecto (incorrecto) para que fallen aserciones
    using Ret = std::invoke_result_t<F>;
    std::promise<Ret> p;
    auto fut = p.get_future();
    std::thread([p = std::move(p)]() mutable {
        p.set_value(Ret{});  // valor por defecto (0, string vacía, etc.)
    }).detach();
    return fut;
}

// TODO 3: PackagedTaskQueue
class PackagedTaskQueue {
    std::queue<std::packaged_task<void()>> q_;
    std::mutex mtx_;
    std::condition_variable cv_;
    bool stop_ = false;
    std::jthread worker_;

public:
    PackagedTaskQueue() : worker_([this](std::stop_token st) { worker_loop(st); }) {}

    ~PackagedTaskQueue() { shutdown(); }

    template <typename F>
    auto enqueue(F&& f) -> std::future<std::invoke_result_t<F>> {
        using Ret = std::invoke_result_t<F>;
        // TU CÓDIGO: empaquetar en packaged_task, encolar, notify_one
        // Devuelve valor por defecto (incorrecto) para que fallen aserciones
        auto task = std::make_shared<std::packaged_task<Ret()>>(
            []() -> Ret { return Ret{}; }
        );
        std::future<Ret> fut = task->get_future();
        std::thread([task]() { (*task)(); }).detach();
        return fut;
    }

    void shutdown() {
        // TU CÓDIGO: stop_=true, notify_all, worker_.join()
        stop_ = true;
        cv_.notify_all();
        // worker_ hace join automático (jthread)
    }

private:
    void worker_loop(std::stop_token st) {
        while (!st.stop_requested()) {
            std::packaged_task<void()> task;
            {
                std::unique_lock<std::mutex> lk(mtx_);
                cv_.wait(lk, [this] { return !q_.empty() || stop_; });
                if (q_.empty() && stop_) break;
                task = std::move(q_.front());
                q_.pop();
            }
            task();
        }
    }
};

// TODO 4: shared_future demo
std::shared_future<int> crear_shared_future() {
    // TU CÓDIGO: promise -> future -> share() -> set_value en thread separado
    // Devuelve valor incorrecto (0 en lugar de 999) para que fallen aserciones
    std::promise<int> p;
    auto sf = p.get_future().share();
    std::thread([p = std::move(p)]() mutable {
        p.set_value(0);  // valor incorrecto (debería ser 999)
    }).detach();
    return sf;
}

// ============= TESTS =============
TEST_CASE(AsyncTaskRetornaResultado) {
    auto fut = async_task([] { return 42; });
    ASSERT_EQ(fut.get(), 42);
}

TEST_CASE(PromiseTaskRetornaResultado) {
    auto fut = promise_task([] {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return std::string("hola");
    });
    ASSERT_EQ(fut.get(), "hola");
}

TEST_CASE(PackagedTaskQueueEjecutaTareas) {
    PackagedTaskQueue queue;
    auto f1 = queue.enqueue([] { return 1 + 2; });
    auto f2 = queue.enqueue([] {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return 10 * 10;
    });
    ASSERT_EQ(f1.get(), 3);
    ASSERT_EQ(f2.get(), 100);
    queue.shutdown();
}

TEST_CASE(SharedFutureMultiplesLectores) {
    auto sf = crear_shared_future();
    std::atomic<int> lectores{0};

    auto lector = [&] {
        int v = sf.get();
        if (v == 999) lectores++;
    };

    std::thread t1(lector), t2(lector), t3(lector);
    t1.join(); t2.join(); t3.join();
    ASSERT_EQ(lectores.load(), 3);
}

int main() {
    return RUN_ALL_TESTS();
}