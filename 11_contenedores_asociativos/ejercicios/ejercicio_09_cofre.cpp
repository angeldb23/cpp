/* =============================================================================
   MÓDULO 11 - EJERCICIO 09: el cofre del botín (lección 13)
   =============================================================================
   DIFICULTAD: Basico
   CONSIGNA (tema 7 de TEMAS.md: std::optional):
     Un cofre guarda UN item de tipo std::string; a veces está vacío
     (eso se modela con std::optional<std::string>):
       1. void llamar(Cofre& c, const std::string& item): guarda el item
          (reemplaza lo anterior: has_value siempre true al salir).
       2. std::string vaciar(Cofre& c): si hay item, lo DEVUELVE y
          el cofre quede vacío (reset); sino devuelve "".
       3. std::string mirar(const Cofre& c): el item SIN quitarlo
          (value() de la L13) o "" si está vacío.
       4. std::optional<std::string> buscarCompatible(const Cofre& c,
          const std::string& patron): si el item CONTIENE el patrón,
          devuelve el optional lleno (con el item); sino nullopt.
          (string::contains: C++20/23... si no, find: revisa este.)
       5. std::string usarOvaciar(Cofre& c, const std::string& orElse):
          "dame el item o el default: " (value_or de la L13).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_09_cofre.cpp -o ej09 && ./ej09
   ============================================================================= */
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include "../../include/test_framework.hpp"

class Cofre {
  public:
    // TODO 1: llenar (reemplaza lo anterior):
    void llenar([[maybe_unused]] const std::string& item) { }

    // TODO 2: devolver Y vaciar ( "") si ya venía vacío:
    std::string vaciar() {
        return "";    // placeholder
    }

    // TODO 3: devolver SIN tocar (value) o "" si vacío:
    std::string mirar() const {
        return "";    // placeholder
    }

    // TODO 4: optional lleno si contiene objeto y matching; nullopt sino:
    std::optional<std::string> buscarCompatible(
        [[maybe_unused]] const std::string& patron) const {
        return std::nullopt;    // placeholder
    }

    // TODO 5: el item si hay, o default (value_or):
    std::string usarOrDefault([[maybe_unused]] const std::string& orElse) const {
        return "";    // placeholder
    }

  private:
    std::optional<std::string> item;    // el fondo del cofre
};

// ============= TESTS =============
TEST_CASE(LlenarReemplaza) {
    Cofre c;
    c.llenar("espada");
    c.llenar("arco");
    ASSERT_EQ(c.mirar(), "arco");      // el último reemplaza al anterior (L13)
}

TEST_CASE(VaciarDevuelveYResta) {
    Cofre c;
    ASSERT_EQ(c.vaciar(), "");          // vacía de entrada: ""
    c.llenar("anillo");
    ASSERT_EQ(c.vaciar(), "anillo");
    ASSERT_EQ(c.mirar(), "");           // quedó vacío
    ASSERT_EQ(c.vaciar(), "");          // una vez vaciado: ya no hay
}

TEST_CASE(MirarNoSaca) {
    Cofre c;
    c.llenar("vial");
    ASSERT_EQ(c.mirar(), "vial");
    ASSERT_EQ(c.mirar(), "vial");       // doble mirada: nada se va
    ASSERT_EQ(c.vaciar(), "vial");
}

TEST_CASE(BuscarCompatible) {
    Cofre c;
    ASSERT_FALSE(c.buscarCompatible("orco").has_value());   // cofre vacío
    c.llenar("pluma de orco");
    ASSERT_TRUE(c.buscarCompatible("orco").has_value());
    ASSERT_EQ(c.buscarCompatible("orco").value(), "pluma de orco");
    ASSERT_FALSE(c.buscarCompatible("troll").has_value());
}

TEST_CASE(UsarOrDefault) {
    Cofre c;
    ASSERT_EQ(c.usarOrDefault("piedra"), "piedra");   // vacío: el default
    c.llenar("gema");
    ASSERT_EQ(c.usarOrDefault("piedra"), "gema");
    ASSERT_EQ(c.mirar(), "gema");       // no se vacio: solo se USO
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 09 (cofre del botin)...\n";
    return RUN_ALL_TESTS();
}
