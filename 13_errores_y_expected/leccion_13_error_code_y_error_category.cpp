/* =============================================================================
   MÓDULO 13 - LECCIÓN 13: std::error_code, std::errc y std::error_category
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Las excepciones no son universales: C++ permite compilar sin ellas
       (-fno-exceptions) y muchas APIs de sistema (POSIX, Windows) entregan
       errores por CÓDIGOS NUMÉRICOS (errno, GetLastError). std::error_code
       es el PUENTE: un tipo que guarda (valor, categoría) y sabe si
       representa "éxito" (valor == 0) o "fallo".

       - std::errc: enum class con los CÓDIGOS PORTABLES (subset de POSIX:
         errc::no_such_file_or_directory, errc::permission_denied,
         errc::timed_out, errc::resource_unavailable_try_again...).
         Es la "llave" estándar; cada SO mapea sus códigos a errc.

       - std::error_category: clase POLIMÓRFICA que sabe CÓMO interpretar
         un valor numérico dentro de SU dominio. Tres que ya vienen:
           * std::generic_category(): mapea errc -> mensaje ("Permission denied")
           * std::system_category(): mapea errno del SO -> mensaje
           * std::iostream_category(): para flags de iostream
         Podés escribir la TUYA para dominios propios (protocolos de red,
         formatos de archivo, errores de motor) heredando y sobrescribiendo
         name(), message(), default_error_condition(), equivalent().

       - std::error_code: la PAREJA (int valor, const error_category* cat).
         Construcción: error_code(errno, system_category());
         error_code(errc::no_such_file_or_directory); // implícito a generic
         Consulta: ec.value(), ec.category(), ec.message(), ec == errc::X.
         bool(ec) es true si value() != 0.

       La regla práctica: funciones que NO usan excepciones devuelven
       std::error_code (o expected<T, error_code>). El que llama decide
       si lanza, loguea o ignora. std::expected<T, std::error_code> es
       la firma moderna para "resultado o error con categoría".

   2. ¿QUÉ PASA POR DEBAJO?
       error_code son DOS PUNTEROS (valor + categoría): 16 bytes en 64 bits,
       trivially copyable, sin alocación. La categoría es un SINGLETON
       (instancia global por tipo): el puntero apunta a la misma dirección
       en todo el proceso. message() hace dispatch virtual al category
       concreto; es O(1) y el string se construye al vuelo (no hay tabla
       global de mensajes). equivalent() permite comparar cross-category:
         ec1 == errc::X  usa  ec1.category().equivalent(ec1.value(), errc::X)

   3. BUENAS PRÁCTICAS:
       - Usá errc cuando el error ES portable (archivo no existe, permiso).
       - category propia SOLO si el dominio no cabe en errc (p.ej. códigos
         de un protocolo de red: 1001=handshake_fail, 1002=version_mismatch).
       - NO inventes categorías para envolver errno: usá system_category().
       - message() es para LOG/debug; la LÓGICA usa value() o equivalent().
       - std::error_condition (hermana de error_code) es para COMPARAR
         cross-category: ec.default_error_condition() te da la condition
         portable (errc) si la categoría sabe mapear.
============================================================================= */
#include <iostream>
#include <system_error>
#include <string>
#include <vector>
#include <cerrno>
#include <cstring>

namespace Leccion13 {

// Enum de nuestros errores (empezamos en 1 porque 0 = éxito)
enum class ProtocoloError {
    HandshakeFallido   = 1,
    PaqueteCorrupto    = 2,
    Desincronizado     = 3,
    ConexionCerrada    = 4
};

// ============================================================================
// Categoría PROPIA: errores de un protocolo de juego imaginario
// ============================================================================
class ProtocoloJuegoCategory : public std::error_category {
public:
    const char* name() const noexcept override {
        return "protocolo_juego";
    }

    std::string message(int ev) const override {
        switch (static_cast<ProtocoloError>(ev)) {
            case ProtocoloError::HandshakeFallido:   return "handshake fallido: versión incompatible";
            case ProtocoloError::PaqueteCorrupto:    return "paquete corrupto: checksum inválido";
            case ProtocoloError::Desincronizado:     return "estado desincronizado: se esperaba ACK";
            case ProtocoloError::ConexionCerrada:    return "conexión cerrada por el par";
            default:                                 return "código de protocolo desconocido";
        }
    }

    // Mapea nuestro código a la condition portable (errc) más cercana.
    // Esto permite: ec == errc::connection_reset  (si mapeamos bien).
    std::error_condition default_error_condition(int ev) const noexcept override {
        switch (static_cast<ProtocoloError>(ev)) {
            case ProtocoloError::ConexionCerrada:
                return std::errc::connection_reset;
            case ProtocoloError::HandshakeFallido:
                return std::errc::protocol_error;
            case ProtocoloError::PaqueteCorrupto:
                return std::errc::bad_message;
            case ProtocoloError::Desincronizado:
                return std::errc::protocol_error;
            default:
                return std::error_condition(ev, *this);
        }
    }

