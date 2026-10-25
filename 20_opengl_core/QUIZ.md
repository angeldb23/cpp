# 📝 QUIZ — Módulo 20: OpenGL 3.3 Core (pipeline, shaders y buffers)

*Respuestas al final. Antes de mirar, intentá contestar de cabeza; después
contrastá con las lecciones (cada pregunta nombra la lección de respaldo).
El cierre del módulo ES el contexto/los helpers: gl_util.hpp con 18
lecciones, 9 ejercicios resueltos y este quiz. Nada acá abre ventanas: el
código GL se verifica POR COMPILACIÓN; la lógica (los 9 ejercicios) corre
en la CPU.*

1. **código** (L01): ¿Qué atributos se piden con `SDL_GL_SetAttribute` ANTES de abrir la ventana (¿major, minor, profile, flags?) y por qué DESPUÉS de crear el contexto hay que VALIDAR con `SDL_GL_GetAttribute` la versión que quedó (¿qué le pasó al "pediste 3.3"?).
2. **P/I** (L01): ¿Qué función libera el contexto y en qué ORDEN se cierra todo (¿contexto antes o después de la ventana?)? ¿Por qué en el perfil CORE no existe `glBegin/glEnd` ni las matrices fijas?
3. **conceptual** (L02): Enumerá las etapas del pipeline 3.3 core del vértice al píxel (¿y cuáles son PROGRAMABLES vs fijas?) y cuáles son las DOS que sí o sí escribís vos.
4. **código** (L02): El vertex shader mínimo escribe `gl_Position = vec4(aPos, 1.0)`: ¿por qué el w es 1.0 y qué haría un 0.0 (¿cómo se conecta esto con el `w <= 0` del módulo 19)?
5. **P/I** (L03): ¿Qué es un VARYING (la `out` del vertex emparejada con la `in` del fragment) y por qué el fragment NO ve el valor exacto del vértice (¿quién interpola y con qué pesos?)? ¿Quién escribe un `uniform` aquí y cuándo?
6. **código** (L03): ¿Por qué `glGetUniformLocation` se llama UNA vez tras el link y se cachea? ¿Qué devuelve si el compilador eliminó el uniform (y por qué NO la rompe nada)?
7. **código** (L04): Contá el flujo del build de shader (¿qué funciones en qué orden, qué consulta dice si compiló, dónde va el LOG?) y por qué el LINK del program puede fallar AUNQUE cada shader compile solo.
8. **código** (L05): ¿Qué habilita el HOT-RELOAD de shaders (¿de dónde sale el código?) y qué chequea el mini-linter del ejercicio 01 antes de dárselo a la GPU?
9. **código** (L05): UBO std140: ¿cuántos bytes consume un `vec3` en un bloque std140 y con qué regla se alinea el siguiente miembro? ¿A qué se lo subís (¿qué buffer y qué binding point) y a cuántos shaders impacta de una vez?
10. **conceptual** (L06): ¿Qué guarda cada contenedor (VBO, EBO, VAO) y por qué el VAO "graba" la asociación vértices↔buffer en el momento de los `glVertexAttribPointer`? ¿Qué diferencia hay entre `glGenBuffers` y `glBindBuffer`?
11. **código** (L07): `glBufferData` vs `glBufferSubData`: ¿qué hace cada uno, qué uso tiene cada uno y QUÉ COSTO pago si llamo glBufferData dentro del update de cada frame?
12. **código** (L07): STATIC_DRAW vs DYNAMIC_DRAW vs STREAM_DRAW: describí el patrón de uso que cada hint declara y qué pasa si MENTÍS (¿crashea o qué?).
13. **código** (L08): Vértice interleaved pos3+uv2+color4 (float + float + ubyte): ¿cuánto vale el STRIDE y cuáles son los OFFSETS de cada atributo? ¿Por qué el último parámetro de glVertexAttribPointer es `const void*` y qué valor le paso (¿el cast `(void*)12`?).
14. **código** (L08): ¿Qué hace `normalized = GL_TRUE` sobre un atributo GL_UNSIGNED_BYTE y por qué no hace nada útil sobre un float? ¿Dónde se asigna la `location` de un atributo en 3.3 core (¿en el shader con layout o previo al link?).
15. **P/I** (L09): ¿Por qué stb_image exige `#define STB_IMAGE_IMPLEMENTATION` en EXACTAMENTE un .cpp del build (¿qué genera el header sin el define?) y qué le PASA `stbi_load` (¿quién devuelve width/height/canales, qué fuerza el último parámetro, qué dice una falla)?
16. **codigo** (L09): ¿Con qué llamada GL subís los bytes de la imagen a la VRAM (¿internalformat vs format vs type?) y por qué `stbi_image_free` va INMEDIATAMENTE DESPUÉS de esa llamada GL? (¿Dónde vive la textura después?)
17. **código** (L10): ¿Qué interpolación hace GL_LINEAR_MIPMAP_LINEAR (¿entre texels y entre mips?) y por qué el NEAREST mipmap se "ve saltos" al caminar? ¿Cuántas veces se llama `glGenerateMipmap` y cuándo (¿por qué no por frame?).
18. **código** (L10): ¿Qué pasa si la textura es NO POTENCIA DE 2 en 3.3 core con wrap REPEAT (¿permitted o NO?), y por qué el CAMINO SEGURO del curso es CLAMP_TO_EDGE para atlas y HUD?
19. **código** (L11): ¿Qué hace exacto `glEnable(GL_FRAMEBUFFER_SRGB)` (¿de quién a quién convierte el color: shader lineal vs gamma del monitor?) y POR QUÉ lo habilito SOLO en el pase final (¿qué bug doble-gamma produce si no?).
20. **conceptual** (L12): El estado GL es GLOBAL (un solo "activo" de cada vez): ¿cuáles DOS bugs clásicos produce? (¿Bind fantasma de textura? ¿Blend que quedó activado desde la UI?) ¿Qué patrón de la L13 lo aborda?
21. **código** (L13): ¿Qué guarda el cache de estado y por qué el delta con VALORES ABSOLUTOS es IDEMPOTENTE (¿el Roundtrip del ej 06?) mientras un delta de "toggles" podría acumularse?
22. **conceptual** (L14): ¿Qué COSTO paga el CPU con 6 sprites de 6 texturas distintas dibujados en 6 draw calls (¿qué comandos son 6 de cada uno?) y cómo lo baja el INSTANCING (¿un draw con N instancias, un buffer de instancias con divisor?).
23. **código** (L14): ¿Por qué el batcher del ej 07 usa sort ESTABLE (¿qué rompería un inestable con el z-order de gameplay?) y qué EXACTAMENTE es un RANGO = una draw call?
24. **código** (L15): ¿Qué ES un FBO y con qué se COMPLETA para render-to-texture (¿textura color como GL_COLOR_ATTACHMENT0 + renderbuffer de depth?) y por qué leer y escribir LA MISMA textura en el mismo draw es undefined behavior (¿feedback loop?).
25. **P/I** (L15): ¿Por qué el pase final pinta UN QUAD fullscreen y "mueve" la textura del FBO a la pantalla (¿qué es el backbuffer/default framebuffer, y por qué el QUAD es el único draw del pase post?)?
26. **código** (L16): ¿Qué hace glBlitFramebuffer (¿regiones del FBO READ_FRAMEBUFFER → WRITE_FRAMEBUFFER, con qué filtro?) y cómo es el "resolve" de un FBO MSAA (¿de multisample a normal en 2 pasos)?
27. **código** (L16): ¿Qué memoria pide `glRenderbufferStorageMultisample` (¿samples por píxel, clamped a GL_MAX_SAMPLES por el sampleValido del ej 08, potencia de 2?) y qué significa samples = 0 (¿y por eso el ej devuelve 0 = "usa FBO simple"?).
28. **P/I** (L17): ¿Qué brinda KHR_debug (callback de mensajes de debug del driver) y CÓMO se carga en 3.3 core (¿extensión: SDL_GL_GetProcAddress + typedef PFNGL...)? ¿Qué particularidad tiene glGetError (¿UN flag leído = limpiado; el bucle de "drenar errores")?
29. **código** (L17): ¿Cuál es la secuencia completa de una timer query GL_TIME_ELAPSED (¿genQuery → Begin → draw → End → glGetQueryObject en cuál frame?) y por qué el resultado NO está disponible en el mismo frame (¿la GPU es asíncrona del CPU)?
30. **código** (L18): ¿Qué estructura entrega tinyobjloader (¿attrib.vertices plano + shapes[].mesh.indices por ESQUINA, y qué 0-based normalization ya VIENE desde el loader) y qué complejidad del formato REAL quedó fuera del mini parser del ej 09 (¿mtl? ¿ngons? ¿índices negativos?) y por eso usamos el loader?

