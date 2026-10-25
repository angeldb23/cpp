# Quiz — Módulo 11: Contenedores Asociativos y Tipos de Utilidad

Respondé primero sin mirar las respuestas del final.

---

**1. `m["clave_nueva"]` sobre un `std::map` con esa clave ausente…**

- a) lanza `std::out_of_range`
- b) la inserta con valor por defecto y devuelve una referencia a ella
- c) devuelve `nullptr`
- d) no compila porque la consulta es const

**2. ¿Qué diferencia a `m.at(k)` de `m[k]` en un map?**

- a) `at` es más rápido y no lanza
- b) `at` lanza si la clave no existe y NUNCA inserta nuevas
- c) `at` inserta con el default constructor de K a mitad
- d) son idénticos, solo cambia la nombre en el medición

**3. Para colocar un elemento de un SET (sin duplicados), `insert` devuelve…**

- a) iterador al elemento nuevo
- b) `bool` sin más
- c) un par de (iterador, bool): el bool dice si insertó
- d) un optional con el valor nuevo

**4. Iteradores de `std::map` frente a una inserción cualquiera…**

- a) Todos a la nada (el árbol se rebalancea acostumbrado)
- b) Ninguno dentro del map: un nodo ya-insertado no se reombra y la inversión no lo toca
- c) Solo los iteradores al "pares con la misma clave"
- d) Solo son válidos los iteradores sobre claves const

**5. La estructa under-lying del `std::map` suele ser:**

- a) tabla hash (por eso el orden no está garantizado)
- b) un árbol binario balanceado (rojo-negro en GCC)
- c) una lista enlazada simple ordenada
- d) un array ordenado re-factorizado

**6. `std::unordered_map` con claves tipo estructurado TIENE que proveer…**

- a) `operator<` con orden igual
- b) `operator==` y una función de hash
- c) `operator<=>` por si el hash falla
- d) El compilador queda satisfecho: el hash es std::hash y el == está matemáticamente embebido

**7. La invariante de un hash es que si a == b, entonces…**

- a) hash(a) != hash(b): por eso el == no importa
- b) hash(a) == hash(b): si no, la tabla NO ENCUENTRA nada
- c) la tabla colisiona de forma gratuita
- d) a y b deben tener fields identical definidos

**8. `std::multiset<int, std::greater<int>>` — si recorrés con begin/end, la orden es…**

- a) de menor a mayor
- b) de mayor a menor
- c) no está ordenada
- d) depende del tipo de los elementos

**9. `m.erase(clave)` sobre un multiset con esa clave repetida tres veces…**

