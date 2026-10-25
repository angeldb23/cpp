/* =============================================================================
   MÓDULO 14 - SOLUCIÓN 03: Mutex y RAII locks (ej. 03)
   =============================================================================
   L05-L06: lock_guard para scopes simples, scoped_lock para múltiples mutexes
   atómicamente (evita deadlock). unique_lock para flexibilidad (defer, unlock temprano). */
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include "../../include/test_framework.hpp"

class CuentaBancaria {
    int saldo_ = 0;
    mutable std::mutex mtx_;  // mutable para obtener_saldo() const

public:
    explicit CuentaBancaria(int saldo_inicial = 0) : saldo_(saldo_inicial) {}

    bool depositar(int monto) {
        if (monto <= 0) return false;
        std::lock_guard<std::mutex> lk(mtx_);
        saldo_ += monto;
        return true;
    }

    bool extraer(int monto) {
        if (monto <= 0) return false;
        std::lock_guard<std::mutex> lk(mtx_);
        if (saldo_ < monto) return false;
        saldo_ -= monto;
        return true;
    }

    int obtener_saldo() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return saldo_;
    }

    std::mutex& get_mutex() { return mtx_; }
    int& get_saldo_ref() { return saldo_; }
};

bool transferir(CuentaBancaria& desde, CuentaBancaria& hasta, int monto) {
    if (monto <= 0) return false;
    // scoped_lock toma AMBOS mutexes atómicamente (evita deadlock)
    std::scoped_lock lock(desde.get_mutex(), hasta.get_mutex());
    int& saldo_desde = desde.get_saldo_ref();
    if (saldo_desde < monto) return false;
    saldo_desde -= monto;
    hasta.get_saldo_ref() += monto;
    return true;
}

TEST_CASE(CuentaBasica) {
    CuentaBancaria c(100);
    ASSERT_TRUE(c.depositar(50));
    ASSERT_EQ(c.obtener_saldo(), 150);
    ASSERT_TRUE(c.extraer(30));
    ASSERT_EQ(c.obtener_saldo(), 120);
    ASSERT_FALSE(c.extraer(200));
    ASSERT_EQ(c.obtener_saldo(), 120);
}

TEST_CASE(TransferenciaSimple) {
    CuentaBancaria a(100), b(50);
    ASSERT_TRUE(transferir(a, b, 30));
    ASSERT_EQ(a.obtener_saldo(), 70);
    ASSERT_EQ(b.obtener_saldo(), 80);
    ASSERT_FALSE(transferir(a, b, 100));
}

TEST_CASE(TransferenciaConcurrenteSinDeadlock) {
    CuentaBancaria a(1000), b(1000);
    constexpr int HILOS = 10;
    constexpr int TRANSFERENCIAS = 100;

    std::vector<std::thread> hilos;
    for (int i = 0; i < HILOS; ++i) {
        hilos.emplace_back([&] {
            for (int j = 0; j < TRANSFERENCIAS; ++j) {
                int monto = 10;
                if (i % 2 == 0) transferir(a, b, monto);
                else transferir(b, a, monto);
            }
        });
    }
    for (auto& t : hilos) t.join();

    int total = a.obtener_saldo() + b.obtener_saldo();
    ASSERT_EQ(total, 2000);
}

TEST_CASE(ScopedLockDosMutexes) {
    std::mutex m1, m2;
    std::scoped_lock lock(m1, m2);
    ASSERT_TRUE(true);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 03 (mutex + RAII)...\n";
    return RUN_ALL_TESTS();
}