---

## 🎯 Respuestas

> Las respuestas están en orden y remiten a la lección. Todo lo citado de
> SDL/GL fue verificado contra los headers locales (`SDL3/SDL_video.h`,
> `GL/glcorearb.h`, `GL/glext.h`) durante la construcción; las cifras de
> tests salen de los 9 ejercicios, todos con `./verificar.sh 20` OK.

1. **(L01)** ANTES de abrir la ventana: `SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3)` +
   MINOR 3 + `SDL_GL_CONTEXT_PROFILE_MASK = SDL_GL_CONTEXT_PROFILE_CORE` +
   `SDL_GL_CONTEXT_FLAGS = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG`.
   Después de `SDL_GL_CreateContext` se consulta con `SDL_GL_GetAttribute`
   la versión que HAY: la versión pedida es una sugerencia — el driver entrega lo
   que puede (lo que expone), y un objeto shader escrito "para 3.3" sobre
   un contexto menor revienta en el compile. Por eso L01 valida y chapa
   (si la versión no es la esperada no continúa).

2. **(L01)** `SDL_GL_DeleteContext` libera el contexto; el orden de cierre
   es el inverso de la apertura: delete contexto → destroy ventana →
   SDL_Quit (en gl_util.hpp eso es glu::cerrar). Core profile QUITÓ el
   acceso al pipeline fijo de llamada inmediata (glBegin/glVertex,
   glRotate, la matriz model-view): no existe el acceso — por eso el
   curso entero programa shaders y graba buffers: en core ESAS funciones
   YA NO EXISTEN (ni siquiera por "deprecated": el glcorearb 3.3 core no
   las declara).

