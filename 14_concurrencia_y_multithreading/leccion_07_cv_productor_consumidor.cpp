/* =============================================================================
   MÓDULO 14 - LECCIÓN 07: condition_variable avanzado — patrones realistas
   =============================================================================
   TEMA 4 de TEMAS.md (3 lecciones): condition_variable predicados,
   espera/notificación, productor-consumidor completo.

   PATRONES AVANZADOS DE CONDITION_VARIABLE:

   1. COLA ACOTADA (bounded queue) con backpressure:
      - Productores esperan si cola llena (cv_not_full).
      - Consumidores esperan si cola vacía (cv_not_empty).
      - Dos CVs para evitar "thundering herd" (notificar a todos innecesario).

   2. MÚLTIPLES CONDICIONES en la misma cola:
      - Un solo mutex, múltiples CVs (una por condición distinta).
      - notify_one en la CV correcta según qué cambió.

   3. TERMINACIÓN GRACEFUL:
      - Flag 'terminado' + notify_all para despertar a TODOS.
      - Consumidores drenan la cola antes de salir.

   4. WAIT CON TIMEOUT + REINTENTO:
      - wait_for devuelve cv_status::timeout si expira.
      - Útil para health checks, heartbeat, cancelación por tiempo.

   5. PREDICADOS COMPLEJOS:
      - El predicado puede ser cualquier callable que capture por referencia.
      - Debe ser thread-safe (leer bajo lock).
      - Evitá lógica compleja en predicado; mantenelo simple.

   6. SPURIOUS WAKEUPS (despertares espurios):
      - El estándar PERMITE que wait() despierte sin notify.
      - Por eso SIEMPRE predicado: while (!pred) wait();  (cv.wait lo hace).
      - NUNCA hagas: if (cond) wait();  -> BUG latente.

   7. NOTIFICACIÓN BAJO LOCK vs FUERA:
      - notify_one/all PUEDE llamarse con o sin lock.
      - Con lock: el waiter despierta e intenta tomar lock -> contención.
      - Sin lock: waiter toma lock limpio al despertar (menos contención).
      - RECOMENDACIÓN: notificá FUERA del lock (después de unlock).
        Excepto si necesitás atomicidad estricta entre cambio y notificación.

   EJEMPLOS: Cola acotada completa, múltiples CVs, terminación graceful,
   wait_for con reintento, notificación fuera de lock. */

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <chrono>
#include <optional>
#include <functional>

// -----------------------------------------------------------------------------
// 1) Cola acotada (bounded queue) thread-safe completa
// -----------------------------------------------------------------------------
template <typename T>
class BoundedQueue {
    std::queue<T> q_;
    size_t capacidad_;
    std::mutex mtx_;
    std::condition_variable cv_not_full_;   // para productores
    std::condition_variable cv_not_empty_;  // para consumidores
    bool cerrado_ = false;

public:
    explicit BoundedQueue(size_t cap) : capacidad_(cap) {}

    // Productor: bloquea si lleno, retorna false si cerrado
    bool push(T valor) {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_not_full_.wait(lk, [this] { return q_.size() < capacidad_ || cerrado_; });
        if (cerrado_) return false;
        q_.push(std::move(valor));
        lk.unlock();  // notificar FUERA del lock (menos contención)
        cv_not_empty_.notify_one();
        return true;
    }

    // Productor con timeout: retorna nullopt si timeout, false si cerrado
    std::optional<T> try_push_for(T valor, std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lk(mtx_);
        if (!cv_not_full_.wait_for(lk, timeout,
                                   [this] { return q_.size() < capacidad_ || cerrado_; })) {
            return std::nullopt;  // timeout
        }
        if (cerrado_) return std::nullopt;
        q_.push(std::move(valor));
        lk.unlock();
        cv_not_empty_.notify_one();
        return valor;  // éxito
    }

