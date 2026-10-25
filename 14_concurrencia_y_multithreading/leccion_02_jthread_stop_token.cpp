/* =============================================================================
   MÓDULO 14 - LECCIÓN 02: jthread, stop_token/stop_source en profundidad
   =============================================================================
   TEMA 1 (continuación): jthread, stop_token, stop_source, stop_callback.
   Patrones de cancelación, join automático, y manejo de múltiples hilos.

   RECORDATORIO: jthread = thread + join automático en destructor + stop_token.
   stop_source: crea tokens y puede pedir stop (request_stop()).
   stop_token:  solo lee (stop_requested(), stop_possible()).
   stop_callback: registra callback que se ejecuta AL PEDIR stop.

   PATRÓN COMÚN: "Worker pool con parada cooperativa"
   - Cada worker tiene su stop_token.
   - El dueño guarda stop_source (o vector de sources).
   - Al apagar: recorre sources pidiendo stop, luego los jthreads hacen join solos.

   DETALLE IMPORTANTE: stop_token es COPIABLE (barato), stop_source NO.
   El token se pasa por valor a los hilos; el source se queda en el dueño.

   stop_possible(): devuelve false si no hay source asociado (token "muerto").
   Útil para optimizar: si !stop_possible(), no hace falta chequear en el loop.

   EJEMPLOS:
   1. stop_source + múltiples tokens (un source controla varios hilos).
   2. stop_callback para limpieza de recursos (sockets, handles, etc.).
   3. Combinación con condition_variable (patrón productor-consumidor).
   4. RAII wrapper para hilos legacy (std::thread) que quiera join auto. ===== */

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <stop_token>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>

