/* =============================================================================
   MÓDULO 06 - EJERCICIO 08: Figuras de UI: abstracta + interfaz pura
   =============================================================================
   DIFICULTAD: Medio
   PRACTICA: lecciones 09 y 10 (clases abstractas, métodos virtuales puros,
   interfaz sin datos y sus promesas).

   CONSIGNA:
   Están dadas las INTERFACES (NO las toques):
     (ojo: `Figura` tiene dos métodos puros que usan estado protected)
     - `Figura`     (abstracta de dominio, tiene datos protected).
     - `Clicable`   (interfaz pura de capacidad, sin datos).

   1. Completá `Boton`:
      - (YA HEREDA de ambas bases: Figura y Clicable; ese rol doble es
        parte del esqueleto; el trabajo tuyo son los metodos).

      - constructor que reciba (nombre, x, y) y llame al de Figura.
      - `dimeNombre()` y `dimeArea()` implementan los puros de Figura:
        nombre devuelto TAL CUAL; área = 100 (un botón fijo).
      - `alClic()` de la interfaz devuelva true SOLO si `activable`
        es true (estado propio del botón).
   2. `Slider` es una segunda derivada CONCRETA: implementá los puros
      (nombre tal cual y área 400, alClic false siempre).
   3. NO cambies las interfaces: si te falta un puro, no compila al
      instanciar (y esa es la clase de error que querés ver temprano).

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_08_abstracta_e_interfaz.cpp -o ej08 && ./ej08
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

// ---------------------------------------------------------------------------
// Interfaces DADAS (no las cambies).
// ---------------------------------------------------------------------------
class Figura {
protected:
    std::string nombre;
    int x = 0, y = 0;

public:
    Figura(std::string n, int x_, int y_) : nombre(std::move(n)), x(x_), y(y_) {}
    virtual ~Figura() = default;

    virtual std::string dimeNombre() const = 0;   // puro de dominio
    virtual int         dimeArea()   const = 0;   // puro de dominio
    virtual void        mover(int dx, int dy) = 0;  // puro con estado
protected:
    std::string nom() const { return nombre; }
};

class Clicable {
public:
    virtual ~Clicable() = default;
    virtual bool alClic() = 0;                    // promesa de capacidad
};

// ---------------------------------------------------------------------------
// 1. Tu primera derivada: datos de la base + capacidad de clic.
// ---------------------------------------------------------------------------
class Boton : public Figura, public Clicable {   // dada: las dos bases
    bool activable = false;

public:
    // TODO: constructor completo (llama a Figura y guarda `activable`):
    Boton(const std::string& n, int x_, int y_, bool act_)
        : Figura(n, x_, y_) {
        (void)act_;                      // TODO: usá act_ para `activable`
    }

    // TODO 1b: dimeNombre -> tal cual; dimeArea -> 100
    std::string dimeNombre() const override { return "placeholder"; }
    int dimeArea() const override { return -1; }

    // TODO 1c: mover() del puro con estado (x += dx; y += dy)
    void mover(int dx, int dy) override { (void)dx; (void)dy; }  // PLACEHOLDER

    // TODO 1d: alClic() -> true solo si activable
    // TODO 1d: alClic() debe sobrescribir el de Clicable: acá va la correcta
    bool alClic()       { return false; }       // (sin override todavia)
};

// ---------------------------------------------------------------------------
// 2. Otra concreta: nombre del usuario, área grande, nunca cliquable.
// ---------------------------------------------------------------------------
class Slider : public Figura, public Clicable {
public:
    // (firma dada porque no es el foco del ejercicio)
    Slider(const std::string& n, int x_, int y_) : Figura(n, x_, y_) {}

    std::string dimeNombre() const override { return nom(); }
    int dimeArea() const override { return -1; }        // TODO: 400
    void mover(int dx, int dy) override { (void)dx; (void)dy; }
    bool alClic() override { return false; }       // (esta SI es la idea)
};

// ============= TESTS =============
TEST_CASE(TestBotonFigura) {
    Boton b("aceptar", 10, 20, true);
    ASSERT_EQ(b.dimeNombre(), std::string("aceptar"));
    ASSERT_EQ(b.dimeArea(), 100);
}

TEST_CASE(TestBotonClic) {
    Boton activo("aceptar", 10, 20, true);
    Boton dormido("cancelar", 0, 0, false);
    ASSERT_TRUE(activo.alClic());
    ASSERT_FALSE(dormido.alClic());
}

TEST_CASE(TestPolimorfismoDobleInterfaz) {
    Boton b("menu", 1, 1, true);
    Figura* f = &b;
    Clicable* c = &b;
    ASSERT_EQ(f->dimeNombre(), std::string("menu"));
    ASSERT_TRUE(c->alClic());

    Slider s("brillo", 0, 0);
    Clicable* sc = &s;
    ASSERT_FALSE(sc->alClic());
    ASSERT_EQ(s.dimeArea(), 400);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 08...\n";
    return RUN_ALL_TESTS();
}
