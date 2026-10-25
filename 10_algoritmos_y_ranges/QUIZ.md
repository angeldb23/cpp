# Quiz — Módulo 10: Algoritmos STL, Iteradores, Ranges y Views

Respondé primero sin mirar las respuestas del final. Cada pregunta tiene UNA opción correcta.

---

**1. `std::begin(v)` sobre un array nativo `int a[5]`…**

- a) No compila: los arrays no tienen `.begin()`
- b) Devuelve un puntero al primer elemento
- c) Devuelve `nullptr`
- d) Convierte el array en un `vector`

**2. Desplazás 12 posiciones un `std::vector<int>::iterator`. ¿Qué categoría tiene este iterador?**

- a) Solo input, porque avanza de a uno
- b) Bidireccional
- c) Random-access: `it + 12` en O(1)
- d) Ninguna: `vector` no tiene iteradores

**3. `std::find` devuelve el iterador donde está el elemento, o…**

- a) `nullptr`
- b) El primero de la secuencia
- c) El `end` que le pasaste
- d) Un índice negativo

**4. ¿Por qué `find_if` puede ser mejor que un `count_if(...) > 0` para responder "¿existe alguno"?**

- a) `count_if` solo funciona con números
- b) `find_if` corta al primer hallazgo; `count_if` siempre recorre TODA la secuencia
- c) Es idéntico: el compilador optimiza igual
- d) `count_if` no acepta lambdas

**5. ¿Qué hace `all_of` sobre una secuencia VACÍA?**

- a) Devuelve `false`: no hay nada que cumpla
- b) Es comportamiento indefinido
- c) Devuelve `true`: la vacuidad es trivialmente cierta (cuantificador universal)
- d) Error de compilación: necesita al menos un elemento

**6. `std::transform(src.begin(), src.end(), dst.begin(), f)` con `dst` más corto que `src`…**

- a) Redimensiona `dst` automáticamente
- b) Escribe fuera de rango: comportamiento indefinido
- c) Ignora los elementos que no caben
- d) Devuelve error

**7. Después de `v.erase(std::remove(v.begin(), v.end(), x), v.end())`, ¿qué pasó con `v`?**

- a) Nada: `remove` borra de una
- b) `x` desapareció y `v.size()` bajó exactamente lo que se borró
- c) Quedaron "agujeros" con elementos inválidos
- d) `v` se vació entero

**8. Ordenás `{"Mora", "zorro", "Ana"}` con `std::sort` y comparador `<` de `string`. ¿Resultado?**

- a) `{"Ana", "Mora", "zorro"}`: alfabético
- b) `{"Ana", "zorro", "Mora"}`: las mayúsculas van siempre al final
- c) Orden indefinido entre llamadas
- d) No compila

**9. Tenés un vector YA ordenado con 1.000.000 de elementos. `std::binary_search` encuentra un valor en…**

- a) Recorrido lineal: hasta 1.000.000 comparaciones
- b) ~20 comparaciones: corta el rango por la mitad cada paso
- c) Exactamente 1.000.000 siempre
- d) Dos pasadas completas

**10. ¿Qué devuelve `std::accumulate(v.begin(), v.end(), 0)`?**

- a) El máximo
- b) El primer elemento como semilla
- c) La suma partiendo de `0` como acumulador inicial
- d) Un iterador a la suma

**11. ¿Qué no garantiza `std::partition` (a diferencia de `stable_partition`)?**

- a) Que los que cumplan el predicado queden adelante
- b) Que los que NO cumplan queden atrás
- c) El ORDEN RELATIVO dentro de cada grupo
- d) Que devuelva el punto de corte

**12. Una lambda captura `contador` por valor con `mutable`. ¿Qué pasa si copiás la lambda a otra variable?**

- a) La copia comparte el estado con la original
- b) La copia arranca con el valor actual capturado y su estado es independiente
- c) Error de compilación: las lambdas no se copian
- d) El `mutable` se pierde en la copia

**13. ¿Para qué sirve una lambda genérica `[]<typename T>(T x) { … }`?**

- a) Para capturar por template
- b) El parámetro deduce su tipo por llamada (como un template de función)
- c) Solo acepta tipos enteros
- d) Para que la lambda sea `constexpr` obligatoriamente

**14. ¿Cuál es la promesa central de un view de ranges?**

- a) Es más rápido que cualquier algoritmo clásico
- b) No copia ni recorre nada al ARMARSE: trabaja cuando alguien lo ITERA
- c) Siempre devuelve un `vector`
- d) Garantiza que el origen no cambie

**15. `puntajes | views::filter(p) | views::transform(f) | views::take(3)`: ¿en qué orden se evalúa un elemento?**

