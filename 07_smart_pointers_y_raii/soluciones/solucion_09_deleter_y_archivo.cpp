/* =============================================================================
   MÓDULO 07 - SOLUCIÓN 09: El deleter propio y el ARCHIVO sin fugas
   =============================================================================
   Práctica de la lección 12: un unique_ptr con deleter propio envuelve el
   FILE* estilo C: el cierre (y su flush) es el dtor del RAII, no una línea
   que puede olvidarse.
   =============================================================================
 */
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include "../../include/test_framework.hpp"

struct DeleterArchivo {
    void operator()(FILE* f) const;
};

using ArchivoPtr = std::unique_ptr<FILE, DeleterArchivo>;

void DeleterArchivo::operator()(FILE* f) const {
    if (f != nullptr) std::fclose(f);
}

ArchivoPtr abrirParaEscritura(const char* ruta) {
    // El constructor del unique_ptr adopta el haya o no haya recurso:
    // si fopen dio nullptr, el RAII queda vacío y seguro.
    return ArchivoPtr(std::fopen(ruta, "w"));
}

bool escribirYCerrar(const char* ruta, const std::string& contenido) {
    ArchivoPtr f = abrirParaEscritura(ruta);   // RAII desde el primer paso
    if (f == nullptr) {
        return false;                          // fopen falló: no hay recurso
    }
    if (std::fputs(contenido.c_str(), f.get()) == EOF) {
        return false;                          // la E/S falla DE VERDAD
    }
    return true;
    // acá sale el scope: dtor de ArchivoPtr -> fclose -> flush a disco.
}

struct Limpieza {
    const char* ruta;
    explicit Limpieza(const char* r) : ruta(r) {}
    ~Limpieza() { std::remove(ruta); }
    Limpieza(const Limpieza&)            = delete;
    Limpieza& operator=(const Limpieza&) = delete;
};

// ============= TESTS =============
TEST_CASE(AbrirYCerrarConDeleter) {
    Limpieza limpia("archivito_test.txt");
    {
        ArchivoPtr f = abrirParaEscritura("archivito_test.txt");
        ASSERT_TRUE(f != nullptr);
    }
    FILE* releer = std::fopen("archivito_test.txt", "r");
    ASSERT_TRUE(releer != nullptr);
    if (releer != nullptr) std::fclose(releer);
}

TEST_CASE(EscribeYCierraOk) {
    Limpieza limpia("puntaje.txt");
    ASSERT_TRUE(escribirYCerrar("puntaje.txt", "hola\nmundo\n"));
    FILE* releer = std::fopen("puntaje.txt", "r");
    ASSERT_TRUE(releer != nullptr);
    char buffer[64] = {};
    const std::size_t leidos = std::fread(buffer, 1, 63, releer);
    ASSERT_TRUE(std::string(buffer, leidos) == std::string("hola\nmundo\n"));
    std::fclose(releer);
}

TEST_CASE(ArchivoImposibleEsFalse) {
    ASSERT_TRUE(!escribirYCerrar("/carpeta/que/no/existe/x.txt", "x"));
}

int main() {
    std::cout << "Ejecutando SOLUCIÓN 09 (deleter/archivo)...\n";
    return RUN_ALL_TESTS();
}
