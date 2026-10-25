/* =============================================================================
   MÓDULO 06 - LECCIÓN 06: vtable y vptr por debajo — qué cuesta un `virtual`
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      ¿Cómo hace el compilador para llamar a Arquero::atacar() cuando solo
      ve un Enemigo*? Con una tabla de funciones virtuales:
      - La VTABLE (tabla virtual) es una estructura por CLASE: una lista de
        punteros a función para cada método virtual de esa clase.
        (Se genera una vez por clase con métodos virtuales, en la sección
        `.rodata` del binario o similar; es un detalle de implementación,
        pero así lo hacen todos los compiladores que usás.)
      - El VPTR (puntero virtual) es UN PUNTERO por OBJETO. Al construir
        cada objeto, su constructor instala en ese campo la vtable de SU
        clase. Ese es el marcador del tipo dinámico.

      La llamada `e->atacar()` cuando e es Enemigo* se compila a algo como:
          (e->vptr)[indice_de_atacar](e)
      Es decir: 1) leer el vptr del objeto; 2) indexar la tabla; 3) llamar
      el puntero a función con `e` como this.

      Consecuencias prácticas:
      a) Costo por objeto: un vptr (8 bytes en 64 bits) más el padding que
         se agrega. sizeof(Base) crece con el primer virtual.
      b) Costo por llamada: una indirección extra que el compilador (y el
         predictor de saltos del CPU) suelen pagar bien; la inlinedeza,
         en general, NO se puede hacer por no conocer el target.
      c) Es una decisiones de cada CLASE, no del objeto: el primer método
         virtual de una clase enciende el mecanismo para la clase entera.

   2. ¿QUÉ PASA POR DEBAJO?
      Resolver por tipo estático (call directa) es: dirección codificada en
      el binario, sin indirección, ideal para inline. Resolverlo dinámico
      agrega esa tabla. LA REGLA DE DISEÑO: los métodos que van a variar por
      subtipo (atacar, render, actualizar) van virtual; los que son igual
      para la familia (getVida, mostrarInfo) NO: mantenen el costo en el
      mínimo y permiten que el inliner haga magia.

      El compilador no lo hace automático: se llama "desvirtualización"
      (devirtualization) cuando puede probar el tipo dinámico (por ejemplo
      con final o cuando el objeto está en el stack del mismo scope) y ahí
      sí llama directa y inlinedea.

   3. ERRORES COMUNES
      - Marcar todo virtual "por miedo": en motores de juego, millones de
        objetos con vptr se despiden de la caché por línea. Perf: elegir.
      - Creer que virtual "copia" el método en cada objeto: el código es
        compartido UNA vez por clase; el objeto solo lleva el puntero.
      - Copiar un objeto polimórfico a un tipo base por valor (slicing):
        el vptr queda, pero solo a la vtable del tipo estático... el objeto
        la derivada ya no está más por haber copiado la parte base (L13).
   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/virtual (semántica)
      https://en.wikipedia.org/wiki/Virtual_method_table (layout, ensamblado)
   ============================================================================= */
#include <iostream>
#include <string>
#include <typeinfo>

// ---------------------------------------------------------------------------
// 1. Dos clases: una SIN virtual aunque hay override (como quedaría) y una
//    CON virtual para comparar pesos.
// ---------------------------------------------------------------------------
class SinVirtual {
    int vida = 0;

public:
    int getVida() const { return vida; }     // estático: sin tabla
    void recibir(int d) {
        vida = (vida > d) ? vida - d : 0;
    }
};

class ConVirtual {
public:
    virtual ~ConVirtual() = default;         // el virtual "enciende la tabla"
    virtual int getVida() const { return 0; }
};

class Juguete : public SinVirtual {};
class JugueteVirtual : public ConVirtual {
public:
    int getVida() const override { return 7; }
};

// ---------------------------------------------------------------------------
// 2. La base de juego de esta lección: una interfaz chica.
// ---------------------------------------------------------------------------
class Accion {
public:
    virtual ~Accion() = default;
    virtual void ejecutar() const = 0;       // virtual puro (L09)
};

class Golpe : public Accion {
public:
    void ejecutar() const override { std::cout << "  golpe!\n"; }
};

class Pausa : public Accion {
public:
    void ejecutar() const override { std::cout << "  pausa!\n"; }
};

int main() {
    SinVirtual s;
    ConVirtual cv;
    Juguete j;
    JugueteVirtual jv;
    (void)j; (void)jv;

    std::cout << "=== 1. EL COSTO EN MEMORIA ===\n";
    std::cout << "  sizeof(SinVirtual)         = " << sizeof(s) << "\n";
    std::cout << "  sizeof(new ConVirtual) en bytes = " << sizeof(cv) << "\n";
    std::cout << "  sizeof(Juguete)            = " << sizeof(Juguete) << "\n";
    std::cout << "  sizeof(JugueteVirtual)     = " << sizeof(JugueteVirtual)
              << " (el vptr entra; en 64 bits son 8 bytes)\n";

    std::cout << "\n=== 2. EL MECANISMO EN CADA LLAMADA ===\n";
    Golpe g;
    Pausa p;
    const Accion* seleccion[] = {&g, &p, &g};
    for (const Accion* a : seleccion) {
        a->ejecutar();       // vptr -> vtable -> entrada[i] -> (this)
    }

    std::cout << "\n=== 3. DEVIRTUALIZACION CUANDO EL COMPILADOR SABE ===\n";
    Golpe directa;           // tipo dinámico conocido en este scope
    directa.ejecutar();      // casi seguro: llamada directa (no indirecta)
    return 0;
}
