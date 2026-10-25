/* =============================================================================
   MÓDULO 13 - LECCIÓN 14: APIs sin excepciones: error_code, expected<..., error_code>
   y patrones de interoperabilidad
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Muchas librerías y sistemas (C, POSIX, Win32, motores de juegos,
       bases de datos embebidas) NO usan excepciones. La interfaz C++ moderna
       para convivir con ellas tiene tres pilares:

       A) Funciones que devuelven std::error_code (out-param opcional):
            std::error_code leerArchivo(const std::string& path, std::string& contenido);
          El código 0 = éxito; cualquier otro valor = error con categoría.

       B) std::expected<T, std::error_code>: la firma que UNE valor y error
          en el tipo de retorno (C++23; en C++20 usamos mini::expected).
            mini::expected<std::string, std::error_code> leerArchivo(const std::string& path);
          Ventaja: el error VIAJA en el tipo, no hay out-param, y el que
          llama usa and_then/transform/or_else (L12) o if (!r) { ec = r.error(); }.

       C) Interoperabilidad con C / POSIX / Win32:
            - errno -> std::error_code(errno, std::system_category())
            - GetLastError() -> std::error_code(GetLastError(), std::system_category())
            - Códigos propios de librería C -> categoría propia (L13) + make_error_code.
          La regla: CADA boundary (límite) con código sin excepciones
          convierte INMEDIATAMENTE a error_code/expected. NO dejes errno
          suelto en variables globales; se pisa en la próxima llamada.

       Patrones de adaptación:
         - "Try" wrapper: template <typename F> expected<decltype(f()), error_code> try_call(F f)
           que atrapa excepciones internas y las convierte a error_code.
         - "Resultado o error": una función que puede fallar de N formas
           devuelve expected<T, error_code>; el error_code lleva LA categoría
           correcta (system, generic, o propia) según el origen.

       Conversión entre las tres familias del módulo:
         - Código de retorno (enum) -> error_code: make_error_code(mi_enum)
         - Excepción -> error_code: catch(...) { return error_code(...); }
         - expected<T, E> -> expected<T, error_code>: transform_error(a_error_code)
           donde el E propio sabe convertirse (p.ej. enum con is_error_code_enum).

       noexcept en APIs sin excepciones:
         - Si TU API promete no lanzar, marcala noexcept.
         - std::error_code y expected<T, error_code> son noexcept-movibles
           (sus operaciones no lanzan si T y E no lanzan).
         - Esto permite que contenedores y algoritmos usen move sin penalidad.

   2. ¿QUÉ PASA POR DEBAJO?
       error_code es trivially copyable (dos punteros). expected<T, error_code>
       contiene un T opcional y un error_code opcional (dos optionales en mini,
       una unión en std::expected). Mover un expected<T, error_code> es
       mover el T (si hay valor) o copiar el error_code (barato). No hay
       alocación, no hay unwinding. El costo es CERO en el camino feliz
       (solo registros), y en el camino de error es la construcción del
       error_code (dos asignaciones de puntero).

   3. BUENAS PRÁCTICAS:
       - En el boundary (llamar a C/POSIX/Win32): convierte YA a error_code.
       - En tu API pública: elegí UNA convención (error_code out-param
         O expected<T, error_code>) y sé consistente en el módulo.
       - NO mezcles excepciones y error_code en la misma función:
         o la función lanza, o devuelve error_code/expected. Si una
         función interna lanza, atrapa en el boundary y convierte.
       - Usá mini::expected<T, std::error_code> (o std::expected en C++23)
         para funciones que devuelven un valor Y pueden fallar.
       - Para funciones void que solo fallan: devuelven std::error_code
         (0 = ok). El que llama hace `if (ec) return ec;`.
       - Documentá la categoría esperada: "devuelve error_code con
         system_category en fallos de I/O, generic_category en fallos lógicos".
============================================================================= */
#include <iostream>
#include <system_error>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <functional>
#include <optional>

#include "../include/mini_expected.hpp"

namespace Leccion14 {

// ============================================================================
// Boundary: envolver llamada a C/POSIX (fopen, read, etc.) -> error_code
// ============================================================================
std::error_code leerArchivoRaw(const std::string& path, std::string& outContenido) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) {
        // errno ya está seteado por el constructor de ifstream fallido
        // Nota: ifstream no garantiza errno portable; en la práctica
        // usamos filesystem para error_code directo (abajo).
        return std::error_code(errno, std::system_category());
    }
    std::streamsize tam = f.tellg();
    f.seekg(0);
    outContenido.resize(static_cast<size_t>(tam));
    if (!f.read(outContenido.data(), tam)) {
        return std::error_code(errno, std::system_category());
    }
    return std::error_code{}; // éxito
}

// ============================================================================
// API moderna: expected<string, error_code> (usa filesystem que ya da error_code)
// ============================================================================
mini::expected<std::string, std::error_code> leerArchivo(const std::string& path) {
    namespace fs = std::filesystem;
    std::error_code ec;
    auto tam = fs::file_size(path, ec);
    if (ec) return mini::expected<std::string, std::error_code>(ec); // error_code viaja como error

    std::ifstream f(path, std::ios::binary);
    if (!f) {
        return mini::expected<std::string, std::error_code>(
            std::error_code(errno, std::system_category()));
    }

    std::string contenido;
    contenido.resize(static_cast<size_t>(tam));
    f.read(contenido.data(), static_cast<std::streamsize>(tam));
    if (!f) {
        return mini::expected<std::string, std::error_code>(
            std::error_code(errno, std::system_category()));
    }
    return contenido; // éxito implícito
}

