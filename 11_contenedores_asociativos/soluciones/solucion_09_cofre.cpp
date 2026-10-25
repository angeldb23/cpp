/* =============================================================================
   MÓDULO 11 - SOLUCIÓN 09: el cofre del botín (ejercicio 09)
   =============================================================================
   L13 es la lección de respaldo: optional como "hay valor o no", reset(),
   value(), value_or() y has_value() con bad_optional_access fuera del
   flujo normal.
   ============================================================================= */
#include <iostream>
#include <optional>
#include <string>
#include "../../include/test_framework.hpp"

class Cofre {
  public:
    void llenar(const std::string& nuevo) { item = nuevo; }   // reemplaza

    std::string vaciar() {
        if (!item.has_value()) return "";
        const std::string sacado = *item;
        item.reset();                 // L13: vaciar sin dejar basura
        return sacado;
    }

    std::string mirar() const {
        return item.value_or("");     // value_or: "lo que hay o el default"
    }

    std::optional<std::string> buscarCompatible(const std::string& patron) const {
        if (item.has_value() && item->find(patron) != std::string::npos) {
            return item;              // optional lleno
        }
        return std::nullopt;
    }

    std::string usarOrDefault(const std::string& orElse) const {
        return item.value_or(orElse);   // L13: "dámelo o usa esto"
    }

  private:
    std::optional<std::string> item;    // el fondo del cofre
};

// ============= TESTS =============
TEST_CASE(LlenarReemplaza) {
    Cofre c;
    c.llenar("espada");
    c.llenar("arco");
    ASSERT_EQ(c.mirar(), "arco");
}

TEST_CASE(VaciarDevuelveYResta) {
    Cofre c;
    ASSERT_EQ(c.vaciar(), "");
    c.llenar("anillo");
    ASSERT_EQ(c.vaciar(), "anillo");
    ASSERT_EQ(c.mirar(), "");
    ASSERT_EQ(c.vaciar(), "");
}

TEST_CASE(MirarNoSaca) {
    Cofre c;
    c.llenar("vial");
    ASSERT_EQ(c.mirar(), "vial");
    ASSERT_EQ(c.mirar(), "vial");
    ASSERT_EQ(c.vaciar(), "vial");
}

TEST_CASE(BuscarCompatible) {
    Cofre c;
    ASSERT_FALSE(c.buscarCompatible("orco").has_value());
    c.llenar("pluma de orco");
    ASSERT_TRUE(c.buscarCompatible("orco").has_value());
    ASSERT_EQ(c.buscarCompatible("orco").value(), "pluma de orco");
    ASSERT_FALSE(c.buscarCompatible("troll").has_value());
}

TEST_CASE(UsarOrDefault) {
    Cofre c;
    ASSERT_EQ(c.usarOrDefault("piedra"), "piedra");
    c.llenar("gema");
    ASSERT_EQ(c.usarOrDefault("piedra"), "gema");
    ASSERT_EQ(c.mirar(), "gema");
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 09 (cofre del botin)...\n";
    return RUN_ALL_TESTS();
}