    // equivalent: permite ec.equivalent(errc::X) y errc::X.equivalent(ec)
    bool equivalent(int code, const std::error_condition& cond) const noexcept override {
        return default_error_condition(code) == cond;
    }
    bool equivalent(const std::error_code& code, int condition) const noexcept override {
        return code.category() == *this && default_error_condition(code.value()) == std::error_condition(condition, std::generic_category());
    }
};

// Instancia global (singleton) de la categoría
const ProtocoloJuegoCategory& protocoloCategory() {
    static const ProtocoloJuegoCategory instance;
    return instance;
}

// Helper para construir error_code de nuestro dominio
inline std::error_code make_error_code(ProtocoloError e) {
    return std::error_code(static_cast<int>(e), protocoloCategory());
}

// ============================================================================
// Funciones que devuelven error_code (estilo sin excepciones)
// ============================================================================
// Simula recibir un paquete: puede fallar por red (system) o por protocolo (propio)
std::error_code recibirPaquete(std::vector<char>& buffer, bool simularFalloRed, bool simularFalloProtocolo) {
    if (simularFalloRed) {
        // Error del SO: usamos system_category() con un errno típico
        return std::error_code(ECONNRESET, std::system_category());
    }
    if (simularFalloProtocolo) {
        // Error de nuestro dominio
        return make_error_code(ProtocoloError::PaqueteCorrupto);
    }
    buffer = {'P', 'A', 'C', 'K', 'E', 'T'};
    return std::error_code{}; // éxito (value() == 0)
}

// Ejemplo de comparación cross-category
void diagnosticar(const std::error_code& ec) {
    std::cout << "  value: " << ec.value()
              << ", category: " << ec.category().name()
              << ", message: " << ec.message() << '\n';

    // ¿Es un "archivo no existe" portable?
    if (ec == std::errc::no_such_file_or_directory) {
        std::cout << "  -> coincide con errc::no_such_file_or_directory\n";
    }
    // ¿Es un "conexión reseteada" portable (vía default_error_condition)?
    if (ec == std::errc::connection_reset) {
        std::cout << "  -> coincide con errc::connection_reset (vía condition)\n";
    }
    // ¿Es un error de NUESTRO protocolo?
    if (ec.category() == protocoloCategory()) {
        std::cout << "  -> ES error de protocolo de juego\n";
    }
}

} // namespace Leccion13

// Habilitar comparación: error_code == ProtocoloError (ADL)
namespace std {
    template <>
    struct is_error_code_enum<Leccion13::ProtocoloError> : true_type {};
}

int main() {
    using namespace Leccion13;

    std::cout << "== error_code con system_category (errno) ==\n";
    std::error_code ec1(ENOENT, std::system_category());
    diagnosticar(ec1);

    std::cout << "\n== error_code con generic_category (errc) ==\n";
    std::error_code ec2 = std::make_error_code(std::errc::permission_denied);
    diagnosticar(ec2);

    std::cout << "\n== error_code con categoría PROPIA ==\n";
    std::error_code ec3 = make_error_code(ProtocoloError::HandshakeFallido);
    diagnosticar(ec3);

    std::cout << "\n== error_code con categoría PROPIA: conexión cerrada (mapea a errc::connection_reset) ==\n";
    std::error_code ec4 = make_error_code(ProtocoloError::ConexionCerrada);
    diagnosticar(ec4);

    std::cout << "\n== función estilo sin excepciones: recibirPaquete ==\n";
    std::vector<char> buf;
    std::error_code ec5 = recibirPaquete(buf, false, true); // fallo protocolo
    if (ec5) {
        std::cout << "  falló: " << ec5.message() << '\n';
    }
    std::error_code ec6 = recibirPaquete(buf, true, false); // fallo red
    if (ec6) {
        std::cout << "  falló: " << ec6.message() << " (errno=" << ec6.value() << ")\n";
    }
    std::error_code ec7 = recibirPaquete(buf, false, false); // éxito
    if (!ec7) {
        std::cout << "  éxito: recibido " << buf.size() << " bytes\n";
    }

    std::cout << "\n== bool(error_code): true si hay error ==\n";
    std::cout << "  ec1 (ENOENT): " << (ec1 ? "error" : "ok") << '\n';
    std::cout << "  ec7 (éxito):  " << (ec7 ? "error" : "ok") << '\n';

    return 0;
}