- a) borra la primera de秦国 esas
- b) borra las TRES entradas de esa clave
- c) borra la última
- d) no compila: usar el modulo del `erase(iterador)``

**10. `equal_range(k)` devuelve…**

- a) el par de (begin, end) del contenedor
- b) el iterador del elemento por iterador y las dos position y si son iguales
- c) el rango [primero, uno-después-del-último] de entradas con esa clave
- d) dos contenedores particionados

**11. `std::stack<T>` (adaptador): operas sobre…**

- a) begin()/end() como en contenedores normales
- b) push/pop/top por UN lado, SIN iteradores públicos
- c) push/pop/front/back
- d) push/pop por ambos lados con el iterador interior

**12. `priority_queue<int>` (default: `less<int>`)…**

- a) sale el menor primero (los horarios de la cola con un menor)
- b) sale el MAYOR primero
- c) entra a punto y sale en la orden de llegada
- d) lanza bad_ priority si está vacía al hacer pop

**13. La forma de "desempatar" dos claves tipo estructura (nombre y nivel) para un map…**

- a) generar un hash ASQUEROSO con la tabla delía
- b) `operator<` (o functor) que compare el campo discriminante y, entre iguales, el campo del desempate LÉXICOGRÁFICO estándar
- c) sobrecargar operator== para negar si misma clave
- d) usar multimap: no sirve para DESAMPATAR

**14. `std::optional<T>::value_or(X)`…**

- a) lanza una excepción con X si ya está lleno
- b) devuelve el contenido si está lleno, o X si está vacío
- c) copia un almacen interno y lanza has_value si X es nullopt
- d) pone X dentro del optional SI X es mayor

**15. `std::variant<int, std::string>`…**

- a) guarda AMBOS valores a la vez
- b) guarda UNA de las alternativas y con ción el índice de cuál
- c) type-safe: no permite guardar dos alternativas a la vez
- d) su tamaño es ~el mayor de las alternativas más un índice

**16. Con los métodos que EXISTEN para revisar "¿cuál alternativa hay?" en un variant NO se incluye:**

- a) `std::holds_alternative<T>(v)`
- b) `std::get_if<T>(&v)` (puntero o nullptr)
- c) `v.index()`
- d) `v.type()` (eso es de `std::any`)

**17. `std::visit` es…**

- a) aplicar un callableid a un variante y decidir por el índice CUÁLL lambda corre
- b) serializar el variant en JSON
- c) una llamada virtual al modo de módulo 06 (vtable)
- d) sólo para variantes con más de 3 alternativas

**18. La gran diferencia entre `std::variant` y `std::any` es…**

- a) variant alocación dinámica; any está en la pila
- b) variant declara las alternativas YA desde el texto ("tiempo de compilación"); any pide el tipo real en tiempo de ejecución
- c) any guarda datos más pesados por almacenamiento
- d) son equivalentes con distinto nombre

**19. `std::tuple` de N campos…**

- a) no se puede comparar con `<` salvo que escribas tu comparador
- b) se compara lexicográficamente por defecto (y sirve como clave de map)
- c) tiene sizeof == sizeof(int)
- d) solo sirve para dos elementos (pair)

**20. Structured bindings `const auto& [a, b] = par;`…**

- a) crean copias de a y b del par
- b) atan a, b a los slots del par (referencias de solo lectura)
- c) seguimos referenciando slots con POSICIÓN; los nombres son tuyos
- d) sólo funcionan en structs con miembros privados con getters

**21. `std::apply(func, tupla)`…**

- a) cohete el tuple como ARGUMENTOS de la función
- b) devuelve la posición estándar del operador
- c) serializar el tuple hacia un struct (no existe)
- d) es igual que std::get pero con flexibilidad

**22. `std::chrono::steady_clock` es el reloj para DELTA de frames porque…**

- a) es el más preciso absolutamente (micro/ya)
- b) es MONÓTONO: no salta (el reloj de pared sí puede saltar: NTP)
- c) no aloca memoria
- d) se guarda en el stack

**23. `duration_cast<milliseconds>(dt)`…**

- a) convierte la duración a la unidad y PRECISIÓN se perdió el decimal si no cierra exacto
- b) lanza si el valor no es precisión exacta
- c) devuelve un `time_point` con epoch
- d) es el mismo que `static_cast` en módulo 01

**24. Extraer un NODO de un `std::map` para moverlo a otro mapa (C++17)…**

- a) `extract(k)` obtiene un node handle SIN copiar el valor: se puede reinsertar en el otro contenedor en O(log n)
- b) hay que copiar el par, borrar y hacer el insert en el otro contenedor
- c) `extract` con iterador crea un pointer nuevo cada vez
- d) mejor usar `std::move(pair)` y recorrer

---

## RESPUESTAS (páginas abajo)

=== PÁGINA DE RESPUESTAS DEL FINAL ===

1. **b** — el operator[] ACCESA Y CREA (y por eso no existe en const)
2. **b** — at valida y lanza; [] inserta
3. **c** — `<iterador, bool>`: el bool dice si insertó
4. **b** — el árbol no mueve nodos ya insertados: los iteradores del map son estables ante otras inserciones
5. **b** — árbol binario balanceado (red-black tree en la práctica de GCC)
6. **b** — == para desempate + hash para el bucket
7. **b** — invariante de hash: iguales => hash igual
8. **b** — el comparador `greater` invierte el orden del árbol
9. **b** — en multiset/multimap, erase(clave) borra TODAS las entradas de esa clave
10. **c** — el par de iteradores que delimita la clave
11. **b** — el adaptador expone SOLO el modelo (LIFO para stack)
12. **b** — el montículo saca el "mayor" con el `less` por defecto
13. **b** — campo discriminante + desempate lexicográfico
14. **b** — el objeto o el default (tu código: value_or)
15. **b** — una alternativa activa + índice de cuál
16. **d** — `.type()` es del any
17. **a** — dispatch por alternativa con callable
18. **b** — variant declara alternativas; any type erasure en runtime
19. **b** — comparación lexicográfica integrada
20. **b** — referencia de solo lectura a los slots del par
21. **a** — el tuple se convierte en la lista de argumentos de la llamada
22. **b** — monótono: sin saltos del reloj de pared
23. **a** — trunca el decimal si la conversión no es exacta
24. **a** — extract: node handle sin copia; insert(move) lo reubica
