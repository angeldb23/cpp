/* =============================================================================
   MÓDULO 14 - EJERCICIO 04: condition_variable productor-consumidor (L07-L09)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (tema 4 de TEMAS.md: condition_variable, predicados, wait/notify, productor-consumidor):
     Implementá una cola acotada (bounded queue) thread-safe con condition_variable.

     1. BoundedQueue<T>:
        - capacity fija en constructor
        - push(T): bloquea si llena, retorna void
        - pop(): bloquea si vacía, retorna T
        - try_push(T, timeout): retorna bool (false si timeout)
        - try_pop(T&, timeout): retorna bool (false si timeout)
        - close(): marca cerrada, despierta a todos
        - Usar DOS condition_variables: not_full (productores), not_empty (consumidores)
        - SIEMPRE wait con predicado: cv.wait(lk, pred)
        - notify_one para eficiencia (un consumidor por item)

     2. Productor-consumidor con 2 productores y 3 consumidores:
        - Cada productor encola 20 items (1..20)
        - Consumidores desencolan y suman
        - Verificar suma total = 2 * (1+..+20) = 420

    COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../../include ejercicio_04_condition_variable.cpp -o ej04 && ./ej04
    ============================================================================= */
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

    // TODO 1: push bloqueante
    void push(T val) {
        // TU CÓDIGO: unique_lock, cv_not_full_.wait(lk, pred), q_.push, notify_one
        std::lock_guard<std::mutex> lk(mtx_);
        if (q_.size() < cap_) {
            q_.push(val);
            cv_not_empty_.notify_one();
        }
        // Si está llena, no bloquear: test espera que falle
    }

    // TODO 2: pop bloqueante
    T pop() {
        // TU CÓDIGO: unique_lock, cv_not_empty_.wait(lk, pred), q_.front/pop, notify_one
        std::lock_guard<std::mutex> lk(mtx_);
        if (!q_.empty()) {
            T val = q_.front();
            q_.pop();
            cv_not_full_.notify_one();
            return val;
        }
        return T{};  // Queue vacía: test fallará en aserción
    }

    // TODO 3: try_push con timeout
    bool try_push(T val, [[maybe_unused]] std::chrono::milliseconds timeout) {
        // TU CÓDIGO: wait_for con predicado, return false si timeout
        std::lock_guard<std::mutex> lk(mtx_);
        if (q_.size() < cap_) {
            q_.push(val);
            cv_not_empty_.notify_one();
            return true;
        }
        return false;
    }

    // TODO 4: try_pop con timeout
    bool try_pop(T& out, [[maybe_unused]] std::chrono::milliseconds timeout) {
        // TU CÓDIGO: wait_for, return false si timeout o cerrado+vacío
        std::lock_guard<std::mutex> lk(mtx_);
        if (!q_.empty()) {
            out = q_.front();
            q_.pop();
            cv_not_full_.notify_one();
            return true;
        }
        return false;
    }

    // TODO 5: close - marcar cerrada y despertar a todos
    void close() {
        // TU CÓDIGO: lock, closed_=true, notify_all en AMBAS cv
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

// Helper: RAII thread que hace join automático al destruirse
struct ScopedThread {
    std::thread t;
    ScopedThread() = default;
    explicit ScopedThread(std::thread&& t_) : t(std::move(t_)) {}
    ScopedThread(ScopedThread&&) = default;
    ScopedThread& operator=(ScopedThread&&) = default;
    ~ScopedThread() { if (t.joinable()) t.join(); }
};

#define JTHREAD(f) ScopedThread(std::thread(f))

// ============= TESTS =============
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
    auto t = JTHREAD([&] {
        q.push(3);  // debería bloquear hasta que haya espacio
        bloqueado = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_FALSE(bloqueado);  // aún bloqueado

    q.pop();  // libera espacio
    // t se une automáticamente al destruirse
    ASSERT_TRUE(bloqueado);
}

TEST_CASE(ProductorConsumidor) {
    BoundedQueue<int> q(5);
    std::atomic<int> suma{0};
    constexpr int ITEMS = 20;

    auto prod = JTHREAD([&] {
        for (int i = 1; i <= ITEMS; ++i) q.push(i);
    });

    auto cons = JTHREAD([&] {
        for (int i = 0; i < ITEMS; ++i) suma += q.pop();
    });

    // prod, cons se unen automáticamente
    ASSERT_EQ(suma.load(), ITEMS * (ITEMS + 1) / 2);  // 210
}

TEST_CASE(ProductorConsumidorMultiple) {
    BoundedQueue<int> q(5);
    std::atomic<int> suma{0};
    constexpr int ITEMS = 20;

    auto p1 = JTHREAD([&] { for (int i = 1; i <= ITEMS; ++i) q.push(i); });
    auto p2 = JTHREAD([&] { for (int i = 1; i <= ITEMS; ++i) q.push(i); });
    auto c1 = JTHREAD([&] { for (int i = 0; i < ITEMS; ++i) suma += q.pop(); });
    auto c2 = JTHREAD([&] { for (int i = 0; i < ITEMS; ++i) suma += q.pop(); });
    auto c3 = JTHREAD([&] { for (int i = 0; i < ITEMS; ++i) suma += q.pop(); });

    // Todos se unen automáticamente
    ASSERT_EQ(suma.load(), 2 * (ITEMS * (ITEMS + 1) / 2));
}

TEST_CASE(CloseDespiertaTodos) {
    BoundedQueue<int> q(2);
    std::atomic<int> pops{0};

    auto c1 = JTHREAD([&] { while (true) { int v; if (!q.try_pop(v, std::chrono::milliseconds(100))) break; pops++; } });
    auto c2 = JTHREAD([&] { while (true) { int v; if (!q.try_pop(v, std::chrono::milliseconds(100))) break; pops++; } });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    q.close();
    // c1, c2 se unen automáticamente
    ASSERT_TRUE(true);
}

int main() {
    return RUN_ALL_TESTS();
}