3. **(L02)** Recorrido: vertex shader (PROG) → tessellation (no en 3.3) →
   geometry (opcional) → clipping → rasterizador (FIJO: tris → fragments
   con interpolación) → fragment shader (PROG) → depth/stencil test (FIJO)
   → blending (FIJO) → framebuffer. Las DOS programables que SIEMPRE
   escribís: vertex y fragment. El resto es pipeline fijo que se CONFIGURA.

4. **(L02)** El W del clip = 1 marca el vértice como PUNTO: la proyección
   divide por w (módulo 19) y con 1 no colapsa nada. Con w=0 es una
   DIRECCIÓN: la perspectiva divide por cero (1/0 infinito) y el test
   `w <= 0` (el plano de la cámara) CLIPPEA el vértice: el triángulo
   entero desaparece. El fragmento entrega `out vec4` RGBA al target
   activo tal cual (o convertido a sRGB si GL_FRAMEBUFFER_SRGB está habilitado: L11).

5. **(L03)** Varying: la `out` del vertex con la `in` homónima del
   fragment (mismo NOMBRE y TIPO): el RASTERIZADOR interpola el valor por
   fragmento con los pesos baricéntricos dentro del triángulo (el
   fragmento ve el "color local" del punto, no el valor EXACTO del vértice).
   Un `uniform` NO varía entre invocaciones dentro del draw: lo escribe
   EL CPU con glUniform* (cámaras, luces, colores del UI): el mismo valor
   para todos los fragmentos del draw.

