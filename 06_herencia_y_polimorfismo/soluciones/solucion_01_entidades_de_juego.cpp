/* =============================================================================
   MÓDULO 06 - SOLUCIÓN 01: Entidades de juego: herencia y orden de vida
   =============================================================================
   Claves de la lección 01-02 puestas en prática:
   - El ctor derivado LLAMA al ctor base en la lista de inicialización.
   - Los datos protected de la base se usan (y respetan) desde la
     derivada; el invariante (vida 0..N) se mantiene en un metodo propio.
   - El orden de vida es: base, derivada..., y al destruir, al revés.
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

std::vector<std::string>& vidaRegistro() {
    static std::vector<std::string> r;
    return r;
}

class Entidad {
protected:
    std::string nombre;
    int vida = 0;

public:
    Entidad(std::string nombre_, int vida_) : nombre(nombre_), vida(vida_) {
        vidaRegistro().push_back("Entidad");
    }
    virtual ~Entidad() { vidaRegistro().push_back("~Entidad"); }
    int getVida() const { return vida; }
    const std::string& getNombre() const { return nombre; }
};

class Torreta : public Entidad {
    int disparos = 0;

public:
    // La base va PRIMERO en la lista; el registro después del cuerpo.
    Torreta(const std::string& n, int vida_, int disp_)
        : Entidad(n, vida_), disparos(disp_) {
        vidaRegistro().push_back("Torreta");
    }
    ~Torreta() { vidaRegistro().push_back("~Torreta"); }

    std::string informe() const {
        return "TORRETA_" + nombre + "_" + std::to_string(vida);
    }

    int disparosRestantes() const { return disparos; }
    void gastar(int cuantos) {
        if (cuantos < 0) return;                  // defensa barata y honesta
        disparos = (disparos > cuantos) ? disparos - cuantos : 0;
    }
};

TEST_CASE(TestHerenciaBasica) {
    Torreta t("T03", 100, 4);
    ASSERT_EQ(t.getNombre(), std::string("T03"));
    ASSERT_EQ(t.getVida(), 100);
    ASSERT_EQ(t.disparosRestantes(), 4);
}

TEST_CASE(TestInforme) {
    Torreta t("T07", 80, 2);
    ASSERT_EQ(t.informe(), std::string("TORRETA_T07_80"));
}

TEST_CASE(TestGastar) {
    Torreta t("T01", 50, 2);
    t.gastar(1);
    ASSERT_EQ(t.disparosRestantes(), 1);
    t.gastar(5);
    ASSERT_EQ(t.disparosRestantes(), 0);
}

TEST_CASE(TestOrdenVida) {
    vidaRegistro().clear();
    {
        Torreta t("A", 10, 1);
    }
    const std::vector<std::string>& r = vidaRegistro();
    ASSERT_TRUE(r.size() == 4);
    ASSERT_EQ(r[0], std::string("Entidad"));
    ASSERT_EQ(r[1], std::string("Torreta"));
    ASSERT_EQ(r[2], std::string("~Torreta"));
    ASSERT_EQ(r[3], std::string("~Entidad"));
}

int main() {
    std::cout << "Solucion Ejercicio 01: vida de Entidad -> Torreta.\n";
    return RUN_ALL_TESTS();
}
