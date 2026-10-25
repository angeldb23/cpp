/* =============================================================================
   MÓDULO 07 - SOLUCIÓN 07: Escena con hijos y flechas hacia atrás
   =============================================================================
   Práctica de las lecciones 08 y 09: la flecha de la escena a sus hijos es
   la ÚNICA flecha strong; los hijos miran la escena con weak_ptr y usan
   lock() para hablar con ella "si todavía está".
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

class Escena;

class Nodo {
private:
    std::string nombre_;
    std::weak_ptr<Escena> escena_;   // la flecha DEBIL hacia atrás

public:
    explicit Nodo(std::string nombre) : nombre_(std::move(nombre)) {}

    const std::string& nombre() const { return nombre_; }

    // Guardamos la flecha débil: NO sostiene la vida de la escena.
    void conectado(std::weak_ptr<Escena> s) { escena_ = std::move(s); }

    // El rito lock(): decide y sostiene en un paso atómico (L09).
    // (cuerpo después: Escena es aún un forward declaration acá)
    std::string nombreDeEscena() const;
};

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

    std::shared_ptr<Nodo> agregarNodo(const std::string& nombreNodo,
                                      const std::shared_ptr<Escena>& yo) {
        auto n = std::make_shared<Nodo>(nombreNodo);
        n->conectado(yo);              // flecha DÉBIL: escena <- nodo
        nodos_.push_back(n);           // flecha STRONG: escena -> nodo
        return n;
    }
};

std::string Nodo::nombreDeEscena() const {
    if (auto escena = escena_.lock()) {
        return escena->nombre();
    }
    return "(sin escena)";
}

// ============= TESTS =============
TEST_CASE(UnaFlechaStrongUnaWeak) {
    ASSERT_EQ(Escena::vivas(), 0);
    {
        auto escena = std::make_shared<Escena>("bosque");
        ASSERT_EQ(escena.use_count(), 1);
    }
    ASSERT_EQ(Escena::vivas(), 0);
}

TEST_CASE(NodoVeAEscenaViva) {
    auto escena = std::make_shared<Escena>("desierto");
    auto nodo = escena->agregarNodo("cactus", escena);
    ASSERT_EQ(nodo->nombreDeEscena(), std::string("desierto"));
}

TEST_CASE(NodoVeQueLaEscenaMurió) {
    std::weak_ptr<Nodo> nodoGuardado;
    {
        auto escena = std::make_shared<Escena>("cueva");
        auto nodo = escena->agregarNodo("murcielago", escena);
        nodoGuardado = nodo;
        ASSERT_EQ(nodo->nombreDeEscena(), std::string("cueva"));
    }
    auto nodo = nodoGuardado.lock();
    ASSERT_TRUE(nodo == nullptr);       // nodo destruido con su dueña
}

TEST_CASE(EscenaMuereAntesQueElNodo) {
    std::weak_ptr<Escena> mirada;
    std::shared_ptr<Nodo> colgado;
    {
        auto escena = std::make_shared<Escena>("isla");
        colgado = escena->agregarNodo("cangrejo", escena);
        mirada = escena;
        ASSERT_EQ(colgado->nombreDeEscena(), std::string("isla"));
    }
    ASSERT_EQ(Escena::vivas(), 0);      // el weak del nodo NO la retuvo
    ASSERT_EQ(colgado->nombreDeEscena(), std::string("(sin escena)"));
}

int main() {
    std::cout << "Ejecutando SOLUCIÓN 07 (escena/weak)...\n";
    return RUN_ALL_TESTS();
}
