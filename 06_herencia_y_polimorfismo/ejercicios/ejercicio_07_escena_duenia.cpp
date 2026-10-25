/* =============================================================================
   MÓDULO 06 - EJERCICIO 07: La escena dueña de sus entidades
   =============================================================================
   DIFICULTAD: Medio
   PRACTICA: lección 08 (destrucción polimórfica en contenedores; dueño
   único de cada puntero). Se apoya también en la L07 (dtor virtual).

   CONSIGNA:
   Completá la clase `Escena` para que SEA EL DUEÑO de un vector de
   punteros a la base polimórfica `Enemigo` (dada):

     1. `void agregar(Enemigo* e)`: lo guarda (la escena se apropia).
     2. `int contar(const std::string& tipo) const`: recorre el vector y
        devuelve cuántos responden "obrero"/"esqueleto" (según el tipo
        DINÁMICO, via `tipoDe()` que te da la base).
     3. ~Escena(): un solo PUESTO DE LIMPIEZA: `delete cada puntero una
        sola vez, por la base (que es virtual)`. Al salir de la escena,
        TODOS los enemigos tienen que dejar registro: ni uno colgado,
        ni un doble delete.
     4. `ciclo()`: llama a `atacar()` de cada uno (despacho dinámico).

   REGLA: no uses smart pointers (son el módulo 07): new + delete
   manual, UN dueño, dtor de la escena como único punto de limpieza.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_07_escena_dueña.cpp -o ej07 && ./ej07
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

// ---------------------------------------------------------------------------
// Base polimórfica DADA (dtor virtual: la regla de la casa, L07).
// ---------------------------------------------------------------------------
std::vector<std::string>& registroVidas() {
    static std::vector<std::string> r;
    return r;
}

class Enemigo {
protected:
    std::string nombre;

public:
    explicit Enemigo(std::string n) : nombre(std::move(n)) {}
    virtual ~Enemigo() {
        registroVidas().push_back("~" + nombre);
    }
    virtual void atacar() const = 0;
    virtual std::string tipoDe() const = 0;
};

class Obrero : public Enemigo {
public:
    Obrero() : Enemigo("obrero") {}
    void atacar() const override {
        std::cout << "  golpe de pico\n";
    }
    std::string tipoDe() const override { return "obrero"; }
};

class Esqueleto : public Enemigo {
public:
    Esqueleto() : Enemigo("esqueleto") {}
    void atacar() const override {
        std::cout << "  zas: hueso arma\n";
    }
    std::string tipoDe() const override { return "esqueleto"; }
};

// ---------------------------------------------------------------------------
// Tu clase Escena.
// ---------------------------------------------------------------------------
class Escena {
    std::vector<Enemigo*> enemigos;      // dueña de los punteros

public:
    // TODO 1: agregar(Enemigo* e)
    // TODO 2: contar(tipo) via tipoDe() de cada uno
    // TODO 3: ~Escena(): delete de TODOS (registran al salir); vector queda
    // TODO 4: ciclo(): atacar() de cada uno

    // PLACEHOLDER que hace que todo compile (reemplazá por TODOs):
    void agregar(Enemigo* e) { (void)e; }
    int contar(const std::string& tipo) const {
        (void)tipo;
        return -1;
    }
    void ciclo() const {}
    // ~Escena() no existe: el vector se muere sin destruir lo apuntado
};

// ============= TESTS =============
TEST_CASE(TestContarPorTipoDinamico) {
    registroVidas().clear();
    {
        Escena escena;
        escena.agregar(new Obrero{});       // dueño único: la escena
        escena.agregar(new Obrero{});
        escena.agregar(new Esqueleto{});
        ASSERT_EQ(escena.contar("obrero"), 2);
        ASSERT_EQ(escena.contar("esqueleto"), 1);
        ASSERT_EQ(escena.contar("dragon"), 0);
    }   // ~Escena: aquí se destruye TODO lo agregado (su limpieza)
    ASSERT_EQ(registroVidas().size(), std::size_t(3));
}

TEST_CASE(TestCicloDestruyeTodo) {
    registroVidas().clear();
    {
        Escena escena;                          // esta escena NEWea a todos
        escena.agregar(new Obrero{});           // dueño único: la escena
        escena.agregar(new Esqueleto{});
        escena.agregar(new Obrero{});
        escena.ciclo();
    }                                           // ~Escena = limpieza única
    ASSERT_EQ(registroVidas().size(), std::size_t(3));
    ASSERT_EQ(registroVidas()[0], std::string("~obrero"));
    ASSERT_EQ(registroVidas()[1], std::string("~esqueleto"));
    ASSERT_EQ(registroVidas()[2], std::string("~obrero"));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 07...\n";
    return RUN_ALL_TESTS();
}
