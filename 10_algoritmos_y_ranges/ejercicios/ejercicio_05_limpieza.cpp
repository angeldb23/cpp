/* =============================================================================
   MÓDULO 10 - EJERCICIO 05: remove/erase y erase_if (lección 07)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (tema 4 de TEMAS.md): limpieza de inventario.
     1. vaciarPuntos(v): borrar DE VERDAD todos los 0 de v con EL IDIOM
        clásico (remove + erase en una expresión; el remove SOLO no
        cambia el tamaño: el test lo exige: v.size() tiene que ser el
        nuevo).
     2. limpiarRotos(inventario): con std::erase (la C++20, que devuelve
        cuántos borró) borrar todos los "roto"; devolver cuántos.
     3. quitarDebiles(caracteristicas): con std::erase_if borrar los
        elementos que cumplen el criterio (debil: hp < 20) y devolver el
        contador de erase_if.
     4. dejarUnicosAdyacentes(v): unique + erase (el idiom de la
        familia: unique también devuelve el nuevo fin).
     5. purgarEnList(lst, valor): usar los MÉTODOS de list (list::remove)
        list::remove) que borran de una vez (la interfaz distinta de la
        lección).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_05_limpieza.cpp -o ej05 && ./ej05
   ============================================================================= */
#include <algorithm>
#include <list>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

// TODO 1: vaciarPuntos: remove+erase de todos los 0 (idiom en una linea).
// TU CODIGO AQUI:
void vaciarPuntos([[maybe_unused]] std::vector<int>& v) {
}

// TODO 2: limpiarRotos: std::erase de los "roto", devolver el contador.
// TU CODIGO AQUI:
std::size_t limpiarRotos([[maybe_unused]] std::vector<std::string>& inventario) {
    return 0;    // placeholder
}

// TODO 3: quitarDebiles: erase_if por hp < 20, devolver el contador.
// TU CODIGO AQUI:
std::size_t quitarDebiles([[maybe_unused]] std::vector<int>& v) {
    return 0;    // placeholder
}

// TODO 4: dejarUnicosDeLaFila: unique + erase de adyacentes iguales.
// TU CODIGO AQUI:
void dejarUnicosAdyacentes([[maybe_unused]] std::vector<int>& v) {
}

// TODO 5: purgarEnList: borrar TODOS los iguales a valor usando los
//         METODOS propietarios de std::list (no algoritmo).
// TU CODIGO AQUI:
void purgarEnList([[maybe_unused]] std::list<int>& lst,
                  [[maybe_unused]] int valor) {
}

// ============= TESTS =============
TEST_CASE(VaciarPuntosBorraTodos) {
    std::vector<int> v{10, 0, 80, 0, 45, 0};
    vaciarPuntos(v);
    ASSERT_EQ(v.size(), 3u);
    ASSERT_TRUE(v[0] == 10 && v[1] == 80 && v[2] == 45);
}

TEST_CASE(LimpiarRotosCuenta) {
    std::vector<std::string> inv{"pocion", "roto", "flecha", "roto", "pocion"};
    const auto n = limpiarRotos(inv);
    ASSERT_EQ(n, 2u);
    ASSERT_EQ(inv.size(), 3u);
    ASSERT_TRUE(inv[0] == "pocion" && inv[1] == "flecha" && inv[2] == "pocion");
    std::vector<std::string> vacia{"roto", "roto"};
    ASSERT_EQ(limpiarRotos(vacia), 2u);
    ASSERT_TRUE(vacia.empty());
}

TEST_CASE(QuitarDebilesConEraseIf) {
    std::vector<int> hp{45, 15, 80, 5, 90, 19};
    ASSERT_EQ(quitarDebiles(hp), 3u);
    ASSERT_TRUE((hp == std::vector<int>{45, 80, 90}));
    std::vector<int> sanos{50, 60};
    ASSERT_EQ(quitarDebiles(sanos), 0u);
    ASSERT_EQ(sanos.size(), 2u);
}

TEST_CASE(DejarUnicosAdyacentes) {
    std::vector<int> v{1, 1, 2, 3, 3, 3, 4, 1};
    dejarUnicosAdyacentes(v);
    ASSERT_TRUE((v == std::vector<int>{1, 2, 3, 4, 1}));
}

TEST_CASE(PurgarEnListConLosMetodos) {
    std::list<int> l{7, 7, 5, 7, 2};
    purgarEnList(l, 7);
    ASSERT_EQ(l.size(), 2u);
    ASSERT_TRUE(l.front() == 5 && l.back() == 2);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 05 (limpieza)...\n";
    return RUN_ALL_TESTS();
}
