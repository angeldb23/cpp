/* =============================================================================
   MÓDULO 15 - LECCIÓN 01: CMake — targets, target_*, PUBLIC/PRIVATE/INTERFACE
   =============================================================================
   TEMA 1 de TEMAS.md (2 lecciones): CMake moderno basado en targets.

   CMAKE MODERNO (3.10+) = TARGET-BASED:
   - Se acabó el include_directories(), link_libraries(), add_definitions() globales.
   - Todo se hace sobre targets: add_executable, add_library.
   - Propiedades por target: target_include_directories, target_link_libraries,
     target_compile_options, target_compile_definitions, target_sources.

   VISIBILIDAD (scope) — LA CLAVE:
   - PUBLIC:   se propaga a quien linkea este target (interface + implementación).
   - PRIVATE:  solo para compilar ESTE target (solo implementación).
   - INTERFACE: solo para quien linkea (solo interface, header-only libs).

   REGLAS PRÁCTICAS:
   - Headers que usan tus .cpp:           PRIVATE (o PUBLIC si los expone tu API).
   - Headers que forman parte de tu API:  PUBLIC (clientes los necesitan).
   - Librerías que usas internamente:     PRIVATE.
   - Librerías que forman parte de tu API: PUBLIC (clientes deben linkearlas).
   - Header-only libs:                    INTERFACE (no hay .cpp que compilar).

   EJEMPLO CLÁSICO:
     add_library(milib lib.cpp)
     target_include_directories(milib
         PUBLIC   include/        # headers de la API
         PRIVATE  src/            # headers internos
     )
     target_link_libraries(milib PUBLIC dependencia_externa)

   MODERNO: target_link_libraries(propaga includes y defines automáticamente). */

#include <iostream>
#include <vector>

// Demo: simular comportamiento de target_* con structs
// (CMake es un meta-lenguaje; esto ilustra la semántica en C++)

struct Target {
    std::vector<std::string> public_includes;
    std::vector<std::string> private_includes;
    std::vector<std::string> interface_includes;
    std::vector<std::string> public_deps;
    std::vector<std::string> private_deps;
    std::vector<std::string> interface_deps;

    void target_include_directories(const std::string& scope, const std::string& dir) {
        if (scope == "PUBLIC") {
            public_includes.push_back(dir);
            interface_includes.push_back(dir);  // PUBLIC propaga a interface
        } else if (scope == "PRIVATE") {
            private_includes.push_back(dir);
        } else if (scope == "INTERFACE") {
            interface_includes.push_back(dir);
        }
    }

    void target_link_libraries(const std::string& scope, const std::string& lib) {
        if (scope == "PUBLIC") {
            public_deps.push_back(lib);
            interface_deps.push_back(lib);
        } else if (scope == "PRIVATE") {
            private_deps.push_back(lib);
        } else if (scope == "INTERFACE") {
            interface_deps.push_back(lib);
        }
    }

    // Simular propagación transitiva
    std::vector<std::string> get_effective_includes() const {
        std::vector<std::string> result = public_includes;
        result.insert(result.end(), private_includes.begin(), private_includes.end());
        return result;
    }

    std::vector<std::string> get_consumer_includes() const {
        return interface_includes;  // Lo que ve quien linkea
    }
};

void demo_basico() {
    std::cout << "--- target_include_directories / target_link_libraries ---\n";

    Target mi_lib;

    // Headers de API (PUBLIC): clientes los necesitan
    mi_lib.target_include_directories("PUBLIC", "include/milib/");
    // Headers internos (PRIVATE): solo para compilar la lib
    mi_lib.target_include_directories("PRIVATE", "src/");

    // Dependencia que forma parte de la API (PUBLIC)
    mi_lib.target_link_libraries("PUBLIC", "fmt");
    // Dependencia solo interna (PRIVATE)
    mi_lib.target_link_libraries("PRIVATE", "spdlog");

    std::cout << "Includes para compilar mi_lib: ";
    for (auto& inc : mi_lib.get_effective_includes()) std::cout << inc << " ";
    std::cout << "\n";

    std::cout << "Includes que ve quien linkea mi_lib: ";
    for (auto& inc : mi_lib.get_consumer_includes()) std::cout << inc << " ";
    std::cout << "\n";

    std::cout << "Deps que ve quien linkea mi_lib: ";
    for (auto& dep : mi_lib.interface_deps) std::cout << dep << " ";
    std::cout << "\n\n";
}

// -----------------------------------------------------------------------------
// 2) Header-only library: solo INTERFACE
// -----------------------------------------------------------------------------
struct HeaderOnlyLib {
    std::vector<std::string> interface_includes;
    std::vector<std::string> interface_deps;

    void target_include_directories(const std::string& scope, const std::string& dir) {
        if (scope == "INTERFACE" || scope == "PUBLIC") {
            interface_includes.push_back(dir);
        }
    }
    void target_link_libraries(const std::string& scope, const std::string& lib) {
        if (scope == "INTERFACE" || scope == "PUBLIC") {
            interface_deps.push_back(lib);
        }
    }
};

void demo_header_only() {
    std::cout << "--- Header-only library (INTERFACE) ---\n";

    HeaderOnlyLib nlohmann_json;
    nlohmann_json.target_include_directories("INTERFACE", "include/");
    // No hay target_sources, no se compila nada

    std::cout << "Consumer includes: ";
    for (auto& inc : nlohmann_json.interface_includes) std::cout << inc << " ";
    std::cout << "\n";
    std::cout << "No hay binario, solo headers. CMake propaga includes a quien linkea.\n\n";
}

// -----------------------------------------------------------------------------
// 3) Alias targets: target_alias para nombres amigables
// -----------------------------------------------------------------------------
void demo_alias() {
    std::cout << "--- Alias targets ---\n";
    std::cout << "add_library(milib::milib ALIAS mi_lib)\n";
    std::cout << "Permite: target_link_libraries(mi_app PRIVATE milib::milib)\n";
    std::cout << "Namespacing automático, evita colisiones.\n\n";
}

// -----------------------------------------------------------------------------
// 4) Generator expressions: $<BUILD_INTERFACE>, $<INSTALL_INTERFACE>
// -----------------------------------------------------------------------------
void demo_genex() {
    std::cout << "--- Generator expressions ($<...>) ---\n";
    std::cout << "target_include_directories(mi_lib\n";
    std::cout << "  PUBLIC\n";
    std::cout << "    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>\n";
    std::cout << "    $<INSTALL_INTERFACE:include>\n";
    std::cout << ")\n";
    std::cout << "\n";
    std::cout << "BUILD_INTERFACE:  solo al compilar en el source tree\n";
    std::cout << "INSTALL_INTERFACE:  solo al instalar (ruta relatival al prefix)\n";
    std::cout << "Permite layouts distintos en source vs install.\n\n";
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main() {
    std::cout << "=== LECCIÓN 01: CMake — targets, target_*, PUBLIC/PRIVATE/INTERFACE ===\n\n";

    demo_basico();
    demo_header_only();
    demo_alias();
    demo_genex();

    std::cout << "=== FIN LECCIÓN 01 ===\n";
    return 0;
}