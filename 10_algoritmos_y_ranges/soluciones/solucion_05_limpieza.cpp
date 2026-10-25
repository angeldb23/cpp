/* =============================================================================
   MÓDULO 10 - SOLUCIÓN 05: remove/erase y erase_if (lección 07)
   =============================================================================
   La clave de TODO el set: borrar de verdad necesita tocar al
   CONTENEDOR (erase, erase_if, list::remove): los algoritmos solos
   (remove/unique) solo compactan y devuelven el nuevo fin.
       - vaciarPuntos: el idiom clásico remove+erase en una expresión.
       - limpiarRotos: std::erase devuelve el número que borró.
       - quitarDebiles: std::erase_if (C++20) tópico idéntico.
       - dejarUnicosAdyacentes: unique + erase de la cola.
       - purgarEnList: el METODO de list (borra su nodo y su tamaño).
   ============================================================================= */
#include <algorithm>
#include <list>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

// SOLUCIÓN 1: el idiom remove+erase en UNA expresión:
void vaciarPuntos(std::vector<int>& v) {
    v.erase(std::remove(v.begin(), v.end(), 0), v.end());
}

// SOLUCIÓN 2: std::erase (C++20) hace compactar+borrar en una llamada:
std::size_t limpiarRotos(std::vector<std::string>& inventario) {
    return std::erase(inventario, "roto");
}

// SOLUCIÓN 3: erase_if con el predicado de "débiles":
std::size_t quitarDebiles(std::vector<int>& v) {
    return std::erase_if(v, [](int hp) { return hp < 20; });
}

// SOLUCIÓN 4: unique TAMBIÉN devuelve el nuevo fin: mismo idiom:
void dejarUnicosAdyacentes(std::vector<int>& v) {
    v.erase(std::unique(v.begin(), v.end()), v.end());
}

// SOLUCIÓN 5: list borra su nodo en el MÉTODO (allá sí es un remove real):
void purgarEnList(std::list<int>& lst, int valor) {
    lst.remove(valor);
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
    std::cout << "Solución 05 (limpieza)...\n";
    return RUN_ALL_TESTS();
}
