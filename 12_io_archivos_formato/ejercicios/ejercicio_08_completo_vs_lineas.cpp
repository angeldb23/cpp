/* =============================================================================
   MÓDULO 12 - EJERCICIO 08: leer TODO de una vs línea por línea (L13)
   =============================================================================
   DIFICULTAD: Media
   CONSIGNA (tema 7 de TEMAS.md: archivo completo vs línea por línea):
     El loader de niveles se formula con DOS lecturas, y las dos son
     válidas: el problema de cada una es distinto (L13):
       1. leerTodo(ruta, ok): UNA pasada: medí el tamaño (seekg a
          bytes + tellg) y tirá el read() directo sobre un string con
          reserva (la técnica A2 de la L13). ok (si no es nullptr)
          queda false SOLO si la apertura falla o badbit; el eofbit
          del read final NO es fail (L12). Archivo vacío: true y "".
       2. leerLineas(ruta): el loop getline (la técnica B): UNA línea
          en memoria a la vez; conserva las líneas VACÍAS y NO-agrega
          una línea fantasma por el '\n' final.
       3. contarSubcadenas(ruta, patron): cuántas veces aparece patron
          EXACTO, SIN superposiciones, en TODO el contenido. Leé el
          archivo UNA vez (completo) y escaneá en memoria: no re-abrir
          el archivo ni re-read por lado de match (L13: el completo
          es el idioma de este tema). Patrón vacío: 0.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_08_completo_vs_lineas.cpp -o ej08 && ./ej08
   ============================================================================= */
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

// TODO 1: seekg + tellg + UN read() sobre string reservado (L13 A2):
std::string leerTodo([[maybe_unused]] const std::string& ruta,
                     [[maybe_unused]] bool* ok) {
    // TU CODIGO AQUI
    return "";    // placeholder
}

// TODO 2: getline en loop a un vector (L13 B; líneas VACÍAS se conservan):
std::vector<std::string> leerLineas([[maybe_unused]] const std::string& ruta) {
    // TU CODIGO AQUI
    return {};    // placeholder
}

// TODO 3: apuestas al completo: UNA lectura y contador de matches SIN
//         superposición (string::find con posición de avance):
size_t contarSubcadenas([[maybe_unused]] const std::string& ruta,
                        [[maybe_unused]] const std::string& patron) {
    // TU CODIGO AQUI
    return 0;    // placeholder
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
        todo += '\n';        // con salto SIEMPRE: solo sirve si el
    }                        // archivo terminó en '\n'
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
    ASSERT_EQ(unir(lineas), todo);     // el mismo contenido por 2 caminos
}

TEST_CASE(UltimaLineaSinSaltoCuentaIgual) {
    const std::string ruta = "/tmp/opencode/curso12_ej08_r2.txt";
    escribir(ruta, "una\ndos");        // SIN '\n' final
    const auto lineas = leerLineas(ruta);
    ASSERT_EQ(lineas.size(), 2u);
    ASSERT_EQ(lineas[0], "una");
    ASSERT_EQ(lineas[1], "dos");
    bool ok = false;
    ASSERT_EQ(leerTodo(ruta, &ok), "una\ndos");   // byte a byte igual
    ASSERT_TRUE(ok);
}

TEST_CASE(LineasVaciasNoSeTiran) {
    const std::string ruta = "/tmp/opencode/curso12_ej08_r3.txt";
    escribir(ruta, "a\n\n\nb\n");      // dos vacías en el medio
    const auto lineas = leerLineas(ruta);
    ASSERT_EQ(lineas.size(), 4u);      // a, "", "", b
    ASSERT_EQ(lineas[1], "");
    ASSERT_EQ(lineas[2], "");
}

TEST_CASE(ArchivoVacioYAperturaFallida) {
    const std::string vacio = "/tmp/opencode/curso12_ej08_vacio.txt";
    escribir(vacio, "");
    bool ok = false;
    ASSERT_TRUE(leerTodo(vacio, &ok).empty());
    ASSERT_TRUE(ok);                   // vacío NO es error
    ASSERT_TRUE(leerLineas(vacio).empty());
    ok = true;
    ASSERT_TRUE(leerTodo("/curso12_ej08/no_existe.txt", &ok).empty());
    ASSERT_FALSE(ok);                  // apertura fallida: se reporta
    ASSERT_TRUE(leerLineas("/curso12_ej08/no_existe.txt").empty());
}

TEST_CASE(ContarSubcadenasSinSuperponer) {
    const std::string ruta = "/tmp/opencode/curso12_ej08_r4.txt";
    escribir(ruta, "ab ab ab ab");     // 4 matches con espacio
    ASSERT_EQ(contarSubcadenas(ruta, "ab"), 4u);
    ASSERT_EQ(contarSubcadenas(ruta, "ab "), 3u);
    escribir(ruta, "aaaa");            // "aa" superponible: no se permite
    ASSERT_EQ(contarSubcadenas(ruta, "aa"), 2u);
    ASSERT_EQ(contarSubcadenas(ruta, "aaaa"), 1u);
    ASSERT_EQ(contarSubcadenas(ruta, "a"), 4u);
    ASSERT_EQ(contarSubcadenas(ruta, "z"), 0u);
    ASSERT_EQ(contarSubcadenas(ruta, ""), 0u);   // patrón vacío: 0
}

int main() {
    return RUN_ALL_TESTS();
}
