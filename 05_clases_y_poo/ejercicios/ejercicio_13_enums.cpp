/* =============================================================================
   MÓDULO 05 - EJERCICIO 13: enum y enum class
   =============================================================================
   DIFICULTAD: Básico-Intermedio
   PRACTICA: lección 19 (enum class con alcance y tipo fuerte, tipo
   subyacente elegible, static_cast para las conversiones).

   CONSIGNA:
     1. Definí (acá abajo) los DOS enum class de la consigna:
        - EstadoPorta con tipo subyacente `unsigned char` y los
          valores Verde, Amarillo, Rojo.
        - Prioridad con Baja = 1, Media = 10, Alta = 100.
     2. `const char* nombreEstado(EstadoPorta e)`: switch SIN default
        que devuelva "verde", "amarillo" o "rojo" (el compilador
        avisa si falta un caso: esa es la utilidad del switch sobre
        enums completos).
     3. `EstadoPorta siguiente(EstadoPorta e)`: la rueda
        Verde→Amarillo→Rojo→Verde.
     4. `EstadoPorta desdeInt(int n)`: si n está en [0, 2],
        static_cast al enum; cualquier otro valor devuelve Verde.
     5. `int valorPrioridad(Prioridad p)`: el número del enumerator,
        con static_cast explícito (enum class → int no es implícito).
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"
//         `enum class EstadoPorta : unsigned char { ... };`)
// ESTADO DE PLACEHOLDER PARA QUE COMPILE COMPLETO:
enum class EstadoPorta : unsigned char { Verde, Amarillo, Rojo };
enum class Prioridad { Baja = 1, Media = 10, Alta = 100 };

// TODO 2: switch sin default.
const char* nombreEstado(EstadoPorta e) {
    (void)e;
    return "?";  // placeholder
}

// TODO 3: la rueda Verde→Amarillo→Rojo→Verde.
EstadoPorta siguiente(EstadoPorta e) {
    (void)e;
    return EstadoPorta::Verde;  // placeholder
}

// TODO 4: desdeInt con static_cast y validación de rango.
EstadoPorta desdeInt(int n) {
    (void)n;
    return EstadoPorta::Verde;  // placeholder
}

// TODO 5: lectura numérica de un enumerator.
int valorPrioridad(Prioridad p) {
    (void)p;
    return -1;  // placeholder
}

// ============= TESTS =============
TEST_CASE(TestTipoSubyacente) {
    ASSERT_EQ(sizeof(EstadoPorta), 1);   // unsigned char: 1 byte
}

TEST_CASE(TestNombres) {
    ASSERT_EQ(std::string(nombreEstado(EstadoPorta::Verde)), "verde");
    ASSERT_EQ(std::string(nombreEstado(EstadoPorta::Amarillo)), "amarillo");
    ASSERT_EQ(std::string(nombreEstado(EstadoPorta::Rojo)), "rojo");
}

TEST_CASE(TestRueda) {
    ASSERT_TRUE(siguiente(EstadoPorta::Verde) == EstadoPorta::Amarillo);
    ASSERT_TRUE(siguiente(EstadoPorta::Amarillo) == EstadoPorta::Rojo);
    ASSERT_TRUE(siguiente(EstadoPorta::Rojo) == EstadoPorta::Verde);
}

TEST_CASE(TestDesdeInt) {
    ASSERT_TRUE(desdeInt(0) == EstadoPorta::Verde);
    ASSERT_TRUE(desdeInt(1) == EstadoPorta::Amarillo);
    ASSERT_TRUE(desdeInt(2) == EstadoPorta::Rojo);
    ASSERT_TRUE(desdeInt(-1) == EstadoPorta::Verde);   // fuera de rango
    ASSERT_TRUE(desdeInt(99) == EstadoPorta::Verde);   // fuera de rango
}

TEST_CASE(TestPrioridadesNumeradas) {
    ASSERT_EQ(valorPrioridad(Prioridad::Baja), 1);
    ASSERT_EQ(valorPrioridad(Prioridad::Media), 10);
    ASSERT_EQ(valorPrioridad(Prioridad::Alta), 100);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 13...\n";
    return RUN_ALL_TESTS();
}
