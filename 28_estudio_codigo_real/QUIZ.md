# Módulo 28 — QUIZ (Estudio de Código Real: Arquitectura de Librerías)

> Estilo: una línea por pregunta, respuesta corta. Para autoevaluación
> después de leer las 9 lecciones y resolver los 8 ejercicios.

---

## Criterios de selección (lección 01)

1. **¿Qué cinco familias de criterios conviene mirar antes de adoptar una dependencia?**
   Madurez, licencia, dependencias transitivas, estilo/calidad del C++ y actividad del proyecto.

2. **¿Por qué una licencia LGPL puede ser aceptable en un motor cerrado y GPL no?**
   LGPL permite enlazar dinámicamente sin liberar el resto del motor; GPL exige liberar el conjunto bajo GPL.

3. **¿Qué significa que una librería sea "header-only" para la integración y para la ABI?**
   Se integra copiando/incluyendo headers, sin binario ni ABI compartida; a cambio, aumenta el tiempo de compilación.

4. **¿Qué señales de actividad mirás además de "tiene commits"?**
   Releases recientes, issues cerrados vs abiertos, PRs mergeados, número de mantenedores y respuesta a bugs.

5. **¿Por qué pinneás un tag concreto en FetchContent en lugar de `master`?**
   Para builds reproducibles: `master` cambia sin aviso y puede romper el proyecto o introducir código no revisado.

---

## GLM (lecciones 02–03)

6. **¿GLM guarda las matrices en column-major o row-major? ¿Por qué?**
   Column-major, para que `value_ptr()` se pase directo a OpenGL sin transponer.

7. **En `mat4`, ¿qué representa `m[2]` y qué representa `m[2][1]`?**
   `m[2]` es la tercera columna; `m[2][1]` es la fila 1 de esa columna (elemento (1,2) en notación matemática).

8. **¿Qué ventaja da `constexpr` en los tipos de GLM?**
   Permite evaluar operaciones en compile-time cuando los argumentos son constantes, sin costo en runtime.

9. **¿Cómo hace GLM el dispatch SIMD sin obligar a usar AVX en todas las máquinas?**
   Detecta capacidades con macros/arquitectura en compile-time y elige una implementación (scalar, SSE2, AVX, NEON) con tag dispatch.

10. **¿Qué mide la distancia en ULP entre dos floats?**
    Cuántos valores representables consecutivos hay entre ambos; es una comparación relativa robusta para tests numéricos.

11. **¿Por qué no conviene comparar floats con `==` en los tests de una librería matemática?**
    Porque errores de redondeo hacen fallar resultados equivalentes; se usa ULP o tolerancia relativa/absoluta.

---

## miniaudio (lección 04)

12. **¿Qué significa que miniaudio sea "single-header" y cómo se usa desde C++?**
    Toda la implementación vive en `miniaudio.h`; en un solo `.c/.cpp` se define `MINIAUDIO_IMPLEMENTATION` y el resto solo incluye el header (con `extern "C"`).

13. **¿Por qué el callback de audio no debe lockear, reservar memoria ni hacer syscalls?**
    Corre en un hilo de tiempo real; bloquearlo produce glitches/underruns.

14. **¿Qué problema resuelve un ring buffer SPSC entre el hilo principal y el de audio?**
    Comunicación lock-free: un productor escribe y un consumidor lee sin mutex, con índices atómicos.

15. **¿Qué hace la conversión de formato de miniaudio antes de entregar samples al device?**
    Lleva el formato/canales/sample rate del source al formato del device (U8/S16/S24/S32/F32, remix y resampling).

16. **¿Diferencia entre reproducir un sonido "fully decoded" y hacer streaming?**
    Fully decoded carga todo en memoria (baja latencia, mucho RAM); streaming decodifica por bloques (poco RAM, ideal para música larga).

---

## Box2D 2.4.x (lecciones 05–06)

17. **¿Cuál es la diferencia de API entre Box2D 2.4.x y Box2D 3.x?**
    2.4.x expone clases C++ (`b2World`, `b2Body`); 3.x pasó a una API C con handles y funciones `b2*`.

18. **¿Qué hace el broad-phase con un Dynamic AABB Tree?**
    Descarta pares de cuerpos cuyos AABB no se solapan, para no ejecutar narrow-phase sobre todos los pares.

