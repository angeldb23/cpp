/* =============================================================================
   MÓDULO 14 - EJERCICIO 09: latch, barrier, semaphore (L13)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (tema 4 cont: C++20 sync primitives):
     1. WorkerPool con latch: N workers hacen trabajo, main espera con latch.wait()
     2. Pipeline con barrier: 3 etapas, cada iteración sincronizada con barrier
     3. ResourcePool con counting_semaphore: max N recursos concurrentes
     4. flex_barrier simulado: barrier con callback que imprime fase

    COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../../include ejercicio_09_sync_primitives.cpp -o ej09 && ./ej09
    ============================================================================= */
#include <iostream>
#include <thread>
#include <vector>
#include <latch>
#include <barrier>
#include <semaphore>
#include <chrono>
#include <atomic>
#include <mutex>
#include "../../include/test_framework.hpp"

// TODO 1: WorkerPool con latch
class WorkerPoolLatch {
    std::latch inicio_;
    std::latch fin_;
    std::vector<std::jthread> workers_;

public:
    WorkerPoolLatch(size_t n) : inicio_(n), fin_(n) {
        for (size_t i = 0; i < n; ++i) {
            workers_.emplace_back([this, i](std::stop_token) {
                inicio_.count_down();  // "estoy listo"
                std::this_thread::sleep_for(std::chrono::milliseconds(10 + i * 5));
                fin_.count_down();  // "terminé"
            });
        }
    }
    void esperar_inicio() { inicio_.wait(); }
    void esperar_fin() { fin_.wait(); }
};

// TODO 2: Pipeline con barrier
class PipelineBarrier {
    std::barrier<> barr_;
    std::vector<std::jthread> workers_;
    std::atomic<int> etapa_{0};

public:
    PipelineBarrier(size_t n) : barr_(n) {
        for (size_t i = 0; i < n; ++i) {
            workers_.emplace_back([this](std::stop_token) {
                for (int iter = 0; iter < 5; ++iter) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    auto token = barr_.arrive();
                    etapa_++;
                    barr_.wait(std::move(token));
                }
            });
        }
    }
    void ejecutar(int iteraciones) {
        (void)iteraciones;
    }
};

// TODO 3: ResourcePool con counting_semaphore
class ResourcePool {
    std::counting_semaphore<10> sem_{10};
    std::mutex mtx_;
    std::vector<int> disponibles_;

public:
    ResourcePool() { for (int i = 1; i <= 10; ++i) disponibles_.push_back(i); }

    int adquirir() {
        sem_.acquire();
        std::lock_guard<std::mutex> lk(mtx_);
        int id = disponibles_.back(); disponibles_.pop_back();
        return id;
    }
    void liberar(int id) {
        { std::lock_guard<std::mutex> lk(mtx_); disponibles_.push_back(id); }
        sem_.release();
    }
};

// TODO 4: Barrier con callback (simula flex_barrier)
void demo_barrier_callback() {
    std::barrier<> barr(3);
    std::atomic<int> fase{0};
    std::atomic<int> llegados{0};

    auto worker = [&](int id) {
        for (int f = 0; f < 3; ++f) {
            std::cout << "[worker " << id << "] fase " << f+1 << " trabajando\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            
            auto token = barr.arrive();
            if (++llegados == 3) {
                ++fase;
                std::cout << "[barrier callback] fase " << fase << " completada por todos\n";
                llegados = 0;
            }
            barr.wait(std::move(token));
        }
    };

    std::thread t1(worker, 1), t2(worker, 2), t3(worker, 3);
    t1.join(); t2.join(); t3.join();
    std::cout << "Barrier con callback OK: 3 fases completadas\n\n";
}

// ============= TESTS =============
TEST_CASE(WorkerPoolLatchEspera) {
    class WorkerPoolLatch {
        std::latch inicio_;
        std::latch fin_;
        std::vector<std::jthread> workers_;

    public:
        WorkerPoolLatch(size_t n) : inicio_(n), fin_(n) {
            for (size_t i = 0; i < n; ++i) {
                workers_.emplace_back([this, i](std::stop_token) {
                    inicio_.count_down();  // "estoy listo"
                    std::this_thread::sleep_for(std::chrono::milliseconds(10 + i * 5));
                    fin_.count_down();  // "terminé"
                });
            }
        }
        void esperar_inicio() { inicio_.wait(); }
        void esperar_fin() { fin_.wait(); }
    };

    WorkerPoolLatch pool(4);
    pool.esperar_inicio();  // todos workers listos
    pool.esperar_fin();     // todos terminaron
    ASSERT_TRUE(true);
}

TEST_CASE(ResourcePoolLimitaConcurrencia) {
    class ResourcePool {
        std::counting_semaphore<10> sem_{10};
        std::mutex mtx_;
        std::vector<int> disponibles_;
    public:
        ResourcePool() { for (int i = 1; i <= 10; ++i) disponibles_.push_back(i); }
        int adquirir() {
            sem_.acquire();
            std::lock_guard<std::mutex> lk(mtx_);
            int id = disponibles_.back(); disponibles_.pop_back();
            return id;
        }
        void liberar(int id) {
            { std::lock_guard<std::mutex> lk(mtx_); disponibles_.push_back(id); }
            sem_.release();
        }
    };

    ResourcePool pool;
    std::atomic<int> concurrentes{0}, max_concurrentes{0};

    auto worker = [&](int) {
        int r = pool.adquirir();
        concurrentes++;
        max_concurrentes = std::max(max_concurrentes.load(), concurrentes.load());
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        concurrentes--;
        pool.liberar(r);
    };

    std::vector<std::thread> hilos;
    for (int i = 0; i < 20; ++i) hilos.emplace_back([&]{ worker(i); });
    for (auto& t : hilos) t.join();

    ASSERT_EQ(max_concurrentes.load(), 10);  // máx 10 a la vez
}

TEST_CASE(BarrierSincronizaFases) {
    std::barrier<> barr(3);
    std::atomic<int> fase{0};
    std::atomic<bool> error{false};

    auto worker = [&](int) {
        for (int i = 0; i < 5; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            auto token = barr.arrive();
            if (fase.load() != i) error = true;  // todos en misma fase
            barr.wait(std::move(token));
        }
    };

    std::thread t1([&]{ worker(1); }), t2([&]{ worker(2); }), t3([&]{ worker(3); });
    t1.join(); t2.join(); t3.join();
    ASSERT_FALSE(error.load());
}

int main() {
    return RUN_ALL_TESTS();
}