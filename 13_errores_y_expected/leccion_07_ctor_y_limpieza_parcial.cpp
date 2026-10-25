/* =============================================================================
   MÓDULO 13 - LECCIÓN 07: excepciones en constructores y limpieza parcial
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Un constructor no puede "devolver error": su trabajo es construir
       un objeto. Para decidir "esto no se pudo construir" tiene dos
       caminos:
         a) un estado "zombie" (objeto construido pero inválido, con un
            flag `bool ok_` que todo el resto tiene que consultar): el
            módulo 05 ya mostró que envenena cada método.
         b) LANZAR: el diseño por defecto en C++ para invariantes que no
            se pueden cumplir. Si el ctor lanza, el objeto "no llegó a
            existir": no habrá dtor del MISMO objeto la primera vez.
       ¿Qué se limpia cuando el ctor lanza? La regla:
         - todos los miembros y bases YA COMPLETAMENTE CONSTRUIDOS se
           destruyen en orden inverso (como locales de una función);
         - el objeto que se estaba construyendo NO llama su propio
           dtor (no existe todavía, su dtor no corre).
       Eso es la "limpieza parcial": mitad a mitad, pero correcta. Si
       tus miembros son RAII (módulo 07), la limpieza es automática:
       el ctor lanza, los RAII ya construidos se destruyen, no hay
       fuga. Un recurso crudo adquirido ANTES del lanzamiento y guardado
       en un miembro crudo queda FUGADO si nadie lo libera: por eso el
       orden que PESA es "primero lo seguro", y si hace falta adquirir
       dos recursos en secta, que cada uno esté envuelto en RAII
       (o adquiridos en el orden correcto).
       Para datos de "validación": `throw std::invalid_argument(...)`;
       para violaciones de INVARIANTE por estado interno:
       `std::runtime_error` o una excepción propia (L09). El que llama
       ve un objeto que nunca existió: no hay checks de "ok_?" en el
       resto de la clase.
    2. ¿QUÉ PASA POR DEBAJO?
        El compilador genera el "cleanup parcial": para cada sub-objeto
        completamente construido, destrúyelo igual antes de propagar.
        El ctor lanzó y el "objeto" quedó en el estado intermedio: son
        las bases, a unos `new` de memoria: la memoria del objeto se
        libera (la memoria del objeto se libera con un delete inverso
        automático) y los sub-objetos construidos se destruyen., y los sub-objetos construidos se destruyen.
        Con herencia: bases y miembros se construyen en orden de la
        declaración (módulo 06), y un throw en el tercero destruye los
        dos primeros (orden exactamente inverso).
        Delegación de ctors también activa la limpieza parcial del
        ctor delegado (el módulo 05 L05).
    3. BUENAS PRÁCTICAS:
        - Preferí lanzar de un ctor YA esté el RAII: si todo miembro
          es RAII, nada se pierde.
        - Si un recurso crudo se adquiere con un new en el cuerpo del
          ctor, englobalo (unique_ptr local) o adquirilo al final, después
          de todo lo que puede lanzar: ese orden protege.
        - Jamás atrapes la excepción del ctor y "sigo igual": el objeto
          quedaría INVÁLIDO pero vivo (y el dtor correría sobre un
          estado a medias). Si atrapás en la función-try del ctor, el
          estándar obliga a re-lanzar (implícito al caer del catch).
        - Si no querés excepciones para un "input inválido del
          usuario", construye una FÁBRICA que devuelva
          std::expected (L10-L12): la decisión del canal (L02).
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace Leccion07 {

// Contador de vidas para VER la limpieza parcial en acción.
static int vivos = 0;
struct Miembro {
    std::string nombre_;
    explicit Miembro(std::string nombre) : nombre_(std::move(nombre)) {
        ++vivos;
        std::cout << " ctor " << nombre_ << " (vivos: " << vivos << ")\n";
    }
    ~Miembro() { --vivos; std::cout << " dtor " << nombre_ << " (vivos: " << vivos << ")\n"; }
};

// Un ctor que lanza: despejamos los recursos YA construidos.
class Sesion {
public:
    // función-try del ctor: el try cubre TAMBIÉN la lista de init:
    Sesion(int id) try : ventana_("ventana"), gestor_("gestor") {
        if (id <= 0) {
            throw std::invalid_argument("id de sesión inválido");
        }
        id_ = id;
        std::cout << " sesion " << id_ << " completa\n";
    } catch (const std::exception& e) {
        std::cout << " ctor lanzó: " << e.what()
                  << " (el objeto no existe; nada que rescatar)\n";
    }

    bool completa_ = false;
private:
    Miembro ventana_;
    Miembro gestor_;
    int id_ = 0;
};

// El miembro crudo y el recurso "no RAII": la fuga del día a día:
// (un recurso crudo adquirido ANTES de lanzar quedó SIN dueño).
class CargadorViejo {
public:
    explicit CargadorViejo(std::size_t n) : datos_(new int[n]{}) {  // (A)
        if (n == 0) {
            // (A) ya puso el recurso... y lanzamos: la fuga clásica.
            throw std::invalid_argument("n = 0");
        }
    }
    ~CargadorViejo() { delete[] datos_; }
private:
    int* datos_;
};

} // namespace Leccion07

int main() {
    std::cout << "== ctor que lanza: los miembros YA construidos se destruyen ==\n";
    std::cout << "  -- caso Sesion(1) OK --\n";
    {
        const Leccion07::Sesion s(1);
        (void)s;
    }
    std::cout << "  -- caso Sesion(0) lanza --\n";
    try {
        const Leccion07::Sesion s(0);
        (void)s;
    } catch (...) {
        std::cout << " (el catch interno relanzó: la atrapó este try de main)\n";
    }

    std::cout << "== ¿y los miembros crudos en la mitad? la fuga ==\n";
    try {
        Leccion07::CargadorViejo c(0);   // (A) ya allocó... y el cuerpo lanza
        (void)c;
    } catch (const std::invalid_argument&) {
        std::cout << " invalid_argument atrapada; el new mortal de (A) está fugado\n";
        std::cout << " (mirá la solución con unique_ptr en la del ejercicio 04)\n";
    }
    std::cout << "vivos finales: " << Leccion07::vivos << '\n';
    return 0;
}