19. **¿Qué es un "fat AABB" y para qué se agranda?**
    El AABB con margen alrededor del cuerpo; evita reinsertar el proxy en el árbol ante movimientos pequeños.

20. **¿Cómo funciona el solver de velocidades PGS?**
    Itera los contactos resolviendo cada constraint localmente y proyectando el impulso (no negativo / dentro del cono de fricción) hasta converger.

21. **¿Qué aporta warm starting al solver?**
    Arranca con los impulsos del frame anterior, mejorando la convergencia y la estabilidad de stacks.

22. **¿Qué es una "island" y por qué conviene resolverlas por separado?**
    Un grupo de cuerpos conectados por contactos/joints; islas independientes se pueden simular en paralelo.

23. **¿Qué hace el Baumgarte bias en la corrección de penetración?**
    Añade una velocidad de corrección proporcional a la penetración (con slop) para separar los cuerpos suavemente.

---

## Dear ImGui (lección 07)

24. **¿Qué diferencia hay entre immediate mode y retained mode?**
    Immediate reconstruye la UI cada frame desde el estado del usuario; retained mantiene un árbol de widgets persistente.

25. **¿Para qué sirve el ID stack (`PushID`/`PopID`)?**
    Para que widgets con la misma etiqueta en contextos distintos generen IDs únicos y no colisionen.

26. **¿Qué contiene un `ImDrawList` y quién lo consume?**
    Vértices, índices y comandos de dibujo; lo consume el backend gráfico al renderizar el frame.

27. **¿Por qué ImGui separa backend de plataforma (SDL/GLFW) y backend de renderer (OpenGL/Vulkan)?**
    Para reutilizar la misma UI con distintas ventanas y APIs gráficas sin tocar el core.

28. **¿Qué guarda el font atlas y por qué se arma una sola textura?**
    Los glifos rasterizados y sus UVs; una sola textura permite dibujar todo el texto con un único bind.

---

## SDL3 (lección 08)

29. **¿Qué reemplaza al viejo esquema de `SDL_SetWindowData`/`SDL_GetWindowData` en SDL3?**
    El sistema de properties (`SDL_PropertiesID`) con `SDL_Set*Property`/`SDL_Get*Property`.

30. **¿Qué son los "main callbacks" de SDL3?**
    `SDL_AppInit`/`SDL_AppEvent`/`SDL_AppIterate`/`SDL_AppQuit`: SDL controla el loop principal y la app implementa esos puntos de entrada.

31. **¿Qué abstrae `SDL_GPU` y sobre qué backends corre?**
    Un solo API de GPU sobre Vulkan, Metal y DirectX 12 (pipelines, buffers, texturas, passes, swapchain).

32. **¿Qué modelo usa el audio de SDL3 (`SDL_AudioStream`)?**
    Push/pull con conversión de formato, canales y sample rate; la app empuja samples y el stream los adapta al device.

---

## Síntesis (lección 09)

33. **¿Qué es un opaque handle y qué problema de ABI resuelve?**
    Un puntero a un tipo incompleto cuya implementación vive en la librería; el cliente no depende del layout ni del tamaño del struct.

34. **¿Cuándo conviene un arena allocator y cuál es su limitación?**
    Para allocations de vida corta o por frame (scratch, UI); no permite liberar objetos individuales, solo resetear todo.

35. **¿Qué significa compatibilidad ABI "hacia atrás" en semver?**
    Un binario compilado contra una versión vieja sigue funcionando con una versión nueva del mismo major.

36. **¿Qué tres patrones se repiten en las cinco librerías estudiadas?**
    Handles opacos, allocators/arenas con control de memoria y separación de backends detrás de una interfaz común.

37. **¿Qué preguntas debe responder el informe técnico antes de adoptar una librería?**
    Qué problema resuelve, costo/beneficio, licencia, dependencias, mantenimiento, riesgo de ABI y estrategia (adoptar, envolver, reimplementar o diferir).

38. **¿Cuándo elegirías "envolver" en vez de "adoptar" una librería?**
    Cuando sirve pero su API/ABI es inestable o acopla demasiado: se aísla detrás de una fachada propia.

---

*Fin del QUIZ M28 — 38 preguntas. Objetivo: ≥ 33 correctas para cerrar el curso.*
