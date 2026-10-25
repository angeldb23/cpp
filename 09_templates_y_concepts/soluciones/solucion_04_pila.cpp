/* =============================================================================
   MÓDULO 09 - SOLUCIÓN 04: Template de clase: Pila<T>
   =============================================================================
   POR QUÉ FUNCIONA:
     - Pila<int> y Pila<std::string> son dos clases distintas generadas desde el mismo template.
     - Los métodos de un template de clase se instancian solo si se usan.
     - Lanzar std::out_of_range convierte un error silencioso (UB) en un error visible y capturable.
     - top() devuelve T&: permite leer y también modificar el elemento de arriba.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_04_pila.cpp -o sol04 && ./sol04
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstddef>
#include "../../include/test_framework.hpp"

// Delegamos el almacenamiento en std::vector: push/pop/back ya son O(1) amortizado.
// Lanzamos std::out_of_range en vez de dejar que back() sobre un vector vacío
// cause comportamiento indefinido.
template <typename T>
class Pila {
public:
    void push(const T& v) { datos_.push_back(v); }

    void pop() {
        if (datos_.empty()) throw std::out_of_range("pila vacia");
        datos_.pop_back();
    }

    T& top() {
        if (datos_.empty()) throw std::out_of_range("pila vacia");
        return datos_.back();
    }

    std::size_t size() const { return datos_.size(); }
    bool vacia() const { return datos_.empty(); }

private:
    std::vector<T> datos_;
};

// ============= TESTS (los mismos del ejercicio) =============
TEST_CASE(TestPilaVacia) {
    Pila<int> p;
    ASSERT_TRUE(p.vacia());
    ASSERT_EQ(p.size(), 0u);
}

TEST_CASE(TestPushYTop) {
    Pila<int> p;
    p.push(10);
    p.push(20);
    ASSERT_EQ(p.top(), 20);
    ASSERT_EQ(p.size(), 2u);
    ASSERT_FALSE(p.vacia());
}

TEST_CASE(TestPop) {
    Pila<int> p;
    p.push(1);
    p.push(2);
    p.push(3);
    p.pop();
    ASSERT_EQ(p.top(), 2);
    ASSERT_EQ(p.size(), 2u);
}

TEST_CASE(TestPilaDeStrings) {
    Pila<std::string> p;
    p.push("a");
    p.push("b");
    ASSERT_EQ(p.top(), std::string("b"));
    p.pop();
    ASSERT_EQ(p.top(), std::string("a"));
}

TEST_CASE(TestErroresEnPilaVacia) {
    Pila<int> p;
    bool lanzoTop = false;
    try { p.top(); } catch (const std::out_of_range&) { lanzoTop = true; }
    ASSERT_TRUE(lanzoTop);
    bool lanzoPop = false;
    try { p.pop(); } catch (const std::out_of_range&) { lanzoPop = true; }
    ASSERT_TRUE(lanzoPop);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 04 (Template de clase: Pila<T>)...\n";
    return RUN_ALL_TESTS();
}
