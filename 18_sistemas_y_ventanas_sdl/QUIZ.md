# 📝 QUIZ — Módulo 18: Sistemas y Ventanas con SDL2 y SDL3

*Respuestas al final. Antes de mirar, intentá contestar de cabeza; después
contrasto con las lecciones (cada pregunta nombra la lección de respaldo).*

1. **P/I** (L01): ¿Qué tiene que devolver `SDL_Init` en SDL3 y qué devolvía en SDL2? ¿Qué significa cada valor?
2. **P/I** (L01): `SDL_RenderCopy` y `SDL_RenderCopyEx` no existen en SDL3. ¿Cuáles son sus reemplazos exactos y qué *tipo* de dato nuevo aceptan para las coordenadas?
3. **P/I** (L01): El flag SDL2 `SDL_WINDOW_ALLOW_HIGHDPI` se llama distinto en SDL3. ¿Cuál es el nombre nuevo? ¿El valor de bits cambió?
4. **P/I** (L01): ¿Qué devuelve `SDL_GetBasePath` en SDL3 respecto de la propiedad de la memoria del string, y qué diferencia hay con SDL2 (¿qué libera quién)?
5. **P/I** (L01): SDL_QueueUserEvent fue removido. ¿Cuál es la forma de pasar datos desde un thread de fondo al main loop en SDL3?
6. **P/I** (L01): La version de headers y la de la lib linkeada pueden diferir. ¿Qué función te da la versión de la lib a TIEMPO DE EJECUCIÓN en SDL3?
7. **conceptual** (L02): ¿Cuál es el ORDEN de creación y de destrucción de (SDL_Init, SDL_CreateWindow, SDL_CreateRenderer) y por qué el dtor va en reversa?
8. **código** (L02/L03): Si `SDL_CreateRenderer` falla, ¿se puede quedar la ventana viva? Lucho>CreateWindow OK, CreateRenderer null. ¿Qué debo hacer a mano — y qué hace por vos SDL3 si te olvidás?
9. **P/I** (L03): ¿Para qué usa cada uno `SDL_SetRenderVSync` y por qué llama una sola vez al inicio? ¿Qué pasa con el frame-time medido si dejo vsync ON y comparo timings?
10. **P/I** (L03): ¿Por qué el flag `SDL_WINDOW_VULKAN` no es el default y cuándo sobra? (imágenes de módulos 25/26: no dibujes una surface SDL2 sola.)
11. **código** (L04): Tenés un `SDL_EVENT_KEY_DOWN` con `repeat == true` — *qué NO es eso* y cómo el evento repetidor de la app (tu binder móvil, ej 02).
12. **código** (L04/L05): ¿"Drenar todo" frase signfica? TD: un loop while SDL_PollEvent en un render (por frame) y luego simulación. ¿Por qué es pid en el timestep del módulo 16?
13. **P/I** (L05): ¿Cuál es la diferencia entre `SDL_EVENT_TEXT_INPUT` y `SDL_EVENT_TEXT_EDITING`? ¿Qué contiene `.text.text` exactamente (format, límite)?
14. **P/I** (L05): ¿Qué les pasó al input de SDL2 `SDL_SetTextInputRect` en SDL3? (new name + nuevas features).
15. **P/I** (L05): ¿En SDL el mapeo gamepad es "el botón X del jugador" o abstracto? Nombrate los 2 botones '~' del caro NAO.
16. **P/I** (L06): ¿Por qué `SDL_FRect` es más idiomático para tu render que `SDL_Rect` en SDL3, y cuándo sí conviene el `int`? (conectá con el ej 03.)
17. **código** (L06): El "tint" de daño (flash rojo) del módulo 17 con raylib era `SetTextureColorMod` del SDK mismo chico de SDL: ¿cómo lo harías en SDL3 y qué componente de color es la base?
18. **código** (L07): ¿Qué es un "batch" mate, y cuándo NO se mergea si hay items de la MISMA textura separados en el medi disco? (hint: Capas.)
19. **código** (L08): Con SDL_AudioSpec{48000, 2 canales, F32}, ¿cuántos bytes ocupa UN frame y cuántos bytes tiene EXACTAMENTE 1 segundo? (cuenta directa de la lección/es las fórmulas del ej 05.)
20. **P/I** (L08): ¿Qué hace SDL_AudioStream cuando ponés datos con un formato distinto al del device (ej: S16 44.1 kHz mono contra un device F32 48 kHz stereo)?
21. **P/I** (L10): ¿Por qué el callback de audio corre en un thread distinto y qué operaciones ESTÁN permitidas dentro? (lechas del mismo: push/pop to stream / renderer / allocate).
22. **P/I** (L10): ¿Diferencia entre SDL_WaitThread y SDL_DetachThread? ¿Qué pasa si te olvidás de esperar y el handle quedó "suspendido"?
23. **código** (L10): La cola de trabajo del ej 07 hace notify_all en cerrar(). ¿Por qué ALL y no uno? ¿Qué pasaría con notify_one si hay N>1 consumidores esperando?
24. **P/I** (L11): `SDL_GetWindowDisplayScale` y `SDL_GetWindowPixelDensity` parecen lo mismo. ¿En qué se diferencia exactamente uno del otro?
25. **conceptual** (L11): ¿Por qué en Wayland no podés "fijar ventana (x,y) global" como en X11, y qué opciones tenés (usar cerca de Escritorio tu app SDL3)?
26. **P/I** (L11): ¿La CIERRE del port "Asteroids" del módulo 16 con SDL3? (¡enunciado del módulo!) ¿Qué piezas del 16 serían 1:1 (timestep fijo, estados, SoA, action mapping) y cuáles TARGET SDL pegado? (renderer, eventos, audio)
27. **arquitectura** (todas): `Binder` del ej 02 no incluye SDL de ninguna. ¿Qué beneficio tiene mantener la LÓGICA en tipos tuyos y SDL en "el margen"? CTA-full: ¿con qué OTRO módulo (20/22/24) el patrón manda 1:a1?

