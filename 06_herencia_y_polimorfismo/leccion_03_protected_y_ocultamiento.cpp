/* =============================================================================
   MÓDULO 06 - LECCIÓN 03: Acceso protected, ocultamiento de nombres y `using`
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      a) Qué significa `protected` exactamente
         Un miembro protected se ve desde:
           - la propia clase,
           - las clases derivadas,
           - funciones de esas clases derivadas.
         Pero con una regla fina: una clase derivada solo puede accesar a un
         miembro protected de la base a través de UN OBJETO DE LA DERIVADA
         (o de otra derivada más profunda), no a través de un objeto base
         arbitrario. Ej: dentro de un método de Mago, puedes leer `personaje.vida`
         si `personaje` es Mago o Mura, no si es Entidad&.

      b) Por qué no abusar
         El invariante del módulo 05 acostumbra: CADA clase garantiza sus
         atributos. Si los dejás protected, cada hijo también los escribe,
         y ya no hay un solo lugar que los clamped. Pregunta: ¿el hijo
         "conoce" el invariante del padre? Si no, hacé métodos protegidos
         (ganchos de extensión) y los datos quedan privados.

      c) Ocultamiento de nombres (name hiding)
         Si la derivada declara un método con el MISMO nombre que otro del
         padre, TODA la sobrecarga del padre con ese nombre queda oculta
         para quien usa la derivada. No se registra por firma: el nombre
         bloquea la vista a todo lo que comparta nombre.

             class Base { public: void f(int); void f(double); };
             class Deriv : public Base { public: void f(const char*); };
             Deriv d;
             d.f(1);          // ERROR: `f(int)` está oculta por `f(const char*)`
             d.f("hola");     // OK (ocultado lo otro)

         Solución: `using Base::f;` en el scope público de la derivada, que
         vuelve a hacer visibles las sobrecargas del padre.

      d) `using` para heredar constructores (C++11)
         `using Base::Base;` inyecta los constructores del padre en la hija,
         útil en jerarquías de envoltorios/passthrough donde la hija agrega
         comportamiento pero no estado nuevo.

   2. ¿QUÉ PASA POR DEBAJO?
      La búsqueda de nombres es FIFO por scope: compila buscando el nombre,
      cuando lo encuentra en un scope, SE DETIENE. Lo que sigue en las clases
      del vecindario no se revisa por si "hubiera una mejor sobrecarga": el
      ocultamiento existe y es intencional, para que volver a dibujar una
      interfaz no dependa del azar de qué hay en la base.

      `using` en realidad no "copia" nada: declara que esos nombres se ven
      aquí. En el binario, las llamadas siguen siendo a la función original.

   3. ERRORES COMUNES
      - Accesar a datos protected de un objeto base arbitrario dentro de
        un método de la derivada: error de compilación.
      - Olvidar `using` tras ocultar un nombre y no entender el error de
        "no hay función igual" (el mensaje del compilador lo informa,
        pero es confuso hasta que sabés el patrón).
      - `using Base::f;` donde no corresponde (en private, se oculta):
        declará los `using` en el mismo nivel de acceso en que los usías.
   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/using_declaration
      https://en.cppreference.com/w/cpp/language/lookup
   ============================================================================= */
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// 1. El nuance de `protected`: la derivada solo ve el subobjeto derivado.
// ---------------------------------------------------------------------------
class Entidad {
protected:
    int vida = 0;

public:
    explicit Entidad(int vida_) : vida(vida_) {}
};

class Mago : public Entidad {
public:
    using Entidad::Entidad;          // hereda los constructores del padre

    void golpearDirecto(const Entidad& e) {
        // ERROR (comentado: compilaría mal):
        // int x = e.vida;           // protected de una Entidad& arbitraria
        (void)e;
        int x = vida;                 // la vida DE ESTE Mago: sí
        std::cout << "  this->vida (via Mago) = " << x << "\n";
    }

    void curar(int cantidad) {
        vida += cantidad;             // el hijo escribe el dato protected
        std::cout << "  Mago curado: vida=" << vida << "\n";
    }
};

// ---------------------------------------------------------------------------
// 2. Name hiding y `using` para revivir sobrecargas.
// ---------------------------------------------------------------------------
class Base {
public:
    void atacar(int danio)  { std::cout << "  atacar(int: " << danio << ")\n"; }
    void atacar(const std::string& s) { std::cout << "  atacar(\"" << s << "\")\n"; }
};

class Derivada : public Base {
public:
    // Sin `using Base::atacar;`, la única `atacar` visible es esta:
    using Base::atacar;              // trae de nuevo TODAS las sobrecargas
    void atacar(const char* texto) {
        std::cout << "  Derivada::atacar(const char*)\n";
        atacar(std::string(texto));   // delega en la versión del padre
    }
};

int main() {
    std::cout << "=== 1. PROTECTED SOLO ENTRE PARTE DERIVADA ===\n";
    Mago gandalf(80);
    gandalf.golpearDirecto(gandalf);
    gandalf.curar(5);

    std::cout << "\n=== 2. NAME HIDING: `using` REVIVE LAS SOBRECARGAS ===\n";
    Derivada d;
    d.atacar(13);                     // viene de Base (gracias al using)
    d.atacar("flecha");               // viene de Base
    d.atacar("piedra");               // la propia de Derivada

    std::cout << "\n=== 3. CTOR HEREDADO CON `using Base::Base` ===\n";
    Entidad base_via_mago = Mago(70);  // conversión Mago -> Entidad
    (void)base_via_mago;
    return 0;
}