6. **(L03)** glGetUniformLocation recorre la tabla uniforms del programa:
   barato pero innecesario por frame: se llama UNA vez post-link y se
   cachea (la struct Programa del gl_util.hpp la cachea). Si el
   compilador ELIMINÓ el uniform (nadie lo usa = dead code) devuelve -1:
   los glUniform con location -1 son no-op SILENCIOSOS por spec (el curso
   lo usa para poder omitir if por todos lados, sin romper nada).

7. **(L04)** Flujo del shader: glCreateShader → glShaderSource → glCompileShader →
   glGetShaderiv(GL_COMPILE_STATUS) → si falló el log via
   glGetShaderInfoLog (stdout/stderr según el helper; si el log es VACÍO
   igual hubo una falla: algunos drivers fallan sin comentar nada) →
   glDeleteShader del objeto del shader (ya quedó adjuntado al program) —
   y mismo flujo glCreateProgram → glAttachShader → glLinkProgram →
   glGetProgramiv(GL_LINK_STATUS) + glGetProgramInfoLog. El LINK falla
   aparte del compile: in/out con NOMBRE o TIPO discordantes entre vertex y
   fragment, 2 uniforms del mismo nombre con tipos distintos, o un main
   ausente: cada shader compila SOLO, pero el par no encaja.

8. **(L05)** Hot-reload = el shader vive en un ARCHIVO aparte (no un
   string embebido en el .cpp) y el juego recompila cuando el archivo
   cambia (comparando mtime). El mini-linter del ej 01 valida que el par
   vertex/fragment venga limpio ANTES de mandarlo: in/out emparejados por
   NOMBRE y TIPO, `void main` presente en ambos, `#version` primera
   línea, y sin uniforms duplicados con tipo distinto (las causas de
   link failure de la L04 detectadas MANUALMENTE).

9. **(L05)** std140: un `vec3` reserva 16 bytes (redondea al múltiplo de
   16 de su base alignment) y el SIGUIENTE miembro arranca en el próximo
   múltiplo de 16 — por eso el buffer de CPU replica ese padding (replicado
   "a mano" con un array plano de floats). Se sube el bloque a un buffer con
   GL_UNIFORM_BUFFER y `glUniformBlockBinding` lo engancha a un binding
   point: TODOS los programas que declaren el mismo bloque en ese
   binding leen el MISMO byte stream (una subida de cámara sirve N
   shaders).

10. **(L06)** VBO: los BYTES crudos (posición/uv/normal interleaved o no).
    EBO: índices uint que REUTILIZAN vértices del VBO. VAO: la RECETA:
    qué buffer estaba activo en qué binding y qué glVertexAttribPointer se
    aplicó por location — la asociación se GRAVA en el
    VAO EN EL MOMENTO de la call (después queda "conectado por siempre"
    hasta que otro comando la cambie). Gen vs bind: glGenBuffers solo
    REPARTO nombres; glBindBuffer activa el objeto: el glVertexAttribPointer lee el buffer CURRENT del binding
   GL_ARRAY_BUFFER en ese instante.

11. **(L07)** glBufferData (re)ALOCA todo el buffer y sube los datos
    (dimension + datos en un paso): se usa UNA vez al armar el VAO.
    glBufferSubData SUBE A ESE TAMAÑO ya asignado (actualización parcial
    o total sin realocar): es lo que va en el hot path. Llamar glBufferData
    por frame realoca TODO el bloque (el driver puede mover memoria,
    re-sincronizar con la GPU — cuello de botella); el patrón correcto:
    bufferData una vez, SubData/mutación en el frame.

