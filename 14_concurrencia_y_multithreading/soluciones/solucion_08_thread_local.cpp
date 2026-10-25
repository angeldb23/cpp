/* =============================================================================
   MÓDULO 14 - SOLUCIÓN 08: thread_local y sharded counter (ej. 08)
   =============================================================================
   L14: thread_local para estado por hilo (RNG, buffers), sharded counter
   con 64 shards aligned(64) para evitar false sharing, COW con shared_ptr<const>
   + atomic publish (release/acquire) para lectores sin locks. */
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <random>
#include <chrono>
#include <memory>
#include <string>
#include "../../include/test_framework.hpp"

namespace ThreadLocalRNG {
    thread_local std::mt19937 rng([]{
        std::random_device rd;
        return rd() ^ std::hash<std::thread::id>{}(std::this_thread::get_id());
    }());

    int random_int(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng);
    }
}

class ShardedCounter {
    static constexpr size_t SHARDS = 64;
    struct alignas(64) Shard { std::atomic<long long> valor{0}; };
    Shard shards_[SHARDS];

public:
    void increment() {
        size_t idx = std::hash<std::thread::id>{}(std::this_thread::get_id()) & (SHARDS - 1);
        shards_[idx].valor.fetch_add(1, std::memory_order_relaxed);
    }

    long long load() const {
        long long total = 0;
        for (size_t i = 0; i < SHARDS; ++i) {
            total += shards_[i].valor.load(std::memory_order_acquire);
        }
        return total;
    }
};

struct Config {
    int max_conn;
    double timeout;
    std::string server;
    Config(int mc, double t, std::string s) : max_conn(mc), timeout(t), server(std::move(s)) {}
};

class CopyOnWriteConfig {
    std::atomic<std::shared_ptr<const Config>> ptr_{nullptr};
public:
    const Config* leer() const {
        return ptr_.load(std::memory_order_acquire).get();
    }
    void actualizar(int mc, double t, std::string s) {
        auto nueva = std::make_shared<const Config>(mc, t, std::move(s));
        ptr_.store(nueva, std::memory_order_release);
    }
};

TEST_CASE(ThreadLocalRNGIndependiente) {
    std::vector<int> resultados(4, 0);
    std::vector<std::thread> hilos;
    for (int i = 0; i < 4; ++i) {
        int idx = i;
        hilos.emplace_back([&resultados, idx] {
            int suma = 0;
            for (int j = 0; j < 200; ++j) suma += ThreadLocalRNG::random_int(1, 10);
            resultados[idx] = suma;
        });
    }
    for (auto& t : hilos) t.join();
    bool todos_iguales = true;
    for (int i = 1; i < 4; ++i) if (resultados[i] != resultados[0]) todos_iguales = false;
    ASSERT_FALSE(todos_iguales);
}

TEST_CASE(ShardedCounterCorrecto) {
    ShardedCounter c;
    constexpr int HILOS = 4;

    std::vector<std::thread> hilos;
    for (int i = 0; i < HILOS; ++i) {
        hilos.emplace_back([&] { for (int j = 0; j < 2000; ++j) c.increment(); });
    }
    for (auto& t : hilos) t.join();
    ASSERT_EQ(c.load(), 4 * 2000);
}

TEST_CASE(CopyOnWriteConfigLectoresSinLocks) {
    struct Config { int max_conn; double timeout; std::string server; Config(int mc, double t, std::string s) : max_conn(mc), timeout(t), server(std::move(s)) {} };
    class CopyOnWriteConfig {
        std::atomic<std::shared_ptr<const Config>> ptr_{nullptr};
    public:
        const Config* leer() const { return ptr_.load(std::memory_order_acquire).get(); }
        void actualizar(int mc, double t, std::string s) {
            auto nueva = std::make_shared<const Config>(mc, t, std::move(s));
            ptr_.store(nueva, std::memory_order_release);
        }
    };

    CopyOnWriteConfig config;
    config.actualizar(10, 1.0, "v1");

    std::atomic<int> lectores_ok{0};
    std::vector<std::thread> hilos;
    for (int i = 0; i < 5; ++i) {
        hilos.emplace_back([&] {
            for (int j = 0; j < 20; ++j) {
                auto cfg = config.leer();
                if (cfg && cfg->max_conn == 10 && cfg->server == "v1") lectores_ok++;
            }
        });
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    config.actualizar(20, 2.0, "v2");
    for (auto& t : hilos) t.join();
    ASSERT_EQ(lectores_ok.load(), 5 * 20);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 08 (thread_local + particionado)...\n";
    return RUN_ALL_TESTS();
}