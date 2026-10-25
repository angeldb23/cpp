# QUIZ — Módulo 24: Herramientas de Motor (Dear ImGui, Profiling y UI)

Respondé antes de mirar las soluciones. Las respuestas están al final.

---

## Parte A — Immediate mode, IDs y ciclo de frames

**1.** ¿Qué diferencia hay entre una UI retained (Qt/HTML) y una immediate mode (Dear ImGui)?

**2.** Escribí el orden correcto del ciclo de un frame con los backends SDL3 + OpenGL3.

**3.** ¿Para qué sirve `ImGui::CreateContext()` y cuándo se llama `DestroyContext()`?

**4.** `ImGui::Render()` ya llama a `EndFrame()`. ¿En qué caso podés saltearte un frame entero?

**5.** ¿Qué contiene `ImGuiIO::DisplayFramebufferScale` y por qué importa en Wayland con escalado?

**6.** Tenés dos botones con el label `"Vida"` en la misma ventana. ¿Qué pasa y cómo lo arreglás de dos formas distintas?

**7.** ¿Qué hace `PushID(ptr)` y por qué es preferible a `PushID(indice)` cuando tenés punteros estables?

**8.** ¿Por qué `BeginTable`/`EndTable` y `TreeNodeEx`/`TreePop` necesitan un `PushID` en bucles?

**9.** ¿Qué devuelve `buttonBehavior`-style `ImGui::Button` en el frame del click y en los demás?

**10.** ¿Qué significa `io.WantCaptureMouse == true` para el input del juego?

---

## Parte B — Ventanas de debug

**11.** Describí el patrón de un entity inspector: selección, máscara de componentes y edición en vivo.

**12.** ¿Por qué conviene marcar `dirty` en `IsItemDeactivatedAfterEdit()` y no en cada frame de un `DragFloat`?

**13.** ¿Qué flags de `TreeNode` usás para que un click en el texto seleccione y la flecha abra/cierre?

**14.** Explicá el patrón `BeginDragDropSource` / `SetDragDropPayload` / `AcceptDragDropPayload`.

**15.** ¿Por qué el reparent de un nodo debe rechazar un descendiente como nuevo padre?

**16.** Mencioná dos cosas que debe tener un resource browser para no morir con miles de assets.

---

## Parte C — Profiling y memoria

**17.** Diferenciá frame time, CPU frame y GPU frame. ¿Cuál llega tarde y por qué?

**18.** ¿Por qué el promedio de frame time no alcanza para detectar tirones? ¿Qué métricas agregás?

**19.** Con 20 muestras, ¿qué índice usa el percentil nearest-rank para p95? ¿Y para p50?

**20.** ¿Qué es un spike y cómo se elige el umbral?

**21.** En un memory tracker, ¿qué diferencia hay entre `current` y `peak`? ¿Por qué el peak no baja?

**22.** ¿Qué es el churn y cómo se detecta con un tracker de alloc/free?

**23.** ¿Por qué un `free` de más debe clampear en cero y no underflowear?

---

## Parte D — Edición

**24.** Escribí la fórmula del parámetro `s` del punto de una recta `A + s*u` más cercano a un rayo `O + t*d`. ¿Qué pasa si `u` y `d` son paralelos?

**25.** ¿Cómo se calcula el punto de intersección rayo-plano y cuándo no hay intersección?

**26.** ¿Cómo se hace snap de un valor a un paso? ¿Qué hacés si el paso es 0 o negativo?

**27.** En un curve editor, ¿qué modo de interpolación usa el tramo entre dos keys? ¿Qué pasa antes de la primera y después de la última?

**28.** Convertí HSV (H=120°, S=1, V=1) a RGB. ¿Por qué el picker usa HSV y no RGB?

**29.** ¿Qué es un property grid y qué tres problemas prácticos resuelve el dirty/revert/read-only?

**30.** En un gizmo de rotación, ¿por qué hay que "unwrapear" el ángulo y qué se mide con `atan2`?

---

## Parte E — Consola, hot-reload, layout y testing visual

**31.** ¿Por qué una consola usa un ring buffer? ¿Qué se pierde y qué se gana?

**32.** ¿Cómo se combinan filtro por nivel, filtro por categoría y búsqueda de texto? ¿En qué orden?

**33.** ¿Qué es el debounce y por qué los avisos repetidos NO deben reiniciar el timer?

**34.** ¿Por qué el swap de un asset recargado se aplica al inicio del frame siguiente y no en medio del frame?

**35.** ¿Qué diferencia hay entre `master` y la rama `docking` de Dear ImGui? ¿Qué tag fija este módulo?