12. **(L07)** STATIC_DRAW: subís pocas veces, dibujás muchas (malla del
    modelo, atlas). DYNAMIC_DRAW: modificás seguido y dibujás muchas
    (partículas largas vivas). STREAM_DRAW: subís y dibujás UNA vez
    (HUD/sprites de este frame). Es un HINT (no un constant): mentir
    NO crashea; el driver coloca el buffer en memoria menos óptima
    (pérdidas de rendimiento, no funcional).

13. **(L08)** El interleaved: pos3 float (12 B) + uv2 float (8 B) +
    color4 ubyte (4 B) = 36 bytes por vértice. El stride
    es 36 y los OFFSETS de cada atributo son 0, 12 y 20 (el último es
    `const void*` por legado de GL 1.x cuando era un PUNTERO DE LA RAM;
    en GL 3.3 con VAO es un OFFSET en bytes dentro del buffer bound y
    se escribe `(const void*)12` con cast — el compilador ve el número,
    pero el tipo del parámetro sigue siendo pointer).

14. **(L08)** normalized=GL_TRUE sobre GL_UNSIGNED_BYTE divide el byte
    0..255 por 255 y entrega float 0..1 en el shader (el COLOR como
    float). Un float ya viene como float ([-1..1] o [0..1]): el normalized NO
    aporta nada (se pone GL_FALSE). La location se
    asigna EN EL SHADER: `layout(location = 0) in vec3 aPos;` — así el
    shader declara dónde va qué (la forma recomendada core; glBindAttribLocation
    es la legacy pre-3.3).

15. **(L09)** stb_image es header-only: SIN el define, el header declara
    solo PROTOTIPOS; con `STB_IMAGE_IMPLEMENTATION` definido en UN SOLO
    .cpp (dos veces = duplicate symbols del linker) el header pasa a
    incluir las DEFINICIONES completas de las funciones.
    `stbi_load(path, &w, &h, &nroCanales, 4)`: los bytes (NULL si falló
    — stbi_failure_reason dice qué pasó); el 4 FUERZA el número de
    canales del resultado (siempre RGBA, aunque el .png viniese RGB: el
    formato típico del GL).

16. **(L09)** Los bytes van al VRAM con
    glTexImage2D(GL_TEXTURE_2D, level 0, internalformat GL_RGBA8, w, h,
    0, format GL_RGBA, type GL_UNSIGNED_BYTE, data): el internalformat
    (formato de LA TEXTURA) y el format+type (formato de LOS DATOS)
    combinan según la tabla del tipo (RGBA/UNSIGNED_BYTE ↔ RGBA8). El
    stbi_image_free(data) va INMEDIATO: la textura VIVE en la VRAM (o la
    caché local del driver); quedarse con los bytes en la RAM después
    es gastar memoria que nadie lee.

17. **(L10)** GL_LINEAR_MIPMAP_LINEAR (trilinear): bilineal DENTRO de
    cada mip + BILINEAL ENTRE las dos mips más cercanas (mezcla según la
    derivada del uv). Con GL_NEAREST el sample es "cajita cuadrada" y
    el pasaje de una mip a la otra SE VE como saltos de nitidez (imaginá
    el LOD del pixel: el LOD cambia de píxel a píxel y camina con el salto).
    glGenerateMipmap UNA vez tras la carga (la GPU construye la pirámide:
    +33% de VRAM contado en el ej 05). Por frame es tontera: el contenido
    de la textura no cambió.

18. **(L10)** En 3.3 core la textura no potencia de 2 ESTÁ PERMITIDA
    (el límite era de 1/2): PERO el acceso no es gratis en todos los casos
    del sampler (repeat + mipmap sobre NPOT puede no tener mips completos
    en hardware muy viejo — y el error saldría en el draw). El curso
    usa CLAMP_TO_EDGE para atlas y HUD y repeat (potencias de 2) para
    texturas de piso/tile: repeat en el borde de la pantalla traería
    texels del lado opuesto de la textura (el artefacto clásico visible).

