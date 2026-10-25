/* =============================================================================
   MÓDULO 07 - EJERCICIO 09: El deleter propio y el ARCHIVO sin fugas
   =============================================================================
   DIFICULTAD: Medio
   PRACTICA: lección 12 (deleters: recursos estilo C que no son memoria;
   unique_ptr<FILE, D> con functor sin capturas; el fclose con flush).

   El archivo se escribe EN DISCO: los tests lo releen, verifican y BORRAN.

   CONSIGNA:
      1. `abrirParaEscritura(const char* ruta)`: devuelve un
            std::unique_ptr<FILE, DeleterArchivo>
         construido con fichero y TODO (el DeleterArchivo ya está dado:
         hace fclose). Si fopen falla, unique_ptr vacío (sin crash).
      2. `escribirYCerrar(ruta, contenido)`: abre con tu fábrica, escribe
         el contenido con std::fputs y cierra... SIN llamar fclose: corre
         por RAII (el dtor del unique_ptr). Devuelve true solo si se abrió
         Y la escritura salió bien (fputs != EOF); false en cualquier
         contradicción.
      3. El test de fopen nulo: ruta imposible -> tu función devuelve
         false sin abrir nada raro.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_09_deleter_y_archivo.cpp -o ej09 && ./ej09
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

// (dado): el deleter abierto: cerrar ES la liberación del recurso FILE*.
void DeleterArchivo::operator()(FILE* f) const {
    if (f != nullptr) std::fclose(f);
}

// TODO 1: la fábrica del RAII de archivo.
ArchivoPtr abrirParaEscritura(const char* ruta) {
    (void)ruta;
    return ArchivoPtr(nullptr);       // PLACEHOLDER
}

// TODO 2: abrir, escribir (con chequeo de fputs) y DEJAR que el RAII cierre.
bool escribirYCerrar(const char* ruta, const std::string& contenido) {
    (void)ruta; (void)contenido;
    return false;                     // PLACEHOLDER
}

// El borrado del archivo de prueba al terminar los tests (dado):
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
        ASSERT_TRUE(f != nullptr);   // el fopen funcionó
    }   // el dtor del unique hace fclose (nunca a mano)
    FILE* releer = std::fopen("archivito_test.txt", "r");
    ASSERT_TRUE(releer != nullptr);  // se pudo reabrir: el fclose corrió
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
    // RUTA IMPOSIBLE: fopen devuelve nullptr: el RAII queda vacío y no
    // hay crash: la función reporta false.
    ASSERT_TRUE(!escribirYCerrar("/carpeta/que/no/existe/x.txt", "x"));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 09 (deleter/archivo)...\n";
    return RUN_ALL_TESTS();
}
