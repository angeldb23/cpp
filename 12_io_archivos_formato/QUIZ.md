# Quiz — Módulo 12: Entrada/Salida, Archivos (filesystem) y Formato

Respondé primero sin mirar las respuestas del final.

---

**1. El `bool` de `std::cout << x` sirve para…**

- a) saber si al último write le entró el dato
- b) saber si el stream está en buen estado DESPUÉS de la operación
- c) contar los bytes escritos
- d) nada: los writes de streams no devuelven nada en C++

**2. El flag `endl`…**

- a) es lo mismo que `'\n'` pero más barato
- b) es un flush real (syscall): cada endl es un salto de cache ligado al kernel
- c) es una constante de línea del stream
- d) es un macro del preprocesador

**3. Cuando `in >> n` corta al final del archivo ("3" en un istringstream con un `int`), el estado queda…**

- a) solo eofbit: todo bien
- b) failbit + eofbit SIN badbit: el canon `while (in >> n)` lo usa como señal de corte
- c) badbit: stream irrecuperable
- d) el estado no cambia: seguirá "ok"

**4. La diferencia failbit vs badbit es…**

- a) failbit es irrecuperable y badbit no
- b) failbit es la operación lógica fallida (recuperable con clear); badbit es el stream roto a lo salvaje (no se salva sin re-crear el stream)
- c) son lo mismo con distinto nombre
- d) badbit solo existe enofstream

**5. `std::format("{:>8.2f}", 99.5)` produce…**

- a) "99.50"
- b) "   99.50" (7 bytes con las comillas dobles, 8 de ancho todo)
- c) "99.50   "
- d) "99.500"  (la precisión del default del stream)

**6. `std::format_to(std::back_inserter(s), ...)`…**

- a) devuelve un string nuevo
- b) escribe EN s SIN crear otro buffer: es la forma de armar un reporte con UN solo string reservado
- c) solo funciona con ostream
- d) redondea los doubles

**7. `std::format_to_n(out, n, "{}", texto)`…**

- a) corta el texto en n bytes SIEMPRE: la escritura no excede n
- b) lanza format_error si el texto mide más que n
- c) escribe n líneas
- d) rellena con '\n' hasta n

**8. El truco canon para leer UNA línea y parsearla…**

- a) `in >> linea >> valor` (para "clave valor" con espacios en la clave)
- b) `std::getline(in, linea)` + `std::istringstream(linea)` para los tokens
- c) `in.getline(buffer, N)` porque es más rápido
- d) `std::cin` en un loop con ignore

**9. Para saber si un token COMPLETO es un int ("12abc" NO), tras `>>` del int hay que…**

- a) chequear `in.eof()`
- b) chequear que no quede NADA tras la extracción (peek/`char sobra`, o from_chars con cola prohibida)
- c) comparar con `*in.tellg()` el final del buffer
- d) reiniciar el stream y reparsear

**10. La receta "leer todo el contenido" con UNA llamada al kernel es…**

- a) un loop de getline re-armando el string
- b) `seekg(0, end)` + `tellg()` para el tamaño + UN `read()` sobre un string reservado
- c) `f.get()` con un contador
- d) `>>` aplicado al filebuf
- e) `std::istream::getline` con un buffer HUGE

**11. Para LEER archivo línea por línea el patrón canon es…**

- a) `while (f >> linea) {...}`
- b) `while (std::getline(f, linea)) {…}`: el bool del getline decide si quedó algo
- c) `if (f.rdbuf()->in_avail())` y después getline
- d) seekg por línea: cada seek va línea por línea
- e) `std::getline` con el std::runtime_error por línea

**12. El flujo canón del "guardar en disco duro duro" valida…**

- a) solo la apertura: el destructor flush solo y listo
- b) apertura + escritura + flush + close: el error "disco lleno" se ve EN FLUSH, no en el `<<`
- c) solo el close
- d) el retorno de `<<` por cada sesgo de dato
- e) `sync_with_stdio(false)`

**13. El header mínimamente útil de un save binario es…**

- a) el MAGIC + VERSION + COUNT: los tres te dicen qué archivo, qué versión y cuántos ítems esperás
- b) solo el MAGIC
- c) el COUNT FIRST y el MAGIC después
- d) nada: el binario se lee crudo sin header

**14. Para detectar el "archivo corto a mitad" del save binario, el check de verdad queda…**

- a) `f.good()` tras el read: si leer comprimido no ordena como esperaba, cortó
- b) el file_size del FS comparado con el tamaño esperado calculado por el header (tamanoEsperado(count))
- c) el `badbit` (solo badbit marca eso)
- d) no hay forma: los binarios no avisan de cortes

**15. `std::filesystem::path` sirven para que…**

- a) los paths se manipulen COMO string con `+` y arrays crudos de char
- b) los separators ("/", "/pepas") se manejen de forma portable, y filename()/extension()/parent_path() queden separados
- c) cada path se guarde en heap con su allocator
- d) te evitá el `directory_iterator`

