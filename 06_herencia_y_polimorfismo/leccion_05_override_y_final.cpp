/* =============================================================================
   MÓDULO 06 - LECCIÓN 05: `override` y `final` — errores que el compilador
                              agarra por vos
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      En C++ un "¡sobrescribo el método del padre!" es solo una intención.
      Si la firma no coincide (argumento de otro tipo, distinto de const, typo
      distinto), el compilador cree una NUEVA función, no una sobrescribe.
      Resultado: silenciosamente ya no hay polimorfismo.

      `override` al final de la firma convierte eso en error de compilación:
      "este método NO sobrescribe nada del padre". Regla de la casa: TODO
      método que sobrescribe lleva `override`.

      Ejemplos que SIN override no son sobrescritura:
          en la base:  virtual void atacar() const;
          en el hijo:  void atacar() const override;  // ok (realmente sobrescribe)
                       void atacar() const;           // oculta (nueva funcion)
                       void atacar() override;        // ERROR: firma distinta
                       void atacar() const override;  // typo... ya lo viste arriba

      a) `final` en un método virtual: prohibe que las derivadas siguientes
         lo sobrescriban. Puede ir solo (donde no puede haber otro) o junto
         con `override`.
      b) `final` en una clase: nadie puede heredar de ella. Útil para
         cerrar jerarquías de valor (un Estado ya no se extiende).

      Ni `override` ni `final` son palabras reservadas fuera de ese contexto:
      podés llamarte `int override;` en otro lado (mal estilo, pero legal).

   2. ¿QUÉ PASA POR DEBAJO?
      `override` es solo una anotación de verificación: el compilador
      compará la firma (nombre, número y tipos de parámetros, cv-cualificadores
      y también si la base era virtual). Si no matchea, error. No cambia el
      código generado. `final` sí optimiza: un método final se puede
      "desvirtualizar" (llamada directa), porque el compilador sabe que no hay
      otra versión por debajo de esa clase.

   3. ERRORES COMUNES
      - Sobrescribir fallando por falta de `const` en el hijo: con override
        lo agarra el compilador; sin override, un silencioso "el hijo no
        arranca el método del padre".
      - Diferencia de `&` en ref cualificados: extraños, pero cuentan en la firma.
      - Usar final "por si acaso": es UN COMPROMISO — quien después
        investiga no lo puede extender. Ponelo solo cuando esté claro.
      - Confundir `final` en un método con cerrar la jerarquía: para eso es
        final en la CLASE, con la casita `class X final { ... };`.

   4. PARALELO DEL DÍA: los otros compiladores (Java, C#) marcan override
      por defecto. C++ te lo dejó a vos: explicita siempre.

   5. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/override (override specifier)
      https://en.cppreference.com/w/cpp/language/final (final specifier)
   ============================================================================= */
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// 1. Base con un método virtual polimórfico.
// ---------------------------------------------------------------------------
class Arma {
public:
    virtual ~Arma() = default;
    virtual int danio() const { return 5; }
    virtual std::string nombre() const { return "arma generica"; }
};

// ---------------------------------------------------------------------------
// 2. Hijo con override: la firma tiene que matchear EXACTO.
// ---------------------------------------------------------------------------
class Espada : public Arma {
public:
    int danio() const override { return 12; }
    std::string nombre() const override { return "espada"; }
    // Descomenta y prueba (error de compilación, como debe ser):
    // int danio() override {}                 // falta `const`
    // int danio() const noexcept override;    // noexcept cambia la firma
    // int danio() const ensayar;              // typo: nueva función sin connection
};

// ---------------------------------------------------------------------------
// 3. `final`: cierra la evolución en UNA parte o en TODA la clase.
// ---------------------------------------------------------------------------
class ArmaLegendaria final : public Espada {   // nadie hereda de acá
public:
    std::string nombre() const final { return "espada legendaria"; }
};

// ---------------------------------------------------------------------------
// 4. La cadenita del final: de legendaria para abajo no se puede extender.
// ---------------------------------------------------------------------------
class ArmaRota : public Arma {        // ok: Arma no es final
public:
    int danio() const override { return 0; }
};

int main() {
    std::cout << "=== 1. POLIMORFISMO CON override ===\n";
    Espada espada;
    ArmaLegendaria legendaria;
    ArmaRota rota;
    std::cout << "  " << espada.nombre() << ": " << espada.danio() << "\n";
    std::cout << "  " << legendaria.nombre() << ": " << legendaria.danio() << "\n";
    std::cout << "  " << rota.nombre() << ": " << rota.danio() << "\n";

    std::cout << "\n=== 2. EL COMPILADOR COMO GUARDIÁN ===\n";
    // El ejemplo comentado no compila: esa es la postura del curso.
    // Si en un proyecto sin override escribes `int danio() noexcept` y la
    // base era `int danio() const`, el compilador NO AVISA y se te rompe
    // el juego en runtime que el dragon no usa la nueva el equipamiento.
    Arma* a = &legendaria;
    std::cout << "  via Arma*: " << a->nombre() << " (tipo dinamico manda)\n";
    return 0;
}
