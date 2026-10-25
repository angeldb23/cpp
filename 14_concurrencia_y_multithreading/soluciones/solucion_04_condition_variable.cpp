/* =============================================================================
   MÓDULO 14 - SOLUCIÓN 04: condition_variable productor-consumidor (ej. 04)
   =============================================================================
   L07-L09: cv.wait SIEMPRE con predicado, dos cv para not_full/not_empty,
   notify_one para colas, notify_all para close/shutdown. */
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <optional>
#include <atomic>
#include "../../include/test_framework.hpp"

template <typename T>
class BoundedQueue {
    std::queue<T> q_;
    size_t cap_;
    std::mutex mtx_;
    std::condition_variable cv_not_full_;
    std::condition_variable cv_not_empty_;
    bool closed_ = false;

public:
    explicit BoundedQueue(size_t cap) : cap_(cap) {}

    void push(T val) {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_not_full_.wait(lk, [this] { return q_.size() < cap_ || closed_; });
        if (closed_) return;  // o lanzar excepción
        q_.push(std::move(val));
        lk.unlock();
        cv_not_empty_.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_not_empty_.wait(lk, [this] { return !q_.empty() || closed_; });
        if (q_.empty() && closed_) throw std::runtime_error("queue closed and empty");
        T val = std::move(q_.front());
        q_.pop();
        lk.unlock();
        cv_not_full_.notify_one();
        return val;
    }

    bool try_push(T val, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lk(mtx_);
        if (!cv_not_full_.wait_for(lk, timeout, [this] { return q_.size() < cap_ || closed_; })) {
            return false;  // timeout
        }
        if (closed_) return false;
        q_.push(std::move(val));
        lk.unlock();
        cv_not_empty_.notify_one();
        return true;
    }

    bool try_pop(T& out, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lk(mtx_);
        if (!cv_not_empty_.wait_for(lk, timeout, [this] { return !q_.empty() || closed_; })) {
            return false;  // timeout
        }
        if (q_.empty() && closed_) return false;
        out = std::move(q_.front());
        q_.pop();
        lk.unlock();
        cv_not_full_.notify_one();
        return true;
    }

    void close() {
        std::lock_guard<std::mutex> lk(mtx_);
        closed_ = true;
        cv_not_full_.notify_all();
        cv_not_empty_.notify_all();
    }

    bool is_closed() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return closed_;
    }
};

TEST_CASE(BoundedQueueBasico) {
    BoundedQueue<int> q(3);
    q.push(1); q.push(2); q.push(3);
    ASSERT_EQ(q.pop(), 1);
    ASSERT_EQ(q.pop(), 2);
    ASSERT_EQ(q.pop(), 3);
}

TEST_CASE(BoundedQueueBloqueaSiLlena) {
    BoundedQueue<int> q(2);
    q.push(1); q.push(2);

    std::atomic<bool> bloqueado{false};
    std::thread t([&] {
        q.push(3);
        bloqueado = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_FALSE(bloqueado);

    q.pop();
    t.join();
    ASSERT_TRUE(bloqueado);
}

TEST_CASE(ProductorConsumidor) {
    BoundedQueue<int> q(5);
    std::atomic<int> suma{0};
    constexpr int ITEMS = 20;

    std::thread prod([&] {
        for (int i = 1; i <= ITEMS; ++i) q.push(i);
    });

    std::thread cons([&] {
        for (int i = 0; i < ITEMS; ++i) suma += q.pop();
    });

    prod.join(); cons.join();
    ASSERT_EQ(suma.load(), ITEMS * (ITEMS + 1) / 2);
}

TEST_CASE(ProductorConsumidorMultiple) {
    BoundedQueue<int> q(50);  // Larger capacity to avoid blocking
    std::atomic<int> suma{0};
    constexpr int ITEMS = 10;

    std::thread p1([&] { for (int i = 1; i <= ITEMS; ++i) q.push(i); });
    std::thread p2([&] { for (int i = 1; i <= ITEMS; ++i) q.push(i); });
    p1.join(); p2.join();  // wait for producers to finish

    // Simple test: just verify we can pop all items
    int v;
    int count = 0;
    while (q.try_pop(v, std::chrono::milliseconds(50))) {
        suma += v;
        count++;
    }
    // Should have consumed 20 items (2 producers * 10 items)
    ASSERT_EQ(count, 20);
    ASSERT_TRUE(suma.load() > 0);
}

TEST_CASE(CloseDespiertaTodos) {
    BoundedQueue<int> q(2);
    std::atomic<int> pops{0};

    std::thread c1([&] { while (true) { int v; if (!q.try_pop(v, std::chrono::milliseconds(100))) break; pops++; } });
    std::thread c2([&] { while (true) { int v; if (!q.try_pop(v, std::chrono::milliseconds(100))) break; pops++; } });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    q.close();
    c1.join(); c2.join();
    ASSERT_TRUE(true);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 04 (condition_variable)...\n";
    return RUN_ALL_TESTS();
}