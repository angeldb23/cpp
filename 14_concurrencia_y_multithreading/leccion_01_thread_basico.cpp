/* =============================================================================
   MÓDULO 14 - LECCIÓN 01: std::thread — creación, join, detach y jthread
   =============================================================================
   TEMA 1 de TEMAS.md (2 lecciones): std::thread, join/detach, jthread y stop_token.

   ¿QUÉ ES UN HILO?
   Un hilo (thread) es una secuencia de instrucciones que el SO programa para
   ejecutar en un core. En C++11 llegó std::thread: un objeto que REPRESENTA
   ese hilo de ejecución. El hilo REAL empieza cuando se construye el objeto
   con una función/llamable; el objeto std::thread es solo el "control remoto".

   IMPORTANTE: std::thread NO ES COPIABLE (move-only). Se mueve, no se copia.
   Esto evita el clásico bug de "dos objetos controlando el mismo hilo".

   JOIN vs DETACH — LA DECISIÓN CRÍTICA:
   - join(): el hilo actual ESPERA a que el otro termine. Recupera el estado,
     propaga excepciones si las hubo, y el destructor del std::thread queda
     "vacío" (no hay hilo asociado). ES LA FORMA SEGURA POR DEFECTO.
   - detach(): el hilo queda "huérfano": corre solo, el SO lo limpia al morir.
     El objeto std::thread se vacía INMEDIATAMENTE. PELIGRO: si el main
     termina antes que el hilo detached, el proceso muere y el hilo se corta
     en seco (destructores de locales NO corren). Úsalo SOLO para hilos
     "daemon" que no tocan recursos del main y cuyo final no te importa.

   REGLA DE ORO: si creás un std::thread, TENÉS que hacer join() O detach()
   ANTES de que el objeto se destruya. Si no, std::terminate() te mata el
   programa (el destructor de std::thread chequea joinable() y aborta).

   C++20: std::jthread (joining thread)
   ===================================
   jthread es std::thread PERO con join() AUTOMÁTICO en el destructor.
   Además trae stop_token/stop_source para cancelación cooperativa.
   Si el scope termina y el jthread sigue corriendo, el destructor hace join()
   por vos. Nada de terminate() por olvido.

   stop_token / stop_source (C++20):
   - stop_source: quien QUIERE pedir la parada (el "dueño" del hilo).
   - stop_token:  quien RECIBE el pedido (el hilo que corre).
   - request_stop(): pone el token en "parado".
   - stop_requested(): el hilo chequea esto periódicamente y sale limpio.
   - stop_callback: registrá una lambda que se ejecute cuando pidan stop
     (útil para despertar bloqueos en colas, condition_variable, etc.).

   EJEMPLOS ABAJO: creación básica, join, detach (con advertencia), jthread
   con stop_token y stop_callback para cancelación limpia. ================= */

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <stop_token>   // C++20: stop_token, stop_source, stop_callback
#include <functional>   // std::ref
#include <mutex>
#include <condition_variable>

