/* =============================================================================
   MÓDULO 06 - LECCIÓN 15: Herencia virtual y el problema del diamante
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      El DIAMANTE: una clase hereda de dos clases que a su vez heredan
      de una MISMA base abuela:

              A
             / \
            B   C
             \ /
              D

          class A { public: int codigo; };
          class B : public A {};
          class C : public A {};
          class D : public B, public C {};   // pregunta: ¿cuántas A tiene D?

      Sin herencia virtual: D tiene DOS subobjetos A independientes,
      uno dentro de B y otro dentro de C. Consecuencias:
      - d.codigo no compila: ¿el de la B-A o el de la C-A? Ambiguo.
      - Un upcast de D a A es ambiguo: no compila sin calificar camino.
      - Los datos quedan DUPLICADOS: dos copias de lo mismo, que hay
        que manterner sincronizadas a mano (pesadilla).

      HERENCIA VIRTUAL: en la línea, las bases intermedias marcan a la
      base común como VIRTUAL:

          class B : public virtual A {};
          class C : public virtual A {};
          class D : public B, public C {};   // D tiene UNA sola A

      Ahora D comparte el subobjeto A entre B y C: una sola copia, un
      único camino de upcast, sin ambigüedad en `d.codigo`.

      El costo y la rareza: la construcción cambia. Con regularidad, la
      base VIRTUAL la construye LA CLASE MÁS DERIVADA, sin importar que
      B o C la invocan en sus listas de inicialización (sus llamadas se
      ignoran cuando alguien más derivado está creando el objeto). Si
      A no tiene constructor por defecto, TODA clase del diamante tiene
      que saber constructor de A: acople feo, mejor que A tenga default.

   2. ¿QUÉ PASA POR DEBAJO?
      El layout ya no es una cadena lineal de subobjetos: la base virtual
      se coloca en una sola posición (normalmente al final) y las clases
      B y C llevan un puntero (o offset, según el compilador) a esa base
      compartida. Pagar: cada acceso a lo virtual requiere leer ese
      offset (indirección extra), y objetos ocupan algo más.

      Este mecanismo es un último recurso: la STL lo usa en un lugar
      célebre, std::iostream (hereda de istream y ostream SIN duplicar
      el stream base ios_base): por eso las bibliotecas lo usan con
      moderación, para jerarquías de verdad necesarias.

   3. ERRORES COMUNES
      - Agregar `virtual` SOLO en la más derivada (D): no comparte la
        base común: la palabra se marca en las bases INTERMEDIAS
        (B y C en el ejemplo).
      - Sobrevenderla: usar herencia virtual "por miedo" a duplicar
        datos cuando lo correcto es un rediseño (la base común era un
        detalle accidental, no del concepto).
      - Olvidar el constructor de la base virtual en la clase final:
        se usa el default de A silenciosamente, lo que puede inicializar
        mal (o explotar si no hay default).
      - Confundir herencia virtual con métodos virtual: son conceptos
        100 % distintos, el nombre no ayuda: la palabra `virtual` reusa
        el token pero el mecanismo no tiene nada que ver.
   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/virtual_inheritance
      https://en.cppreference.com/w/cpp/language/constructor (regla del más derivado)
   ============================================================================= */
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// 1. El diamante SIN herencia virtual: dos copias, ambigüedad.
// ---------------------------------------------------------------------------
class Nucleo {
public:
    int energia = 100;
};

class Motores : public Nucleo {};         // hereda normal
class Sensores : public Nucleo {};        // hereda normal

class NaveNodriza : public Motores, public Sensores {
public:
    void diagnosticar() const {
        std::cout << "  energia(?): hay DOS nucleos, no se cual\n";  // ambiguo
    }
    int energiaMotores()  const { return Motores::energia; }
    int energiaSensores() const { return Sensores::energia; }
};

// ---------------------------------------------------------------------------
// 2. El diamante CON herencia virtual: UNA sola Nucleo.
// ---------------------------------------------------------------------------
class NucleoVirtual {
public:
    int energia = 0;
    NucleoVirtual() : energia(100) { std::cout << "  [ctor NucleoVirtual]\n"; }
    explicit NucleoVirtual(int e) : energia(e) { std::cout << "  [ctor NucleoVirtual(" << e << ")]\n"; }
};

class MotoresV : public virtual NucleoVirtual {};   // virtual acá...
class SensoresV : public virtual NucleoVirtual {};  // y acá

class NaveFederada : public MotoresV, public SensoresV {
public:
    // CLAVE: la clase MÁS DERIVADA construye la base virtual.
    NaveFederada() : NucleoVirtual(77) {          // (la mas derivada construye la base virtual)
        std::cout << "  [ctor NaveFederada]\n";
    }
    void diagnosticar() const {
        std::cout << "  energia unica: " << energia << "\n";
    }
};

// ---------------------------------------------------------------------------
// 3. Demostración.
// ---------------------------------------------------------------------------
int main() {
    std::cout << "=== 1. DIAMANTE SIN VIRTUAL: DOS NUCLEOS ===\n";
    {
        NaveNodriza nave;
        nave.energiaMotores();    // 100 (del subobjeto de Motores)
        nave.diagnosticar();
        // nave.energia;          // NO compila: ambiguo entre Motores y Sensores
        std::cout << "  (acceder a energia a efectos nave-> direct: compilador se enoja)\n";

        Nucleo* nucleo = nullptr;
        // nucleo = &nave;         // NO compila tambien: dos caminos posibles.
        (void)nucleo;
    }

    std::cout << "\n=== 2. DIAMANTE CON VIRTUAL: UN NUCLEO COMPARTIDO ===\n";
    {
        NaveFederada nave;
        nave.diagnosticar();      // la casi-única base, construida una vez
        NucleoVirtual* nucleo = &nave;   // UN solo camino: no es ambiguo
        std::cout << "  via NucleoVirtual*: " << nucleo->energia << "\n";
    }
    return 0;
}