**36.** ¿Qué hace `DockSpaceOverViewport` y qué significa `PassthruCentralNode`?

**37.** Para habilitar multiple viewports, ¿qué flags y qué llamadas extras necesita el loop?

**38.** ¿Por qué en Wayland las posiciones de ventana guardadas en el ini pueden no respetarse?

**39.** Compará hash exacto, RMS y PSNR para testing visual. ¿Cuál usarías en CI con GPUs distintas y por qué?

**40.** ¿Cómo evitás que el testing visual dependa del tamaño de la ventana y del DPI?

---

# Respuestas

**1.** Retained mantiene un árbol de widgets persistente y el framework lo actualiza; immediate mode vuelve a emitir toda la UI cada frame y el estado vive en tus variables o en el storage por ID. Immediate mode simplifica el flujo de datos pero re-emite todo cada frame.

**2.** `SDL_PollEvent` + `ImGui_ImplSDL3_ProcessEvent`, `ImGui_ImplOpenGL3_NewFrame`, `ImGui_ImplSDL3_NewFrame`, `ImGui::NewFrame`, widgets, `ImGui::Render`, `ImGui_ImplOpenGL3_RenderDrawData`, `SDL_GL_SwapWindow`.

**3.** Crea el estado global (ventanas, tabla de IDs, estilo, font atlas). Se destruye con `DestroyContext()` al final, después de apagar los backends.

**4.** Cuando la ventana está minimizada o no vas a dibujar nada: mejor no llamar a `NewFrame` que llamarlo y no renderizar. `EndFrame` sin `Render` desperdicia CPU y puede desincronizar el reloj.

**5.** Es la relación píxeles/punto lógico del framebuffer. En Wayland con escalado fraccional `DisplaySize` (puntos) y el tamaño en píxeles no coinciden; el picking y los viewports deben usar el espacio correcto.

**6.** Colisionan: comparten ID y por lo tanto estado (hover, foco, activo). Se arregla con labels distintos (`"Vida##a"`, `"Vida##b"`) o con `PushID("a")`/`PopID()` alrededor de cada uno.

**7.** Apila el hash de la dirección del puntero. Es estable mientras el objeto viva y evita colisiones entre objetos distintos aunque el índice cambie; con índices, borrar una entidad puede hacer que el inspector edite otra.

**8.** Cada fila necesita un ID propio. Sin `PushID(fila)`, todos los `Selectable`/`TreeNode` del bucle comparten estado y se abren, seleccionan o arrastran juntos.

**9.** Devuelve true en el frame en que se suelta el mouse sobre el widget (click completo). Mientras está apretado devuelve false, y si soltás afuera tampoco devuelve true.

**10.** Que ImGui consumió el mouse: el juego no debe disparar, rotar la cámara ni hacer picking con ese click. Lo mismo aplica a `WantCaptureKeyboard` para el teclado.

**11.** Se elige una entidad por ID estable; se consulta su máscara de componentes; por cada bit presente se muestra un panel con sus campos; los widgets editan el componente en el lugar y marcan dirty al terminar la edición.

**12.** `DragFloat` devuelve true en cada píxel del arrastre. Marcar dirty sólo al soltar (`IsItemDeactivatedAfterEdit`) produce un cambio por gesto, no cien entradas de undo.

**13.** `ImGuiTreeNodeFlags_OpenOnArrow` (click en el texto selecciona), `ImGuiTreeNodeFlags_Leaf` para hojas y `ImGuiTreeNodeFlags_Selected` para resaltar la selección. `IsItemClicked()` + `IsItemToggledOpen()` distinguen click de toggle.

**14.** El source declara el tipo y copia el payload (`SetDragDropPayload`); el target valida el tipo con `AcceptDragDropPayload` y decide si la operación es legal. El payload se copia a un buffer interno, no guardes punteros al stack.

**15.** Porque se formaría un ciclo en el grafo: el nodo pasaría a ser descendiente de sí mismo y el recorrido recursivo nunca termina (o el world matrix entra en un ciclo de dependencias).

**16.** Filtro de texto y por tipo, virtualización de filas (`ImGuiListClipper`), estado de carga visible y no emitir un widget por asset si hay miles.

**17.** Frame time es el tiempo entre presents (incluye vsync/espera); CPU frame es lo que tardó tu código; GPU frame es lo que tardó la GPU. El GPU frame llega uno o más frames tarde porque la GPU todavía no terminó cuando la CPU quiere el resultado.

**18.** Un promedio bajo puede esconder un frame de 300 ms. Se agregan p95/p99, máximo y conteo de spikes (frames que superan un umbral, por ejemplo 2x el presupuesto de 16.67 ms).

