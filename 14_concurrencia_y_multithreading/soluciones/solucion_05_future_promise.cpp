/* =============================================================================
   MÓDULO 14 - SOLUCIÓN 05: future, promise, async, packaged_task (ej. 05)
   =============================================================================
   L10-L11: async con launch::async garantiza hilo nuevo, promise+thread para
   control manual, packaged_task para colas type-erased, shared_future para
   múltiples lectores. */
#include <iostream>
#include <thread>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>
#include "../../include/test_framework.hpp"

template <typename F>
auto async_task(F&& f) -> std::future<std::invoke_result_t<F>> {
    return std::async(std::launch::async, std::forward<F>(f));
}

template <typename F>
auto promise_task(F&& f) -> std::future<std::invoke_result_t<F>> {
    using Ret = std::invoke_result_t<F>;
    std::promise<Ret> prom;
    auto fut = prom.get_future();
    std::thread([prom = std::move(prom), f = std::forward<F>(f)]() mutable {
        try {
            if constexpr (std::is_void_v<Ret>) {
                f();
                prom.set_value();
            } else {
                prom.set_value(f());
            }
        } catch (...) {
            prom.set_exception(std::current_exception());
        }
    }).detach();
    return fut;
}

class PackagedTaskQueue {
    std::queue<std::packaged_task<void()>> q_;
    std::mutex mtx_;
    std::condition_variable cv_;
    bool stop_ = false;
    std::thread worker_;

public:
    PackagedTaskQueue() : worker_([this] { worker_loop(); }) {}

    ~PackagedTaskQueue() { shutdown(); }

    template <typename F>
    auto enqueue(F&& f) -> std::future<std::invoke_result_t<F>> {
        using Ret = std::invoke_result_t<F>;
        auto task = std::make_shared<std::packaged_task<std::invoke_result_t<F>()>>(
            std::forward<F>(f));
        std::future<Ret> fut = task->get_future();
        {
            std::lock_guard<std::mutex> lk(mtx_);
            if (stop_) throw std::runtime_error("queue stopped");
            q_.emplace([task]() { (*task)(); });
        }
        cv_.notify_one();
        return fut;
    }

    void shutdown() {
        {
            std::lock_guard<std::mutex> lk(mtx_);
            stop_ = true;
        }
        cv_.notify_all();
        if (worker_.joinable()) worker_.join();
    }

private:
    void worker_loop() {
        while (true) {
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

std::shared_future<int> crear_shared_future() {
    std::promise<int> prom;
    auto fut = prom.get_future();
    std::thread([prom = std::move(prom)]() mutable {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        prom.set_value(999);
    }).detach();
    return fut.share();
}

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
    std::cout << "Ejecutando pruebas para Solución 05 (future/promise/async)...\n";
    return RUN_ALL_TESTS();
}