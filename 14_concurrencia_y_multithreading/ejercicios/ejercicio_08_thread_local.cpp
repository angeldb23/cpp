/* =============================================================================
   MÓDULO 14 - EJERCICIO 08: thread_local y sharded counter (L14)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (tema 7: thread_local, inmutabilidad, particionado):
     1. ThreadLocalRNG: cada hilo tiene su mt19937 independiente
        - random_int(min, max) usa thread_local rng
        - Verificar que hilos generan secuencias independientes

     2. ShardedCounter: 64 shards (cache line aligned)
        - increment(): hash de thread_id para elegir shard
        - load(): suma todos los shards con acquire
        - Verificar conteo correcto bajo alta contención

     3. CopyOnWriteConfig: shared_ptr<const Config> + atomic
        - leer_config(): load(acquire) -> leer config inmutable
        - actualizar_config(): nuevo shared_ptr + store(release)
        - Múltiples lectores concurrentes sin locks

    COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../../include ejercicio_08_thread_local.cpp -o ej08 && ./ej08
    ============================================================================= */
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <random>
#include <chrono>
#include <memory>
#include <string>
#include "../../include/test_framework.hpp"

// TODO 1: ThreadLocalRNG
namespace ThreadLocalRNG {
    // TU CÓDIGO: thread_local mt19937 rng con semilla única por hilo
    int random_int(int min, int max) { return 0; }
}

// TODO 2: ShardedCounter (64 shards, cache line aligned)
class ShardedCounter {
public:
    // TU CÓDIGO: array de 64 atomic<long long> aligned(64)
    void increment() { /* hash thread_id -> shard */ }
    long long load() const { return 0; }
};

// TODO 3: CopyOnWriteConfig
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
        // TU CÓDIGO: load(acquire)
        return nullptr;
    }
    void actualizar(int mc, double t, std::string s) {
        // TU CÓDIGO: nuevo shared_ptr + store(release)
    }
};

// ============= TESTS =============
TEST_CASE(ThreadLocalRNGIndependiente) {
    std::vector<int> resultados(4, 0);
    std::vector<std::thread> hilos;
    for (int i = 0; i < 4; ++i) {
        int idx = i;
        hilos.emplace_back([&resultados, idx] {
            int suma = 0;
            for (int j = 0; j < 1000; ++j) suma += ThreadLocalRNG::random_int(1, 10);
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
    constexpr int HILOS = 8;

    std::vector<std::thread> hilos;
    for (int i = 0; i < HILOS; ++i) {
        hilos.emplace_back([&] { for (int j = 0; j < 10000; ++j) c.increment(); });
    }
    for (auto& t : hilos) t.join();
    ASSERT_EQ(c.load(), 8 * 10000);
}

TEST_CASE(CopyOnWriteConfigLectoresSinLocks) {
    CopyOnWriteConfig config;
    config.actualizar(10, 1.0, "v1");

    std::atomic<int> lectores_ok{0};
    std::vector<std::thread> hilos;
    for (int i = 0; i < 10; ++i) {
        hilos.emplace_back([&] {
            for (int j = 0; j < 100; ++j) {
                auto cfg = config.leer();
                if (cfg && cfg->max_conn == 10 && cfg->server == "v1") lectores_ok++;
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        });
    }

    // Actualizar mientras leen
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    config.actualizar(20, 2.0, "v2");

    for (auto& t : hilos) t.join();
    ASSERT_EQ(lectores_ok.load(), 10 * 100);  // todos leyeron correctamente
}

int main() {
    return RUN_ALL_TESTS();
}