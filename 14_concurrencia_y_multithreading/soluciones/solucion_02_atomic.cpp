/* =============================================================================
   MÓDULO 14 - SOLUCIÓN 02: atomic y memory ordering (ej. 02)
   =============================================================================
   L03-L04: relaxed solo garantiza atomicidad, seq_cst orden total,
   acquire-release sincroniza pares publicador-suscriptor, CAS loop
   maneja fallos espurios con retry. */
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include <cassert>
#include "../../include/test_framework.hpp"

struct ContadorRelaxed {
    std::atomic<int> valor{0};
    void incrementar() { valor.fetch_add(1, std::memory_order_relaxed); }
    int obtener() const { return valor.load(std::memory_order_relaxed); }
};

struct ContadorSeqCst {
    std::atomic<int> valor{0};
    void incrementar() { valor.fetch_add(1, std::memory_order_seq_cst); }
    int obtener() const { return valor.load(std::memory_order_seq_cst); }
};

struct ContadorAcqRel {
    std::atomic<int> valor{0};
    void incrementar() { valor.fetch_add(1, std::memory_order_acq_rel); }
    int obtener() const { return valor.load(std::memory_order_acquire); }
};

struct PublicadorSuscriptor {
    int datos[10] = {0};
    std::atomic<bool> listo{false};

    void publicar(const int arr[10]) {
        for (int i = 0; i < 10; ++i) datos[i] = arr[i];
        listo.store(true, std::memory_order_release);  // release: publica datos
    }

    bool suscribir(int out[10]) {
        if (listo.load(std::memory_order_acquire)) {  // acquire: ve datos publicados
            for (int i = 0; i < 10; ++i) out[i] = datos[i];
            return true;
        }
        return false;
    }
};

bool incrementar_cas(std::atomic<int>& atomic) {
    int esperado = atomic.load(std::memory_order_relaxed);
    while (true) {
        int nuevo = esperado + 1;
        // weak: puede fallar espuriamente, retry con valor actualizado
        if (atomic.compare_exchange_weak(esperado, nuevo,
                                         std::memory_order_acq_rel,
                                         std::memory_order_relaxed)) {
            return true;
        }
        // esperado ya tiene el valor actual tras fallo
    }
}

TEST_CASE(ContadorRelaxedCorrecto) {
    struct ContadorRelaxed {
        std::atomic<int> valor{0};
        void incrementar() { valor.fetch_add(1, std::memory_order_relaxed); }
        int obtener() const { return valor.load(std::memory_order_relaxed); }
    };
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
    struct ContadorSeqCst {
        std::atomic<int> valor{0};
        void incrementar() { valor.fetch_add(1, std::memory_order_seq_cst); }
        int obtener() const { return valor.load(std::memory_order_seq_cst); }
    };
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
    struct ContadorAcqRel {
        std::atomic<int> valor{0};
        void incrementar() { valor.fetch_add(1, std::memory_order_acq_rel); }
        int obtener() const { return valor.load(std::memory_order_acquire); }
    };
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
    std::cout << "Ejecutando pruebas para Solución 02 (atomic + memory ordering)...\n";
    return RUN_ALL_TESTS();
}