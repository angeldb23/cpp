# Informe técnico — Módulo 28: Estudio de Código Real

> Cierre del módulo. Objetivo: comparar cinco librerías reales (GLM,
> miniaudio, Box2D, Dear ImGui y SDL3) y dejar documentada una decisión de
> adopción para el motor propio. No es una recomendación genérica: es la
> decisión para *este* motor 2D, escrito en C++20, con CMake y sin ánimo de
> reescribir lo que ya está resuelto.

---

## 1. Método

Cada librería se evaluó con la misma grilla de la lección 01:

| Criterio | Qué se miró |
|---|---|
| Madurez | años en producción, releases, semver, changelog |
| Licencia | compatibilidad con un motor de código cerrado |
| Dependencias | transitivas, header-only vs binario, backend |
| Estilo C++ | C99, C++11/14, C++17/20, templates, API C |
| Actividad | releases e issues recientes, mantenedores |
| ABI | estabilidad binaria, handles opacos, versionado |
| Build | CMake, pkg-config, FetchContent, single-header |
| Testing | unit tests, fuzzing, CI, sanitizers |

La evaluación se hizo sobre el código y la documentación pública de cada
proyecto, más los recortes didácticos de las lecciones 02–08. Las versiones
exactas se fijan al integrar (ver §6); este informe no inventa tags.

---

## 2. Fichas comparativas

| Librería | Rol | Licencia | Estilo | Deps | ABI | Integración |
|---|---|---|---|---|---|---|
| **GLM** | matemática (vec/mat/quat) | MIT | C++ header-only, templates, `constexpr` | ninguna | no aplica (source) | copiar/FetchContent |
| **miniaudio** | audio (device, decoding, mixing) | Public Domain / MIT-0 | C99 single-header, usable desde C++ | ninguna | no aplica (source) | single-header |
| **Box2D 2.4.x** | física 2D | MIT | C++11, clases (`b2World`, `b2Body`) | ninguna | C++ ABI frágil | FetchContent estático |
| **Box2D 3.x** | física 2D | MIT | C API con handles (`b2WorldId`) | ninguna | C ABI estable | FetchContent estático |
| **Dear ImGui** | UI de herramientas | MIT | C++11, immediate mode, backend aparte | ninguna en el core | C++ ABI dependiente de config | copiar fuentes + backend |
| **SDL3** | plataforma, ventana, input, GPU, audio | zlib | C99, API plana, properties | libs del sistema | C ABI versionada | pkg-config / CMake |

### 2.1 GLM

Fortalezas: API prácticamente estándar en el ecosistema gráfico, column-major
compatible con OpenGL, `value_ptr()`, `constexpr` y dispatch SIMD por
arquitectura. Debilidades: los headers son grandes y aumentan el tiempo de
compilación; los modos `gtx/*` experimentales no tienen la misma garantía que
`glm/core`. Riesgo bajo: al ser header-only, actualizar es recompilar.

### 2.2 miniaudio

Fortalezas: un solo header, sin dependencias, backends (ALSA/PulseAudio/
WASAPI/CoreAudio) detrás de la misma API, decoding de WAV/MP3/FLAC y un
`ma_engine` de alto nivel. Debilidades: el callback corre en un hilo de tiempo
real (nada de locks, allocs ni syscalls); las APIs de más alto nivel ocultan
parte del control. La implementación se compila en *un* `.c/.cpp`.

### 2.3 Box2D

Fortalezas: solver PGS probado en producción, broad-phase con AABB tree,
islands, CCD, joints y contact listeners. Box2D 3.x migró a C con handles
opacos, lo que mejora la ABI y permite bindings. Debilidades: 2.4.x expone
clases C++ con ABI frágil; el solver y el orden de los contactos afectan la
reproducibilidad. Para un motor propio, la decisión razonable es **envolver**
la API detrás de una fachada (o fijar 2.4.x y aislarla).

### 2.4 Dear ImGui

Fortalezas: immediate mode, código pequeño, draw lists portables, font atlas,
docking y backends separados (plataforma vs renderer). Debilidades: no es una
UI de producción para el jugador final; el estado vive en el frame y el ABI
C++ depende de flags de compilación. Se usa para herramientas del editor,
no para el HUD del juego.

