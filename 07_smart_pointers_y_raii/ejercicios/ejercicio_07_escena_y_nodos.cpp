/* =============================================================================
   MÓDULO 07 - EJERCICIO 07: Escena con hijos y flechas hacia atrás
   =============================================================================
   DIFICULTAD: Medio
   PRACTICA: lección 08 (ciclos: solo una flecha strong) y lección 09
   (weak_ptr + lock() como observador).

   CONSIGNA:
   Una `Escena` DUEÑE sus nodos (vector<shared_ptr<Nodo>>). Cada nodo
   quiere SABER a su escena (para consultar tamaño, Audio, etc.), pero no
   puede exigir que la escena viva:

      1. `Nodo::conectado(std::weak_ptr<Escena>)`: recibe la flecha débil
         hacia su escena y la GUARDA en el miembro `escena_`.
      2. `Nodo::nombreDeEscena()`: útiliza lock() (L09): si la escena
         está viva, devuelve su nombre; si no, devuelve "(sin escena)".
      3. `Escena::agregarNodo(nombre)`: crea un Nodo con make_shared, lo
         guarda (dueño) y lo CONECTA con la escena (flecha débil) usando
         enable_shared_from_this de la Escena... simplificado: la Escena
         administra make_shared propia (el test maneja la vida).
      4. EL TEST FUERZA el anti-ciclo: si el nodo GUARDA shared_ptr, la
         escena no podrá morir mientras existan nodos: contador de escenas
         vivas queda > 0: el último test FALLA.

   AYUDA: la flecha del hijo es DEBIL: la única flecha fuerte va de la
   escena hacia abajo (dueño de hijos).
   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_07_escena_y_nodos.cpp -o ej07 && ./ej07
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

class Escena;   // forward declaration (la clase completa va después)

class Nodo {
private:
    std::string nombre_;
    std::weak_ptr<Escena> escena_;   // la flecha HACIA ATRÁS

public:
    explicit Nodo(std::string nombre) : nombre_(std::move(nombre)) {}

    const std::string& nombre() const { return nombre_; }

    // TODO 1: guardar la flecha débil hacia la escena.
    void conectado(std::weak_ptr<Escena> s) {
        (void)s;
    }

    // TODO 2: usá lock() (L09): nombre de la escena o "(sin escena)".
    std::string nombreDeEscena() const {
        (void)escena_;
        return "PLACEHOLDER";          // PLACEHOLDER
    }
};

// ----------------------------------------------------------------------------
// La Escena DADA: dueña de los hijos. Solo falta conectarlos.
// ----------------------------------------------------------------------------
class Escena {
private:
    std::string nombre_;
    std::vector<std::shared_ptr<Nodo>> nodos_;
    inline static int vivas_ = 0;

public:
    explicit Escena(std::string nombre) : nombre_(std::move(nombre)) {
        ++vivas_;
    }
    ~Escena() { --vivas_; }

    const std::string& nombre() const { return nombre_; }
    static int vivas() { return vivas_; }

    // (en este ejercicio, el test maneja la vida de la escena con su
    //  propio shared_ptr; agregarNodo hace que escena sea dueña del nodo.)
    std::shared_ptr<Nodo> agregarNodo(const std::string& nombreNodo,
                                      const std::shared_ptr<Escena>& yo) {
        auto n = std::make_shared<Nodo>(nombreNodo);
        n->conectado(yo);              // (L08): FLECHA DÉBIL hacia atrás
        nodos_.push_back(n);
        return n;
    }
};

// ============= TESTS =============
TEST_CASE(UnaFlechaStrongUnaWeak) {
    ASSERT_EQ(Escena::vivas(), 0);
    {
        auto escena = std::make_shared<Escena>("bosque");
        ASSERT_EQ(escena.use_count(), 1);   // SOLO main la sostiene
    }   // la escena muere SÓLO con el fin del bloque...
    ASSERT_EQ(Escena::vivas(), 0);          // ...si los nodos NO sostienen
}

TEST_CASE(NodoVeAEscenaViva) {
    auto escena = std::make_shared<Escena>("desierto");
    auto nodo = escena->agregarNodo("cactus", escena);
    ASSERT_EQ(nodo->nombreDeEscena(), std::string("desierto"));
}

TEST_CASE(NodoVeQueLaEscenaMurió) {
    std::weak_ptr<Nodo> nodoGuardado;       // test: sostiene el NODO solo
    {
        auto escena = std::make_shared<Escena>("cueva");
        auto nodo = escena->agregarNodo("murcielago", escena);
        nodoGuardado = nodo;                // el test sostiene al NODO
        ASSERT_EQ(nodo->nombreDeEscena(), std::string("cueva"));
    }   // acá muere la escena, y con ella su nodo (era su dueño)
    auto nodo = nodoGuardado.lock();
    ASSERT_TRUE(nodo == nullptr);           // nodo destruido con la escena
}

TEST_CASE(EscenaMuereAntesQueElNodo) {
    std::weak_ptr<Escena> mirada;
    std::shared_ptr<Nodo> colgado;          // un nodo SIN dueño (controlado)
    {
        auto escena = std::make_shared<Escena>("isla");
        colgado = escena->agregarNodo("cangrejo", escena);
        mirada = escena;
        ASSERT_EQ(colgado->nombreDeEscena(), std::string("isla"));
    }   // la escena muere...
    ASSERT_EQ(Escena::vivas(), 0);
    // ...y el nodo sigue (lo sostiene main), pero SIN escena:
    ASSERT_EQ(colgado->nombreDeEscena(), std::string("(sin escena)"));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 07 (escena/weak)...\n";
    return RUN_ALL_TESTS();
}
