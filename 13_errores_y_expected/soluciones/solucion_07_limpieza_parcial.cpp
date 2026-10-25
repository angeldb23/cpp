/* =============================================================================
   MÓDULO 13 - SOLUCIÓN 07: Constructor y limpieza parcial
   =============================================================================
   POR QUÉ FUNCIONA:
     - Si un constructor lanza, el DESTRUCTOR NO SE EJECUTA (el objeto no
       "existe"). Por eso cada recurso adquirido antes del fallo debe
       liberarse MANUALMENTE en el constructor (try/catch + rethrow, o
       RAII intermedio).
     - Patrón: adquirir recurso 1 -> try { adquirir recurso 2 } catch { liberar 1; throw; }
     - Mejor aún: usar RAII para CADA recurso (unique_ptr, wrapper de socket,
       guard de transacción). Así el destructor del member libera si el
       constructor de la clase falla tras inicializar ese member.
     - En esta solución usamos members RAII (unique_ptr, SocketGuard, TransactionGuard)
       para que la limpieza sea automática aunque el constructor lance.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_07_limpieza_parcial.cpp -o sol07 && ./sol07
   ============================================================================= */
#include <iostream>
#include <memory>
#include <stdexcept>
#include <cstddef>
#include "../../include/test_framework.hpp"

// Contadores globales para verificar limpieza
static int g_sockets_abiertos = 0;
static int g_buffers_asignados = 0;
static int g_transacciones_activas = 0;

// RAII para socket
struct SocketGuard {
    int id = -1;
    SocketGuard() = default;
    explicit SocketGuard(int socket_id) : id(socket_id) {}
    ~SocketGuard() { if (id >= 0) --g_sockets_abiertos; }
    SocketGuard(const SocketGuard&) = delete;
    SocketGuard& operator=(const SocketGuard&) = delete;
    SocketGuard(SocketGuard&& other) noexcept : id(other.id) { other.id = -1; }
    SocketGuard& operator=(SocketGuard&& other) noexcept {
        if (this != &other) {
            if (id >= 0) --g_sockets_abiertos;
            id = other.id;
            other.id = -1;
        }
        return *this;
    }
    int get() const noexcept { return id; }
    void reset() noexcept { id = -1; }
};

// RAII para buffer
struct BufferDeleter {
    void operator()(char* p) const noexcept {
        if (p) {
            delete[] p;
            --g_buffers_asignados;
        }
    }
};

using BufferGuard = std::unique_ptr<char[], BufferDeleter>;

// RAII para transacción
struct TransactionGuard {
    bool activa = false;
    TransactionGuard() = default;
    explicit TransactionGuard(bool act) : activa(act) {}
    ~TransactionGuard() { if (activa) --g_transacciones_activas; }
    TransactionGuard(const TransactionGuard&) = delete;
    TransactionGuard& operator=(const TransactionGuard&) = delete;
    TransactionGuard(TransactionGuard&& other) noexcept : activa(other.activa) { other.activa = false; }
    TransactionGuard& operator=(TransactionGuard&& other) noexcept {
        if (this != &other) {
            if (activa) --g_transacciones_activas;
            activa = other.activa;
            other.activa = false;
        }
        return *this;
    }
    bool get() const noexcept { return activa; }
    void reset() noexcept { activa = false; }
};

// Funciones de adquisición (simuladas)
int acquireSocket() {
    if (g_sockets_abiertos >= 10) return -1;  // límite simulado
    ++g_sockets_abiertos;
    return g_sockets_abiertos - 1;  // id único
}

BufferGuard acquireBuffer(std::size_t tam) {
    if (g_buffers_asignados >= 10) return BufferGuard(nullptr);
    ++g_buffers_asignados;
    return BufferGuard(new char[tam]);
}

bool beginTransaction() {
    if (g_transacciones_activas >= 10) return false;
    ++g_transacciones_activas;
    return true;
}

// No necesitamos release explícitos: los guards lo hacen en su destructor