// -----------------------------------------------------------------------------
// 1) Un stop_source controlando MÚLTIPLES hilos (tokens compartidos)
// -----------------------------------------------------------------------------
void worker_compartido(std::stop_token st, int id) {
    while (!st.stop_requested()) {
        std::cout << "[worker " << id << "] trabajando...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
    std::cout << "[worker " << id << "] stop recibido -> fin\n";
}

void demo_stop_source_compartido() {
    std::cout << "--- stop_source compartido ---\n";
    std::stop_source source;           // UN source para todos
    std::vector<std::jthread> hilos;

    for (int i = 0; i < 3; ++i) {
        // Cada jthread recibe una COPIA del token (barato, atómico interno)
        hilos.emplace_back(worker_compartido, source.get_token(), i);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    std::cout << "[main] pidiendo stop a TODOS via source...\n";
    source.request_stop();             // ¡Un solo llamado para todos!

    // Los jthreads hacen join automático al destruirse el vector al salir del scope.
    // Pero como pedimos stop ANTES, salen del loop y joinean rápido.
}

// -----------------------------------------------------------------------------
// 2) stop_callback para limpieza RAII (cerrar socket, handle, archivo)
// -----------------------------------------------------------------------------
struct RecursoCostoso {
    int id;
    RecursoCostoso(int i) : id(i) { std::cout << "[recurso " << id << "] adquirido\n"; }
    ~RecursoCostoso() { std::cout << "[recurso " << id << "] LIBERADO (dtor)\n"; }
    void usar() { std::cout << "[recurso " << id << "] en uso\n"; }
};

void worker_con_recurso(std::stop_token st, int id) {
    // El recurso se crea ANTES de registrar el callback, así el callback
    // captura el puntero/referencia y lo libera al pedir stop.
    auto recurso = std::make_unique<RecursoCostoso>(id);

    // Callback que se ejecuta CUANDO request_stop() en el token asociado.
    // Capturamos recurso por referencia (válido mientras el hilo viva).
    std::stop_callback cb(st, [&recurso]() {
        std::cout << "[callback] liberando recurso en stop...\n";
        recurso.reset();  // llama dtor de RecursoCostoso
    });

    while (!st.stop_requested()) {
        recurso->usar();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    // Si el loop sale por stop_requested(), el callback YA corrió.
    // Si sale por otra razón, el unique_ptr libera en su dtor igual.
    std::cout << "[worker " << id << "] fin\n";
}

void demo_stop_callback_raii() {
    std::cout << "\n--- stop_callback para RAII ---\n";
    std::jthread jt(worker_con_recurso, 42);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    jt.request_stop();  // dispara callback -> libera recurso -> join auto
    // jt destructor hace join (ya terminó)
}

// -----------------------------------------------------------------------------
// 3) Patrón: condition_variable + stop_token (productor-consumidor básico)
// -----------------------------------------------------------------------------
void consumidor_con_stop(std::stop_token st,
                         std::mutex& mtx,
                         std::condition_variable& cv,
                         std::vector<int>& cola,
                         bool& terminado) {
    while (true) {
        std::unique_lock<std::mutex> lk(mtx);
        // wait con predicado: espera hasta que haya datos O stop pedido
        cv.wait(lk, [&] { return !cola.empty() || st.stop_requested() || terminado; });

        if (st.stop_requested() && cola.empty()) {
            std::cout << "[consumidor] stop y cola vacía -> salgo\n";
            break;
        }
        if (terminado && cola.empty()) {
            std::cout << "[consumidor] terminado y cola vacía -> salgo\n";
            break;
        }

        int valor = cola.back();
        cola.pop_back();
        lk.unlock();  // desbloquear antes de procesar (buena práctica)
        std::cout << "[consumidor] procesó " << valor << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void productor(std::mutex& mtx, std::condition_variable& cv,
               std::vector<int>& cola, int items, std::stop_source& source) {
    for (int i = 0; i < items; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        {
            std::lock_guard<std::mutex> lk(mtx);
            cola.push_back(i);
        }
        cv.notify_one();
    }
    // Cuando termina de producir, pide stop para que el consumidor sepa
    // que no vendrán más items (además de la flag 'terminado').
    source.request_stop();
}

void demo_cv_con_stop_token() {
    std::cout << "\n--- condition_variable + stop_token ---\n";
    std::mutex mtx;
    std::condition_variable cv;
    std::vector<int> cola;
    bool terminado = false;
    std::stop_source source;

    std::jthread cons(consumidor_con_stop, source.get_token(), std::ref(mtx),
                      std::ref(cv), std::ref(cola), std::ref(terminado));
    std::jthread prod(productor, std::ref(mtx), std::ref(cv),
                      std::ref(cola), 10, std::ref(source));

    // Ambos jthreads hacen join automático al salir del scope.
    // El productor pide stop al terminar; el consumidor ve stop_requested()
    // y cola vacía -> sale.
}

// -----------------------------------------------------------------------------
// 4) Wrapper RAII para std::thread legacy (join automático)
// -----------------------------------------------------------------------------
class ThreadRAII {
    std::thread t_;
public:
    explicit ThreadRAII(std::thread&& t) : t_(std::move(t)) {}
    ~ThreadRAII() { if (t_.joinable()) t_.join(); }  // join auto como jthread
    ThreadRAII(ThreadRAII&&) = default;
    ThreadRAII& operator=(ThreadRAII&&) = default;
    ThreadRAII(const ThreadRAII&) = delete;
    ThreadRAII& operator=(const ThreadRAII&) = delete;
    std::thread::id get_id() const { return t_.get_id(); }
    bool joinable() const { return t_.joinable(); }
    void join() { if (t_.joinable()) t_.join(); }
    void detach() { if (t_.joinable()) t_.detach(); }
};

void demo_thread_raii() {
    std::cout << "\n--- ThreadRAII wrapper para std::thread ---\n";
    ThreadRAII tr(std::thread([]{
        for (int i = 0; i < 3; ++i) {
            std::cout << "[ThreadRAII] iter " << i << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }));
    // Sin join() explícito: el dtor de ThreadRAII lo hace.
    std::cout << "[main] ThreadRAII scope ending -> join auto\n";
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main() {
    std::cout << "=== LECCIÓN 02: jthread, stop_token/stop_source en profundidad ===\n\n";

    demo_stop_source_compartido();
    demo_stop_callback_raii();
    demo_cv_con_stop_token();
    demo_thread_raii();

    std::cout << "\n=== FIN LECCIÓN 02 ===\n";
    return 0;
}