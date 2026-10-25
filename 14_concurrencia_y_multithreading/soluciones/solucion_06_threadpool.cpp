/* =============================================================================
   MÓDULO 14 - SOLUCIÓN 06: Thread pool básico (ej. 06)
   =============================================================================
   L12: ThreadPool con jthread (join auto), stop_token, cola de packaged_task. */
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
    explicit ThreadPool(size_t n = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < n; ++i) {
            workers_.emplace_back([this](std::stop_token st) { worker_loop(st); });
        }
    }

    ~ThreadPool() { shutdown(); }

    template <typename F>
    auto submit(F&& f) -> std::future<std::invoke_result_t<F>> {
        using Ret = std::invoke_result_t<F>;
        auto task = std::make_shared<std::packaged_task<Ret()>>(std::forward<F>(f));
        std::future<Ret> fut = task->get_future();
        {
            std::lock_guard<std::mutex> lk(mtx_);
            if (stop_) throw std::runtime_error("pool stopped");
            tasks_.emplace([task]() { (*task)(); });
        }
        cv_.notify_one();
        return fut;
    }

    void shutdown() {
        stop_ = true;
        cv_.notify_all();
        // jthreads join auto en destructor de workers_
    }

private:
    void worker_loop(std::stop_token st) {
        while (!st.stop_requested() && !stop_) {
            std::packaged_task<void()> task;
            {
                std::unique_lock<std::mutex> lk(mtx_);
                cv_.wait(lk, [this, &st] { return !tasks_.empty() || st.stop_requested() || stop_; });
                if (tasks_.empty() && (st.stop_requested() || stop_)) break;
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            task();
        }
    }
};

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
    ASSERT_EQ(suma, 9900);
}

TEST_CASE(ShutdownEsperaTareas) {
    ThreadPool pool(2);
    auto f = pool.submit([] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return 1;
    });
    ASSERT_EQ(f.get(), 1);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 06 (ThreadPool)...\n";
    return RUN_ALL_TESTS();
}