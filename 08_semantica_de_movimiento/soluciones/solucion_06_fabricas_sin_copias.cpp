/* =============================================================================
   MÓDULO 08 - SOLUCIÓN 06: Fábricas sin copias: prvalue y NRVO
   =============================================================================
   EXPLICACIÓN:
     Ambas fábricas devuelven PRVALUES (temporales sin nombre). Con
     eso, la copy elision es GARANTÍA del estándar desde C++17 (L08):
     el objeto se construye DIRECTO en el destino (`c`, `facil`,
     `duro`, cada `m` del bucle). Ni constructor de copia ni de
     movimiento: los contadores ni se mueven, aunque los
     constructores tengan `++` adentro: no hay llamada que hacer.

     En las dos ramas de `variante` pasa lo mismo: cada `return`
     ya es una prvalue, no hay variable intermedia que mover.

     El ANTI-patrón que el ejercicio evita a propósito:
        Conteo mal(int valor) {
            Conteo local(valor * 2);
            return std::move(local);   // <<< MATA la NRVO: +1 move gratis
        }
     El return simple de un local (NRVO) también queda en cero
     copias, pero el prvalue directo NI DEPENDE del optimizador.
   ============================================================================= */

#include <cstddef>
#include <iostream>
#include <utility>
#include "../../include/test_framework.hpp"

struct Conteo {
    static inline int copias_ = 0;
    static inline int movimientos_ = 0;
    int valor_ = 0;

    explicit Conteo(int v) : valor_(v) {}
    Conteo(const Conteo& otro) : valor_(otro.valor_) { ++copias_; }
    Conteo(Conteo&& otro) noexcept : valor_(otro.valor_) { ++movimientos_; }
};

// prvalue directo: construcción in-place en el destino. Cero llamadas extra.
Conteo duplica(int valor) {
    return Conteo{valor * 2};
}

// Cada rama devuelve su prvalue: tampoco hay nada que mover.
Conteo variante(bool preferir, int valor) {
    if (preferir) return Conteo{valor + 1};
    return Conteo{valor - 1};
}

TEST_CASE(PRValueElisionGarantizada) {
    const int copiasAntes = Conteo::copias_;
    const int movimientosAntes = Conteo::movimientos_;
    const Conteo c = duplica(7);
    ASSERT_EQ(c.valor_, 14);
    ASSERT_EQ(Conteo::copias_, copiasAntes);
    ASSERT_EQ(Conteo::movimientos_, movimientosAntes);
}

TEST_CASE(RamasSinCopias) {
    const int copiasAntes = Conteo::copias_;
    const Conteo facil = variante(true, 10);
    const Conteo duro = variante(false, 10);
    ASSERT_EQ(facil.valor_, 11);
    ASSERT_EQ(duro.valor_, 9);
    ASSERT_EQ(Conteo::copias_, copiasAntes);
}

TEST_CASE(VariasLlamadasSinAcarrearCopias) {
    const int copiasAntes = Conteo::copias_;
    for (int i = 0; i < 8; ++i) {
        const Conteo m = duplica(i);
        if (m.valor_ != i * 2) ASSERT_TRUE(false);
    }
    ASSERT_EQ(Conteo::copias_, copiasAntes);
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Solución 06...\n";
    return RUN_ALL_TESTS();
}
