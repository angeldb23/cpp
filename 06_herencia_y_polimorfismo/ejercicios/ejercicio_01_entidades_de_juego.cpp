/* =============================================================================
   MÓDULO 06 - EJERCICIO 01: Entidades de juego: herencia y orden de vida
   =============================================================================
   DIFICULTAD: Básico
   PRACTICA: lecciones 01-02 (herencia pública, protected, orden de
   construcción y destrucción; material rehecho desde el viejo ejercicio
   de entidades de _pendiente/).

   CONSIGNA:
     La base `Entidad` ya está dada (NO la cambies: los tests la usan).
     Registran el orden de vida en el vector global (vidaRegistro()).

     1. Completá la clase `Torreta`:
        - Constructor `Torreta(const std::string& n, int vida_, int disp_)`:
          llamá a Entidad(n, vida_) en TU lista de inicialización e
          inicializá `disparos`; al final del cuerpo, `registrarTorreta()`.
        - `std::string informe() const` -> "TORRETA_" + nombre + "_" +
          std::to_string(vida).  (nombre y vida son protected de Entidad.)
        - `int disparosRestantes() const`.
        - `void gastar(int cuantos)`: baja disparos sin quedar en negativo.
     2. El test TestOrdenVida exige el orden EXACTO:
        Entidad -> Torreta (construcción) y ~Torreta -> ~Entidad:
        cada clase deja su marca llamando a vidaRegistro().push_back(...).

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_01_entidades_de_juego.cpp -o ej01 && ./ej01
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

// Registro global del orden de vida (lo usan base y derivada):
extern std::vector<std::string>& vidaRegistro();

// ---------------------------------------------------------------------------
// Base DADA: no la modifiques.
// ---------------------------------------------------------------------------
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

std::vector<std::string>& vidaRegistro() {
    static std::vector<std::string> r;
    return r;
}

// ---------------------------------------------------------------------------
// Tu clase derivada: complete TODOs.
// ---------------------------------------------------------------------------
class Torreta : public Entidad {
    int disparos = -1;          // sentinela placeholder (no dejes -1 al final)

public:
    // TODO 1: constructor que llame a Entidad y registre "Torreta":
    //         (la firma ya es la buena: falta completar el cuerpo)
    Torreta(const std::string& n, int vida_, int disp_)
        : Entidad(n, vida_) {
        (void)disp_;   // TODO: inicializá `disparos` con disp_ y llamá a
                       //       vidaRegistro().push_back("Torreta");
    }

    // TODO 1b: destructor que registre "~Torreta":
    ~Torreta() {
        // TODO: vidaRegistro().push_back("~Torreta");
    }

    // TODO 2: informe() -> "TORRETA_" + nombre + "_" + std::to_string(vida)
    std::string informe() const { return "placeholder"; }

    // TODO 3: disparos REALES:
    int disparosRestantes() const { return -1; }   // PLACEHOLDER
    void gastar(int cuantos) { (void)cuantos; }    // PLACEHOLDER
};

// ============= TESTS =============
TEST_CASE(TestHerenciaBasica) {
    Torreta t("T03", 100, 4);
    ASSERT_EQ(t.getNombre(), std::string("T03"));    // viene de Entidad
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
    t.gastar(5);                                    // más que la munición
    ASSERT_EQ(t.disparosRestantes(), 0);            // clamp a 0
}

TEST_CASE(TestOrdenVida) {
    vidaRegistro().clear();
    {
        Torreta t("A", 10, 1);
    }
    const std::vector<std::string>& r = vidaRegistro();
    ASSERT_TRUE(r.size() == 4);
    ASSERT_EQ(r[0], std::string("Entidad"));         // base primero
    ASSERT_EQ(r[1], std::string("Torreta"));         // hoy la derivada
    ASSERT_EQ(r[2], std::string("~Torreta"));        // dtor derivada primero
    ASSERT_EQ(r[3], std::string("~Entidad"));        // dtor base al final
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 01...\n";
    return RUN_ALL_TESTS();
}
