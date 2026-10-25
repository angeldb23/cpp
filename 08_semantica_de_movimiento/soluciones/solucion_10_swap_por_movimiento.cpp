/* =============================================================================
   MÓDULO 08 - SOLUCIÓN 10: Swap escrito a puro movimiento
   =============================================================================
   EXPLICACIÓN:
     El swap clásico a 3 movimientos (la aleta de la L04 "Cómo se
     aplica el movimiento"):

       Cofre temp = std::move(a);   // 1 construcción con robo
       a = std::move(b);            // 1 asignación con robo
       b = std::move(temp);         // 1 asignación con robo

     Y NINGUNA copia: el contenido de cada string solo cambia de
     dueño, no de dirección de memoria. Al final `temp` está vacío
     (moved-from: "casi apagado", L07) y muere sin dolor destruyendo
     un string vacío.

     Que el std::swap de la STL hace exactamente ESTO es la razón
     para exigir operadores de movimiento noexcept: cualquier campo
     movible hace que tu clase sea swappeable en O(1) de todo lo que
     pesa (la asignación de copia tiene que recorrer y duplicar).

     Deuda de estilo: con `if (this == &otro) return *this;` o usando
     un `swap()` miembro, la asimetría sería aún más notoria; acá el
     swap está SIEMPRE sobre dos objetos DISTINTOS a y b (nunca son
     el mismo): la receta de 3 movimientos funciona.
   ============================================================================= */

#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include "../../include/test_framework.hpp"

class Cofre {
public:
    explicit Cofre(std::string tesoro) : tesoro_(std::move(tesoro)) {}

    Cofre(const Cofre& otro) : tesoro_(otro.tesoro_) { ++copias_; }
    Cofre(Cofre&& otro) noexcept : tesoro_(std::move(otro.tesoro_)) { ++construcciones_; }

    Cofre& operator=(const Cofre& otro) {
        tesoro_ = otro.tesoro_;
        ++copias_;
        return *this;
    }
    Cofre& operator=(Cofre&& otro) noexcept {
        tesoro_ = std::move(otro.tesoro_);
        ++asignaciones_;
        return *this;
    }

    const std::string& tesoro() const { return tesoro_; }
    std::size_t tamanio() const { return tesoro_.size(); }
    static int copias() { return copias_; }
    static int asignacionesDeMove() { return asignaciones_; }
    static int construccionesDeMove() { return construcciones_; }

private:
    std::string tesoro_;
    inline static int copias_ = 0;
    inline static int asignaciones_ = 0;
    inline static int construcciones_ = 0;
};

// Los TRES movimientos del swap sin copia alguna.
void intercambiar(Cofre& a, Cofre& b) {
    Cofre temp = std::move(a);   // 1) construyo con robo
    a = std::move(b);            // 2) a recibe el tesoro de b
    b = std::move(temp);         // 3) b recibe el tesoro viejo de a
}

TEST_CASE(ElContenidoCambiaDeDuenio) {
    Cofre a("gema de valor 1");
    Cofre b("estuche de valor 2");
    intercambiar(a, b);
    ASSERT_EQ(a.tesoro(), "estuche de valor 2");
    ASSERT_EQ(b.tesoro(), "gema de valor 1");
}

TEST_CASE(SinCopiasEnElSwap) {
    const int copiasAntes = Cofre::copias();
    Cofre a("oro");
    Cofre b("plata");
    intercambiar(a, b);
    ASSERT_EQ(Cofre::copias(), copiasAntes);
}

TEST_CASE(ElRecuentoDeMovimientos) {
    const int consAntes = Cofre::construccionesDeMove();
    const int asigAntes = Cofre::asignacionesDeMove();
    Cofre a("maquillaje");
    Cofre b("camisa");
    intercambiar(a, b);
    ASSERT_EQ(Cofre::construccionesDeMove(), consAntes + 1);   // el temp
    ASSERT_EQ(Cofre::asignacionesDeMove(), asigAntes + 2);     // los dos swaps
}

TEST_CASE(AmbosSiguenValidos) {
    Cofre a("x");
    Cofre b("y");
    intercambiar(a, b);
    ASSERT_EQ(a.tamanio(), std::size_t(1));
    ASSERT_EQ(b.tamanio(), std::size_t(1));
    ASSERT_EQ(a.tesoro(), "y");                  // ambos leíbles, sin estados raros
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Solución 10...\n";
    return RUN_ALL_TESTS();
}
