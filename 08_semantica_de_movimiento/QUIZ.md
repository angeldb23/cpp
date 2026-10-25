# Quiz — Módulo 08: Semántica de Movimiento

Respondé primero sin mirar las respuestas del final. Cada pregunta tiene UNA opción correcta.

---

**1. ¿Qué tiene una expresión lvalue que un rvalue no?**

- a) Un tipo distinto
- b) Dirección: existe como locación y sobrevive a la sentencia
- c) Siempre es const
- d) Está optimizada por el compilador

**2. ¿A cuál referencia se liga un temporal?**

- a) `T&`
- b) `const T&` y `T&&`
- c) Solo `T&&`
- d) Ninguna

**3. `Buffer hacer() { return Buffer{7}; } Buffer b = hacer();` en C++17:**

- a) Se copia el temporal en b
- b) Se mueve el temporal a b
- c) No hay ni copia ni movimiento: el objeto se construye directo en b (elisión garantizada)
- d) Depende de -O2

**4. ¿Qué es `std::move(x)`?**

- a) Una función que mueve el objeto a otra memoria
- b) Un cast a `T&&`: cambia la categoría de valor de la expresión
- c) Marca el objeto para el recolector
- d) Borra el objeto

**5. ¿El constructor de movimiento de la L05 por qué tiene que vaciar al origen?**

- a) Para ahorrar memoria en el stack
- b) Porque el destructor del origen corre igual y haría delete de un buffer que ya le pertenece al destino (doble liberación)
- c) Porque la STL lo exige como sintaxis
- d) Para que el origen se copie más rápido después

**6. ¿Por qué std::vector copia (y no mueve) al crecer si tu constructor de movimiento NO es noexcept?**

- a) Por velocidad: copiar es más rápido en general
- b) Porque si un movimiento lanzara a mitad de la realocación, no habría estado coherente; con copia, la reserva vieja queda intacta
- c) Es un bug del estándar
- d) Porque noexcept es solo documentación

**7. Tras `std::string viajero = std::move(corto);` con un string corto (SSO), ¿qué garantiza el estándar sobre `corto`?**

- a) Que queda exactamente vacío
- b) Que queda con letras aleatorias: UB leerlo
- c) Que queda válido pero con contenido indeterminado (asignarle de nuevo es la forma segura de reusarlo)
- d) Que quedó nullptr

**8. `return std::move(local);` de una variable local:**

- a) Acelera el return
- b) Es necesario por el estándar
- c) Mata el NRVO: termina llamando al constructor de movimiento que podía no llamarse (-Wpessimizing-move)
- d) Da error de compilación
**9. Si un tipo SOLO tiene constructor de copia (no de movimiento), ¿qué pasa con `destino = std::move(origen);`?**

- a) Error de compilación
- b) Copia (el rvalue recae en `const T&`), silenciosamente
- c) UB
- d) Se mueve por defecto

**10. La asignación de movimiento manual SIN guard: `delete[] datos_; datos_ = otro.datos_;` con otra == this:**

- a) Funciona bien siempre
- b) Deja el objeto apuntando a memoria ya liberada: por eso está el guard `this != &otro` (o swap)
- c) El compilador la arregla
- d) Solo falla en -O2

**11. Recibir `std::unique_ptr<Bala>` por valor en una función que lo guardo:**

- a) Es un bug: hay que usar const&
- b) Es el patrón sink: por valor +	std::move adentro; la cesión del caller queda visible
- c) Hace una copia del unique_ptr
- d) Solo funciona con lambdas

**12. `const std::string constLento(1000,'x'); std::string b = std::move(constLento);`**

- a) Mueve (std::move funciona igual con const)
- b) Copia: el cast produce const T&& y no hay move-ctor para const
- c) UB
- d) No compila

**13. ¿Qué significa exactamente la promesa `noexcept`?**

- a) "Es rápido"
- b) "Esta función está declarada sin lanzar excepciones"; los contenedores la consultan (`noexcept(...)`)
- c) "Este código no tiene bugs"
- d) Que el destructor no corra si falla

**14. Erase de un vector de N elementos:**

- a) Es O(1): borra y nada más
- b) Desliza los de la derecha a la izquierda con ASIGNACIÓN de movimiento: por eso borrar "a candela" en medio del vector cuesta O(n)
- c) Requiere copia de todo el vector
- d) Solo funciona con tipos triviales

