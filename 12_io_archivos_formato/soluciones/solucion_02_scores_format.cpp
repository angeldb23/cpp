/* =============================================================================
   MÓDULO 12 - SOLUCIÓN 02: tabla de scores con std::format (ej. 02)
   =============================================================================
   L03/L04 son las lecciones de respaldo: LOS ANCHOS SON EL CONTRATO
   (nombre {:<10}, puntos {:>8}, mejor {:>8.2f}), y el buffer completo
   se arma con UNA reserva y format_to (un solo algoritmo en memoria).
   La celda del HUD es format_to_n: escribe COMO MUCHO [ancho] chars.
   ============================================================================= */
#include <format>
#include <iterator>
#include <string>
#include <vector>
#include <iostream>
#include "../../include/test_framework.hpp"

struct ItemScore {
    std::string nombre;
    int puntos = 0;
    double mejor = 0.0;
};

// 1) UN format y UNA fila: los anchos los fija el header (el mismo):
std::string filaScore(const std::string& nombre, int puntos, double mejor) {
    return std::format("{:<10}|{:>8}|{:>8.2f}", nombre, puntos, mejor);
}

// 2) el buffer con UNA reserva y format_to (L04):
std::string bufferTabla(const std::vector<ItemScore>& items) {
    std::string tabla;
    tabla.reserve(64 + items.size() * 32);      // estimación física
    std::format_to(std::back_inserter(tabla),
                   "{:<10}|{:>8}|{:>8}\n", "NOMBRE", "PUNTOS", "MEJOR");
    for (const ItemScore& item : items) {
        std::format_to(std::back_inserter(tabla), "{}\n",
                       filaScore(item.nombre, item.puntos, item.mejor));
    }
    return tabla;
}

// 3) format_to_n escribe COMO MUCHO ancho: el truncado es literal:
std::string celdaHUD(const std::string& texto, size_t ancho) {
    std::string recortado;
    std::format_to_n(std::back_inserter(recortado),
                     static_cast<long>(ancho), "{}", texto);
    // el HUD pide SIEMPRE el ancho exacto: recortado va al borde derecho:
    return std::format("{:>{}}", recortado, ancho);
}

// ============= TESTS =============
TEST_CASE(FilaScoreFormato) {
    const std::string fila = filaScore("jefe", 900, 45.5);
    ASSERT_EQ(fila, "jefe      |     900|   45.50");
}

TEST_CASE(FilaScoreAlineados) {
    ASSERT_EQ(filaScore("a", 1, 1.0), "a         |       1|    1.00");
    ASSERT_EQ(filaScore("diez_chars", 12, 100.0),
              "diez_chars|      12|  100.00");
}

TEST_CASE(BufferTablaCompleto) {
    const std::vector<ItemScore> items{
        {"ana", 50, 12.25}, {"pedro", 7000, 999.0}};
    const std::string esperado =
        "NOMBRE    |  PUNTOS|   MEJOR\n"
        "ana       |      50|   12.25\n"
        "pedro     |    7000|  999.00\n";
    ASSERT_EQ(bufferTabla(items), esperado);

    const std::string vacio = bufferTabla({});
    ASSERT_EQ(vacio, "NOMBRE    |  PUNTOS|   MEJOR\n");
}

TEST_CASE(CeldaHUDTruncaZerosYRellena) {
    ASSERT_EQ(celdaHUD("pocion", 8), "  pocion");
    ASSERT_EQ(celdaHUD("arma_granda", 6).size(), 6);
    ASSERT_EQ(celdaHUD("", 4), "    ");
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 02 (scores con std::format)...\n";
    return RUN_ALL_TESTS();
}
