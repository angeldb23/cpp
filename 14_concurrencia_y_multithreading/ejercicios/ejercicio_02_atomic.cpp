/* =============================================================================
   MÓDULO 14 - EJERCICIO 02: atomic y memory ordering (L03-L04)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (tema 2 de TEMAS.md: data races, std::atomic, memory ordering):
     Implementá un contador thread-safe con diferentes memory orderings y
     compará su corrección y performance.

     1. ContadorRelaxed: usa memory_order_relaxed para incrementar
     2. ContadorSeqCst: usa memory_order_seq_cst (default)
     3. ContadorAcqRel: usa memory_order_acq_rel para fetch_add

     4. Publicador-Suscriptor con acquire-release:
        - Publicador: escribe datos en array, luego flag.store(true, release)
        - Suscriptor: flag.load(acquire) -> lee array
        - Verificar que suscriptor ve datos consistentes

     5. CAS loop (compare_exchange_weak):
        - Implementar increment_atomic_cas que use CAS para incrementar
        - Manejar fallos espurios correctamente (retry loop)

    COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../../include ejercicio_02_atomic.cpp -o ej02 && ./ej02
    ============================================================================= */
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include <cassert>
#include "../../include/test_framework.hpp"

// TODO 1: Contador con relaxed
struct ContadorRelaxed {
    std::atomic<int> valor{0};
    void incrementar() { /* TU CÓDIGO: fetch_add relaxed */ }
    int obtener() const { return valor.load(std::memory_order_relaxed); }
};

// TODO 2: Contador con seq_cst
struct ContadorSeqCst {
    std::atomic<int> valor{0};
    void incrementar() { /* TU CÓDIGO: fetch_add seq_cst */ }
    int obtener() const { return valor.load(std::memory_order_seq_cst); }
};

// TODO 3: Contador con acq_rel
struct ContadorAcqRel {
    std::atomic<int> valor{0};
    void incrementar() { /* TU CÓDIGO: fetch_add acq_rel */ }
    int obtener() const { return valor.load(std::memory_order_acquire); }
};

// TODO 4: Publicador-Suscriptor con acquire-release
struct PublicadorSuscriptor {
    int datos[10] = {0};
    std::atomic<bool> listo{false};

    void publicar([[maybe_unused]] const int arr[10]) {
        // TU CÓDIGO: copiar arr a datos, luego listo.store(true, release)
    }

    bool suscribir([[maybe_unused]] int out[10]) {
        // TU CÓDIGO: if listo.load(acquire) copiar datos a out y return true
        return false;
    }
};

// TODO 5: CAS loop para incrementar
bool incrementar_cas([[maybe_unused]] std::atomic<int>& atomic) {
    // TU CÓDIGO: CAS loop con compare_exchange_weak
    // Manejar fallos espurios (retry)
    return true;  // Return true to avoid infinite loop; assertion will fail due to wrong count
}

// ============= TESTS =============
TEST_CASE(ContadorRelaxedCorrecto) {
    ContadorRelaxed c;
    constexpr int HILOS = 4;
    constexpr int VUELTAS = 10000;

    std::vector<std::thread> hilos;
    for (int i = 0; i < HILOS; ++i) {
        hilos.emplace_back([&c] { for (int j = 0; j < VUELTAS; ++j) c.incrementar(); });
    }
    for (auto& t : hilos) t.join();

    ASSERT_EQ(c.obtener(), HILOS * VUELTAS);
}

TEST_CASE(ContadorSeqCstCorrecto) {
    ContadorSeqCst c;
    constexpr int HILOS = 4;
    constexpr int VUELTAS = 10000;

    std::vector<std::thread> hilos;
    for (int i = 0; i < HILOS; ++i) {
        hilos.emplace_back([&c] { for (int j = 0; j < VUELTAS; ++j) c.incrementar(); });
    }
    for (auto& t : hilos) t.join();

    ASSERT_EQ(c.obtener(), HILOS * VUELTAS);
}

TEST_CASE(ContadorAcqRelCorrecto) {
    ContadorAcqRel c;
    constexpr int HILOS = 4;
    constexpr int VUELTAS = 10000;

    std::vector<std::thread> hilos;
    for (int i = 0; i < HILOS; ++i) {
        hilos.emplace_back([&c] { for (int j = 0; j < VUELTAS; ++j) c.incrementar(); });
    }
    for (auto& t : hilos) t.join();

    ASSERT_EQ(c.obtener(), HILOS * VUELTAS);
}

TEST_CASE(PublicadorSuscriptorAcquireRelease) {
    PublicadorSuscriptor ps;
    int datos_orig[10] = {1,2,3,4,5,6,7,8,9,10};
    int datos_leidos[10] = {0};

    std::thread pub([&] { ps.publicar(datos_orig); });
    std::thread sub([&] {
        for (int i = 0; i < 1000 && !ps.suscribir(datos_leidos); ++i) {
            std::this_thread::yield();
        }
    });

    pub.join(); sub.join();

    for (int i = 0; i < 10; ++i) {
        ASSERT_EQ(datos_leidos[i], datos_orig[i]);
    }
}

TEST_CASE(CASLoopIncrementa) {
    std::atomic<int> atomic{0};
    constexpr int HILOS = 4;
    constexpr int VUELTAS = 10000;

    std::vector<std::thread> hilos;
    for (int i = 0; i < HILOS; ++i) {
        hilos.emplace_back([&atomic] {
            for (int j = 0; j < VUELTAS; ++j) {
                while (!incrementar_cas(atomic)) { /* retry */ }
            }
        });
    }
    for (auto& t : hilos) t.join();

    ASSERT_EQ(atomic.load(), HILOS * VUELTAS);
}

int main() {
    return RUN_ALL_TESTS();
}