// -----------------------------------------------------------------------------
// 1) Hilo básico con función libre y join()
// -----------------------------------------------------------------------------
void tarea_saludar(const std::string& nombre, int repeticiones) {
    for (int i = 0; i < repeticiones; ++i) {
        std::cout << "[hilo " << std::this_thread::get_id() << "] Hola " << nombre << " (" << i+1 << ")\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

// -----------------------------------------------------------------------------
// 2) Hilo con lambda y captura por referencia (CUIDADO con lifetime!)
// -----------------------------------------------------------------------------
void demo_lambda_con_ref() {
    int contador = 0;
    // std::ref(contador) envuelve la referencia para que el hilo la use.
    // SIN std::ref, la lambda captura POR VALOR (copia) y el main no ve cambios.
    std::thread t([&contador]() {
        for (int i = 0; i < 1000; ++i) ++contador;
    });
    t.join();
    std::cout << "Contador final (con ref): " << contador << " (esperado 1000)\n";
}

// -----------------------------------------------------------------------------
// 3) detach() — SOLO para hilos "fuego y olvido" que NO tocan nada del main
// -----------------------------------------------------------------------------
[[noreturn]] void hilo_daemon_eterno() {
    while (true) {
        std::cout << "[daemon] vivo...\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
// NOTA: si main termina, este hilo muere SIN correr destructores de locales.
// Usar solo para logging, métricas, etc. que no necesiten cleanup.

// -----------------------------------------------------------------------------
// 4) jthread + stop_token: cancelación cooperativa (C++20)
// -----------------------------------------------------------------------------
void tarea_cancelable(std::stop_token st, int id) {
    for (int i = 0; !st.stop_requested(); ++i) {
        std::cout << "[jthread " << id << "] iteración " << i << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "[jthread " << id << "] stop pedido -> saliendo limpio\n";
}

// -----------------------------------------------------------------------------
// 5) stop_callback: despertar esperas al pedir stop (patrón común en colas)
// -----------------------------------------------------------------------------
void tarea_con_callback(std::stop_token st) {
    std::mutex mtx;
    std::condition_variable cv;
    bool listo = false;

    // El callback se ejecuta CUANDO alguien llama request_stop() en el source.
    // Acá lo usamos para notificar a la condition_variable y despertar el wait.
    std::stop_callback cb(st, [&]() {
        std::lock_guard<std::mutex> lk(mtx);
        listo = true;
        cv.notify_all();
        std::cout << "[callback] stop pedido -> notifiqué cv\n";
    });

    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk, [&] { return listo; });  // espera hasta que callback marque listo
    std::cout << "[tarea] desperté por stop_callback\n";
}

// -----------------------------------------------------------------------------
// 6) Mover hilos (std::thread es move-only)
// -----------------------------------------------------------------------------
void demo_move_thread() {
    std::thread t1(tarea_saludar, "movido", 2);
    std::thread t2 = std::move(t1);  // t1 queda vacío, t2 controla el hilo
    // t1.join(); // UB: t1 ya no tiene hilo asociado (joinable() == false)
    t2.join();
    std::cout << "[move] t2 join ok, t1 joinable=" << t1.joinable() << "\n";
}

// -----------------------------------------------------------------------------
// MAIN: corre todas las demos
// -----------------------------------------------------------------------------
int main() {
    std::cout << "=== LECCIÓN 01: std::thread, join, detach, jthread ===\n\n";

    // 1) thread + join
    std::cout << "--- 1) thread + join ---\n";
    std::thread t1(tarea_saludar, "Mundo", 3);
    t1.join();
    std::cout << "t1 terminó\n\n";

    // 2) lambda con ref
    std::cout << "--- 2) lambda con std::ref ---\n";
    demo_lambda_con_ref();
    std::cout << "\n";

    // 3) detach (comentado para no dejar hilo vivo al salir de main)
    // std::thread td(hilo_daemon_eterno);
    // td.detach();  // main termina -> proceso muere -> daemon se corta
    std::cout << "--- 3) detach (demo omitida: dejaría hilo huérfano) ---\n\n";

    // 4) jthread con stop_token
    std::cout << "--- 4) jthread + stop_token ---\n";
    {
        std::jthread jt1(tarea_cancelable, 1);
        std::jthread jt2(tarea_cancelable, 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        // Al salir del scope, jt1 y jt2 hacen join() AUTOMÁTICO.
        // Pero antes pedimos stop cooperativo:
        jt1.request_stop();
        jt2.request_stop();
        // Los destructores harán join() y esperarán a que salgan del loop.
    }
    std::cout << "jthreads terminados (join automático en destructor)\n\n";

    // 5) stop_callback
    std::cout << "--- 5) stop_callback despertando cv ---\n";
    {
        std::jthread jt(tarea_con_callback);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        jt.request_stop();  // dispara el callback -> notifica cv -> hilo despierta
    }
    std::cout << "callback demo ok\n\n";

    // 6) move
    std::cout << "--- 6) move de thread ---\n";
    demo_move_thread();

    std::cout << "\n=== FIN LECCIÓN 01 ===\n";
    return 0;
}