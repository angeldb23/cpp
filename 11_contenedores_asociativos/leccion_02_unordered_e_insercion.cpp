/* =============================================================================
   MÓDULO 11 - LECCIÓN 02: unordered_map e inserción fina (insert / emplace /
   try_emplace)
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       std::map es un árbol balanceado: toda búsqueda cuesta O(log n) SIEMPRE.
       std::unordered_map<K, V> cambia el mecanismo: con un HASH de la clave
       encuentra la posición en O(1) EN PROMEDIO (peor caso O(n), si todas tus
       claves colisionan). Lo que paga: los elementos NO están ordenados y no
       hay recorridos por rango de claves.

       ¿Cuándo elegir cuál?
         - map: si te importa el orden de las claves (rangos, mínimo/máximo,
           salida ordenada), si querés un caso peor predecible (o claves cuyo
           hash puede salir pésimo).
         - unordered_map: si el orden no te sirve y querés la búsqueda más
           rápida. Es el default para las tablas de entidad por ID en juegos.

       El otro tema de esta lección es INSERTAR BIEN. Hay cuatro caminos y no
       son equivalentes:
         - operator[key] = v: si la clave ya estaba, SOBREESCRIBE el valor en
           silencio. Nunca te dice si creó o reescribió, no te devuelve nada
           de información.
         - insert(valor): si la clave YA estaba, NO cambia nada (protección).
           Devuelve un par<iterador, bool> cuyo bool dice si insertó.
         - emplace(args...): igual que insert, pero CONSTRUYE el par ahí
           adentro con los args. Ojo: aunque la clave ya esté, los argumentos
           se usan para construir el par de todos modos, y ese trabajo no se
           recupera.
         - try_emplace(key, args...): la variante lúcida: los args SOLO se
           pasan al constructor del valor si la clave NO estaba. Es "insertar
           si falta, no tocar lo existente, y no pagar construcción en balde".

       Regla de oro: si al insertar necesitás saber si la clave estaba,
       NADA de operator[]: usá insert o try_emplace y mirá el bool del par
       devuelto.
   2. ¿QUÉ PASA POR DEBAJO?
       Un unordered_map es una TABLA HASH de buckets: hash(k) % n_buckets
       elige el cubo donde vive la entrada. El factor de carga decide si la
       tabla agranda (rehash: re-parte TODA la tabla). Un rehash invalida los
       iteradores; en un map (L01) ninguna inserción los invalidaba: esa es
       LA diferencia de contrato entre ambos.

       Para usar un tipo como clave de unordered_map hacen falta DOS cosas:
         operator== (para decidir si dos claves son la misma, dentro del
         bucket) + std::hash<Clave> (para elegir el bucket). Con un tipo
         propio tenés que proveer AMBOS: es el tema entero de la lección 06.

       La diferencia insert vs emplace vs try_emplace es de plumbing:
         insert construye el par<clave, valor> de tu lado y lo mueve adentro;
         emplace construye el par directamente dentro del contenedor;
         try_emplace es perezoso: los argumentos del valor SE QUEDAN EN TU
         lado si la clave ya estaba, así que ese valor a medio construir
         nunca se llega a pagar.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - "Consultar" con operator[]: buscar con m[k] INSERTA si faltaba. Para
         preguntar: m.contains, m.count o m.find (igual que en el map).
       - Ignorar el bool del insert: la clave ya estaba y tu nuevo valor se
         descartó sin que te enteres.
       - Iterar un unordered_map y esperar un orden: no hay orden prometido;
         si tu lógica DEPENDE del orden, usa map.
       - Guardar iteradores de unordered_map alrededor de más inserciones:
         un rehash los mata; volvé a buscar si te hace falta.
       - emplace cuando el valor es caro de construir y la clave suele estar
         ya presente: preferí try_emplace.
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/container/unordered_map
       https://en.cppreference.com/w/cpp/container/unordered_map/try_emplace
       https://en.cppreference.com/w/cpp/utility/functional/hash
   ============================================================================= */
#include <iostream>
#include <unordered_map>

int main() {
    // --- 1. Mismo problema, dos mecanismos ---------------------------------------
    std::unordered_map<std::string, int> porId{
        {"danio_postrado", 6}, {"danio_lateral", 8}, {"danio_pleno", 12} };

    std::cout << "unordered_map (el orden de la salida NO está garantizado):\n";
    for (const auto& [k, v] : porId) std::cout << "  " << k << " -> " << v << '\n';

    // --- 2. insert NO sobreescribe; operator[] SÍ ---------------------------------
    porId["danio_pleno"] = 99;    // reescribe en silencio
    std::cout << "\noperator[\"danio_pleno\"] = 99 -> queda: "
              << porId.at("danio_pleno") << '\n';

    const auto r1 = porId.insert({"danio_pleno", 11});
    std::cout << "insert de clave YA existente: bool = " << r1.second
              << " ; danio_pleno sigue: " << porId.at("danio_pleno") << '\n';

    const auto r2 = porId.insert({"danio_cargado", 40});
    std::cout << "insert de clave NUEVA: bool = " << r2.second
              << " ; su valor: " << r2.first->second << '\n';

    // --- 3. emplace y try_emplace: no construir de más ----------------------------
    const auto r3 = porId.emplace("danio_lateral", 8);
    std::cout << "\nemplace de clave YA existente: bool = " << r3.second
              << " ; danio_lateral sigue: " << porId.at("danio_lateral") << '\n';

    const auto r4 = porId.try_emplace("danio_postrado", 7);
    std::cout << "try_emplace clave YA existente: bool = " << r4.second
              << " ; queda 6: " << porId.at("danio_postrado") << '\n';

    const auto r5 = porId.try_emplace("danio_rayo", 42);
    std::cout << "try_emplace clave NUEVA: bool = " << r5.second
              << " ; su valor: " << porId.at("danio_rayo") << '\n';

    // --- 4. try_emplace como "insertar si falta": sin reescrituras accidentales ---
    std::unordered_map<std::string, std::string> notas;
    notas.try_emplace("mago", "bola de hielo: 20");
    notas.try_emplace("mago", "bola de hielo: 100");    // ignorado: ya estaba
    std::cout << "\nnotas[mago] tras doble try_emplace: " << notas.at("mago") << "\n";
    return 0;
}
