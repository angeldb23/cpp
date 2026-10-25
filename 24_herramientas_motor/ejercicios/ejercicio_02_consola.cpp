/* =============================================================================
   MÓDULO 24 - EJERCICIO 02: Consola con ring buffer y filtros
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 09 (logging y consola).

   CONSIGNA
   --------
   Completá `Consola`:

   1. `log(nivel, categoria, texto)` agrega una entrada con número de secuencia
      creciente. Si supera la capacidad, descarta la MÁS VIEJA (ring buffer).
   2. `visibles(busqueda)` devuelve, en orden cronológico, las entradas que:
        - nivel >= `minimo_`,
        - categoría == `categoria_` (si `categoria_` no está vacía),
        - contienen `busqueda` en texto O categoría, sin distinguir mayúsculas
          (búsqueda vacía = todas).
   3. `exportar()` devuelve todas las entradas (sin filtros) con el formato:
        `[N][categoria] texto\n`
      donde N es T, D, I, W o E (Trace..Error).
   ============================================================================= */
#include "../include/gui_util.hpp"
#include "../../include/test_framework.hpp"

#include <cstdint>
#include <deque>
#include <string>
#include <string_view>
#include <vector>

enum class Nivel : int { Trace = 0, Debug = 1, Info = 2, Warn = 3, Error = 4 };

inline const char* nombreNivel(Nivel n) {
    switch (n) {
        case Nivel::Trace: return "T";
        case Nivel::Debug: return "D";
        case Nivel::Info: return "I";
        case Nivel::Warn: return "W";
        case Nivel::Error: return "E";
    }
    return "?";
}

struct Entrada {
    Nivel nivel = Nivel::Info;
    std::string categoria;
    std::string texto;
    std::uint64_t secuencia = 0;
};

class Consola {
public:
    explicit Consola(std::size_t capacidad) : capacidad_(capacidad) {}

    void log(Nivel n, std::string categoria, std::string texto) {
        // TODO
        (void)n;
        (void)categoria;
        (void)texto;
    }

    void setNivelMinimo(Nivel n) { minimo_ = n; }
    void setCategoria(std::string c) { categoria_ = std::move(c); }

    std::vector<Entrada> visibles(std::string_view busqueda) const {
        // TODO
        (void)busqueda;
        return {};
    }

    std::string exportar() const {
        // TODO
        return {};
    }

    std::size_t size() const { return entradas_.size(); }
    std::size_t capacidad() const { return capacidad_; }

private:
    std::size_t capacidad_ = 0;
    std::deque<Entrada> entradas_;
    Nivel minimo_ = Nivel::Trace;
    std::string categoria_;
    std::uint64_t secuencia_ = 0;
};

// ============================================================================
// TESTS (no tocar)
// ============================================================================
TEST_CASE(RingBufferDescartaLoViejo) {
    Consola c(3);
    c.log(Nivel::Info, "a", "uno");
    c.log(Nivel::Info, "a", "dos");
    c.log(Nivel::Info, "a", "tres");
    c.log(Nivel::Info, "a", "cuatro");
    ASSERT_EQ(c.size(), 3u);
    const auto v = c.visibles("");
    ASSERT_EQ(v.size(), 3u);
    ASSERT_EQ(v[0].texto, std::string("dos"));
    ASSERT_EQ(v[2].texto, std::string("cuatro"));
    ASSERT_EQ(v[2].secuencia, 4u);
}

TEST_CASE(FiltroPorNivel) {
    Consola c(8);
    c.log(Nivel::Debug, "x", "debug");
    c.log(Nivel::Warn, "x", "warn");
    c.log(Nivel::Error, "x", "error");
    c.setNivelMinimo(Nivel::Warn);
    const auto v = c.visibles("");
    ASSERT_EQ(v.size(), 2u);
    ASSERT_EQ(static_cast<int>(v[0].nivel), static_cast<int>(Nivel::Warn));
    ASSERT_EQ(static_cast<int>(v[1].nivel), static_cast<int>(Nivel::Error));
}

TEST_CASE(FiltroPorCategoria) {
    Consola c(8);
    c.log(Nivel::Info, "render", "a");
    c.log(Nivel::Info, "audio", "b");
    c.setCategoria("audio");
    const auto v = c.visibles("");
    ASSERT_EQ(v.size(), 1u);
    ASSERT_EQ(v[0].categoria, std::string("audio"));
}

TEST_CASE(BusquedaSinMayusculas) {
    Consola c(8);
    c.log(Nivel::Error, "render", "Shaders no compilan");
    c.log(Nivel::Info, "render", "ok");
    const auto v = c.visibles("SHADER");
    ASSERT_EQ(v.size(), 1u);
    ASSERT_EQ(static_cast<int>(v[0].nivel), static_cast<int>(Nivel::Error));
}

TEST_CASE(BusquedaTambienMiraCategoria) {
    Consola c(8);
    c.log(Nivel::Info, "fisica", "42 pares");
    c.log(Nivel::Info, "audio", "volumen 0.8");
    const auto v = c.visibles("FISICA");
    ASSERT_EQ(v.size(), 1u);
}

TEST_CASE(ExportFormato) {
    Consola c(8);
    c.log(Nivel::Error, "render", "boom");
    c.log(Nivel::Info, "script", "ok");
    ASSERT_EQ(c.exportar(), std::string("[E][render] boom\n[I][script] ok\n"));
}

TEST_CASE(ExportIgnoraFiltros) {
    Consola c(8);
    c.log(Nivel::Debug, "a", "uno");
    c.log(Nivel::Error, "b", "dos");
    c.setNivelMinimo(Nivel::Error);
    c.setCategoria("b");
    ASSERT_EQ(c.exportar(), std::string("[D][a] uno\n[E][b] dos\n"));
}

int main() {
    std::cout << "Ejercicio 02 (consola)...\n";
    return RUN_ALL_TESTS();
}
