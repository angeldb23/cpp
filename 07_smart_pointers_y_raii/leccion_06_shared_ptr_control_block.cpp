/* =============================================================================
   MÓDULO 07 - LECCIÓN 06: std::shared_ptr: propiedad compartida y control block
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      unique_ptr dice "yo soy el único dueño". shared_ptr dice "somos varios
      dueños: el recurso vive MIENTRAS ALGUNO de nosotros lo necesite".

      a) Es COPIABLE: copiar un shared_ptr es invitar a otro dueño. Por eso
         se puede pasar por valor, meterlo en miembros, en varios
         contenedores, en varias escenas.

      b) El contador (use_count()): cada vez que nace un dueño, +1; cada vez
         que muere, -1. Cuando llega a 0, el ÚLTIMO destructor hace el
         delete. Sigue siendo determinista como RAII: no hay recolector
        que pase cada tanto; el recurso muere exactamente en el
         destructor del último dueño.

      c) No hay "dueño principal": todos los dueños son iguales entre sí.
         El que creó el objeto con make_shared no tiene más autoridad que
         cualquiera que copió.

      d) El BLOQUE DE CONTROL: para que N punteros coordinen, el heap guarda
         UN bloque compartido que incluye: contador fuerte (dueños vivos),
         contador débil (observadores: L08-L09) y las instrucciones de
         liberación (qué destructor ejecutar). Cada shared_ptr guarda DOS
         punteros: uno al objeto y otro al bloque de control COMÚN.

      e) Los contadores son ATÓMICOS: dos hilos copiando/destruyendo SUS
         PROPIOS shared_ptr al mismo recurso no corrompen el contador (esa
         sincronización sale gratis). Sincronizar el OBJETO compartido (por
         ejemplo, si dos escenas lo modifican) es problema del objeto: tema
         del módulo 14.

      f) ¿Cuándo usar shared_ptr en vez de unique_ptr? Cuando DESIGNAR un
         dueño único es una pelea artificial: caché de recursos (una textura
         por nombre compartida por todas las escenas), nodos de escena a los
         que varios sistemas necesitan referirse, resultados con muchos
         consumidores. La regla práctica de este curso:
            1) intentá unique_ptr primero;
            2) si el diseño exige varios dueños legítimos, shared_ptr;
            3) NUNCA como default "por las dudas".
         Y cuando un dueño necesita SOLO OBSERVAR, ni unique ni shared:
         weak_ptr (L08-L09).

   2. ¿QUÉ PASA POR DEBAJO?
      shared_ptr<T> ocupa DOS punteros (16 bytes en 64 bits): el T* y el
      ControlBlock*. Copiarlo copia los dos campos e INCREMENTA (atómicamente)
      el contador fuerte del bloque. Destruirlo lo decrementa: si llega a 0,
      ejecuta el destructor del objeto y libera el bloque de control.
      Copias más caras que unique_ptr: copiar ES una operación con efecto
      (incremento atómico), no un byte-copy. La indirección doble (objeto ->
      bloque) también paga en caché: por eso make_shared (L07) ahorra una
      de las dos allocaciones.

      Un detalle central: dos shared_ptr que apuntan al MISMO objeto pero
      nacieron independientes (por ejemplo, dos veces con ptr crudo: ver
      L05 notas) tienen DOS bloques de control: contadores desincronizados
      y doble delete. El control block es único SOLO si los shared_ptr
      provienen del mismo origen (copia/copien de copias).

   3. ERRORES COMUNES
      - shared_ptr por defecto "por miedo": cada copia son instrucciones
        atómicas y el bloque de control pesa (y ensucia el caché).
      - shared_ptr<T> p(new T(...)) habiendo make_shared: dos allocaciones
        para lo que toma una (L07).
      - Crear dos shared_ptr desde el MISMO puntero crudo: T* ya mentía:
        dos control blocks: doble delete garantizado. El origen del
        shared_ptr SIEMPRE es make_shared o la copia de otro shared.
      - Resetear/copiar en MITAD de código de múltiples hilos si el MISMO
        objeto shared_ptr es accedido por varios: los contadores son
        atómicos, el acceso concurrente a LA VARIABLE shared_ptr no: módulo
        14 (atomic<shared_ptr> / mutex).
      - Creer que shared_ptr protege el OBJETO: protege la EXISTENCIA. Si
        dos dueños llaman un método que modifica, la carrera es de datos.

   4. DEMO: contar dueños en todos los escenarios
 */
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Textura {
private:
    std::string archivo_;

public:
    explicit Textura(std::string a) : archivo_(std::move(a)) {
        std::cout << "    [ctor ] " << archivo_ << "\n";
    }
    ~Textura() { std::cout << "    [dtor ] " << archivo_ << "\n"; }

    void dibujar() const { std::cout << "    dibujo " << archivo_ << "\n"; }
    const std::string& nombre() const { return archivo_; }
};