---

## 🎯 Respuestas

> Las respuestas están en orden y remiten a la lección. En el código: #if 0
> para comparar tu respuesta, y todos los drills del ejercicio ya prueban esto.

1. **(L01)** SDL2: `int` — `0` OK, negativo = error y setea `SDL_GetError`. SDL3:
   `bool` — `true` OK, `false` = error. La moral: menos semántica ambigua
   (ya no hay `0` que a veces significa éxito y en otra API es error).

2. **(L01)** `SDL_RenderCopy` -> `SDL_RenderTexture`; `SDL_RenderCopyEx` ->
   `SDL_RenderTextureRotated`. Ambos reciben `SDL_FRect` (x, y, w, h como
   float) en lugar de `SDL_Rect` (int), y devuelven bool en vez de int.
   (Además ya tenés `SDL_RenderTextureTiled` que no existía en SDL2.)

3. **(L01)** Se llama `SDL_WINDOW_HIGH_PIXEL_DENSITY`. El valor es el MISMO:
   | 0x2000. El cambio de nombre es semántico: la ventana pide un back
   buffer con alta densidad de píxeles (SDL3 considera densidad, no "dpi").
   Extra: los flags pasan de Uint32 (SDL2) a Uint64 (SDL3, SDL_UINT64_C).

4. **(L01)** SDL2 devolvía `char*` propio que HABÍA que liberar con
   `SDL_free`. SDL3 devuelve `const char*` cacheado QUE ES PROPIEDAD DE SDL:
   NO se libera (liberar sería doble-free de SDL3). `SDL_GetPrefPath`
   sigue devolviendo un string tuyo a liberar (en ambas versiones).

5. **(L01, L10)** `SDL_PushEvent(&evento) -> bool`: armás el SDL_Event con
   tipo propio (rango SDL_EVENT_USER + o predefinidos) y el main loop lo
   recibe por la misma cola de PollEvent. O la cola thread-safe propia del
   ej 07 si no querés eventos (ambos patrones son válidos; el ej combina).

6. **(L01)** `SDL_GetVersion()` devuelve un `int` codificado
   (SDL_VERSIONNUM: 3004018 = 3.4.18) que decomponés con
   SDL_VERSIONNUM_MAJOR/MINOR/MICRO. La macro `SDL_VERSION` (sin struct)
   es la de compilación. En SDL2 era un struct SDL_version con SDL_VERSION(&v).

7. **(L02)** Creación: `SDL_Init` -> `SDL_CreateWindow` ->
   `SDL_CreateRenderer`. Destrucción EXACTA en reversa:
   `SDL_DestroyRenderer` -> `SDL_DestroyWindow` -> `SDL_Quit`.
   El motivo es el ownership: el renderer vive SOBRE la ventana (usa su
   surface/contexto); si la ventana se cerrara primero, el renderer queda
   apuntando a un recurso muerto. Ver comentarios en la solución 01.

8. **(L02/L03)** No: el código correcto destruye la ventana YA (con try_
   RAII-wrapper tipo __invoke_ctor failure guard). Si te olvidás... SDL3 lo
   tolera: al destruir la ventana, el renderer asociado se libera tb
   (SDL libera la ventana + su renderer interno si no fue liberado). PERO
   depender de eso es un anti-pattern de RAII: destruime exclusivamente tú.

