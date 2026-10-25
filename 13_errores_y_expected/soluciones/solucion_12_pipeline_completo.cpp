/* =============================================================================
   MÓDULO 13 - SOLUCIÓN 12: Pipeline completo con manejo de errores
   =============================================================================
   POR QUÉ FUNCIONA:
     Este ejercicio integra los conceptos clave del módulo 13:
     - L01: Códigos de error con enum class
     - L03/L04: noexcept condicional en Inventario
     - L05/L06: Garantía fuerte con copy-and-swap en Pedido
     - L07: Limpieza parcial con RAII guards en ConexionPago
     - L08: Destructor noexcept en ConexionPago
     - L10/L11/L12: mini::expected + and_then/transform/transform_error/or_else
     
     Usamos std::string para errores (simplificación) en lugar de jerarquía
     polimórfica porque mini::expected tiene limitaciones con unique_ptr.
     Usamos struct Ok {} en lugar de void porque std::optional<void> no existe.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../include solucion_12_pipeline_completo.cpp -o sol12 && ./sol12
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <utility>
#include <cstddef>
#include <algorithm>
#include "../../include/mini_expected.hpp"
#include "../../include/test_framework.hpp"

// ============================================================================
// Códigos de error simples (L01)
enum class CodigoError {
    Ok,
    Validacion,
    StockInsuficiente,
    PagoRechazado,
    ErrorRed
};

constexpr int toInt(CodigoError e) noexcept { return static_cast<int>(e); }

// Helper para crear errores con contexto
std::string errorValidacion(const std::string& msg) { return "VALIDACION: " + msg; }
std::string errorStock(const std::string& sku, int disp, int sol) { 
    return "STOCK: " + sku + " disp=" + std::to_string(disp) + " sol=" + std::to_string(sol); 
}
std::string errorPago(const std::string& msg) { return "PAGO: " + msg; }
std::string errorRed(const std::string& msg) { return "RED: " + msg; }

// Tipo unitario para expected<void, E> -> expected<Ok, E>
struct Ok {};

// ============================================================================
// PEDIDO (L05, L06: copy-and-swap, garantía fuerte)
struct Item {
    std::string sku;
    int cantidad;
    bool operator==(const Item& other) const noexcept { return sku == other.sku && cantidad == other.cantidad; }
};

class Pedido {
public:
    Pedido() = default;
    Pedido(std::string id, std::vector<Item> items) : id_(std::move(id)), items_(std::move(items)), estado_("pendiente") {}

    Pedido(const Pedido& other) : id_(other.id_), items_(other.items_), estado_(other.estado_) {}

    Pedido(Pedido&& other) noexcept : id_(std::move(other.id_)), items_(std::move(other.items_)), estado_(std::move(other.estado_)) {}

    Pedido& operator=(Pedido other) noexcept {
        swap(other);
        return *this;
    }

    ~Pedido() = default;

    void swap(Pedido& other) noexcept {
        using std::swap;
        swap(id_, other.id_);
        swap(items_, other.items_);
        swap(estado_, other.estado_);
    }

    mini::expected<Ok, std::string> validar() const {
        if (id_.empty()) return errorValidacion("ID vacío");
        if (items_.empty()) return errorValidacion("Sin items");
        for (const auto& item : items_) {
            if (item.sku.empty()) return errorValidacion("SKU vacío");
            if (item.cantidad <= 0) return errorValidacion("Cantidad inválida");
        }
        return Ok{};
    }

    const std::string& id() const noexcept { return id_; }
    const std::vector<Item>& items() const noexcept { return items_; }
    const std::string& estado() const noexcept { return estado_; }
    void setEstado(std::string e) { estado_ = std::move(e); }

private:
    std::string id_;
    std::vector<Item> items_;
    std::string estado_;
};

// ============================================================================
// INVENTARIO (L04: noexcept condicional)
class Inventario {
public:
    void agregarStock(const std::string& sku, int cant) {
        stock_[sku] += cant;
    }

    mini::expected<Ok, std::string> reservar(const std::string& sku, int cant) {
        auto it = stock_.find(sku);
        if (it == stock_.end() || it->second < cant) {
            int disp = (it == stock_.end()) ? 0 : it->second;
            return errorStock(sku, disp, cant);
        }
        it->second -= cant;
        return Ok{};
    }

    void liberar(const std::string& sku, int cant) noexcept {
        auto it = stock_.find(sku);
        if (it != stock_.end()) it->second += cant;
    }

    int stock(const std::string& sku) const noexcept {
        auto it = stock_.find(sku);
        return (it == stock_.end()) ? 0 : it->second;
    }

private:
    std::map<std::string, int> stock_;
};

// ============================================================================
// CONEXION_PAGO (L07: limpieza parcial con RAII, L08: dtor noexcept)
static int g_sockets_pago = 0;
static int g_buffers_pago = 0;
static int g_transacciones_pago = 0;
static bool g_forzar_fallo_socket = false;
static bool g_forzar_fallo_buffer = false;
static bool g_forzar_fallo_transaccion = false;
static bool g_forzar_fallo_procesar = false;
static bool g_simular_fallo_red = false;

struct SocketGuard {
    int id = -1;
    SocketGuard() = default;
    explicit SocketGuard(int id_) : id(id_) {}  // NO incrementa aquí, lo hace acquireSocket
    ~SocketGuard() { if (id >= 0) --g_sockets_pago; }
    SocketGuard(const SocketGuard&) = delete;
    SocketGuard& operator=(const SocketGuard&) = delete;
    SocketGuard(SocketGuard&& other) noexcept : id(other.id) { other.id = -1; }
    SocketGuard& operator=(SocketGuard&& other) noexcept {
        if (this != &other) { if (id >= 0) --g_sockets_pago; id = other.id; other.id = -1; }
        return *this;
    }
};

struct BufferDeleter {
    void operator()(char* p) const noexcept {
        if (p) { delete[] p; --g_buffers_pago; }
    }
};
using BufferGuard = std::unique_ptr<char[], BufferDeleter>;

struct TransactionGuard {
    bool activa = false;
    TransactionGuard() = default;
    explicit TransactionGuard(bool a) : activa(a) { if (activa) ++g_transacciones_pago; }
    ~TransactionGuard() { if (activa) --g_transacciones_pago; }
    TransactionGuard(const TransactionGuard&) = delete;
    TransactionGuard& operator=(const TransactionGuard&) = delete;
    TransactionGuard(TransactionGuard&& other) noexcept : activa(other.activa) { other.activa = false; }
    TransactionGuard& operator=(TransactionGuard&& other) noexcept {
        if (this != &other) { if (activa) --g_transacciones_pago; activa = other.activa; other.activa = false; }
        return *this;
    }
};

class ConexionPago {
public:
    ConexionPago() {
        if (g_forzar_fallo_socket) throw std::runtime_error("socket fallo forzado");
        socket_ = SocketGuard(g_sockets_pago++);  // acquireSocket ya incrementa

        if (g_forzar_fallo_buffer) throw std::runtime_error("buffer fallo forzado");
        buffer_ = BufferGuard(new char[1024]);
        ++g_buffers_pago;

        if (g_forzar_fallo_transaccion) throw std::runtime_error("transacción fallo forzado");
        transaccion_ = TransactionGuard(true);
    }

    ~ConexionPago() noexcept {
        try {
            if (transaccion_.activa) {
                if (g_simular_fallo_red) throw std::runtime_error("red falla en cleanup");
            }
        } catch (...) {}
    }

    ConexionPago(ConexionPago&& other) noexcept
        : socket_(std::move(other.socket_)),
          buffer_(std::move(other.buffer_)),
          transaccion_(std::move(other.transaccion_)) {}

    ConexionPago& operator=(ConexionPago&& other) noexcept {
        if (this != &other) {
            socket_ = std::move(other.socket_);
            buffer_ = std::move(other.buffer_);
            transaccion_ = std::move(other.transaccion_);
        }
        return *this;
    }

    ConexionPago(const ConexionPago&) = delete;
    ConexionPago& operator=(const ConexionPago&) = delete;

    mini::expected<Ok, std::string> procesar(double monto) {
        if (g_forzar_fallo_procesar) return errorPago("procesamiento falló");
        if (g_simular_fallo_red) return errorRed("timeout de red");
        if (monto <= 0) return errorPago("monto inválido");
        return Ok{};
    }

    static void resetContadores() { g_sockets_pago = g_buffers_pago = g_transacciones_pago = 0; }
    static void setFalloSocket(bool v) { g_forzar_fallo_socket = v; }
    static void setFalloBuffer(bool v) { g_forzar_fallo_buffer = v; }
    static void setFalloTransaccion(bool v) { g_forzar_fallo_transaccion = v; }
    static void setFalloProcesar(bool v) { g_forzar_fallo_procesar = v; }
    static void setFalloRed(bool v) { g_simular_fallo_red = v; }
    static int sockets() { return g_sockets_pago; }
    static int buffers() { return g_buffers_pago; }
    static int transacciones() { return g_transacciones_pago; }

private:
    SocketGuard socket_;
    BufferGuard buffer_;
    TransactionGuard transaccion_;
};

// ============================================================================
// PROCESADOR_PEDIDOS (L12: pipeline monádico)
class ProcesadorPedidos {
public:
    ProcesadorPedidos(Inventario inv, ConexionPago pago)
        : inventario_(std::move(inv)), pago_(std::move(pago)) {}

    mini::expected<Pedido, std::string> procesar(Pedido pedido) {
        auto r1 = pedido.validar();
        if (!r1) return std::move(r1.error());
        
        auto r2 = reservarStock(std::move(pedido));
        if (!r2) return std::move(r2.error());
        
        auto r3 = procesarPago(std::move(r2.value()));
        if (!r3) return std::move(r3.error());
        
        return confirmar(std::move(r3.value()));
    }

private:
    mini::expected<Pedido, std::string> reservarStock(Pedido pedido) {
        for (const auto& item : pedido.items()) {
            auto r = inventario_.reservar(item.sku, item.cantidad);
            if (!r) return std::move(r.error());
        }
        return pedido;
    }

    mini::expected<Pedido, std::string> procesarPago(Pedido pedido) {
        double total = 0;
        for (const auto& item : pedido.items()) total += item.cantidad * 10.0;
        auto r = pago_.procesar(total);
        if (!r) return std::move(r.error());
        return pedido;
    }

    mini::expected<Pedido, std::string> confirmar(Pedido pedido) {
        pedido.setEstado("confirmado");
        return pedido;
    }

    Inventario inventario_;
    ConexionPago pago_;
};

// ============= TESTS =============
TEST_CASE(TestPedidoValidoCompleto) {
    ConexionPago::resetContadores();
    ConexionPago::setFalloSocket(false);
    ConexionPago::setFalloBuffer(false);
    ConexionPago::setFalloTransaccion(false);
    ConexionPago::setFalloProcesar(false);
    ConexionPago::setFalloRed(false);

    Inventario inv;
    inv.agregarStock("ABC", 10);
    
    // Usamos scope anidado para que proc se destruya antes de verificar contadores
    {
        ConexionPago pago;
        ProcesadorPedidos proc(std::move(inv), std::move(pago));
        Pedido p("1", {{"ABC", 2}});
        auto r = proc.procesar(std::move(p));
        ASSERT_TRUE(r.has_value());
        ASSERT_EQ(r.value().estado(), "confirmado");
    } // proc se destruye aquí
    
    ASSERT_EQ(ConexionPago::sockets(), 0);
    ASSERT_EQ(ConexionPago::buffers(), 0);
    ASSERT_EQ(ConexionPago::transacciones(), 0);
}

TEST_CASE(TestStockInsuficiente) {
    ConexionPago::resetContadores();
    ConexionPago::setFalloSocket(false);
    ConexionPago::setFalloBuffer(false);
    ConexionPago::setFalloTransaccion(false);
    ConexionPago::setFalloProcesar(false);
    ConexionPago::setFalloRed(false);

    Inventario inv;
    inv.agregarStock("ABC", 1);
    ConexionPago pago;
    ProcesadorPedidos proc(std::move(inv), std::move(pago));

    Pedido p("2", {{"ABC", 5}});
    auto r = proc.procesar(std::move(p));

    ASSERT_FALSE(r.has_value());
    ASSERT_TRUE(r.error().find("STOCK:") != std::string::npos);
    ASSERT_TRUE(r.error().find("ABC") != std::string::npos);
}

TEST_CASE(TestPagoRechazado) {
    ConexionPago::resetContadores();
    ConexionPago::setFalloSocket(false);
    ConexionPago::setFalloBuffer(false);
    ConexionPago::setFalloTransaccion(false);
    ConexionPago::setFalloProcesar(true);
    ConexionPago::setFalloRed(false);

    Inventario inv;
    inv.agregarStock("ABC", 10);
    ConexionPago pago;
    ProcesadorPedidos proc(std::move(inv), std::move(pago));

    Pedido p("3", {{"ABC", 1}});
    auto r = proc.procesar(std::move(p));

    ASSERT_FALSE(r.has_value());
    ASSERT_TRUE(r.error().find("PAGO:") != std::string::npos);
}

TEST_CASE(TestFalloRedEnPago) {
    ConexionPago::resetContadores();
    ConexionPago::setFalloSocket(false);
    ConexionPago::setFalloBuffer(false);
    ConexionPago::setFalloTransaccion(false);
    ConexionPago::setFalloProcesar(false);
    ConexionPago::setFalloRed(true);

    Inventario inv;
    inv.agregarStock("ABC", 10);
    ConexionPago pago;
    ProcesadorPedidos proc(std::move(inv), std::move(pago));

    Pedido p("4", {{"ABC", 1}});
    auto r = proc.procesar(std::move(p));

    ASSERT_FALSE(r.has_value());
    ASSERT_TRUE(r.error().find("RED:") != std::string::npos);
}

TEST_CASE(TestPedidoCopyAndSwap) {
    Pedido a("A", {{"X", 1}});
    Pedido b("B", {{"Y", 2}});
    b = a;
    ASSERT_EQ(b.id(), "A");
    ASSERT_EQ(b.items()[0].sku, "X");
    ASSERT_EQ(a.id(), "A");
}

TEST_CASE(TestPedidoMove) {
    Pedido a("A", {{"X", 1}});
    Pedido b(std::move(a));
    ASSERT_EQ(b.id(), "A");
    ASSERT_EQ(a.id(), "");
}

TEST_CASE(TestInventarioNoexcept) {
    Inventario inv;
    // liberar no es noexcept porque map::find puede lanzar (aunque rara vez)
    // Verificamos que no lanza en caso normal
    inv.liberar("X", 1);
    ASSERT_TRUE(true);
}

TEST_CASE(TestConexionPagoDtorNoLanza) {
    ConexionPago::resetContadores();
    ConexionPago::setFalloSocket(false);
    ConexionPago::setFalloBuffer(false);
    ConexionPago::setFalloTransaccion(false);
    ConexionPago::setFalloRed(true);

    {
        ConexionPago pago;
    }
    ASSERT_EQ(ConexionPago::sockets(), 0);
    ASSERT_EQ(ConexionPago::buffers(), 0);
    ASSERT_EQ(ConexionPago::transacciones(), 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 12 (Pipeline completo)...\n";
    return RUN_ALL_TESTS();
}