19. **(L11)** GL_FRAMEBUFFER_SRGB le dice al GL: cuando ESCRIBE al
    framebuffer default, convierte lineal → sRGB (curva ~2.2) — el
    shader SIGUE operando lineal, la salida va corregida para el monitor.
    Solo en el pase final: un FBO intermedio es lineal (blending lineal:
    física OK); habilitarlo TAMBIÉN al escribir al FBO aplicaría la
    conversión DOS veces (colores lavados/oscurecidos de sorpresa — el
    bug clásico de double-gamma).

20. **(L12)** El estado global de GL produce: (1) bind fantasma — el
    último bind de textura sigue activo en el siguiente draw (una sprite
    del HUD con la textura del mundo); (2) estado pegajoso — un sistema
    activó blend (UI, sprites alpha) y el SISTEMA SIGUIENTE (mundo 3D)
    se renderiza CON blend de la UI: todo semi-transparente de sorpresa.
    El state-manager de L13 lo "aplaca": cada sistema emite su estado
    COMPLETO como comandos; el delta-cache MANDA SOLO las diferencias.

21. **(L13)** El cache guarda el estado ABSOLUTO (viewport w/h, blend on/
    off, ztest, cull como números). El delta emite comandos con valores
    finales absolutos ("viewport 0 0 800 600", "blend on"): mandarlos
    DOS veces deja el MISMO estado (idempotente — el test Roundtrip del
    ej 06). Un delta de toggles ("toggle blend") acumularía: dos mandos = DOS flips = distinto estado (por eso el curso usa absolutos).

22. **(L14)** Con 6 sprites de 6 texturas en 6 draws el CPU paga 6× cada
    comando: 6 glBindTexture, 6 glUniform (matrices/uv), 6 updates de
    buffer, 6 draws: todas llamadas al driver que (en el peor caso)
    empantanan el CPU en validación y le roban hueco a la GPU. INSTANCING: UN draw
    (glDrawArraysInstanced) con N instancias: la geometría del QUAD es
    1; el buffer de instancias lleva transform/uv por instancia y el VS
    lo lee con divisor 1 (los atributos normales corren UNA vez por
    instancia; el de instancia UNA vez por instancia).

23. **(L14)** Sort estable: sprites de la MISMA textura quedan en el
    orden de SUBMISSION (el z-order que el juego decidió para sprites de
    igual profundidad); un sort inestable puede reordenarlos distinto
    entre frames (el glitch visible: elementos que "flotan" entre sí).
    Un RANGO = una corrida de texId iguales EN LA LISTA ORDENADA =
    1 draw call (el ej 07: 6 sprites de 3 texturas → 3 draws y 1 buffer
    de instancias ordenado).

24. **(L15)** Un FBO es un CONTENEDOR de targets: le podés colgar una
    textura como GL_COLOR_ATTACHMENT0 (los fragment shaders escriben
    ALLÍ, no la ventana) + un renderbuffer de depth como
    GL_DEPTH_ATTACHMENT. El RENDER-to-texture permite post-proceso:
    el pase siguiente LEE esa textura como sampler y escribe en OTRO
    target. Escribir y leer LA MISMA textura en el MISMO draw (attachment
    + sampler) es undefined behavior por spec (feedback loop): el
    resultado es indefinido y el driver puede negarse o barrer.

25. **(L15)** El framebuffer default (el de la ventana) es UNO solo por
    contexto y se presenta al hacer swap: no se puede "renderizar encima
    de lo ya habido". El pase final dibuja UN quad fullscreen: el paso
    de post lee la textura del FBO (sampler uniform 0) y la pinta al
    default — eso es TODO el draw del pase post (uno, con el quad; el
    FBO "no directo" porque un FBO no muestra NADA en pantalla: su
    contenido es una textura que leés).

