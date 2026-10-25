/* =============================================================================
   MÓDULO 09 - EJERCICIO 04: Template de clase: Pila<T>
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA:
     Completá la clase template Pila<T> (estructura LIFO) usando un std::vector<T> por dentro:
     1. push(v): agrega arriba
     2. pop(): saca el de arriba; si está vacía lanza std::out_of_range
     3. top(): devuelve una referencia al de arriba; si está vacía lanza std::out_of_range
     4. size() y vacia()
     Debe funcionar con int, std::string o cualquier tipo copiable.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_04_pila.cpp -o ej04 && ./ej04
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstddef>
#include "../../include/test_framework.hpp"

// TODO: completá todos los métodos. Hoy no hacen nada (los tests fallan).
// TU CÓDIGO AQUÍ:
template <typename T>
class Pila {
public:
    void push([[maybe_unused]] const T& v) { /* placeholder */ }
    void pop() { /* placeholder */ }
    T& top() {
        static T falso{};          // placeholder: reemplazalo
        return falso;
    }
    std::size_t size() const { return 0; }     // placeholder
    bool vacia() const { return true; }        // placeholder

private:
    std::vector<T> datos_;
};

// ============= TESTS =============
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
    std::cout << "Ejecutando pruebas para Ejercicio 04 (Template de clase: Pila<T>)...\n";
    return RUN_ALL_TESTS();
}
