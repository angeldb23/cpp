/* =============================================================================
   MÓDULO 08 - SOLUCIÓN 09: Parámetro sink: por valor y mover adentro
   =============================================================================
   EXPLICACIÓN:
     La regla de firma de la L11:

     - La función VA A GUARDAR el dato (es su dueño nuevo) -> firma
       POR VALOR. El robo explícito queda en el CALLER (que escribe
       std::move si cede) y ADENTRO de la función el parámetro es
       una variable local más: se puede mover a su lugar final con
       OTRA llamada movida... o no: sabiendo que push_back(std::move)
       roba.
       Bonus: si el caller pasa un rvalue (temporal), el parámetro
       ya se mueve implícito (L02: la sobrecarga T&& no existe ACÁ,
       pero el valor-parametrado se construye moviendo).

     - La función SOLO LEE -> const&: una sola lectura, la copia se
       hace donde es inevitable (push_back), y el ORIGEN no cambia.

     Nota de rendimiento: `archivar(pesado)` mueve UNA sola vez
     (llamada de transferencia), `chismear(original)` copia una vez.
     Los tests miden EXACTAMENTE eso con el estado del orígen.
   ============================================================================= */

#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

class Carpeta {
public:
    // Sink: por valor + robo adentro.
    int archivar(std::string nombre) {
        nombres_.push_back(std::move(nombre));     // roba al parámetro local
        return static_cast<int>(nombres_.size());
    }

    // Observadora que archiva: const& + copia (destinatario: push_back).
    int chismear(const std::string& nombre) {
        nombres_.push_back(nombre);                // copia: el origen ENTero
        return static_cast<int>(nombres_.size());
    }

    const std::string& etiqueta(std::size_t i) const { return nombres_[i]; }
    std::size_t cantidad() const { return nombres_.size(); }

private:
    std::vector<std::string> nombres_;
};

TEST_CASE(ArchivarRobaElArgumento) {
    Carpeta carpeta;
    std::string pesado = "/niveles/08/nombre_muy_largo_del_boss_final.png";
    const int n = carpeta.archivar(std::move(pesado));   // CESION del caller
    ASSERT_EQ(n, 1);
    ASSERT_EQ(carpeta.etiqueta(0), "/niveles/08/nombre_muy_largo_del_boss_final.png");
    ASSERT_EQ(pesado.size(), std::size_t(0));
}

TEST_CASE(ArchivarAceptaTemporales) {
    Carpeta carpeta;
    ASSERT_EQ(carpeta.archivar(std::string("temporal directo")), 1);
    ASSERT_EQ(carpeta.archivar("literal"), 2);
}

TEST_CASE(ChismearCopiaSinTocarElOrigen) {
    Carpeta carpeta;
    std::string original = "solo la miro";
    const int n = carpeta.chismear(original);
    ASSERT_EQ(n, 1);
    ASSERT_EQ(original, "solo la miro");
    ASSERT_EQ(carpeta.etiqueta(0), "solo la miro");
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Solución 09...\n";
    return RUN_ALL_TESTS();
}