// ============================================================================
// API void + error_code out: para operaciones que no devuelven valor
// ============================================================================
std::error_code escribirArchivo(const std::string& path, const std::string& datos) {
    namespace fs = std::filesystem;
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    if (!f) {
        return std::error_code(errno, std::system_category());
    }
    f.write(datos.data(), static_cast<std::streamsize>(datos.size()));
    if (!f) {
        return std::error_code(errno, std::system_category());
    }
    return std::error_code{}; // éxito
}

// ============================================================================
// Wrapper "try_call": atrapa excepciones y las convierte a error_code
// Útil cuando usas librerías que SÍ lanzan pero tu API no lanza.
// ============================================================================
template <typename F>
auto try_call(F&& f) -> mini::expected<std::decay_t<decltype(f())>, std::error_code> {
    try {
        return f();
    } catch (const std::system_error& e) {
        return e.code(); // error_code de la excepción
    } catch (const std::bad_alloc&) {
        return std::make_error_code(std::errc::not_enough_memory);
    } catch (const std::exception& e) {
        // Error genérico de lógica: mapeamos a generic_category
        return std::error_code(static_cast<int>(std::errc::invalid_argument),
                               std::generic_category());
    } catch (...) {
        return std::make_error_code(std::errc::io_error); // desconocido
    }
}

// ============================================================================
// Ejemplo de uso: pipeline con expected<..., error_code> y and_then
// ============================================================================
mini::expected<std::size_t, std::error_code> procesarArchivo(const std::string& path) {
    return leerArchivo(path)
        .and_then([](const std::string& contenido) -> mini::expected<std::size_t, std::error_code> {
            // Simulamos procesamiento: contar líneas
            std::size_t lineas = 1;
            for (char c : contenido) if (c == '\n') ++lineas;
            if (lineas == 0) {
                return std::make_error_code(std::errc::invalid_argument);
            }
            return lineas;
        })
        .transform_error([](std::error_code ec) -> std::error_code {
            // Traducimos el error a la capa de arriba: añadimos contexto
            // (en la práctica: log + mapear a código de dominio propio)
            return ec;
        });
}

} // namespace Leccion14

int main() {
    using namespace Leccion14;
    namespace fs = std::filesystem;

    // Preparar un archivo de prueba
    const char* testPath = "leccion14_test.txt";
    std::string testData = "linea 1\nlinea 2\nlinea 3\n";
    std::error_code ecWrite = escribirArchivo(testPath, testData);
    if (ecWrite) {
        std::cerr << "Setup falló: " << ecWrite.message() << '\n';
        return 1;
    }

    std::cout << "== API void + error_code out ==\n";
    std::string contenido;
    std::error_code ec1 = leerArchivoRaw(testPath, contenido);
    if (ec1) {
        std::cout << "  leerArchivoRaw falló: " << ec1.message() << '\n';
    } else {
        std::cout << "  leerArchivoRaw ok: " << contenido.size() << " bytes\n";
    }

    std::cout << "\n== API expected<string, error_code> ==\n";
    auto r1 = leerArchivo(testPath);
    if (r1) {
        std::cout << "  leerArchivo ok: " << r1.value().size() << " bytes\n";
    } else {
        std::cout << "  leerArchivo falló: " << r1.error().message() << '\n';
    }

    std::cout << "\n== Archivo que no existe ==\n";
    auto r2 = leerArchivo("no_existe_ni_en_suenos.txt");
    if (!r2) {
        std::cout << "  error esperado: " << r2.error().message()
                  << " (categoría: " << r2.error().category().name() << ")\n";
    }

    std::cout << "\n== Pipeline con and_then / transform_error ==\n";
    auto r3 = procesarArchivo(testPath);
    if (r3) {
        std::cout << "  procesarArchivo ok: " << r3.value() << " líneas\n";
    } else {
        std::cout << "  procesarArchivo falló: " << r3.error().message() << '\n';
    }

    std::cout << "\n== Pipeline con archivo inexistente ==\n";
    auto r4 = procesarArchivo("fantasma.txt");
    if (!r4) {
        std::cout << "  error propagado: " << r4.error().message() << '\n';
    }

    std::cout << "\n== try_call: envolver código que lanza ==\n";
    auto r5 = try_call([]() -> std::string {
        throw std::system_error(std::make_error_code(std::errc::permission_denied), "denegado");
    });
    if (!r5) {
        std::cout << "  atrapado y convertido: " << r5.error().message() << '\n';
    }

    auto r6 = try_call([]() -> int { throw std::bad_alloc(); });
    if (!r6) {
        std::cout << "  bad_alloc -> errc::not_enough_memory: " << r6.error().message() << '\n';
    }

    // Limpieza
    fs::remove(testPath);
    return 0;
}