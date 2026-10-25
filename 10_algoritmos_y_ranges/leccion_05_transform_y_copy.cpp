/* =============================================================================
   MÓDULO 10 - LECCIÓN 05: modificantes I: transform, copy/copy_if y std::move
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Los algoritmos MODIFICANTES toman la secuencia y producen otra (o la
       cambian en el lugar). Los dos pilares de hoy:

       `std::transform(prin, fin, salida, op)` — versión UNARIA: aplica `op`
       a cada elemento y escribe el resultado a partir de `salida`.
         - in-place: el destino puede ser el MISMO rango (op sobre sí mismo).
         - a otro contenedor: `salida` es el begin() del destino, que TIENE
           que tener espacio ya reservado para tantos elementos como entre
           en el rango (los algoritmos NUNCA hacen push_back: escriben sobre
           espacio que ya existe).
       `std::transform(p1, fin1, p2, salida, op)` — BINARIA: recorre DOS
       secuencias en paralelo y aplica `op(a, b)` sobre cada par. Sirve para
       combinar elemento a elemento (sumar stats, restar armadura, etc.).

       `std::copy(prin, fin, salida)` y `std::copy_if(..., pred)`: copian
       los elementos (los que cumplen pred) hacia `salida`. Igual que
       transform, no agrandan nada. El patrón del módulo cuando el destino
       es un vector vacío:
           1. salida.resize(entrada.size());            // lugar de sobra
           2. auto nuevo_fin = std::copy_if(prin, fin, salida.begin(), pred);
           3. salida.resize(distance(salida.begin(), nuevo_fin));
         El copy devuelve el iterador al FINAL del rango copiado: del resto
         (lo que no se escribió) te deshacés con resize. El caso "copiar a
         un contenedor que todavía no existe" se cierra en la L18 con
         back_inserter; el patrón resize+copy resuelve lo mismo sin nada
         nuevo, y es lo que te vas a encontrar en código viejo.

       `std::move(prin, fin, salida)` — el ALGORITMO move: mueve cada
       ELEMENTO del rango al destino (invoca el constructor de movimiento,
       módulo 08). No confundir con el std::move de un solo objeto: esta es
       la versión masiva para trasladar un rango barato (strings, vectors).
   2. ¿QUÉ PASA POR DEBAJO?
       transform y copy son asignaciones elemento a elemento: "salida[i] =
       op(entrada[i])" / "salida[i] = entrada[i]". Todo el trabajo real lo
       hace `op`: como el algoritmo es un template, tu lambda concreta se
       inlinea tal como en la L04. El algoritmo `std::move` hace, en cada
       posición, el traspaso del módulo 08: le transfiere el contenido al
       destino y el origen queda moved-from (válido pero sin datos:
       quedale la promesa de no leerlo como si tuviera datos).
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - Pasar como destino un vector vacío: el algoritmo escribe donde NO
         hay elementos: comportamiento indefinido. El resize de arriba es
         obligatorio (o back_inserter, L18).
       - transform in-place es válido PERO cuidado si `op` depende de otros
         elementos de la MISMA secuencia: el algoritmo escribe el resultado
         de a uno y puede pisar datos que todavía no leíste.
       - copy_if con resize de más está bien: el resize final al iterador
         devuelto es el paso que achica al tamaño real.
       - Si necesitás construir el destino desde cero con cada resultado,
         lo cómodo es copy + back_inserter (L18) o un bucle a mano.
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/algorithm/transform
       https://en.cppreference.com/w/cpp/algorithm/copy
       https://en.cppreference.com/w/cpp/algorithm/move
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

int main() {
    // --- 1. transform unario in-place ----------------------------------------
    std::vector<int> niveles{3, 5, 8, 10};
    std::transform(niveles.begin(), niveles.end(), niveles.begin(),
                   [](int n) { return n * 10; });        // nivel -> HP base
    std::cout << "HP por nivel: ";
    for (int hp : niveles) std::cout << hp << ' ';
    std::cout << '\n';

    // --- 2. transform binario: dos secuencias en paralelo --------------------
    const std::vector<int> hpBase{30, 45, 80};
    const std::vector<int> armadura{10, 0, 25};
    std::vector<int> hpFinal(hpBase.size());             // lugar preciso
    std::transform(hpBase.begin(), hpBase.end(), armadura.begin(),
                   hpFinal.begin(),
                   [](int base, int arm) { return base - arm; });
    std::cout << "HP final: ";
    for (int hp : hpFinal) std::cout << hp << ' ';
    std::cout << '\n';

    // --- 3. copy_if: el patrón resize + copy ---------------------------------
    const std::vector<int> candidatos{30, 45, 80, 5, 92};
    std::vector<int> invitados;
    invitados.resize(candidatos.size());                 // 1) lugar de sobra
    const auto nuevo_fin = std::copy_if(                // 2) copio los útiles
        candidatos.begin(), candidatos.end(), invitados.begin(),
        [](int hp) { return hp > 40; });
    invitados.resize(static_cast<std::size_t>(           // 3) achico al útil
        std::distance(invitados.begin(), nuevo_fin)));
    std::cout << "los de HP > 40: ";
    for (int hp : invitados) std::cout << hp << ' ';
    std::cout << '\n';

    // --- 4. std::move (algoritmo): mudar un rango barato ---------------------
    std::vector<std::string> nombres{"Lumen", "Quasar", "Vulcano"};
    std::vector<std::string> archivados(3);              // lugar preciso
    std::move(nombres.begin(), nombres.end(), archivados.begin());
    std::cout << "archivados: ";
    for (const std::string& s : archivados) std::cout << s << ' ';
    std::cout << "| origen queda: \"" << nombres[0]
              << "\" (moved-from: valido pero sin datos)\n";

    return 0;
}
