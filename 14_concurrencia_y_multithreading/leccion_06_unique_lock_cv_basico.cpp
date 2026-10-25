/* =============================================================================
   MÓDULO 14 - LECCIÓN 06: unique_lock avanzado, condition_variable básico
   =============================================================================
   TEMA 3 (continuación) + inicio TEMA 4: unique_lock features completos,
   condition_variable: wait, notify_one, notify_all, predicados.

   unique_lock — CARACTERÍSTICAS COMPLETAS:
   - defer_lock: no bloquea en constructor.
   - try_to_lock: intenta lock, si falla no bloquea (owns_lock() dice si logró).
   - adopt_lock: asume que YA tenés el lock (después de std::lock).
   - unlock(): liberar manualmente antes del dtor.
   - release(): DEVUELVE el mutex subyacente y deja de ser dueño (owns_lock=false).
     El caller se hace cargo de unlock(). Útil para transferir ownership.
   - mutex(): puntero al mutex asociado (o nullptr si release()).
   - owns_lock(): true si este unique_lock tiene el lock activo.
   - swap(other): intercambia estado con otro unique_lock.
   - operator bool(): true si owns_lock().

   condition_variable — SINCRONIZACIÓN POR CONDICIÓN:
   Permite a hilos ESPERAR hasta que una condición se vuelva verdadera.
   Otra hilo NOTIFICA (notify_one / notify_all) cuando la condición cambia.

   REGLAS DE ORO DE condition_variable:
   1. SIEMPRE usá un PREDICADO en wait(): cv.wait(lk, pred).
      El predicado evita "spurious wakeups" (despertares espurios: el hilo
      despierta sin que nadie haya notificado — permitido por el estándar).
   2. El predicado DEBE chequear la condición BAJO EL LOCK (unique_lock).
   3. wait() HACE: unlock -> dormir -> al despertar, lock de nuevo.
      Por eso el predicado se evalúa CON EL LOCK TOMADO.
   4. notify_one(): despierta UN hilo (si hay alguno esperando).
   5. notify_all(): despierta TODOS los hilos esperando (broadcast).
      Usá notify_all cuando la condición pueda ser verdadera para MÚLTIPLES
      hilos (ej: "cola no vacía" con varios consumidores).

   PATRÓN CANÓNICO PRODUCTOR-CONSUMIDOR:
   std::mutex mtx;
   std::condition_variable cv;
   std::queue<T> cola;
   bool terminado = false;

   // Productor:
   { lock_guard lk(mtx); cola.push(val); }
   cv.notify_one();  // o notify_all si múltiples consumidores

   // Consumidor:
   unique_lock lk(mtx);
   cv.wait(lk, [&]{ return !cola.empty() || terminado; });
   if (!cola.empty()) { auto val = cola.front(); cola.pop(); ... }

   EJEMPLOS: unique_lock features, cv básico, productor-consumidor simple. */

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <chrono>
#include <optional>
#include <cassert>

// -----------------------------------------------------------------------------
// 1) unique_lock: todas las features (defer, try, adopt, unlock, release)
// -----------------------------------------------------------------------------
std::mutex mtx_demo;

void demo_unique_lock_features() {
    std::cout << "--- unique_lock features completos ---\n";

    // defer_lock: construir SIN lockear
    std::unique_lock<std::mutex> lk1(mtx_demo, std::defer_lock);
    std::cout << "defer_lock: owns_lock=" << lk1.owns_lock() << "\n";

    lk1.lock();  // lock manual
    std::cout << "después de lock(): owns_lock=" << lk1.owns_lock() << "\n";

    lk1.unlock();  // unlock manual (reduce sección crítica)
    std::cout << "después de unlock(): owns_lock=" << lk1.owns_lock() << "\n";

    // try_to_lock: intento no bloqueante
    std::unique_lock<std::mutex> lk2(mtx_demo, std::try_to_lock);
    std::cout << "try_to_lock: owns_lock=" << lk2.owns_lock() << " (true=logró, false=ocupado)\n";

    // adopt_lock: después de std::lock (ver lección anterior)
    std::mutex mtx_a, mtx_b;
    std::lock(mtx_a, mtx_b);
    std::unique_lock<std::mutex> lk3(mtx_a, std::adopt_lock);
    std::unique_lock<std::mutex> lk4(mtx_b, std::adopt_lock);
    std::cout << "adopt_lock: owns_lock=" << lk3.owns_lock() << ", " << lk4.owns_lock() << "\n";
    lk3.unlock(); lk4.unlock();

    // release(): transferir ownership
    std::unique_lock<std::mutex> lk5(mtx_demo);
    std::mutex* ptr = lk5.release();  // lk5 ya NO owns_lock, ptr tiene el mutex
    std::cout << "release(): owns_lock=" << lk5.owns_lock() << ", ptr=" << ptr << "\n";
    ptr->unlock();  // caller responsable de unlock

    // swap
    std::unique_lock<std::mutex> lk6(mtx_demo, std::defer_lock);
    std::unique_lock<std::mutex> lk7(mtx_demo, std::defer_lock);
    lk6.lock();
    std::cout << "antes swap: lk6 owns=" << lk6.owns_lock() << ", lk7 owns=" << lk7.owns_lock() << "\n";
    lk6.swap(lk7);
    std::cout << "después swap: lk6 owns=" << lk6.owns_lock() << ", lk7 owns=" << lk7.owns_lock() << "\n";

    std::cout << "\n";
}