- a) Primero take saca 3, luego filter filtra todo, luego transform
- b) Cada elemento atraviesa el pipeline entero de a uno: se pide, se filtra, se transforma, hasta llenar los 3 del take
- c) Se ejecuta todo sobre una copia del vector primero
- d) El orden depende del compilador

**16. Iterás materializando el pipeline anterior con GCC: el filter evaluó 6 veces sobre `{10, 20, 30, 40, 50, 60}` con umbral 20 y take(3). ¿Por qué 6 si solo hay 3 pasantes?**

- a) Errores del compilador: siempre hay 3 evaluaciones de más
- b) Dos fallan (10, 20), tres pasan y llenan el take, y el take pide UN elemento de más (probe del 60) que se evalúa y descarta
- c) El filter corre dos veces por elemento
- d) El transform lo evaluó tres veces extra

**17. `std::ranges::sort(datos, std::greater<int>{}, &Pet::hp)` llama al comparador con…**

- a) `Pet` completos, comparando todo el struct
- b) El resultado de proyectar sobre `hp` (con `greater`: hp DESC)
- c) Los índices de los elementos
- d) Punteros a los elementos

**18. `std::copy(std::istream_iterator<int>{in}, {}, std::back_inserter(out))` sobre "10 20 30" produce…**

- a) Un error: no se puede copiar de un stream
- b) El `vector{10, 20, 30}`: el iterador lee con `>>` hasta agotar el stream y el back_inserter apila sin tamaño previo
- c) El `vector{10}`: solo lee la primera línea
- d) Tres strings

---

## Respuestas

1. **b)** `std::begin/end` son la interfaz genérica: para arrays nativos devuelven puntero (`int*`); el `.begin()` de método solo existe en contenedores.
2. **c)** Los iteradores de `vector` son random-access: sumar/restar enteros es O(1). Las categorías (input → forward → bidirectional → random-access) definen QUÉ algoritmos podés usar.
3. **c)** El contrato: devuelve el `last` que recibiste cuando no encuentra. Ese `end` es la señal de "no estaba", no un error.
4. **b)** `find_if` corta en la primera coincidencia; `count_if` tiene que terminar la cuenta aunque ya la sepa. Una pregunta de existencia no pide un conteo.
5. **c)** Igual que "todos los cisnes de un estanque vacío son blancos": no hay contraejemplo, da `true`. `none_of` también da `true` ahí; `any_of`, `false`.
6. **b)** `transform` no agranda el destino: escribe sobre lo que le diste. Si no entra, es UB. Usa `back_inserter` para crecer.
7. **b)** Es el patrón remove-erase (L07): `remove` compacta hacia adelante SIN borrar y devuelve el nuevo fin lógico; `erase` recorta el sobrante hasta el tamaño real.
8. **a)** El `<` de `char` compara por código: las mayúsculas ANTES que las minúsculas (`'A'` < `'a'`). Y con `string` queda `{"Ana", "Mora", "zorro"}`: si querés ordenar "humano", compará en minúsculas o con un comparador propio.
9. **b)** `binary_search`/`lower_bound` parten el rango por la mitad: log2(1.000.000) ≈ 20. La condición de sorted-order es lo que compra la O(log n).
10. **c)** Acumula partiendo de `init = 0`:  `((0 + v0) + v1) + ...`. Cambiar la semilla cambia el tipo y el punto de partida (numerics de la L10).
11. **c)** `partition` solo separa por predicado; el orden adentro de cada lado puede barajarse. Eso es `stable_partition` (y paga caro si no hay memoria extra).
12. **b)** La lambda es un objeto: capturar por valor con `mutable` hace el estado modificable y cada COPIA tiene el suyo, independiente (por eso cada contador de la L12 es suyo).
13. **b)** Lambda genérica con parámetro template explícito: `T` se deduce por cada llamada, como un template de función embebido, y sirve para lo mismo (algoritmos genéricos).
14. **b)** La L14: armar el view es armar una receta (O(1), sin copias); la iteración es la cocción. Materializá solo si lo necesitás (y una vez).
15. **b)** Lazy pull del L15-16: el take tira del transform, que tira del filter, que tira del vector. Elemento por elemento, no fase por fase.
16. **b)** Con umbral 20: fallan 10 y 20 (2), pasan 30 40 50 y llenan el take (3 más = 5), y el take todavía pide UN elemento de más para ver si queda algo: el 60 se evalúa y se descarta (6). Si el fuente se agota antes, ese probe no existe (por eso {10,35,40,50} da 4 y {1,20,30,99,99} da 5).
17. **b)** La proyección se aplica ANTES de comparar: el comparador `greater` ve solo el campo `hp`. Comp y proyección se combinan en la MISMA llamada (L17): menos lambdas pegamento.
18. **b)** El patrón "copy de streams" (L18): leer un rango no conocido de antemano con iteradores es leer hasta fin de stream y apilar con `back_inserter`, sin bucles ni `push_back` manual.
