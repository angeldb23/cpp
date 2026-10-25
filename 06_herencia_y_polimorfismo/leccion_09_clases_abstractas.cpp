/* =============================================================================
   MÓDULO 06 - LECCIÓN 09: Clases abstractas y métodos virtuales puros
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Un método virtual puro se declara anexando `= 0` a la firma:

          virtual void atacar() const = 0;

      La base NO da implementación para esa operación: solo promete que
      existe. Una clase con AL MENOS un virtual puro es ABSTRACTA:
      - No se puede instanciar directamente (`Enemigo e;` no compila).
      - Cada derivada concreta DEBE implementar los puros que herede;
        si le falta alguno, también es abstracta.
      - Sí se pueden tener punteros y referencias `Enemigo*` / `Enemigo&`:
        ahí vive el polimorfismo.

      ¿Para qué pensar así?
      - La base define LOS CONCEPTOS del dominio: "todo enemigo sabe
        atacar" sin decir cómo.
      - Compartir datos y código común (nombre, vida, mostrarInfo) sin
        fingir con una implementación vacía heredable por accidente.

      Detalle que sorprende: un virtual puro PUEDE tener cuerpo. Se
      define fuera de la clase (`void Enemigo::atacar() const {...}`) y
      una derivada lo reutiliza llamándolo explícito
      (`Enemigo::atacar();` dentro de su propio override). Es poco común,
      pero existe.

   2. ¿QUÉ PASA POR DEBAJO?
      `= 0` no pinta el método de nada: anuncia que la vtable de la clase
      abstracta NO va a traer una implementación común; cada derivada
      concreta escribe la suya (o hace fallback al default del puro).
      Intentar crear un objeto de clase abstracta es error de compilación:
      no tendría qué ejecutar en esas entradas de su vtable.

   3. ERRORES COMUNES
      - Intentar `Enemigo var;` o pasar `Enemigo` por valor: no compila
        (y por valor además choca con slicing, L13).
      - Olvidar un puro en una derivada intermedia: esa derivada sigue
        abstracta y el error aparece más tarde, cuando el ABSTRACTO de
        arriba se intenta instanciar: mensaje confuso si no sabés el patrón.
      - Instanciar la derivada que "casi" implementó todo: buscar el
        mirá la lista de funciones puras pendientes en el mensaje
        del compilador.
      - Marcar el destructor como puro (`virtual ~Enemigo() = 0;`) sin
        proveerle cuerpo: es legal, pero OBLIGA a definir el cuerpo aparte
        o el linker explota (las derivadas llaman al dtor de la base).
   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/abstract_class
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// 1. Clase abstracta: datos comunes + un virtual puro.
// ---------------------------------------------------------------------------
class Enemigo {
protected:
    std::string nombre;
    int vida = 0;

public:
    Enemigo(std::string n, int v) : nombre(std::move(n)), vida(v) {}
    virtual ~Enemigo() = default;

    virtual void atacar() const = 0;    // PURO: cada enemigo ataca SU forma

    void mostrarInfo() const {          // código común, heredado gratis
        std::cout << "  [" << nombre << " | " << vida << "HP]\n";
    }
    void herir(int danio) {
        vida = (vida > danio) ? vida - danio : 0;
    }
};

// Implementación "por defecto" de un puro: legal, pero NO dispensa a las
// derivadas: para reutilizarla tienen que llamarla explícitamente.
void Enemigo::atacar() const {
    std::cout << "  golpesito generico de " << nombre << "\n";
}

// ---------------------------------------------------------------------------
// 2. Derivadas CONCRETAS: implementan el puro obligatoriamente.
// ---------------------------------------------------------------------------
class Esqueleto : public Enemigo {
public:
    Esqueleto() : Enemigo("esqueleto", 30) {}
    void atacar() const override {
        std::cout << "  zas: hueso arma\n";
    }
};

class Slime : public Enemigo {
public:
    Slime(int v) : Enemigo("slime", v) {}
    void atacar() const override {
        Enemigo::atacar();              // reutiliza el default del puro
        std::cout << "  (y deja babosa)\n";
    }
};

// ---------------------------------------------------------------------------
// 3. Derivada que olvida el puro: SIGUE abstracta.
//
//    class Sombra : public Enemigo {};
//
//    Descomentá la clase y su uso abajo: el compilador va a decir
//    "cannot declare variable 's' to be of abstract type 'Sombra'",
//    y debajo la lista de funciones puras pendientes.
// ---------------------------------------------------------------------------

int main() {
    std::cout << "=== 1. LA ABSTRACTA NO SE INSTANCIA (por diseño) ===\n";
    std::cout << "  (seria error: Enemigo e;)\n";

    std::cout << "\n=== 2. POLIMORFISMO SOBRE LA ABSTRACTA ===\n";
    {
        std::vector<Enemigo*> ejercito;          // punteros crudos (modulo 04)
        Esqueleto hueso;
        Slime baba(40);
        ejercito.push_back(&hueso);              // el dueno es el stack, ok
        ejercito.push_back(&baba);
        for (const Enemigo* e : ejercito) {
            e->mostrarInfo();
            e->atacar();                          // virtual: version real
        }
    }

    std::cout << "\n=== 3. DERIVADA QUE OLVIDA EL PURO (si el caso se descomenta) ===\n";
    std::cout << "  (caso comentado arriba: sigue abstracta si falta alguno)\n";
    return 0;
}