class ConexionBD {
public:
    ConexionBD() {
        // Paso 1: socket
        int sock_id = acquireSocket();
        if (sock_id < 0) throw std::runtime_error("socket");
        socket_ = SocketGuard(sock_id);

        // Paso 2: buffer
        auto buf = acquireBuffer(1024);
        if (!buf) throw std::runtime_error("buffer");
        buffer_ = std::move(buf);

        // Paso 3: transacción
        if (!beginTransaction()) throw std::runtime_error("transaccion");
        transaccion_ = TransactionGuard(true);
    }

    // Getters para tests
    int socketId() const noexcept { return socket_.get(); }
    bool tieneBuffer() const noexcept { return static_cast<bool>(buffer_); }
    bool transaccionActiva() const noexcept { return transaccion_.get(); }

private:
    SocketGuard socket_;
    BufferGuard buffer_;
    TransactionGuard transaccion_;
};

// ============= TESTS =============
TEST_CASE(TestConstruccionExitosa) {
    g_sockets_abiertos = 0; g_buffers_asignados = 0; g_transacciones_activas = 0;
    {
        ConexionBD c;
        ASSERT_EQ(g_sockets_abiertos, 1);
        ASSERT_EQ(g_buffers_asignados, 1);
        ASSERT_EQ(g_transacciones_activas, 1);
    }
    ASSERT_EQ(g_sockets_abiertos, 0);
    ASSERT_EQ(g_buffers_asignados, 0);
    ASSERT_EQ(g_transacciones_activas, 0);
}

TEST_CASE(TestFalloEnSocket) {
    g_sockets_abiertos = 10;  // forzar fallo
    g_buffers_asignados = 0; g_transacciones_activas = 0;
    bool lanzo = false;
    try {
        ConexionBD c;
    } catch (const std::runtime_error&) { lanzo = true; }
    ASSERT_TRUE(lanzo);
    ASSERT_EQ(g_sockets_abiertos, 10);  // no cambió (no se adquirió)
    ASSERT_EQ(g_buffers_asignados, 0);
    ASSERT_EQ(g_transacciones_activas, 0);
}

TEST_CASE(TestFalloEnBufferLiberaSocket) {
    g_sockets_abiertos = 0; g_buffers_asignados = 10; g_transacciones_activas = 0;  // forzar fallo en buffer
    bool lanzo = false;
    try {
        ConexionBD c;
    } catch (const std::runtime_error&) { lanzo = true; }
    ASSERT_TRUE(lanzo);
    // Socket se adquirió pero buffer falló -> SocketGuard destructor libera
    ASSERT_EQ(g_sockets_abiertos, 0);
    ASSERT_EQ(g_buffers_asignados, 10);
    ASSERT_EQ(g_transacciones_activas, 0);
}

TEST_CASE(TestFalloEnTransaccionLiberaBufferYSoket) {
    g_sockets_abiertos = 0; g_buffers_asignados = 0; g_transacciones_activas = 10;  // forzar fallo en transacción
    bool lanzo = false;
    try {
        ConexionBD c;
    } catch (const std::runtime_error&) { lanzo = true; }
    ASSERT_TRUE(lanzo);
    // Socket y buffer se adquirieron, transacción falló -> ambos guards liberan
    ASSERT_EQ(g_sockets_abiertos, 0);
    ASSERT_EQ(g_buffers_asignados, 0);
    ASSERT_EQ(g_transacciones_activas, 10);
}

TEST_CASE(TestDestructorLiberaTodo) {
    g_sockets_abiertos = 0; g_buffers_asignados = 0; g_transacciones_activas = 0;
    {
        ConexionBD c;
    }
    ASSERT_EQ(g_sockets_abiertos, 0);
    ASSERT_EQ(g_buffers_asignados, 0);
    ASSERT_EQ(g_transacciones_activas, 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 07 (Constructor y limpieza parcial)...\n";
    return RUN_ALL_TESTS();
}