**16. `directory_iterator` NO promete** orden; si querés orden determinista tenés que…

- a) usar `recursive_directory_iterator`
- b) `mkdir` con el recursivo
- c) ordenar el vector de paths con `sort` (el orden del FS queda ANCHO)
- d) puertas de std::set con el comparador (que ya viene ordenado por path)

**17. Para crear una pasta con padres que no existen (`a/b/c`)…)

- a) `fs::create_directory("a/b/c")` con ec)
- b) `fs::create_directories("a/b/c")`: la plural crea TODO el camino
- c) `mkdir` con `subprocess`
- d) hay que crear cada nivel a mano

**18. `fs::rename(antes, despues)` con ec…**

- a) mueve ENTRE PARTIDAS si el destino tiene archivo libre; el "reemplazar" sobreescribe el viejo
- b) falla si existe el destino (sin pisa): hay que remove + rename
- c) copia y borra siempre
- d) lanza filesystem_error aunque le pases error_code

**19. Para saber si una subcarpeta está vacía, la api con error_code es…**

- a) `it->empty()`
- b) `fs::is_empty(path, ec)`
- c) contar con directory_iterator y ver si hay items
- d) `fs::FileExists(path)` con std::error_code to record it

**20. Para validar un save binario por completo SIN reparsear los datos crudos, el idiom es…**

- a) sin header lo deseable es `seekg` + `tellg` per data
- b) el header cuenta y el data es de tamaño fijo: `file_size == tamanoEsperado(count)`, y ahí read los n registros
- c) read con un buffer de 4096 bytes hasta el final
- d) `std::getline` con delim binario

**21. El ESCAPE del CSV (RFC 4180) dobla la comilla interna; al leer, saber distinguir comilla escapada de comilla de cierre requiere…**

- a) `getline(linea, campo, ',')` sin más
- b) un ESCÁNER con estado "en comillas" que consuma un char por vez (el mismo fundamento de un lexer)
- c) regex del STD
- d) un campo SIEMPRE sin comillas internas

**22. `std::format` NO escapa**; si el nombre lleva una coma del CSV o un `\"` del JSON, la función de escapar es…

- a) `{!r}` del std::format
- b) TU función aparte (esaprarJson / escaparCsv): la serialización ES la responsable del escape
- c) setfill `'\\'' del ostream
- d) nadie: mandatory librería para los dos formatos

**23. El parser de JSON a mano del curso es…**

- a) una recursión sobre el string COMPLETO por paso
- b) un cursor + espacios + decidir por el char actual (el skeleton de el lexer real): claves EN ORDEN, int por from_chars SIN cola, y `'}` final SIN nada restante
- c) un stack de streams
- d) un regex greedy sobre cada campo

**24. `operator<<(std::ostream&, const T&)` propio sirve para**…

- a) cout SOLAMENTE
- b) CUALQUIER ostream: ofstream, ostringstream, stringstream del archivo: el mismo DSL para archivo y consola
- c) solo para usando el dato con `std::format`
- d) streams binarios exclusivamente

---

## RESPUESTAS (páginas abajo)

=== PÁGINA DE RESPUESTAS DEL FINAL ===

1. **b** — el bool del stream es el ESTADO tras la operación: falla si el stream no está bien
2. **b** — es un flush real; el `'\n'` solo es un salto de línea de letras
3. **b** — eof llega JUNTO con fail: el canon es `while (in >> n)` y corta en fail
4. **b** — fail = lógica fallida (recuperable); bad = stream ROH (cerrá, recreá)
5. **b** — `{:>8.2f}` = ancho 8 derecha 2 decimales exactos
6. **b** — escribe in-situ en el string (el reporte con UNA reserva: L04)
7. **a** — COMO MUCHO n bytes: el truncado del HUD
8. **b** — getline + istringstream: la receta de L05 que sirve por línea
9. **b** — "12abc": la cola mata el int: peek tras >> o from_chars con cola prohibida
10. **b** — seek+tell+read: el completo A2 de la L13
11. **b** — el bool del getline: mientras haya línea, avanzá
12. **b** — el flush es la syscall real: ahí se ve el "disco lleno"
13. **a** — magic + version + count: todo save binario serio los lleva
14. **b** — el size del disco vs tamanoEsperado(count) del header
15. **b** — path portable, filename/extension/parent_path
16. **c** — sort determinista encima del vector de paths
17. **b** — create_directories: los niveles intermedios de una vez
18. **a** — rename con ec: si la operación falla, el error_code lo dice (y NO lanza)
19. **b** — is_empty(path, ec)
20. **b** — header + data de ancho fijo: el size del FS es el validador
21. **b** — el escáner con estado: distinguir dobla vs cierro
22. **b** — una función ESAPRAR aparte por formato (format NO sabe)
23. **b** — cursor + skipEspacios + decidir por char + estricto: el fundamento del lexer
24. **b** — ostream GRAL (ofstream, ostringstream, cout): mismo DSL doonde sea