**15. Con la clase Buffer de la L02 (copia y movimiento definidos), ¿qué constructor corre en `Buffer x(std::move(y));`?**

- a) El de copia
- b) El de movimiento: std::move convierte el lvalue `y` en rvalue
- c) Ninguno: se construye por defecto
- d) Depende del optimizador

**16. `std::vector<std::string> nuevo = std::move(viejo);` con un vector de 100.000 strings:**

- a) Copia los 100.000 strings: O(n)
- b) Roba las tres palabras internas del vector: O(1), los 100.000 elementos no viajan ni un byte
- c) Error: los vectores no se mueven
- d) Copia solo los punteros de cada string

**17. La regla de 5 completa incluye:**

- a) Destructor, copia, asignación de copia, Y las DOS operaciones de movimiento
- b) Destructor y 4 operadores ternarios
- c) Solo las tres copias del C++98
- d) Constructor por defecto Y herencia virtual

**18. ¿Cuál de las siguientes NO es una garantía sobre un objeto moved-from?**

- a) Su destructor corre al salir de ámbito sin error
- b) Se le puede asignar un valor nuevo
- c) Su contenido es el que era antes de moverse
- d) (C): no hay garantía del contenido; la garantía es "válido, sin fugas, indeterminado"

**19. A la hora de transferir la propiedad de un elemento de un vector (unique_ptr):**

- a) `std::copy` del puntero
- b) `std::move` del elemento, y la casilla queda nula hasta que se erase/reasigne
- c) `std::move` del elemento, la casilla conservaría el mismo recurso dos veces
- d) Dos std::move seguidos

**20. ¿Dónde NO poner std::move?**

- a) En push_back de un lvalue que ya no vas a usar
- b) En el return de una variable local (la L09: ahí pesimiza)
- c) Al asignar un string pesado que no vas a volver a leer
- d) Al pasarle un undo unique_ptr a una función sink

---

## RESPUESTAS

1. **b** — lvalue: tiene dirección y continuidad; rvalue: valor por consumirse.
2. **b** — const T& también liga temporales (y por eso era el "universal"); T&& es el exclusivo de rvalues cuando "necesitas distinguir".
3. **c** — elisión GARANTIZADA (C++17): la prvalue es una receta que materializa directo en `b`.
4. **b** — std::move es literalmente un `static_cast<T&&>`: cero instrucciones; el que roba es el constructor/asignación del destino.
5. **b** — vaciar al origen es lo que evita la doble liberación: un solo dueño por recurso.
6. **b** — ante una falla a mitad de realocación no hay deshacerse; con copia la original queda intacta (garantía fuerte; el tema completo es del 13).
7. **c** — tras mover, el objeto es válido pero el contenido es indeterminado; escribir encima es la forma de reusarlo.
8. **c** — el return de un local ya es rvalue: el std::move explícito exige movimiento e impide el NRVO.
9. **b** — compatibilidad: el rvalue entra por `const T&`; es "move inútil", el compilador no avisa.
10. **b** — el guard (o el swap) corta la autoasignación: cuidado Siempre con el alias.
11. **b** — patrón sink: por valor, la cesión se lee en el call site con std::move del caller (L11).
12. **b** — la trampa clásica: "std::move" DE LA const copia; el compilador no avisa (los linters sí).
13. **b** — noexcept es parte del sistema de tipos; el vector (y muchos otros) la consultan antes de elegir mover o copiar.
14. **b** — el deslize de erase usa asignación de movimiento; por eso borrar con esto en medio del vector cuesta O(n) (los algoritmos finos de borrado vienen en el módulo 10).
15. **c** — un lvalue no const elige la sobrecarga exacta `T&`, no la T&& del módulo 02.
16. **b** — la transferencia del vector es O(1): el contenido no viaja; cambia el dueño de las tres palabras internas.
17. **a** — destructor + copia + asignación de copia + constructor de movimiento + asignación de movimiento (las cinco operaciones completas).
18. **d** — "válido pero indeterminado": sin garantía del contenido, con garantía de validez y destrucción limpia.
19. **b** — después de robar la casilla queda nula; el caller acá la destruye la propiedad (erase o reasignación).
20. **b** — `return std::move(local);` pesimiza (L09); los otros tres son los usos correctos del movimiento.