9. **(L03)** `SDL_SetRenderVSync(ren, 1)`: sincroniza el Present al vblank
   del monitor (tearing: nada). Se usa UNA VEZ porque es una PROPIEDAD
   del renderer (cambiarlo cada frame angenta el código y no tiene efecto
   visible). Si VSync va ON, el Present BLOQUEA: los medidos frame-times
   pasan a reflejar el reloj del MONITOR, no el cost del frame del juego:
   para PROFILABA apagalo (prohibición del mismo para hablar de trabajo).

10. **(L03)** `SDL_WINDOW_VULKAN` solo hace falta si vas a crear TU onTap
    surface de Vulkan propia (lecciones 26+). El SDL_Renderer default usa
    OpenGl directamente (sin flag). En Vulkan: SDL_CreateWindow necesita el
    flag ANTES de hacerlo (SDL NO lo activa on the fly); además el Vulkan
    instance debe estar ya creado antes de pedir la surface igual (L26).

11. **(L04)** `repeat == true` NO significa "está presionada" sino "es UN
    REPEAT del evento_key_down" (cuando el SO tiene autorepeat, la tecla
    estaba SIN soltar y el SO re-manda SDL_EVENT_KEY_DOWN). El binder del
    ej 02 IGNORA repeats: una sola vez por pulsación real.

12. **(L04/L05/L16)** "Drenar todo" = while(SDL_PollEvent(&e)) hasta que
    devuelve false: saca TODA la cola acumulada en el frame. Está primero
    (y UNA vez) por frame; entonces la SIMULACIÓN con timestep fijo corre
    determinista (el input quedó ya destilado en acciones, no M covía
    los milisegundos del tic interno). Si leeras events en el tic, el
    resultado dependría del orden input/física: input mapping del 16.

13. **(L05)** `SDL_EVENT_TEXT_INPUT`: texto TERMINADO (el usuario tocó una
    tecla que produce algo final). `SDL_EVENT_TEXT_EDITING`: estado de
    COMPOSICIÓN (IME activo con preedit para japo/chino/koreano).
    `.text.text` es char[32] en codigación UTF-8 (multi-byte: más de UN
    byte por carácter acentuado; por eso el backspace del ej L05 poppea
    bytes de continuation 0x80..0xBF completos).

14. **(L05)** `SDL_SetTextInputRect(rect)` (global) -> `SDL_
    SetTextInputArea(window, rect, cursor)`: por VENTANA y con posición
    del cursor: el editor IME (japonés, etc.) coloca su popWindow cerca
    del punto exacto de cuidado, no en el borde casi global de todo.

15. **(L05)** Abstracto: SDL3 mapea el layout del mando a nombres idiom
    SDL_GAMEPAD_BUTTON_SOUTH (V de XO/PS?). Se repite del módulo 17 aprende
    la map '_' de espacios ( rl_South no la letra literal). EJEMPLO det in lib.

16. **(L06)** En SDL3 la API "fina/smodernizada" (SDL_RenderTexture y otras)
    acepta SDL_FRect: el rasterizador recorta con subpixel precision en
    float, y si tu juego mueve en floats (todo lo sliding de 16/17) NO
    truncás al portar cada sprite y no tenés el jitter de SSR. SDL_Rect
    (int) es cómo para layouts de píxeles YA FIJADOS (o gí no confiné sub
    pixel: JUNTAR el salto de rmaAS).

17. **(L06)** Igual que en raylib tackled de color mod: `SDL_SetTextureColorMod(tex, r, g, b)`
    (o SDL_SetRenderTextureColorMod()/texture Alpha). El modulation ve y
    RGB tint con CADA pixel dibujado (damage flash → r alto, g/b maja).
    La base MNSEL es el product componente del blit framebuffer y la
    liquidación del FRect: color mod multiplica y NO "tinte HDR".

18. **(L07)** Un batch = corrida CONTIGUA de items de la MISMA textura en
    la lista reordenada: un solo draw call (un bind de textura) para N
    quads. Si dos items de la misma textura tienen ENTRE MEDIO items de
    OTRA textura (o cruzan un a línea de capa), NO se mergean: hay que
    lintargar el corte (SDL draws: mismo order).

19. **(L08)** Con SDL_AudioSpec{48000, 2 ch, F32}: bytes/frame = 2 ch × 4 B
    = **8 bytes**. Por segundo: 48000 frames × 8 = **384,000 bytes** que
    era TODO el continuo seguido, de la fuente (CD stereo era al doble):
    agradecé x seconds der Yellowboatj: formatos S16 en la mitad (192000 B/s
    mono 48kHz ya mostrái el update: F32 SIEMPRE m particularly pkg mm.)