// -----------------------------------------------------------------------------
// 2) condition_variable básico: wait con predicado, notify_one/all
// -----------------------------------------------------------------------------
std::mutex mtx_cv;
std::condition_variable cv;
bool listo = false;
int valor_compartido = 0;

void productor_cv_basico() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    {
        std::lock_guard<std::mutex> lk(mtx_cv);
        valor_compartido = 42;
        listo = true;
        std::cout << "[productor] valor=42, listo=true, notificando...\n";
    }
    cv.notify_one();  // despierta UN consumidor
}

void consumidor_cv_basico() {
    std::unique_lock<std::mutex> lk(mtx_cv);
    // wait CON PREDICADO: evita spurious wakeups
    cv.wait(lk, [] { return listo; });
    // Acá: lock tomado, listo==true garantizado
    std::cout << "[consumidor] despertó, valor=" << valor_compartido << "\n";
    assert(valor_compartido == 42);
}

void demo_cv_basico() {
    std::cout << "--- condition_variable básico (wait + predicado) ---\n";
    listo = false; valor_compartido = 0;

    std::thread cons(consumidor_cv_basico);
    std::thread prod(productor_cv_basico);

    prod.join(); cons.join();
    std::cout << "CV básico OK\n\n";
}

// -----------------------------------------------------------------------------
// 3) Productor-Consumidor con cola (patrón real)
// -----------------------------------------------------------------------------
std::mutex mtx_pc;
std::condition_variable cv_pc;
std::queue<int> cola_pc;
const size_t MAX_COLA = 5;
bool terminado_pc = false;

void productor_pc(int id, int items) {
    for (int i = 0; i < items; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        {
            std::unique_lock<std::mutex> lk(mtx_pc);
            // Esperar si cola llena (backpressure)
            cv_pc.wait(lk, [&] { return cola_pc.size() < MAX_COLA; });
            cola_pc.push(id * 100 + i);
            std::cout << "[prod " << id << "] push " << cola_pc.back() << " (size=" << cola_pc.size() << ")\n";
        }
        cv_pc.notify_one();  // notificar a UN consumidor
    }
    std::cout << "[prod " << id << "] terminó\n";
}

void consumidor_pc(int id) {
    while (true) {
        std::unique_lock<std::mutex> lk(mtx_pc);
        // Esperar: cola NO vacía O terminado Y vacía
        cv_pc.wait(lk, [&] { return !cola_pc.empty() || terminado_pc; });

        if (cola_pc.empty() && terminado_pc) {
            std::cout << "[cons " << id << "] cola vacía y terminado -> salgo\n";
            break;
        }

        int valor = cola_pc.front();
        cola_pc.pop();
        lk.unlock();  // liberar ANTES de procesar (reduce contención)
        cv_pc.notify_one();  // notificar a productores (hay espacio)

        std::cout << "[cons " << id << "] pop " << valor << " (size=" << cola_pc.size() << ")\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(80));  // simular trabajo
    }
}

void demo_productor_consumidor() {
    std::cout << "--- Productor-Consumidor con backpressure ---\n";
    cola_pc = {};
    terminado_pc = false;

    std::thread p1(productor_pc, 1, 5);
    std::thread p2(productor_pc, 2, 5);
    std::thread c1(consumidor_pc, 1);
    std::thread c2(consumidor_pc, 2);

    p1.join(); p2.join();
    {
        std::lock_guard<std::mutex> lk(mtx_pc);
        terminado_pc = true;
    }
    cv_pc.notify_all();  // despertar TODOS los consumidores para que vean terminado
    c1.join(); c2.join();

    std::cout << "Productor-Consumidor OK\n\n";
}