**19.** p95: `ceil(0.95*20)-1 = 18` (la muestra 19 de 1..20). p50: `ceil(0.50*20)-1 = 9` (la muestra 10). El índice es 0-based.

**20.** Un frame anormalmente largo respecto del presupuesto. El umbral típico es 2x el frame budget (33 ms a 60 FPS), ajustable según el género y la tolerancia.

**21.** `current` son los bytes vivos ahora; `peak` es el máximo histórico de esa categoría. El peak no baja porque sirve para presupuestar el peor momento, aunque ya se haya liberado.

**22.** Churn es allocar y liberar muchas veces el mismo tipo/tamaño por frame. Se detecta porque el contador de operaciones crece mucho más rápido que los bytes vivos (o que el peak).

**23.** Porque un free de más es un bug del llamador; si el contador es sin signo, underflowearía a un valor gigante y arruinaría todas las métricas y la UI. Clampear deja el tracker usable y el bug visible en otra parte.

**24.** Con `w0 = A - O`, `a=u·u`, `b=u·d`, `c=d·d`, `d=u·w0`, `e=d·w0`, `denom=a*c-b*b`, `s=(b*e-c*d)/denom`. Si `denom ~ 0`, la recta y el rayo son paralelos y el punto no está bien definido.

**25.** `denom = dot(dir, n)`; `t = dot(punto - origen, n)/denom`; el punto es `origen + dir*t`. No hay intersección si `|denom|` es casi 0 (paralelo) o si `t < 0` (plano detrás).

**26.** `round(valor/paso)*paso`. Si el paso es <= 0 se devuelve el valor sin tocar (evita división por cero y resultados absurdos).

**27.** Usa el modo de la key IZQUIERDA del tramo (step, linear o smooth). Antes de la primera y después de la última se clampea al valor del extremo; no se extrapola.

**28.** H=120°, S=1, V=1 es verde puro: RGB (0,1,0). El picker usa HSV porque H (tono), S (saturación) y V (brillo) son más intuitivos para elegir que tres canales RGB correlacionados.

**29.** Es una tabla nombre→valor con edición in-place. Read-only evita editar campos derivados; dirty/revert permite cancelar cambios; el filtro encuentra campos en estructuras grandes. También facilita undo por gesto.

**30.** `atan2(dy, dx)` da el ángulo del mouse alrededor del centro del gizmo; al cruzar 360°→0° hay un salto que hay que compensar sumando/restando 2π para que girar varias vueltas acumule el ángulo real.

**31.** Mantiene memoria acotada y conserva las últimas N líneas, que son las relevantes cuando algo falla. Se pierde el historial viejo; se gana previsibilidad y no crecer sin techo.

**32.** Primero nivel mínimo, después categoría exacta, después búsqueda sin distinguir mayúsculas en texto o categoría. El orden no cambia el resultado si los filtros son independientes, pero la búsqueda se hace al final para no recorrer texto de líneas ya descartadas.

**33.** Es una ventana de espera desde el primer evento para no leer archivos a medio escribir. Los avisos repetidos no deben reiniciar el timer porque una ráfaga de guardado podría posponer la recarga indefinidamente (coalescencia).

**34.** Porque el frame en curso puede estar usando el recurso (textura bindeada, malla en un command buffer). Cargar el nuevo, cambiar el handle y liberar el viejo en un punto seguro evita usar memoria liberada o mezclar versiones.

**35.** `master` es la rama estable sin docking; `docking` agrega dockspace y multi-viewport. Este módulo fija `v1.91.5-docking` (tag verificado en el repo oficial). Con `master`, `DockSpaceOverViewport` no existe.

**36.** Crea un nodo raíz de dockspace que ocupa el viewport principal. `PassthruCentralNode` deja el hueco central transparente para ver el render 3D detrás.

**37.** `io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable` (y `DockingEnable`), backends con soporte de viewports, y después de `ImGui::Render()` llamar `UpdatePlatformWindows()` + `RenderPlatformWindowsDefault()` y restaurar el contexto GL de la ventana principal.

**38.** En Wayland el cliente no controla la posición global: el compositor decide. El ini guarda posiciones que el compositor puede reinterpretar; en X11 sí se respetan.

**39.** El hash exacto detecta cualquier cambio pero falla entre GPUs/drivers por diferencias de redondeo. En CI heterogéneo usá RMS (o PSNR) con tolerancia, y el bbox para saber dónde cambió. El hash sirve para la misma máquina/driver.

**40.** Renderizá a un FBO de tamaño fijo (independiente de la ventana), fijá la fuente y la escala, deshabilitá animaciones o sembrá el reloj, y compará siempre las mismas dimensiones.
