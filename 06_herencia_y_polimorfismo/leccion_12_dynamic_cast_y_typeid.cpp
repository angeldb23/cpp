/* =============================================================================
   MÓDULO 06 - LECCIÓN 12: dynamic_cast a fondo, RTTI y typeid
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      dynamic_cast convierte entre secciones de una jerarquía POLIMÓRFICA
      verificando en runtime el tipo real del objeto. Se sabe expresar
      en dos versiones con resultados distintos:

      a) Con PUNTERO: falla elegantemente devolviendo nullptr.
          Dragon* d = dynamic_cast<Dragon*>(e);
          if (d) { ... }                // el chequeo es del llamador
      b) Con REFERENCIA: no puede devolver "nada", así que falla con
         excepción std::bad_cast (generalmente atrapada y decidida).
          try { Dragon& dr = dynamic_cast<Dragon&>(*e); ... }
          catch (const std::bad_cast&) { ... }

      ¿Cuándo SE PUEDE usar? Solo entre clases polimórficas (la base lleva
      al menos un virtual; el destructor virtual de la casa lo garantiza).
      También puede upcastear: dynamic_cast<void*> devuelve la dirección
      del objeto COMPLETO aunque partas de un subobjeto múltiple (ليل la
      L14 es útil: dos punteros a la misma entidad por distintas bases no
      coinciden numéricamente, dynamic_cast<void*> sí que obtiene el
      mismo "origen").

      La comparación por tipo: `typeid(*e)` devuelve la información de
      TIPO DINÁMICO (requiere polimorfismo); `typeid(Dragon)` es info de
      tipo estático. Comparando: `typeid(*e) == typeid(Dragon)`.
      Por defecto RTTI está activo; con -fno-rtti dynamic_cast y typeid
      se apagan (particularmente útil en motores: muchos equipos lo
      quitan y usan jerarquías propias).

      El costo: dynamic_cast recorre la cadena de clases real hasta
      encontrar el ancestro pedido; no es gratis, pero es CORRECTO.

   2. ¿QUÉ PASA POR DEBAJO?
      RTTI se apoya en lo mismo que la vtable: la tabla guarda (en
      compiladores comunes) un puntero a un objeto std::type_info por
      clase (nombre mangleado, comparador, hash). dynamic_cast consulta
      esa info y camina la jerarquía "hacia arriba" buscando el ancestro.

      typeid sobre un DEREFERENCED puntero (typeid(*e)) mira el vptr;
      sobre una variable normal es estático. Esa distinción es la que
      produce errores de principiantes: typeid(e) != typeid(*e).

   3. ERRORES COMUNES
      - dynamic_cast en código_caliente_siempre (cada frame por entidad):
        costo sumado; en veces conviene tratarlo por su lugar del diseño
        (polimorfismo o enumeración switch).
      - Usar typeof fallback sin entender -fno-rtti: si el motor usa
        RTTI desactivado, dynamic_cast NO compila.
      - typeid(e) comparado contra typeid(Dragon) para chequear UN
        PUNTERO: eso es el tipo del PUNTERO (un Enemigo*), ya no del
        objeto real: para eso es `typeid(*e)`.
      - dynamic_cast entre clases NO polimórficas: no compila: si la
        base no tiene ningún método virtual, no hay lugar para instalar
        RTTI y el compilador te lo dice en el mensaje.
   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/dynamic_cast
      https://en.cppreference.com/w/cpp/language/typeid
   ============================================================================= */
#include <iostream>
#include <string>
#include <typeinfo>

// ---------------------------------------------------------------------------
// 1. La jerarquía (polimórfica: destructor virtual de la casa).
// ---------------------------------------------------------------------------
class Enemigo {
protected:
    std::string nombre;

public:
    explicit Enemigo(std::string n) : nombre(std::move(n)) {}
    virtual ~Enemigo() = default;
    virtual void atacar() const = 0;
};

class Dragon : public Enemigo {
public:
    Dragon() : Enemigo("dragon") {}
    void atacar() const override { std::cout << "  aliento de fuego\n"; }
};

class Goblin : public Enemigo {
public:
    Goblin() : Enemigo("goblin") {}
    void atacar() const override { std::cout << "  chispazo\n"; }
};

// ---------------------------------------------------------------------------
// 2. Demostración: puntero vs referencia, typeid del objeto vs del puntero.
// ---------------------------------------------------------------------------
int main() {
    Goblin g;
    Dragon d;
    Enemigo* e = &d;

    std::cout << "=== 1. dynamic_cast CON PUNTERO (nullptr si falla) ===\n";
    Enemigo* conocidos[] = {&d, &g};   // punteros elegidos "en runtime"
    for (Enemigo* e2 : conocidos) {
        if (Dragon* dr = dynamic_cast<Dragon*>(e2)) { (void)dr;
            std::cout << "  es dragon, listo para aliento\n";
        } else {
            std::cout << "  no es dragon (nullptr, seguro)\n";
        }
    }

    std::cout << "\n=== 2. dynamic_cast CON REFERENCIA (excepcion bad_cast) ===\n";
    try {
        Enemigo& ref = g;
        [[maybe_unused]] Dragon& trabajar = dynamic_cast<Dragon&>(ref);
    } catch (const std::bad_cast&) {
        std::cout << "  bad_cast: la referencia no puede fallar en silencio\n";
    }

    std::cout << "\n=== 3. typeid: TIPO DINAMICO vs TIPO DEL PUNTERO ===\n";
    std::cout << "  typeid(*e).name() = " << typeid(*e).name()
              << "  (el objeto real)\n";
    std::cout << "  typeid(e).name()  = " << typeid(e).name()
              << "  (el puntero, tipo estatico)\n";
    if (typeid(*e) == typeid(Dragon)) {
        std::cout << "  *e es exactamente Dragon ( typeid comparado)\n";
    }
    return 0;
}