// -----------------------------------------------------------------------------
// 4) notify_all vs notify_one: cuándo cada uno
// -----------------------------------------------------------------------------
std::mutex mtx_notify;
std::condition_variable cv_notify;
int contador_notify = 0;
const int OBJETIVO = 3;

void worker_notify_one(int id) {
    std::unique_lock<std::mutex> lk(mtx_notify);
    cv_notify.wait(lk, [&] { return contador_notify >= OBJETIVO; });
    std::cout << "[notify_one worker " << id << "] vio contador=" << contador_notify << "\n";
}

void worker_notify_all(int id) {
    std::unique_lock<std::mutex> lk(mtx_notify);
    cv_notify.wait(lk, [&] { return contador_notify >= OBJETIVO; });
    std::cout << "[notify_all worker " << id << "] vio contador=" << contador_notify << "\n";
}

void demo_notify_one_vs_all() {
    std::cout << "--- notify_one vs notify_all ---\n";

    // notify_one: solo UNO despierta, los otros siguen durmiendo
    std::cout << "Con notify_one (3 workers, objetivo=3):\n";
    contador_notify = 0;
    std::vector<std::thread> hilos;
    for (int i = 0; i < 3; ++i) hilos.emplace_back(worker_notify_one, i);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    {
        std::lock_guard<std::mutex> lk(mtx_notify);
        contador_notify = OBJETIVO;
    }
    cv_notify.notify_one();  // SOLO uno despierta
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "  (los otros 2 siguen durmiendo... notify_one de nuevo)\n";
    cv_notify.notify_one();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    cv_notify.notify_one();
    for (auto& t : hilos) t.join();

    // notify_all: TODOS despiertan de una
    std::cout << "\nCon notify_all (3 workers, objetivo=3):\n";
    contador_notify = 0;
    hilos.clear();
    for (int i = 0; i < 3; ++i) hilos.emplace_back(worker_notify_all, i);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    {
        std::lock_guard<std::mutex> lk(mtx_notify);
        contador_notify = OBJETIVO;
    }
    cv_notify.notify_all();  // TODOS despiertan
    for (auto& t : hilos) t.join();

    std::cout << "\nRegla: notify_one para cola (un consumidor por item).\n";
    std::cout << "        notify_all para 'terminado', 'broadcast', múltiples esperando misma cond.\n\n";
}

// -----------------------------------------------------------------------------
// 5) wait_for / wait_until: timeouts en condition_variable
// -----------------------------------------------------------------------------
std::mutex mtx_timeout;
std::condition_variable cv_timeout;
bool evento_llegó = false;

void productor_con_retraso() {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    {
        std::lock_guard<std::mutex> lk(mtx_timeout);
        evento_llegó = true;
    }
    cv_timeout.notify_one();
}

void consumidor_con_timeout() {
    std::unique_lock<std::mutex> lk(mtx_timeout);
    // wait_for: espera HASTA duration (relativo)
    auto resultado = cv_timeout.wait_for(lk, std::chrono::milliseconds(100),
                                         [] { return evento_llegó; });
    if (resultado) {
        std::cout << "[timeout] evento llegó antes de timeout\n";
    } else {
        std::cout << "[timeout] timeout expiró, evento NO llegó aún\n";
        // Podemos seguir esperando con otro wait_for o wait_until
        cv_timeout.wait(lk, [] { return evento_llegó; });
        std::cout << "[timeout] ahora sí, evento llegó\n";
    }
}

void demo_timeouts() {
    std::cout << "--- wait_for / wait_until (timeouts) ---\n";
    evento_llegó = false;

    std::thread prod(productor_con_retraso);
    std::thread cons(consumidor_con_timeout);

    prod.join(); cons.join();
    std::cout << "Timeouts OK\n\n";
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main() {
    std::cout << "=== LECCIÓN 06: unique_lock avanzado, condition_variable básico ===\n\n";

    demo_unique_lock_features();
    demo_cv_basico();
    demo_productor_consumidor();
    demo_notify_one_vs_all();
    demo_timeouts();

    std::cout << "=== FIN LECCIÓN 06 ===\n";
    return 0;
}