20. **(L08)** El SDL_AudioStream convierte TODO: formato (S16 → F32),
    frecuencia (44.1k → 48k por resampleo) y canales (mono → stereo
    duplicando samples). Se configura con SDL_SetAudioStreamFormat(src,
    dst) y se puede cambiar AL VUELO (el conversoir se reenlaza); vos solo
    infestás con SDL_PutAudioStreamData bytes en TU formato: SDL es
    constante responsable tanto de la mama (little endian Int interblock).

21. **(L10)** Callback de audio corre EN el device thread (SDL lo invoca;
    el sistema del SO exige "cómete bytes YA"): SOLO operaciones rápidas y
    thread-safe del stream (SDL_PutAudioStreamData está PERMITIDO; es el
    patrón oficial) y NO el renderer/window (GUI no es thread-safe y
    cuelgas el device). Regla: trabajo pesado -> main loop u otras cols.

22. **(L10)** `SDL_WaitThread(thread, &status)`: BLOQUEA al invocador
    hasta que el thread termina y LE PASA el resultado, y CONSUME el
    handle (lo libera: no lo uses después). `SDL_DetachThread`: desprende
    el thread (se auto-libera al terminar; NO podes capturar status).
    Si olvidás esperar/detach: leak del handle (como un std::thread
    destruible con cola sin join: std::terminate de módulo 14).

23. **(L10)** notify_all: hay N consumidores esperando y SOLO uno puede
    tomar trabajo; el resto debe darse cuenta de "cerrada" y salir con
    pop false. Con notify_one: UN consumidor despierta, drena; los N-1
    siguen DORMIDOS = deadlock al cerrar (nadie más recibe la señal).
    El patrón "cerrar + notify_all" del ej 07 es el estándar C++ (moFDs
    del módulo 14: el ~scope llama notify_all por la misma frase).

24. **(L11)** `SDL_GetWindowDisplayScale`: la ESCALA del display (escala
    de plataforma/compositor: 1.0, 1.25, 1.5, 2.0...): es un DATO del
    MONITOR/compositor; sirve para decidir tamaños de UI. 
    `SDL_GetWindowPixelDensity`: RATIO entre los PIXELS del back buffer y
    el tamaño lógico de ESA ventana: 1.0 = 1 pixel por punto, 2.0 = 2.
    Si activaste HIGH_PIXEL_DENSITY: density > 1 puede ser distinto del
    display scale (el back buffer puede ser otra cosa que le pediste).

25. **(L11)** Wayland, por diseño, prioriza seguridad/privacidad: los programas
    NO deciden posiciones absolutas globales (el COMPOSITOR manda: es el
    motivo de que las "posiciones exactas de ventana" no existan).
    Opciones: dejar que el compositor decida (el default, funciona),
    pedir una posición RELATIVA/centro (SDL_ShowWindowSystemMenu /
    SDL_SetWindowPosition se convierten en sugerencias que el compositor
    puede seguir o no);
    para full-control legado: forzar X11 con `SDL_VIDEODRIVER=x11`
    (forzás el driver y va por XWayland igual; alternativa oficial y
    bien soportada si el sistema tiene XWayland).

26. **(cierre)** Port "Asteroids" del 16 a SDL3: MIGRA piezas 1:1:
    timestep fijo + interpolación (16), pila de estados (16), entidades
    SoA (16), action mapping (el binder del ej 02 ENCAJA ALLÁ: mismos
    conceptos). Piezas que AHORRA SDL: la ventana/renderer pasan de
    raylib a SDL_CreateWindow+SDL_CreateRenderer (L02/L03); los eventos
    de raylib Polling pasan a cola SDL (L04/L05, el binder); los recursos
    pasan de Texture2D de raylib a SDL_Texture RAII (L06); trivial: los
    sprites con rotación usan SDL_RenderTextureRotated (L07); el audio
    de raylib pasa al modelo streams (L08). El MAIN con callbacks SDL
    (SDL_AppInit/AppIterate) puede ser la entry point portable: la lección
    1 lo menciona y el enunciado del cierre del módulo apunta ahí.

27. **(arquitectura)** El Binder vive en LÓGICA (tipos propios), y SDL es
    UNA implementación del "margen" (adapters). Beneficios: tests SIN
    ejecutar SDL (rápidos, CI-friendly, deterministas); el mismo core
    funciona en raylib/SDL/OpenGL (las Apps mód 25/26 cambian SOLO la
    parte del margen); se pueden FAKEAR inputs raros (repeat, combos,
    IME) sin instalar nada. El PATRÓN técnico 1:1 aparece en el módulo
    20 (renderer OpenGL 3.3 detrás de una interfaz propia) y el módulo
    24/26 CUDA (la GPU detrás de una interfaz propia y sin backend
    intermedio que dependa del SO, MISMO replanteo de separación).

> El "CTA-full" del punto 27: la frase completa sería... yours to define!
> (bromita: releé los ej: el margen = adapters; el centro = lógica pura.)
