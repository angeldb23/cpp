/* =============================================================================
   MÓDULO 12 - EJERCICIO 02: tabla de scores con std::format (L03-L04)
   =============================================================================
   DIFICULTAD: Básico
   CONSIGNA (tema 2 de TEMAS.md: std::format: especificadores y format_to):
     La tabla del final de nivel se armea con std::format (L03) y el
     buffer te lo arma format_to in-situ (L04: un solo alloc).
       1. filaScore(nombre, puntos, mejor): " nombre | puntos | mejor "
          con formato EXACTO: nombre a la IZQUIERDA en ancho 10, puntos
          a la DERECHA en ancho 8 y mejor ("mejor" es double) con
          {:8.2f} a la derecha (con decimales SIEMPRE a DOS).  Sin '\n'
          final: lo agrega el que arma la tabla.
       2. bufferTabla(items): une "NOMBRES    |PUNTOS   |MAYOR\n" (OJO:
          con el MISMO ancho que las filas de abajo) y una fila por item
          + '\n' por fila, CON UN SOLO string y UNA reserva (L04: reporte
          con format_to sobre un string reservado).
       3. celdaHUD(texto, ancho): devuelve el texto truncado a [ancho]
          con format_to_n (L04): SIEMPRE devuelve el ancho exacto, y el
          truncado va morcelado SIN "..." (el HUD no tiene magia).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_02_scores_format.cpp -o ej02 && ./ej02
   ============================================================================= */
#include <format>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

struct ItemScore {
    std::string nombre;
    int puntos = 0;
    double mejor = 0.0;
};

// TODO 1: la Fila: formato " nombre |marte | Mayor " (ver arriba).
//         Devuelve std::string SIN '\n'.
std::string filaScore([[maybe_unused]] const std::string& nombre,
                      [[maybe_unused]] int puntos,
                      [[maybe_unused]] double mejor) {
    // TU CODIGO AQUI (una sola llamada a std::format)
    return "";    // placeholder
}

// TODO 2: el buffer completo del tablero: el header exacto + filaScore
//         por item, cada fila con su '\n'. UN solo string con reserve.
std::string bufferTabla([[maybe_unused]] const std::vector<ItemScore>& items) {
    // TU CODIGO AQUI (format_to + back_inserter, L04)
    return "";    // placeholder
}

// TODO 3: la celda de HUD de ANCHO fijo: SIEMPRE [ancho] chars; si el
//         texto PISA, se trunca RMS-NO-mas (format_to_n: L04).
std::string celdaHUD([[maybe_unused]] const std::string& texto,
                     [[maybe_unused]] size_t ancho) {
    // TU CODIGO AQUI
    return "";    // placeholder
}

// ============= TESTS =============
TEST_CASE(FilaScoreFormato) {
    const std::string fila = filaScore("jefe", 900, 45.5);
    ASSERT_EQ(fila, "jefe      |     900|   45.50");
}

TEST_CASE(FilaScoreAlineados) {
    // nombre corto a izquierda, puntos a derecha, double a 2 decimales fijo
    ASSERT_EQ(filaScore("a", 1, 1.0), "a         |       1|    1.00");
    // el nombre de 10 chars justo: sin desalinear la columna siguiente
    ASSERT_EQ(filaScore("diez_chars", 12, 100.0),
              "diez_chars|      12|  100.00");
}

TEST_CASE(BufferTablaCompleto) {
    const std::vector<ItemScore> items{
        {"ana", 50, 12.25}, {"pedro", 7000, 999.0}};
    const std::string esperado =
        "NOMBRE    |  PUNTOS|   MEJOR\n"   // (header de los mismos anchos)
        "ana       |      50|   12.25\n"
        "pedro     |    7000|  999.00\n";
    ASSERT_EQ(bufferTabla(items), esperado);

    const std::string vacio = bufferTabla({});
    ASSERT_EQ(vacio, "NOMBRE    |  PUNTOS|   MEJOR\n");
}

TEST_CASE(CeldaHUDTruncaZerosYRellena) {
    ASSERT_EQ(celdaHUD("pocion", 8), "  pocion");       // centrado en 8
    ASSERT_EQ(celdaHUD("arma_granda", 6).size(), 6);     // recortado a 6
    ASSERT_EQ(celdaHUD("", 4), "    ");                  // vacia rellena
}

int main() {
    return RUN_ALL_TESTS();
}
