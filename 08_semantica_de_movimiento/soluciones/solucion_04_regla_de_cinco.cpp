/* =============================================================================
   MÓDULO 08 - SOLUCIÓN 04: Regla de 5: asignación de movimiento + noexcept
   =============================================================================
   EXPLICACIÓN:
     La asignación de movimiento con la receta de la L06:

       1. GUARD: `if (this != &otro)`: la autoasignación (`a =
          std::move(a)` pasado por alias) es rara pero legal; la
          versión manual la corta antes de robarse a sí misma.
       2. ROBO: `texto_ = std::move(otro.texto_);`: el string destino
          absorbe el buffer del otro (O(1), sin copiar letras).
       3. DEJO AL OTRO VACIO: es consecuencia del robo del string
          (queda con garantía de vacío tras moverse: L07).
       4. `return *this;` en `T&`: habilita cadenas
          `a = b = std::move(c);`.

     Y el noexcept ES LA PARTE CRÍTICA: es la promesa que los
     contenedores consultan (L10: si el movimiento no es noexcept,
     std::vector copia al crecer).
   ============================================================================= */

#include <cstddef>
#include <iostream>
#include <utility>
#include "../../include/test_framework.hpp"

class Cola {
public:
    explicit Cola(std::string texto) : texto_(std::move(texto)) {}

    Cola(Cola&& otro) noexcept : texto_(std::move(otro.texto_)) { ++movimientos_; }

    // La asignación de movimiento completa: guard + robo + *this + noexcept.
    Cola& operator=(Cola&& otro) noexcept {
        if (this != &otro) {                 // guard anti-autoasignacion
            texto_ = std::move(otro.texto_); // robo (el otro queda vacio)
            ++movimientos_;
        }
        return *this;
    }

    ~Cola() {}

    Cola& operator=(const Cola& otro) {
        texto_ = otro.texto_;
        ++copias_;
        return *this;
    }

    Cola(const Cola& otro) : texto_(otro.texto_) { ++copias_; }

    const std::string& texto() const { return texto_; }
    std::size_t tamanio() const { return texto_.size(); }
    static int copias() { return copias_; }
    static int movimientos() { return movimientos_; }

private:
    std::string texto_;
    inline static int copias_ = 0;
    inline static int movimientos_ = 0;
};

TEST_CASE(AsignacionMueveElContenido) {
    Cola destino("vacio ");
    Cola origen("mensaje largo y pesado del nivel");
    destino = std::move(origen);
    ASSERT_EQ(destino.texto(), "mensaje largo y pesado del nivel");
    ASSERT_EQ(origen.tamanio(), std::size_t(0));   // garantia para std::string movido
    ASSERT_EQ(Cola::movimientos(), 1);
}

TEST_CASE(EsNoexcept) {
    ASSERT_EQ(noexcept(std::declval<Cola&>() = std::declval<Cola&&>()), true);
}

TEST_CASE(AutoasignacionPorAliasEsSegura) {
    Cola a("contenido importante");
    Cola& alias = a;
    a = std::move(alias);                          // this == &otro: no rompe
    ASSERT_EQ(a.texto(), "contenido importante");
}

TEST_CASE(NoSeCopio) {
    const int copiasAntes = Cola::copias();
    Cola destino("vacio ");
    Cola origen("otro mensaje");
    destino = std::move(origen);
    ASSERT_EQ(Cola::copias(), copiasAntes);
}

TEST_CASE(CadenaDeAsignaciones) {
    Cola a("x"), b("y"), c("mensaje final");
    a = b = std::move(c);                          // T& devuelto: habilita cadenas
    ASSERT_EQ(a.texto(), "mensaje final");
    ASSERT_EQ(b.texto(), "mensaje final");
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Solución 04...\n";
    return RUN_ALL_TESTS();
}
