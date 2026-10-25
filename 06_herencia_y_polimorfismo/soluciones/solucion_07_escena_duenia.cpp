/* =============================================================================
   MÓDULO 06 - SOLUCIÓN 07: La escena dueña de sus entidades
   =============================================================================
   Claves de la lección 08:
   - La dueña de los punteros es UNA sola (la Escena); su destructor es
     el ÚNICO punto de limpieza.
   - Cada delete por la base virtual destruye el subobjeto real.
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

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
    void atacar() const override { std::cout << "  golpe de pico\n"; }
    std::string tipoDe() const override { return "obrero"; }
};

class Esqueleto : public Enemigo {
public:
    Esqueleto() : Enemigo("esqueleto") {}
    void atacar() const override { std::cout << "  zas: hueso arma\n"; }
    std::string tipoDe() const override { return "esqueleto"; }
};

class Escena {
    std::vector<Enemigo*> enemigos;

public:
    void agregar(Enemigo* e) {
        if (e != nullptr) enemigos.push_back(e);
    }

    int contar(const std::string& tipo) const {
        int n = 0;
        for (const Enemigo* e : enemigos) {    // tipo DINAMICO: virtual
            if (e->tipoDe() == tipo) ++n;
        }
        return n;
    }

    void ciclo() const {
        for (const Enemigo* e : enemigos) e->atacar();
    }

    ~Escena() {
        for (Enemigo* e : enemigos) delete e;  // delete por base virtual
        // (el vector, al morir, solito no toca los objetos: eso SIEMPRE)
        enemigos.clear();
    }
};

TEST_CASE(TestContarPorTipoDinamico) {
    registroVidas().clear();
    {
        Escena escena;
        escena.agregar(new Obrero{});
        escena.agregar(new Obrero{});
        escena.agregar(new Esqueleto{});
        ASSERT_EQ(escena.contar("obrero"), 2);
        ASSERT_EQ(escena.contar("esqueleto"), 1);
        ASSERT_EQ(escena.contar("dragon"), 0);
    }
    ASSERT_EQ(registroVidas().size(), std::size_t(3));
}

TEST_CASE(TestCicloDestruyeTodo) {
    registroVidas().clear();
    {
        Escena escena;
        escena.agregar(new Obrero{});
        escena.agregar(new Esqueleto{});
        escena.agregar(new Obrero{});
        escena.ciclo();
    }
    ASSERT_EQ(registroVidas().size(), std::size_t(3));
    ASSERT_EQ(registroVidas()[0], std::string("~obrero"));
    ASSERT_EQ(registroVidas()[1], std::string("~esqueleto"));
    ASSERT_EQ(registroVidas()[2], std::string("~obrero"));
}

int main() {
    std::cout << "Solucion Ejercicio 07: escena dueña con RAII manual.\n";
    return RUN_ALL_TESTS();
}