### 2.5 SDL3

Fortalezas: una API C estable para ventana, input, audio, storage, gamepads y
GPU (Vulkan/Metal/D3D12), más `SDL_Properties` y main callbacks. Debilidades:
la capa GPU es joven comparada con Vulkan directo; algunos subsistemas
requieren backend del sistema. Es la opción natural para plataforma.

---

## 3. Diagrama de arquitectura propuesta

```
                         +---------------------------+
                         |        Juego / Editor     |
                         +-------------+-------------+
                                       |
             +-------------------------+-------------------------+
             |                         |                         |
      +------v------+           +------v------+           +------v------+
      |  Renderer   |           |  Simulación |           |    Tools    |
      | (SDL3 GPU / |           |  (física,   |           |  (ImGui +   |
      |  OpenGL)    |           |   lógica)   |           |   SDL3)     |
      +------+------+           +------+------+           +------+------+
             |                         |                         |
      +------v------+           +------v------+           +------v------+
      |     GLM     |           |   Box2D 3.x |           |  Dear ImGui |
      |  (matemática)|          |  (envuelto) |           |  (solo dev) |
      +-------------+           +-------------+           +-------------+
             |                         |                         |
             +-------------------------+-------------------------+
                                       |
                              +--------v--------+
                              |      SDL3       |
                              | plataforma/audio|
                              +--------+--------+
                                       |
                              +--------v--------+
                              |    miniaudio    |
                              | audio del juego |
                              +-----------------+
```

Regla de dependencias: el juego no incluye headers de Box2D ni de ImGui
directamente. Solo habla con las fachadas `Fisica`, `UI` y `Audio`. Así se
puede cambiar la implementación sin tocar la lógica del juego.

---

## 4. Tabla de trade-offs

| Decisión | Opción A | Opción B | Elegida | Motivo |
|---|---|---|---|---|
| Matemática | GLM | matemática propia | **GLM** | probada, `constexpr`, SIMD; el motor ya la usa en módulos 17–27 |
| Física | Box2D 3.x envuelto | física propia | **Box2D 3.x** | solver estable; la fachada limita el acoplamiento |
| Audio | miniaudio | backend propio | **miniaudio** | un header, sin deps, cubre device + decoding + mixing |
| UI de editor | Dear ImGui | UI propia | **Dear ImGui** | costo de desarrollo muy alto para rehacerla |
| Plataforma | SDL3 | GLFW + libs sueltas | **SDL3** | input, ventana, audio, storage y GPU en una API C |
| ABI | C++ directo | handles opacos | **handles opacos** | permite actualizar la librería sin recompilar el juego |
| Build | FetchContent | copiar fuentes | **FetchContent + tag fijo** | reproducible; GLM/ImGui se pueden copiar si hace falta |
| Testing | unit tests + ULP | solo `==` | **ULP/tolerancia** | los tests numéricos no deben depender del redondeo |

---

## 5. Decisión documentada (ADR resumidos)

### ADR-001 — Adoptar GLM

- **Estado:** aceptado.
- **Contexto:** el motor necesita vectores/matrices/quaterniones con
  column-major y `constexpr`.
- **Decisión:** adoptar GLM como header-only, con tag fijo y una fachada
  `math.hpp` que reexporta solo lo usado.
- **Consecuencias:** + compatibilidad con OpenGL; − tiempo de compilación.

### ADR-002 — Envolver Box2D 3.x

- **Estado:** propuesto.
- **Contexto:** 2.4.x tiene ABI C++ frágil; 3.x usa handles C.
- **Decisión:** usar Box2D 3.x detrás de `Fisica` (ids opacos, sin exponer
  `b2*` fuera del wrapper).
- **Consecuencias:** + ABI estable y posibilidad de cambiar de versión;
  − una capa de indirección y conversión de tipos.

### ADR-003 — Adoptar miniaudio

- **Estado:** aceptado.
- **Contexto:** se necesita audio sin sumar dependencias pesadas.
- **Decisión:** miniaudio en un único `.cpp` con `MINIAUDIO_IMPLEMENTATION`;
  ring buffer SPSC para el hilo de audio.
