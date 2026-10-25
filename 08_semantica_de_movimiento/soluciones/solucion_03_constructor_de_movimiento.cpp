/* =============================================================================
   MÓDULO 08 - SOLUCIÓN 03: El constructor de movimiento a mano
   =============================================================================
   EXPLICACIÓN:
     El contrato del movimiento con recursos crudos (L05):

       1. ROBAR los internos: `otro.datos_` y `otro.tamanio_` pasan
          a ser del destino. Son DOS palabras copiadas, no n datos.
       2. VACIAR al origen: `otro.datos_ = nullptr` y
          `otro.tamanio_ = 0`. Si no, su destructor hace delete[]
          ESE MISMO buffer que acabamos de dar al destino: doble
          liberación. Vaciar es obligatorio, no cortesía.
       3. noexcept: anota la promesa; la van a necesitar los
          contenedores (L06/L10).

     Nótese el orden de la lista de inicialización: se parean con los
     MIEMBROs, no con lo robado: primero tomo el puntero del `otro`,
     y el cuerpo solo hace la limpieza del origen. Con `std::swap`
     quedaría incluso más compacto, pero así se ve cada paso.
   ============================================================================= */

#include <cstddef>
#include <iostream>
#include <utility>
#include "../../include/test_framework.hpp"

class Muestras {
public:
    explicit Muestras(std::size_t n) : datos_(new float[n]{}), tamanio_(n) {}

    Muestras(const Muestras& otro)
        : datos_(new float[otro.tamanio_]), tamanio_(otro.tamanio_) {
        for (std::size_t i = 0; i < tamanio_; ++i) datos_[i] = otro.datos_[i];
        ++copias_;
    }

    // EL MOVIMIENTO: robo + origen vacío + promesa de no lanzar.
    Muestras(Muestras&& otro) noexcept
        : datos_(otro.datos_), tamanio_(otro.tamanio_) {  // 1) robo las palabras
        otro.datos_   = nullptr;           // 2) el origen queda sin recurso
        otro.tamanio_ = 0;                 //    (su dtor no toca nada)
        ++movimientos_;
    }

    ~Muestras() { delete[] datos_; }

    std::size_t tamanio() const { return tamanio_; }
    float muestra(std::size_t i) const { return datos_[i]; }
    void guardo(std::size_t i, float v) { if (i < tamanio_) datos_[i] = v; }

    static int copias() { return copias_; }
    static int movimientos() { return movimientos_; }

private:
    float*      datos_;
    std::size_t tamanio_;
    inline static int copias_ = 0;
    inline static int movimientos_ = 0;
};

TEST_CASE(MoverTransfiereLosDatos) {
    Muestras a(4);
    a.guardo(0, 0.5f); a.guardo(1, 1.5f);
    Muestras b(std::move(a));
    ASSERT_EQ(b.tamanio(), std::size_t(4));
    ASSERT_NEAR(b.muestra(0), 0.5f, 0.0001f);
    ASSERT_NEAR(b.muestra(1), 1.5f, 0.0001f);
    ASSERT_EQ(Muestras::movimientos(), 1);
}

TEST_CASE(EsNoexcept) {
    ASSERT_EQ(noexcept(Muestras(std::declval<Muestras&&>())), true);
}

TEST_CASE(OrigenQuedaVacio) {
    Muestras a(6);
    Muestras b(std::move(a));
    ASSERT_EQ(a.tamanio(), std::size_t(0));   // el robado: vacio, no muerto
}

TEST_CASE(NoHuboCopias) {
    const int copiasAntes = Muestras::copias();
    Muestras a(3);
    Muestras b(std::move(a));
    ASSERT_EQ(Muestras::copias(), copiasAntes);   // cero copias al mover
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Solución 03...\n";
    return RUN_ALL_TESTS();
}