    // Consumidor: bloquea si vacío, retorna nullopt si cerrado y vacío
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_not_empty_.wait(lk, [this] { return !q_.empty() || cerrado_; });
        if (q_.empty() && cerrado_) return std::nullopt;
        T valor = std::move(q_.front());
        q_.pop();
        lk.unlock();
        cv_not_full_.notify_one();
        return valor;
    }

    // Consumidor con timeout
    std::optional<T> try_pop_for(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lk(mtx_);
        if (!cv_not_empty_.wait_for(lk, timeout,
                                    [this] { return !q_.empty() || cerrado_; })) {
            return std::nullopt;  // timeout
        }
        if (q_.empty() && cerrado_) return std::nullopt;
        T valor = std::move(q_.front());
        q_.pop();
        lk.unlock();
        cv_not_full_.notify_one();
        return valor;
    }

    void cerrar() {
        std::lock_guard<std::mutex> lk(mtx_);
        cerrado_ = true;
        cv_not_full_.notify_all();   // despertar TODOS los productores
        cv_not_empty_.notify_all();  // despertar TODOS los consumidores
    }

    bool esta_cerrada() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return cerrado_;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return q_.size();
    }
};

void demo_bounded_queue() {
    std::cout << "--- BoundedQueue thread-safe completa ---\n";
    BoundedQueue<int> cola(3);  // capacidad 3
    const int ITEMS_POR_PROD = 5;
    std::atomic<int> producidos{0}, consumidos{0};

    auto productor = [&](int id) {
        for (int i = 0; i < ITEMS_POR_PROD; ++i) {
            int val = id * 100 + i;
            if (!cola.push(val)) break;  // cola cerrada
            ++producidos;
            std::cout << "[prod " << id << "] push " << val << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    };

    auto consumidor = [&](int id) {
        while (true) {
            auto opt = cola.pop();
            if (!opt) break;  // cerrada y vacía
            ++consumidos;
            std::cout << "[cons " << id << "] pop " << *opt << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    };

    std::thread p1(productor, 1);
    std::thread p2(productor, 2);
    std::thread c1(consumidor, 1);
    std::thread c2(consumidor, 2);

    p1.join(); p2.join();
    cola.cerrar();  // señal de fin
    c1.join(); c2.join();

    std::cout << "Producidos: " << producidos << ", Consumidos: " << consumidos << "\n\n";
}

// -----------------------------------------------------------------------------
// 2) Múltiples condition_variables para condiciones distintas
// -----------------------------------------------------------------------------
class AlmacenMultiple {
    std::mutex mtx_;
    std::condition_variable cv_hay_espacio_;
    std::condition_variable cv_hay_items_;
    std::condition_variable cv_orden_especial_;  // condición extra: "item VIP"

    std::queue<int> cola_normal_;
    std::queue<int> cola_vip_;
    const size_t MAX_NORMAL = 10;
    bool terminado_ = false;

public:
    // Productor normal
    void push_normal(int item) {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_hay_espacio_.wait(lk, [this] { return cola_normal_.size() < MAX_NORMAL || terminado_; });
        if (terminado_) return;
        cola_normal_.push(item);
        lk.unlock();
        cv_hay_items_.notify_one();
    }

    // Productor VIP (prioridad alta)
    void push_vip(int item) {
        std::unique_lock<std::mutex> lk(mtx_);
        // VIP no espera por espacio (simplificación), pero podría tener su límite
        cola_vip_.push(item);
        lk.unlock();
        cv_hay_items_.notify_one();      // consumidor normal ve items
        cv_orden_especial_.notify_one(); // consumidor VIP ve orden especial
    }

    // Consumidor normal
    std::optional<int> pop_normal() {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_hay_items_.wait(lk, [this] { return !cola_normal_.empty() || !cola_vip_.empty() || terminado_; });
        if (cola_normal_.empty() && cola_vip_.empty() && terminado_) return std::nullopt;
        // Prioridad: VIP primero
        if (!cola_vip_.empty()) {
            int v = cola_vip_.front(); cola_vip_.pop();
            lk.unlock();
            cv_hay_espacio_.notify_one();
            return v;
        }
        int v = cola_normal_.front(); cola_normal_.pop();
        lk.unlock();
        cv_hay_espacio_.notify_one();
        return v;
    }

    // Consumidor VIP (solo procesa VIPs)
    std::optional<int> pop_vip() {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_orden_especial_.wait(lk, [this] { return !cola_vip_.empty() || terminado_; });
        if (cola_vip_.empty() && terminado_) return std::nullopt;
        int v = cola_vip_.front(); cola_vip_.pop();
        lk.unlock();
        cv_hay_espacio_.notify_one();
        return v;
    }

    void terminar() {
        std::lock_guard<std::mutex> lk(mtx_);
        terminado_ = true;
        cv_hay_espacio_.notify_all();
        cv_hay_items_.notify_all();
        cv_orden_especial_.notify_all();
    }
};

void demo_multiples_cv() {
    std::cout << "--- Múltiples CVs para condiciones distintas ---\n";
    AlmacenMultiple almacen;
    std::atomic<int> procesados_normal{0}, procesados_vip{0};

    auto prod_normal = [&](int id) {
        for (int i = 0; i < 5; ++i) {
            almacen.push_normal(id * 10 + i);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    };
    auto prod_vip = [&](int id) {
        for (int i = 0; i < 3; ++i) {
            almacen.push_vip(id * 100 + i);
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    };
    auto cons_normal = [&] {
        while (true) {
            auto opt = almacen.pop_normal();
            if (!opt) break;
            ++procesados_normal;
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }
    };
    auto cons_vip = [&] {
        while (true) {
            auto opt = almacen.pop_vip();
            if (!opt) break;
            ++procesados_vip;
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
        }
    };

    std::thread pn1(prod_normal, 1), pn2(prod_normal, 2);
    std::thread pv1(prod_vip, 1);
    std::thread cn(cons_normal), cv(cons_vip);

    pn1.join(); pn2.join(); pv1.join();
    almacen.terminar();
    cn.join(); cv.join();

    std::cout << "Procesados normal: " << procesados_normal << ", VIP: " << procesados_vip << "\n\n";
}

// -----------------------------------------------------------------------------
// 3) Terminación graceful: drenar cola antes de salir
// -----------------------------------------------------------------------------
class PipelineGraceful {
    std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<std::function<void()>> tareas_;
    bool parada_solicitada_ = false;
    bool drenando_ = false;

public:
    // Enqueue tarea (puede ser lambda con captura)
    template <typename F>
    bool enviar(F&& f) {
        std::unique_lock<std::mutex> lk(mtx_);
        if (parada_solicitada_) return false;
        tareas_.emplace(std::forward<F>(f));
        lk.unlock();
        cv_.notify_one();
        return true;
    }

    // Worker loop
    void trabajar(int id) {
        while (true) {
            std::function<void()> tarea;
            {
                std::unique_lock<std::mutex> lk(mtx_);
                cv_.wait(lk, [this] { return !tareas_.empty() || drenando_; });
                if (tareas_.empty() && drenando_) break;  // fin graceful
                tarea = std::move(tareas_.front());
                tareas_.pop();
            }
            tarea();  // ejecutar SIN lock
            std::cout << "[worker " << id << "] tarea ejecutada\n";
        }
        std::cout << "[worker " << id << "] terminó graceful\n";
    }

    // Solicita parada: no acepta nuevas tareas, despierta workers
    void solicitar_parada() {
        std::lock_guard<std::mutex> lk(mtx_);
        parada_solicitada_ = true;
        cv_.notify_all();
    }

    // Espera a que cola se vacíe, luego marca drenando y despierta
    void drenar_y_terminar() {
        {
            std::unique_lock<std::mutex> lk(mtx_);
            cv_.wait(lk, [this] { return tareas_.empty(); });
            drenando_ = true;
        }
        cv_.notify_all();
    }
};

void demo_graceful() {
    std::cout << "--- Terminación graceful (drenar cola) ---\n";
    PipelineGraceful pipe;

    std::thread w1([&] { pipe.trabajar(1); });
    std::thread w2([&] { pipe.trabajar(2); });

    // Enviar tareas
    for (int i = 0; i < 10; ++i) {
        int captura = i;
        pipe.enviar([captura] {
            std::cout << "  tarea " << captura << " corriendo\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        });
    }

    pipe.solicitar_parada();  // no más tareas nuevas
    pipe.drenar_y_terminar(); // espera vaciar + avisa fin

    w1.join(); w2.join();
    std::cout << "Pipeline graceful OK\n\n";
}

// -----------------------------------------------------------------------------
// 4) wait_for con reintento y cancelación cooperativa (stop_token)
// -----------------------------------------------------------------------------
void worker_con_stop_y_timeout(std::stop_token st, int id) {
    std::mutex mtx;
    std::condition_variable cv;
    bool listo = false;

    // stop_callback para despertar el wait si piden stop
    std::stop_callback cb(st, [&] {
        std::lock_guard<std::mutex> lk(mtx);
        listo = true;
        cv.notify_all();
    });

    while (!st.stop_requested()) {
        std::unique_lock<std::mutex> lk(mtx);
        // wait_for con timeout: permite chequear stop_requested() periódicamente
        // incluso si no hay notificación (útil si no hay stop_callback)
        // Usamos versión SIN predicado para obtener cv_status
        auto status = cv.wait_for(lk, std::chrono::milliseconds(500));

        if (st.stop_requested()) {
            std::cout << "[worker " << id << "] stop_requested -> salgo\n";
            break;
        }
        if (status == std::cv_status::timeout) {
            std::cout << "[worker " << id << "] timeout, chequeo stop...\n";
            continue;
        }
        // Despertó por notify: chequear predicado manualmente
        if (!listo) continue;  // spurious wakeup
        std::cout << "[worker " << id << "] notificado, procesando...\n";
        listo = false;  // reset para próxima vuelta
    }
}

void demo_stop_con_timeout() {
    std::cout << "--- wait_for + stop_token (cancelación cooperativa) ---\n";
    std::jthread jt(worker_con_stop_y_timeout, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    jt.request_stop();  // despierta via stop_callback
    std::cout << "Stop con timeout OK\n\n";
}

// -----------------------------------------------------------------------------
// 5) Notificación FUERA del lock (patrón recomendado)
// -----------------------------------------------------------------------------
std::mutex mtx_notify_out;
std::condition_variable cv_notify_out;
int dato_notify = 0;
bool hay_dato = false;

void productor_notify_out() {
    for (int i = 1; i <= 3; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        {
            std::lock_guard<std::mutex> lk(mtx_notify_out);
            dato_notify = i * 10;
            hay_dato = true;
        }  // lock liberado AQUÍ
        cv_notify_out.notify_one();  // NOTIFICAR FUERA del lock
        std::cout << "[prod] notificó sin lock\n";
    }
}

void consumidor_notify_out() {
    for (int i = 0; i < 3; ++i) {
        std::unique_lock<std::mutex> lk(mtx_notify_out);
        cv_notify_out.wait(lk, [] { return hay_dato; });
        std::cout << "[cons] recibió " << dato_notify << "\n";
        hay_dato = false;
    }
}

void demo_notify_fuera_lock() {
    std::cout << "--- Notificar FUERA del lock (menos contención) ---\n";
    hay_dato = false;
    std::thread p(productor_notify_out);
    std::thread c(consumidor_notify_out);
    p.join(); c.join();
    std::cout << "Notificación fuera de lock OK\n\n";
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main() {
    std::cout << "=== LECCIÓN 07: condition_variable avanzado — patrones realistas ===\n\n";

    demo_bounded_queue();
    demo_multiples_cv();
    demo_graceful();
    demo_stop_con_timeout();
    demo_notify_fuera_lock();

    std::cout << "=== FIN LECCIÓN 07 ===\n";
    return 0;
}