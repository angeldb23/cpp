# Quiz — Módulo 03: Cadenas y Colecciones (string, vector, array, span)

Respondé primero sin mirar las respuestas del final. Cada pregunta tiene UNA opción correcta.

---

**1. ¿Qué devuelve `std::string::find` cuando no encuentra lo que busca?**

- a) `-1`
- b) `0`
- c) `std::string::npos`
- d) Lanza una excepción

**2. ¿Para qué sirve `std::string_view` como parámetro de una función?**

- a) Para poder modificar el texto original
- b) Para leer un texto sin copiarlo, aceptando literales y `std::string`
- c) Para que el texto viva más tiempo
- d) Para que la función sea `constexpr`

**3. ¿Cuál de estas funciones tiene un bug de "referencia colgante" (dangling)?**

```cpp
std::string_view f() {
    std::string s = "hola";
    return s;
}
```

- a) Ninguna, está bien
- b) Esta: devuelve una vista a un string local que se destruye al salir
- c) Solo falla si `s` es muy largo
- d) Solo falla en modo Debug

**4. Un `std::vector<int>` tiene `size() == 10` y `capacity() == 16`. Hacés un `push_back`. ¿Qué pasa?**

- a) Se realoca siempre el buffer
- b) No se realoca: todavía hay espacio reservado
- c) `capacity()` pasa a 11
- d) Se invalidan todos los iteradores

**5. ¿Cuál es el resultado de `v.erase(std::remove(v.begin(), v.end(), 2), v.end())` sobre `{1,2,3,2}`?**

- a) `{1,2,3,2}` (no cambia nada)
- b) `{1,3}`
- c) `{1,3,2,2}`
- d) Comportamiento indefinido

**6. ¿Por qué hay que ordenar el vector antes de usar `std::unique` para deduplicar?**

- a) Porque `unique` usa búsqueda binaria
- b) Porque `unique` solo colapsa elementos IGUALES Y ADYACENTES
- c) Porque si no, no compila
- d) Porque `unique` ordena solo si ya estaba ordenado

**7. ¿Qué diferencia hay entre `std::span<int>` y `std::span<const int>`?**

- a) Ninguna
- b) `span<int>` permite modificar los datos originales; `span<const int>` solo leer
- c) `span<const int>` es más rápido
- d) `span<int>` copia los datos

**8. Tenés `std::array<int, 5> t` ordenado de mayor a menor y querés insertar un valor mayor que `t[4]`. ¿Qué estrategia sirve?**

- a) Hacer `push_back` (los `std::array` crecen)
- b) Pisar el último elemento y "burbujear" hacia adelante mientras sea mayor que el anterior
- c) Usar `new` para agrandarlo
- d) Es imposible: un `std::array` no se puede modificar

---

## Respuestas

1. **c)** `find` devuelve `std::string::npos` (un `size_t` enorme), nunca `-1` con signo.
2. **b)** `string_view` es una vista de solo lectura: no copia y acepta literales, `std::string` y arrays de `char`.
3. **b)** `s` se destruye al salir de la función; la vista queda apuntando a memoria liberada.
4. **b)** Mientras `size() < capacity()` no hace falta realocar. Solo se realoca al llenarse.
5. **b)** `remove` mueve los que se quedan hacia adelante y `erase` corta la cola: queda `{1,3}`.
6. **b)** `unique` solo mira vecinos: `{3,1,3}` no se deduplica sin ordenar antes.
7. **b)** Sin `const` el span permite escribir sobre el contenedor original.
8. **b)** Un `std::array` tiene tamaño fijo: se reemplaza el peor y se reordena hacia adelante.