26. **(L16)** glBlitFramebuffer copia UNA REGION entre el FBO bind a
    GL_READ_FRAMEBUFFER y el bind a GL_WRITE_FRAMEBUFFER (regiones
    (sx,sy,sw,sh) → (dx,dy,dw,dh), con filtro GL_NEAREST o GL_LINEAR).
    El resolve de MSAA ES un blit: del FBO multisample (READ) al FBO
    normal (WRITE): N samples se PROMEDIAN al copiar (por eso es 2
    pasos: render multisample → resolve a un sample por píxel). Y las
    regiones NUNCA salen de los límites (API undefined behavior: el
    clampRegion del ej 08 lo maneja a mano).

27. **(L16)** glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
    formato, w, h) pide N samples por píxel (4 = 4 submuestras por
    píxel: los bordes de los triángulos quedan más suavizados). El HW
    clampa a GL_MAX_SAMPLES y los valores válidos son potencia de 2
    (> 1: con 1 no hay MSAA). samples = 0 ES el renderbuffer común
    (sin MSAA: depth único por píxel) — por eso el ej 08 devuelve
    0 = "no MSAA, usa FBO simple".

28. **(L17)** KHR_debug registra una callback que el driver llama en los
    ERRORes y avisos (GL_INVALID_OPERATION, avisos de perf, buffers
    incompatibles): glDebugMessageCallback(cb, userParam). En 3.3 core
    ES EXTENSION (se volvió core en 4.3): se carga con
    SDL_GL_GetProcAddress("glDebugMessageCallback") con typedef
    PFNGLDEBUGMESSAGECALLBACKPROC (la manera del curso en L17). Y la
    API "antigua" glGetError: devuelve UN flag por vez y LO LIMPIA al
    leerlo: por eso el bucle while (glGetError() != GL_NO_ERROR) DRENA
    los errores pendientes antes de mirar limpio.

29. **(L17)** Secuencia: glGenQueries UNA vez → BeginQuery(GL_TIME_ELAPSED,
    q) → draws → EndQuery(GL_TIME_ELAPSED) → LEER EN EL FRAME SIGUIENTE
    (glGetQueryObjectui64v, 64-bit para no desbordar el ns de la GPU,
    chequeando antes GL_QUERY_RESULT_AVAILABLE). NO disponible en el
    mismo frame: la GPU ejecuta ASÍNCRONAMENTE (el draw ya FUE ENVIADO
    y seguirá corriendo en el GPU mientras el CPU va adelantado: la
    query termina cuando la GPU termina): leer en bloque MATARÍA el
    paralelismo (por eso se lee con RETRASO de un frame).

30. **(L18)** tinyobjloader entrega attrib.vertices (array plano de
    floats de posiciones) + attrib.normals + attrib.texcoords y
    shapes[0].mesh.indices con UNA ENTRADA POR ESQUINA (ÍNDICES ya
    0-based, con los 3 componentes separados index_vertex /
    index_texcoord / index_normal por esquina): el vertex dedup final lo
    hacés vos (el ej 09 lo hizo a mano con un map key (v, vt, vn)).
    El mini parser se quedó SIN: .mtl (materiales en archivos aparte),
    ngons (caras de 4+ esquinas → triangulate del loader), índices
    NEGATIVOS (relativos al último v/vt/vn) y los tags exóticos — la
    mitad de la razón por la que en L18 se recomienda el loader en vez
    del parser propio.

---

*Fin del quiz. El módulo 20 queda: gl_util.hpp (contexto 3.3 core +
compilado/link de programas con logs + helpers de check), 18 lecciones
(los 12 temas del plan), 9 ejercicios con sus 9 soluciones (todos pasan
por `./verificar.sh 20`) y este quiz. El siguiente consume lo que acá se
abrió: shaders cargados, buffers, texturas y un render-to-texture de
verdad en TU máquina.*