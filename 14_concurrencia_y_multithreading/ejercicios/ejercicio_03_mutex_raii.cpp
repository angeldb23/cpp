/* =============================================================================
   MÓDULO 14 - EJERCICIO 03: Mutex y RAII locks (L05-L06)
   =============================================================================
   DIFICULTAD: Básico-Intermedio
   CONSIGNA (tema 3 de TEMAS.md: mutex, lock_guard, unique_lock, scoped_lock, deadlocks):
     Implementá una cuenta bancaria thread-safe con transferencias entre cuentas
     evitando deadlocks.

     1. CuentaBancaria:
        - saldo (protected by mutex)
        - depositar(), extraer() -> bool (éxito/fallo por fondos)
        - obtener_saldo() -> int
        - Usar lock_guard para operaciones simples

     2. transferir(CuentaBancaria& desde, CuentaBancaria& hasta, int monto):
        - Debe tomar AMBOS mutexes SIN deadlock
        - Usar std::scoped_lock (C++17) o std::lock + adopt_lock
        - Verificar fondos en 'desde' antes de transferir
        - Retornar bool éxito

     3. Deadlock demo (opcional):
        - Mostrar que transferir(A,B) y transferir(B,A) concurrentes NO deadlock

    COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../../include ejercicio_03_mutex_raii.cpp -o ej03 && ./ej03
    ============================================================================= */
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include "../../include/test_framework.hpp"

// TODO 1: CuentaBancaria thread-safe
class CuentaBancaria {
    int saldo_ = 0;
    std::mutex mtx_;

public:
    explicit CuentaBancaria(int saldo_inicial = 0) : saldo_(saldo_inicial) {}

    // TODO: depositar con lock_guard
    bool depositar(int monto) {
        // TU CÓDIGO
        return false;
    }

    // TODO: extraer con lock_guard
    bool extraer(int monto) {
        // TU CÓDIGO
        return false;
    }

    // TODO: obtener_saldo con lock_guard
    int obtener_saldo() const {
        // TU CÓDIGO
        return 0;
    }

    // Acceso al mutex para transferencias (friend o método público)
    std::mutex& get_mutex() { return mtx_; }
    int& get_saldo_ref() { return saldo_; }
};

// TODO 2: transferir sin deadlock usando scoped_lock
bool transferir(CuentaBancaria& desde, CuentaBancaria& hasta, int monto) {
    // TU CÓDIGO: usar std::scoped_lock(desde.get_mutex(), hasta.get_mutex())
    // Verificar fondos en 'desde', si OK: desde.saldo_ -= monto; hasta.saldo_ += monto;
    return false;
}

// ============= TESTS =============
TEST_CASE(CuentaBasica) {
    CuentaBancaria c(100);
    ASSERT_TRUE(c.depositar(50));
    ASSERT_EQ(c.obtener_saldo(), 150);
    ASSERT_TRUE(c.extraer(30));
    ASSERT_EQ(c.obtener_saldo(), 120);
    ASSERT_FALSE(c.extraer(200));  // fondos insuficientes
    ASSERT_EQ(c.obtener_saldo(), 120);
}

TEST_CASE(TransferenciaSimple) {
    CuentaBancaria a(100), b(50);
    ASSERT_TRUE(transferir(a, b, 30));
    ASSERT_EQ(a.obtener_saldo(), 70);
    ASSERT_EQ(b.obtener_saldo(), 80);
    ASSERT_FALSE(transferir(a, b, 100));  // fondos insuficientes
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

    // Total debe conservarse: 2000
    int total = a.obtener_saldo() + b.obtener_saldo();
    ASSERT_EQ(total, 2000);
}

TEST_CASE(ScopedLockDosMutexes) {
    std::mutex m1, m2;
    // scoped_lock toma ambos atómicamente
    std::scoped_lock lock(m1, m2);
    ASSERT_TRUE(true);  // Si compila y no deadlock, OK
}

int main() {
    return RUN_ALL_TESTS();
}