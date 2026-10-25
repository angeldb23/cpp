/* =============================================================================
   MÓDULO 09 - SOLUCIÓN 09: pipelines con std::function
   =============================================================================
   POR QUÉ FUNCIONA:
     - componer/repetir devuelven LAMBDAS capturadas por valor: las funciones
       quedan dentro de una std::function, la caja con el tipo borrado.
     - capturar por valor copia los callables (que son std::function livianos
       de mover); nada apunta al stack de la función que ya terminó.
     - pipeline recorre el vector con for clásico y encadena: valor =
       etapa.paso(valor). El orden es izquierda -> derecha.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_09_std_function.cpp -o sol09 && ./sol09
   ============================================================================= */
#include <functional>
#include <iostream>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

struct Etapa {
    std::function<int(int)> paso;    // cualquier callable int(int) vale
};

std::function<int(int)> componer(std::function<int(int)> g,
                                 std::function<int(int)> f) {
    return [g, f](int x) { return g(f(x)); };
}

std::function<int(int)> repetir(std::function<int(int)> f, int veces) {
    return [f, veces](int x) {
        int valor = x;
        for (int i = 0; i < veces; ++i) {
            valor = f(valor);
        }
        return valor;
    };
}

int pipeline(const std::vector<Etapa>& etapas, int x) {
    int valor = x;
    for (const auto& etapa : etapas) {
        valor = etapa.paso(valor);
    }
    return valor;
}

// ============= TESTS (los mismos del ejercicio) =============
int incrementar(int x) { return x + 1; }        // función común

auto cuadrado = [](int x) { return x * x; };    // lambda sin captura

struct LaMitad {                                 // functor
    int operator()(int x) const { return x / 2; }
};

TEST_CASE(TestComponerEligeElOrden) {
    auto gSobreF = componer(cuadrado, incrementar);     // cuadrado(incrementar(x))
    ASSERT_EQ(gSobreF(3), 16);
    ASSERT_EQ(gSobreF(0), 1);
    auto fSobreG = componer(incrementar, cuadrado);
    ASSERT_EQ(fSobreG(3), 10);
    auto conFunctor = componer(LaMitad{}, cuadrado);
    ASSERT_EQ(conFunctor(6), 18);
}

TEST_CASE(TestRepetirAplicaConsecutivo) {
    auto tresVeces = repetir(incrementar, 3);
    ASSERT_EQ(tresVeces(10), 13);
    auto unaVez = repetir(cuadrado, 1);
    ASSERT_EQ(unaVez(5), 25);
    auto cuatroVeces = repetir(LaMitad{}, 4);
    ASSERT_EQ(cuatroVeces(40), 2);
}

TEST_CASE(TestEtapaGuardaCualquierCallable) {
    const Etapa e1{cuadrado};
    const Etapa e2{incrementar};
    const Etapa e3{LaMitad{}};
    ASSERT_EQ(e1.paso(4), 16);
    ASSERT_EQ(e2.paso(4), 5);
    ASSERT_EQ(e3.paso(4), 2);
}

TEST_CASE(TestPipelineIzquierdaADerecha) {
    std::vector<Etapa> cadena = {{cuadrado}, {incrementar}, {LaMitad{}}};
    ASSERT_EQ(pipeline(cadena, 3), 5);
    ASSERT_EQ(pipeline(cadena, 10), 50);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 09 (std::function / type erasure)...\n";
    return RUN_ALL_TESTS();
}