// shared_ptr como miembro: la Escena es OTRO dueño del mismo recurso.
class Escena {
private:
    std::shared_ptr<Textura> fondo_;
    const char* nombre_;

public:
    Escena(const char* nombre, std::shared_ptr<Textura> tex)
        : fondo_(std::move(tex)), nombre_(nombre) {          // adopta acá
        std::cout << "    [" << nombre_ << "] use_count tras adoptar: "
                  << fondo_.use_count() << "\n";
    }
    ~Escena() { std::cout << "    [dtor ] " << nombre_ << "\n"; }
    void render() const { fondo_->dibujar(); }
};

int main() {
    std::cout << "=== 1. EL CONTADOR SUBE Y BAJA CON LOS DUEÑOS ===\n";
    std::shared_ptr<Textura> base;
    {
        base = std::make_shared<Textura>("bosque.png");
        std::cout << "    use_count = " << base.use_count() << " (1 dueño)\n";

        std::shared_ptr<Textura> copia = base;             // +1: 2 dueños
        std::cout << "    use_count = " << base.use_count() << " (2 dueños)\n";
        copia->dibujar();
    }   // 'copia' muere: -1. NO se destruye la textura: 'base' sigue.
    std::cout << "    use_count = " << base.use_count() << " (quedó 1)\n";
    std::cout << "    (la textura vive: aún hay un dueño)\n\n";

    std::cout << "=== 2. DUEÑOS EN DISTINTAS CAPAS ===\n";
    {
        std::vector<std::shared_ptr<Textura>> galeria;     // dueño colectivo
        galeria.push_back(std::make_shared<Textura>("montana.png"));
        galeria.push_back(std::make_shared<Textura>("cueva.png"));
        std::cout << "    use_count montana = " << galeria[0].use_count() << "\n";
        {
            std::shared_ptr<Textura> prestada = galeria[1]; // +1 dueño
            std::cout << "    use_count cueva   = " << galeria[1].use_count() << "\n";
        }
        std::cout << "    use_count cueva 2v = "
                  << galeria[1].use_count() << " (bajó al salir)\n";
    }   // cada texture queda con 1 dueño (el vector): [dtor] de ambas.

    std::cout << "\n=== 3. EL MISMO RECURSO EN DOS ESCENAS ===\n";
    {
        std::shared_ptr<Textura> cielo = std::make_shared<Textura>("cielo.png");
        std::cout << "    use_count inicial: " << cielo.use_count() << "\n";
        Escena dia("dia", cielo);
        Escena atardecer("atardecer", cielo);
        dia.render();
        atardecer.render();
        std::cout << "    use_count final: "
                  << cielo.use_count() << " (3 dueños)\n";
    }   // mueren los dueños en reversa: 'atardecer', 'dia', 'cielo'.
        // Recién con el último (conteo en 0) muere la textura.
    std::cout << "\nlisto: mismo recurso sincronizado por conteo.\n";
    std::cout << "próximo insight: por qué make_shared y no new: L07.\n";
    return 0;
}
