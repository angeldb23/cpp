/* =============================================================================
   MÓDULO 06 - SOLUCIÓN 02: protected, name hiding y `using`
   =============================================================================
   Claves de la lección 03:
   - `using Escaner::Escaner;` hereda los constructores: no se escriben
     de nuevo, la derivada no agrega datos propios.
   - `using Escaner::leer;` REVIVE las sobrecargas del padre que la
     propia `leer(const char*)` de la derivada había ocultado.
   - El dato protected se ajusta desde la derivada (invariantes propias
     que el método protege).
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

class Escaner {
protected:
    int sensibilidad = 50;
    int canales = 4;

public:
    Escaner(int sens_, int can_) : sensibilidad(sens_), canales(can_) {}
    virtual ~Escaner() = default;

    std::string leer(int canal) const {
        historial().push_back("Esc::leer(int)");
        return "canal" + std::to_string(canal);
    }
    std::string leer(const char* etiqueta) const {
        historial().push_back("Esc::leer(const char*)");
        return std::string("marca:") + etiqueta;
    }
    int getSensibilidad() const { return sensibilidad; }

    static std::vector<std::string>& historial() {
        static std::vector<std::string> h;
        return h;
    }
};

class EscanerVisor : public Escaner {
public:
    // 1a) constructor heredado: una línea, nada más.
    using Escaner::Escaner;

    // 1c) la propia `leer(const char*)` nueva oculta las del padre:
    //     revivir con `using` para que `leer(int)` también exista.
    std::string leer(const char* etiqueta) {
        historial().push_back("Visor::leer(const char*)");
        return "visor[" + std::string(etiqueta) + "]";
    }
    using Escaner::leer;                 // FIX: revivir las del padre

    // 1b) calibrar: los datos protected SIEMPRE con método (no escupir).
    void calibrar() {
        if (sensibilidad != 100) sensibilidad = 100;
    }
};

TEST_CASE(TestCtorHeredado) {
    EscanerVisor v(1, 2);
    ASSERT_EQ(v.getSensibilidad(), 1);
    ASSERT_EQ(v.leer(1), std::string("canal1"));
}

TEST_CASE(TestSobrecargasRevividas) {
    Escaner::historial().clear();
    EscanerVisor v(60, 6);
    ASSERT_EQ(v.leer(3), std::string("canal3"));
    ASSERT_EQ(v.leer("radar"), std::string("visor[radar]"));
    ASSERT_EQ(Escaner::historial()[0], std::string("Esc::leer(int)"));
    ASSERT_EQ(Escaner::historial()[1], std::string("Visor::leer(const char*)"));
}

TEST_CASE(TestCalibrar) {
    EscanerVisor v(20, 2);
    v.calibrar();
    ASSERT_EQ(v.getSensibilidad(), 100);
}

int main() {
    std::cout << "Solucion Ejercicio 02: using y protected.\n";
    return RUN_ALL_TESTS();
}
