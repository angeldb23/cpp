/* =============================================================================
   MÓDULO 12 - SOLUCIÓN 08: leer TODO de una vs línea por línea (ej. 08)
   =============================================================================
   L13: el completo (seekg + tellg + UN read) gana para estado completo
   en un paquete: memoria de una vez y SIEMPRE idéntico al disco.
   El getline-loop gana para PROCESAR línea por línea (una en memoria).
   El contador de matches corre sobre el contenido YA en memoria.
   ============================================================================= */
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

// 1) el completo A2: medir, reservar, UN read:
std::string leerTodo(const std::string& ruta, bool* ok) {
    const auto falla = [&] {
        if (ok) *ok = false;
        return std::string{};
    };
    std::ifstream f(ruta, std::ios::binary);
    if (!f) return falla();                     // apertura fallida
    f.seekg(0, std::ios::end);
    const std::streamsize tamano = f.tellg();
    if (tamano < 0) return falla();             // el path apunta a raro
    f.seekg(0, std::ios::beg);
    std::string datos(static_cast<size_t>(tamano), '\0');
    if (tamano > 0) f.read(datos.data(), tamano);
    if (ok) *ok = !f.bad();          // el eof del read final: NO se reporta
    return datos;
}

// 2) el getline-loop B: UNA línea en memoria a la vez:
std::vector<std::string> leerLineas(const std::string& ruta) {
    std::ifstream f(ruta);
    std::vector<std::string> lineas;
    if (!f) return lineas;
    std::string linea;
    while (std::getline(f, linea)) {
        lineas.push_back(linea);       // las VACÍAS también van
    }
    return lineas;
}

// 3) el counter sobre el contenido ya copiado (SIN re-abrir):
size_t contarSubcadenas(const std::string& ruta, const std::string& patron) {
    bool ok = false;
    const std::string contenido = leerTodo(ruta, &ok);
    if (!ok || patron.empty()) return 0;
    size_t total = 0;
    size_t donde = 0;
    while ((donde = contenido.find(patron, donde)) != std::string::npos) {
        ++total;
        donde += patron.size();        // SIN superposición: salta el match
    }
    return total;
}

// ============= TESTS =============
bool escribir(const std::string& ruta, const std::string& contenido) {
    std::ofstream f(ruta, std::ios::binary);
    f << contenido;
    return true;
}

std::string unir(const std::vector<std::string>& lineas) {
    std::string todo;
    for (const std::string& l : lineas) {
        todo += l;
        todo += '\n';
    }
    return todo;
}

TEST_CASE(DosLecturasConcuerdanConSaltoFinal) {
    const std::string ruta = "/tmp/opencode/curso12_ej08_r1.txt";
    escribir(ruta, "a\nbb\nccc\n");
    bool ok = false;
    const std::string todo = leerTodo(ruta, &ok);
    ASSERT_TRUE(ok);
    const auto lineas = leerLineas(ruta);
    ASSERT_EQ(lineas.size(), 3u);
    ASSERT_EQ(unir(lineas), todo);
}

TEST_CASE(UltimaLineaSinSaltoCuentaIgual) {
    const std::string ruta = "/tmp/opencode/curso12_ej08_r2.txt";
    escribir(ruta, "una\ndos");
    const auto lineas = leerLineas(ruta);
    ASSERT_EQ(lineas.size(), 2u);
    ASSERT_EQ(lineas[0], "una");
    ASSERT_EQ(lineas[1], "dos");
    bool ok = false;
    ASSERT_EQ(leerTodo(ruta, &ok), "una\ndos");
    ASSERT_TRUE(ok);
}

TEST_CASE(LineasVaciasNoSeTiran) {
    const std::string ruta = "/tmp/opencode/curso12_ej08_r3.txt";
    escribir(ruta, "a\n\n\nb\n");
    const auto lineas = leerLineas(ruta);
    ASSERT_EQ(lineas.size(), 4u);
    ASSERT_EQ(lineas[1], "");
    ASSERT_EQ(lineas[2], "");
}

TEST_CASE(ArchivoVacioYAperturaFallida) {
    const std::string vacio = "/tmp/opencode/curso12_ej08_vacio.txt";
    escribir(vacio, "");
    bool ok = false;
    ASSERT_TRUE(leerTodo(vacio, &ok).empty());
    ASSERT_TRUE(ok);
    ASSERT_TRUE(leerLineas(vacio).empty());
    ok = true;
    ASSERT_TRUE(leerTodo("/curso12_ej08/no_existe.txt", &ok).empty());
    ASSERT_FALSE(ok);
    ASSERT_TRUE(leerLineas("/curso12_ej08/no_existe.txt").empty());
}

TEST_CASE(ContarSubcadenasSinSuperponer) {
    const std::string ruta = "/tmp/opencode/curso12_ej08_r4.txt";
    escribir(ruta, "ab ab ab ab");
    ASSERT_EQ(contarSubcadenas(ruta, "ab"), 4u);
    ASSERT_EQ(contarSubcadenas(ruta, "ab "), 3u);
    escribir(ruta, "aaaa");
    ASSERT_EQ(contarSubcadenas(ruta, "aa"), 2u);
    ASSERT_EQ(contarSubcadenas(ruta, "aaaa"), 1u);
    ASSERT_EQ(contarSubcadenas(ruta, "a"), 4u);
    ASSERT_EQ(contarSubcadenas(ruta, "z"), 0u);
    ASSERT_EQ(contarSubcadenas(ruta, ""), 0u);
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 08 (completo vs lineas)...\n";
    return RUN_ALL_TESTS();
}