- **Consecuencias:** + simple, sin deps; − cuidado con el callback de tiempo
  real (nada de locks/allocs).

### ADR-004 — Adoptar Dear ImGui solo para herramientas

- **Estado:** aceptado.
- **Contexto:** el editor necesita UI rápida de iterar.
- **Decisión:** ImGui + backend SDL3/renderer, aislado en `UI`; nunca en el
  runtime del jugador.
- **Consecuencias:** + productividad; − ABI dependiente de flags y un costo
  de draw por frame.

### ADR-005 — Adoptar SDL3 como plataforma

- **Estado:** aceptado.
- **Contexto:** se necesita ventana/input/audio/GPU en Linux y Windows.
- **Decisión:** SDL3 vía pkg-config/CMake, con `SDL_Properties` para datos
  asociados a ventanas y texturas.
- **Consecuencias:** + una sola API C; − la capa GPU todavía evoluciona.

### ADR-006 — No reimplementar lo que ya está probado

- **Estado:** aceptado.
- **Contexto:** el objetivo del curso es aprender arquitectura, no reescribir
  cinco librerías.
- **Decisión:** reimplementar solo lo didáctico (arena, ring buffer, AABB
  tree, ID stack) y reutilizar las librerías en producción.
- **Consecuencias:** + foco; − hay que estudiar y respetar sus APIs.

---

## 6. Plan de integración

1. Fijar tags inmutables (verificados con `git ls-remote --tags`) y anotarlos
   en `DECISIONES.md`.
2. Traer GLM, miniaudio, Box2D e ImGui con `FetchContent`; SDL3 con
   `pkg-config` o el paquete del sistema.
3. Compilar miniaudio en un solo target (`MINIAUDIO_IMPLEMENTATION`).
4. Crear las fachadas `math`, `Fisica`, `Audio` y `UI`; el juego solo incluye
   esas fachadas.
5. Añadir tests headless (los de este módulo) y un test de integración que
   abra ventana/audio solo en la máquina del dueño, nunca en CI.

---

## 7. Checklist final de adopción

- [ ] Licencia compatible con proyecto cerrado (MIT/BSD/Apache/zlib).
- [ ] Tag fijo y `SHA256`/release anotada.
- [ ] ¿Header-only o binario? Definido y documentado.
- [ ] Dependencias transitivas contadas y justificadas.
- [ ] Build reproducible en Linux y Windows.
- [ ] Tests propios + sanitizers en CI.
- [ ] ABI: handles opacos o fachada estable.
- [ ] Plan de actualización y de rollback.
- [ ] Documentación mínima: API, ejemplos, changelog.
- [ ] Decisión registrada en un ADR.

---

## 8. Presentación de 15 minutos (guion)

| Min | Contenido |
|---|---|
| 0–2 | Problema: elegir dependencias sin hipotecar el motor. Grilla de criterios. |
| 2–5 | GLM y miniaudio: header-only, `constexpr`, SIMD, callback de audio. |
| 5–8 | Box2D 2.4 vs 3.x: C++ ABI frágil vs C handles; solver e islands. |
| 8–11 | ImGui y SDL3: immediate mode, draw lists, properties, GPU. |
| 11–13 | Tabla de trade-offs y arquitectura por fachadas. |
| 13–15 | ADR y checklist; qué se adopta, qué se envuelve, qué se reimplementa. |

---

## 9. Riesgos y mitigaciones

| Riesgo | Mitigación |
|---|---|
| Actualización rompe ABI | fachadas + handles opacos + tag fijo |
| Callback de audio bloquea | ring buffer SPSC, cero allocs/locks |
| Física no reproducible | versión fija, orden de contactos estable, tests |
| ImGui acoplado al renderer | backend aislado, solo herramientas |
| SDL_GPU cambia | encapsular en `Renderer`, fallback OpenGL |
| Aumento de tiempo de compilación | precompiled headers / unidad de compilación por capas |

---

*Cierre M28. Las decisiones de este informe se reflejan en `DECISIONES.md`.*
