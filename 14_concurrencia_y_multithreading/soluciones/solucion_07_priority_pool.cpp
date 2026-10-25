/* =============================================================================
   MÓDULO 14 - SOLUCIÓN 07: Thread pool con prioridades (ej. 07)
   =============================================================================
   L12: PriorityThreadPool con priority_queue (min-heap), workers toman
   tareas de mayor prioridad (menor número) primero. */
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
#include <algorithm>
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
    explicit PriorityThreadPool(size_t n = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < n; ++i) {
            workers_.emplace_back([this](std::stop_token st) { worker_loop(st); });
        }
    }

    ~PriorityThreadPool() { shutdown(); }

    template <typename F>
    auto submit(int priority, F&& f) -> std::future<std::invoke_result_t<F>> {
        using Ret = std::invoke_result_t<F>;
        auto task = std::make_shared<std::packaged_task<std::invoke_result_t<F>()>>(
            std::forward<F>(f));
        std::future<Ret> fut = task->get_future();
        {
            std::lock_guard<std::mutex> lk(mtx_);
            if (stop_) throw std::runtime_error("pool stopped");
            pq_.push({priority, std::packaged_task<void()>([task]() { (*task)(); })});
        }
        cv_.notify_one();
        return fut;
    }

    void shutdown() {
        stop_ = true;
        cv_.notify_all();
    }

private:
    void worker_loop(std::stop_token st) {
        while (!st.stop_requested() && !stop_) {
            Task task;
            {
                std::unique_lock<std::mutex> lk(mtx_);
                cv_.wait(lk, [this, &st] { return !pq_.empty() || st.stop_requested() || stop_; });
                if (pq_.empty() && (st.stop_requested() || stop_)) break;
                task = std::move(const_cast<Task&>(pq_.top()));
                pq_.pop();
            }
            task.task();
        }
    }
};

TEST_CASE(PriorityPoolBasico) {
    PriorityThreadPool pool(2);
    auto f = pool.submit(5, [] { return 42; });
    ASSERT_EQ(f.get(), 42);
}

TEST_CASE(PrioridadAltaSeEjecutaPrimero) {
    PriorityThreadPool pool(2);
    std::atomic<int> orden{0};
    std::vector<int> secuencia;

    std::vector<std::future<void>> bajas;
    for (int i = 0; i < 10; ++i) {
        bajas.push_back(pool.submit(10, [&, i] {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            secuencia.push_back(i);
        }));
    }

    auto alta = pool.submit(1, [&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        secuencia.push_back(99);
    });

    alta.wait();

    auto it = std::find(secuencia.begin(), secuencia.end(), 99);
    ASSERT_TRUE(it != secuencia.end());
    ASSERT_TRUE(std::distance(secuencia.begin(), it) < 5);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 07 (PriorityThreadPool)...\n";
    return RUN_ALL_TESTS();
}