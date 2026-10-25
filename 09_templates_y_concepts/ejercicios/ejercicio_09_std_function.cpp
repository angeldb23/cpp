/* =============================================================================
   MÓDULO 09 - EJERCICIO 09: pipelines con std::function (lecciones 04 y 05)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (tema 5 de TEMAS.md: callables y el costo del type erasure):
     La lección 04 mostró que un template acepta cualquier callable (una
     versión por cada tipo); la lección 05 mostró que std::function GUARDA
     cualquier callable de una misma firma borrando el tipo (con su costo).
     Armá una cadena de pasos que se arma EN RUNTIME:

     1. componer(g, f): devuelve una std::function<int(int)> que calcula
        g(f(x)) (primero f, después g encima). Pista: lambda capturando
        por valor.
     2. repetir(f, veces): devuelve una std::function<int(int)> que aplica
        f 'veces' seguidas (se asume veces >= 1).
     3. pipeline(etapas, x): aplica cada etapa en orden (izquierda a
        derecha) sobre x y devuelve el resultado final.
     La struct Etapa YA está armada al final con el tipo de la caja:
     ésa es la respuesta del TODO 3 de la consigna original (fijate en
     ella cuando hagas tus funciones).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_09_std_function.cpp -o ej09 && ./ej09
   ============================================================================= */
#include <functional>
#include <iostream>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

// La caja de la lección 05: el MIEMBRO de una clase no puede ser un
// template, acá es donde el tipo se borra para siempre. Ésa es la
// respuesta del TODO 3 de la consigna original: usá la MISMA caja
// en tus funciones.
struct Etapa {
    std::function<int(int)> paso;    // cualquier callable int(int) vale
};

// TODO 1: componer(g, f) devuelve una std::function<int(int)> que calcula
//         g(f(x)).
// TU CÓDIGO AQUÍ:
std::function<int(int)> componer([[maybe_unused]] std::function<int(int)> g,
                                 [[maybe_unused]] std::function<int(int)> f) {
    return {};   // placeholder
}

// TODO 2: repetir(f, veces) devuelve una std::function<int(int)> que aplica
//         f 'veces' seguidas.
// TU CÓDIGO AQUÍ:
std::function<int(int)> repetir([[maybe_unused]] std::function<int(int)> f,
                                [[maybe_unused]] int veces) {
    return {};   // placeholder
}

// TODO 3: pipeline aplica cada etapa en orden sobre x y devuelve el resultado.
// TU CÓDIGO AQUÍ:
int pipeline([[maybe_unused]] const std::vector<Etapa>& etapas, [[maybe_unused]] int x) {
    return 0;   // placeholder
}

// ============= TESTS =============
int incrementar(int x) { return x + 1; }        // función común

auto cuadrado = [](int x) { return x * x; };    // lambda sin captura

struct LaMitad {                                 // functor
    int operator()(int x) const { return x / 2; }
};

TEST_CASE(TestComponerEligeElOrden) {
    auto gSobreF = componer(cuadrado, incrementar);     // cuadrado(incrementar(x))
    ASSERT_EQ(gSobreF(3), 16);                          // (3+1)^2
    ASSERT_EQ(gSobreF(0), 1);
    auto fSobreG = componer(incrementar, cuadrado);     // incrementar(cuadrado(x))
    ASSERT_EQ(fSobreG(3), 10);
    auto conFunctor = componer(LaMitad{}, cuadrado);
    ASSERT_EQ(conFunctor(6), 18);                       // (6*6)/2
}

TEST_CASE(TestRepetirAplicaConsecutivo) {
    auto tresVeces = repetir(incrementar, 3);           // +1, +1, +1
    ASSERT_EQ(tresVeces(10), 13);
    auto unaVez = repetir(cuadrado, 1);
    ASSERT_EQ(unaVez(5), 25);
    auto cuatroVeces = repetir(LaMitad{}, 4);           // 40/2/2/2/2
    ASSERT_EQ(cuatroVeces(40), 2);
}

TEST_CASE(TestEtapaGuardaCualquierCallable) {
    // Tres ORÍGENES distintos de callable en la misma caja:
    const Etapa e1{cuadrado};
    const Etapa e2{incrementar};
    const Etapa e3{LaMitad{}};
    ASSERT_EQ(e1.paso(4), 16);
    ASSERT_EQ(e2.paso(4), 5);
    ASSERT_EQ(e3.paso(4), 2);
}

TEST_CASE(TestPipelineIzquierdaADerecha) {
    std::vector<Etapa> cadena = {{cuadrado}, {incrementar}, {LaMitad{}}};
    ASSERT_EQ(pipeline(cadena, 3), 5);     // cuadrado(3)=9 -> +1=10 -> /2=5
    ASSERT_EQ(pipeline(cadena, 10), 50);   // cuadrado(10)=100 -> +1=101 -> /2=50
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 09 (std::function / type erasure)...\n";
    return RUN_ALL_TESTS();
}
