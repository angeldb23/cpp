# Módulo 29 — Proyecto Final

> **Este módulo no tiene ejercicios ni soluciones.**  
> El estudiante debe construir, desde cero, un mini motor 2D y un juego de plataformas usando los conocimientos de los módulos 16‑28.  
> El trabajo se evalúa con la rúbrica y el script `verificar_entrega.sh`, no con `./verificar.sh 29`.

---

## 1. Consigna (CONSIGNA.md)

### 1.1 Datos generales
| Campo | Valor |
|---|---|
| **Módulo** | 29 — Proyecto Final |
| **Modalidad** | Individual |
| **Tipo** | Proyecto integrador, sin ejercicios ni soluciones |
| **Duración estimada** | 25 – 40 horas |
| **Entrega** | Código fuente + assets + tests + documentación |
| **Evaluación** | Rúbrica de `RUBRICA.md` + validador `verificar_entrega.sh` |

### 1.2 Objetivo
Diseñar y desarrollar un **mini motor 2D** y, sobre ese motor, un **juego de plataformas 2D jugable** que integre lo aprendido en los módulos 16‑28.

### 1.3 Producto exigido
1. Se configura y compila desde cero con **CMake**.
2. Se ejecuta en Linux, ventana 1280 × 720 mínimo.
3. Un nivel jugable de principio a fin.
4. El jugador puede moverse, saltar, recibir daño, morir y reaparecer en un checkpoint.
5. Al menos un tipo de enemigo con comportamiento propio.
6. Al menos una plataforma móvil o mecanismo cinemático.
7. **Audio**: al menos un efecto de sonido y música/ambiente.
8. **HUD** con vida, estado del nivel y objetivo.
9. Menú principal, pausa y reinicio.
10. Capa de **debug** activable que muestre FPS, entidades y colisiones.
11. **Tests automatizados** de la lógica, ejecutables con CTest (≥ 8 tests).
12. Documentación técnica: diseño, decisiones, README y postmortem.

### 1.4 Alcance funcional obligatorio
- **Flujo de juego** (menú, partida, pausa, victoria, derrota, reinicio).
- **Jugador** (movimiento horizontal, salto con gravedad, colisión, daño/invulnerabilidad, muerte/reaparición, animación/feedback).
- **Nivel y mundo** (tilemap o geometría, cámara, fondo/parallax, checkpoints, objetivo, peligros).
- **Enemigos y mecanismos** (enemigo básico, daño al enemigo, plataforma móvil, estados del enemigo).
- **Presentación y audio** (HUD, sonido, música, mezcla, overlay de debug).

### 1.5 Requisitos técnicos
- **Lenguaje y build**: C++20, CMake 3.20+. Compilar con `cmake -S proyecto -B proyecto/build` y `cmake --build proyecto/build -j2`.
- **Plataforma y render**: SDL3 + OpenGL 3.3 core **o** SDL_GPU (justificar en un ADR).
- **Matemática**: GLM o propia librería `mate3d.hpp`.
- **Física y colisiones**: Box2D 2.4.x envuelto detrás de fachada propia **o** física propia (AABB/SAT, timestep fijo).
- **Audio**: miniaudio, al menos un SFX y música/loop, callback sin bloqueos.
- **Herramientas**: Dear ImGui opcional; overlay de debug que muestre FPS, entidades, colisiones.
- **Assets**: licencia compatible, `assets/LICENCIAS.md`, no assets comerciales ni rips.

### 1.6 Arquitectura exigida
1. Separación clara **motor / juego / herramientas**.
2. Fachadas estables para render, física y audio (el juego no incluye `b2World` ni `ma_device` directamente).
3. Recursos con RAII.
4. ECS o escena consistente.
5. Nivel data‑driven (archivo propio, no hardcodeado en `main`).
6. Un solo game loop.
7. Errores explícitos.

### 1.7 Calidad, pruebas y rendimiento
- **Tests**: ≥ 8 casos de prueba automatizados con CTest (lógica sin ventana).
- **Calidad de código**: `-Wall -Wextra`, `unique_ptr`, `vector`, `string`, `span`, sin `using namespace std;` en headers.
- **Rendimiento**: timestep fijo (1/60 s), interpolación, 60 FPS estables en 1280 × 720 en la máquina del curso (Celeron N4020 / Intel UHD 600).

### 1.8 Entregables
1. Código fuente completo.
2. `CMakeLists.txt` funcional.
3. `assets/` + `assets/LICENCIAS.md`.
4. `tests/` con ≥ 8 tests integrados a CTest.
5. `README.md` del proyecto (build, ejecución, controles).
6. `docs/DISENO.md`.
7. `docs/DECISIONES.md` con ≥ 3 ADR.
8. `docs/POSTMORTEM.md`.
9. `docs/capturas/` con capturas o video.
10. `docs/capturas/` con evidencia visual.

### 1.9 Hitos recomendados
| Hito | Objetivo | Tiempo estimado |
|------|----------|-----------------|
| **H1** | Base compilable | 4‑6 h |
| **H2** | Render y cámara | 5‑8 h |
| **H3** | Física y colisiones | 6‑10 h |
| **H4** | Gameplay y nivel | 6‑10 h |
| **H5** | Audio, UI y debug | 4‑6 h |
| **H6** | Tests, docs y entrega | 4‑6 h |

### 1.10 Criterios de aceptación
- Compila y ejecuta desde cero con CTest (≥ 8 tests pasan).
- El juego se puede jugar de principio a fin sin crashear.
- Menú, pausa, derrota, victoria y reinicio funcionan.
- Hay enemigo, plataforma móvil, checkpoint y HUD.
- Audio y assets con licencia.
- Overlay de debug activable.
- Documentación completa y coherente.
- `verificar_entrega.sh` termina sin errores críticos.

### 1.11 Regla de oro
**No hay código de solución.** Todo lo que no venga de la consigna o las plantillas es decisión de diseño del usuario,documentada con ADR.

### 1.12 Qué **no** se entrega
- Código base del motor o del juego.
- Tests ya escritos.
- Assets de arte o audio.
- Solución de referencia.

---

## 2. Guía de desarrollo (GUIA_DE_DESARROLLO.md)

### 0. Antes de escribir código
- Definí la **vertical slice** (nivel corto pero completo).
- Elegí el alcance con honestidad (1 nivel, 1 tipo de enemigo, 2 checkpoints, etc.).
- Escribid los **ADR** antes de decisiones grandes.

### 1. Hito 1 — Base compilable
- CMakeLists.txt, separación `src/main.cpp`, `src/engine/`, `src/game/`.
- SDL3 + ventana + loop + cierre limpio.
- Sin warnings nuevos, rutas relativas.

### 2. Hito 2 — Render, cámara y tilemap
- Textura con `stb_image`, shader de sprite, sprite batch.
- Cámara 2D con zoom y límites.
- Tilemap cargado desde datos, capa de fondo/parallax.

### 3. Hito 3 — Física y colisiones
- Decisión Box2D vs física propia (ADR).
- Timestep fijo (1/60 s), acumulador.
- Colisiones X/Y separadas, plataformas móviles, contacto para daño/meta.
- Interpolación de render.

### 4. Hito 4 — Gameplay y nivel
- Máquina de estados del jugador (`idle, run, jump, fall, hurt, dead`).
- Enemigo con patrulla/ataque, checkpoints, meta, peligros.
- HUD: vida, checkpoints, objetivo.

### 5. Hito 5 — Audio, UI y debug
- Miniaudio + SFX + música, buses master/efectos.
- Menú principal, pausa, HUD.
- Overlay de debug (FPS, entidades, colliders, tecla).

### 6. Hito 6 — Tests, documentación y entrega
- ≥ 8 tests automatizados (CTest).
- `README.md`, `docs/DISENO.md`, `docs/DECISIONES.md` (≥ 3 ADR), `docs/POSTMORTEM.md`.
- Limpieza de `build/`, `git commit`, `verificar_entrega.sh`.

### 7. Orden de implementación recomendado
```
CMake + ventana
   ↓
loop + input + cierre
   ↓
render de un quad + textura
   ↓
cámara + tilemap
   ↓
física del jugador
   ↓
enemigo + daño + checkpoints
   ↓
nivel completo + meta
   ↓
audio + HUD + menú
   ↓
debug + tests + docs
   ↓
pulido y entrega
```

### 8. Trampas comunes
| Síntoma | Causa | Qué revisar |
|---|---|---|
| El jugador tiembla | colisión X/Y juntos | separar resolución |
| Se atraviesa el suelo | delta variable | timestep fijo |
| El juego va rápido/lento | usa delta del render | acumulador fijo |
| El audio cruje | callback con locks/allocs | ring buffer, no bloquear |
| El sprite se ve mal | atlas o UV invertidas | coordenadas de textura |
| La cámara vibra | se actualiza después del render | interpolación/alpha |
| Fuga de memoria | texturas sin RAII | sanitizers |
| No compila en otra máquina | rutas absolutas | `PROJECT_SOURCE_DIR`, assets relativos |
| Los tests pasan pero el juego no | tests triviales | probar lógica real |
| El proyecto no se entiende | docs genéricas | diseño y ADR concretos |

### 9. Cómo saber que estás listo
Responde “sí” a todas:
- Otra persona puede compilar siguiendo el README.
- El juego se puede terminar sin ayuda.
- El nivel se carga desde datos.
- El jugador no atraviesa el mundo.
- Enemigos y checkpoints funcionan una sola vez.
- Audio y assets tienen licencia.
- Los tests fallan si rompés la lógica que cubren.
- El postmortem menciona bugs reales.
- `verificar_entrega.sh` termina sin errores.

---

## 3. Entregables (ENTREGABLES.md)

1. Código fuente completo.
2. `CMakeLists.txt` funcional.
3. `assets/` + `assets/LICENCIAS.md`.
4. `tests/` con ≥ 8 tests CTest.
5. `README.md` del proyecto (build, ejecución, controles).
6. `docs/DISENO.md`.
7. `docs/DECISIONES.md` con ≥ 3 ADR.
8. `docs/POSTMORTEM.md`.
9. `docs/capturas/` con capturas/video.
10. `docs/capturas/` con evidencia visual.

---

## 4. Rúbrica (RUBRICA.md)

| Área | Peso | Condición eliminatoria |
|------|------|-----------------------|
| Build reproducible y ejecución | 10 | Si no compila → 40 pts máx |
| Gameplay y nivel completo | 25 | Si no se puede jugar → 60 pts máx |
| Arquitectura del motor | 20 | — |
| Física y colisiones | 10 | — |
| Render y presentación | 10 | — |
| Audio y assets | 5 | — |
| Herramientas y debug | 5 | — |
| Tests | 10 | Si no hay tests → 80 pts máx |
| Documentación y postmortem | 5 | — |

**Descriptores por nivel**  
- 90‑100 pts: Sobresaliente  
- 80‑89 pts: Notable  
- 70‑79 pts: Aprobado  
- 60‑69 pts: Suficiente  
- < 60 pts: Insuficiente  

---

## 5. Lista de verificación (CHECKLIST.md)

```
✅ CMakeLists.txt existe y compila sin errores
✅ `build/` se genera y el ejecutable se lanza
✅ Ventana SDL3 se abre, se cierra con Escape/X
✅ Game loop (fixed timestep) funciona
✅ Render: sprite/textura en pantalla
✅ Cámara 2D sigue al jugador
✅ Tilemap se carga desde datos
✅ Física del jugador: no atraviesa suelo
✅ Enemigo con dos estados (patrulla/ataque)
✅ Checkpoints guardan posición y reaparición
✅ Meta/objetivo de nivel
✅ Al menos un SFX y música/loop
✅ HUD muestra vida, checkpoints, objetivo
✅ Menú principal, pausa, reinicio
✅ Overlay de debug (FPS, entidades, colisiones) con tecla
✅ Al menos 8 tests automatizados (CTest) y todos pasan
✅ `README.md` del proyecto con build, controles y estructura
✅ `docs/DISENO.md` presente y coherente con el código
✅ `docs/DECISIONES.md` con al menos 3 ADR
✅ `docs/POSTMORTEM.md` presente
✅ `assets/LICENCIAS.md` presente con cada asset
✅ `verificar_entrega.sh` no reporta errores críticos
```

---

## 6. Plantillas (PLANTILLAS/)

| Archivo | Contenido |
|---|---|
| **ADR‑000‑plantilla.md** | Título, contexto, decisión, opciones, consecuencias, referencias |
| **DISENO.md** | Pitch (3 líneas), alcance obligatorio, fuera de alcance, arquitectura de alto nivel, decisiones tomadas, riesgos principales |
| **POSTMORTEM.md** | Lo que salió bien, lo que no, bugs/conocida, cambios para la próxima iteración |
| **README_ENTREGA.md** | Cómo compilar y probar, cómo correr los tests, instrucciones de entrega |

---

## 7. Script de verificación (`verificar_entrega.sh`)

```bash
#!/usr/bin/env bash
# Valida la estructura mínima y los entregables obligatorios del M29.
# No sustituye la rúbrica, pero asegura que no falte lo esencial.

set -u
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT="${1:-$ROOT/29_proyecto_final/proyecto}"
ERRORS=0

echo "=== Verificación del Proyecto Final M29 ==="

# 1. CMakeLists.txt
if [ ! -f "$PROJECT/CMakeLists.txt" ]; then
    echo "❌ Falta CMakeLists.txt"; ERRORS=$((ERRORS+1))
else echo "✅ CMakeLists.txt presente"; fi

# 2. README.md del proyecto
if [ ! -f "$PROJECT/README.md" ]; then
    echo "❌ Falta README.md"; ERRORS=$((ERRORS+1))
else echo "✅ README.md presente"; fi

# 3. docs/DISENO.md
if [ ! -f "$PROJECT/docs/DISENO.md" ]; then
    echo "❌ Faltan docs/DISENO.md"; ERRORS=$((ERRORS+1))
else echo "✅ docs/DISENO.md presente"; fi

# 4. docs/DECISIONES.md con ≥3 ADR
ADR_COUNT=$(grep -c "^ADR-" "$PROJECT/docs/DECISIONES.md" 2>/dev/null || echo 0)
if [ "$ADR_COUNT" -lt 3 ]; then
    echo "❌ docs/DECISIONES.md necesita ≥3 ADR (actual: $ADR_COUNT)"; ERRORS=$((ERRORS+1))
else echo "✅ docs/DECISIONES.md ($ADR_COUNT ADR)"; fi

# 5. docs/POSTMORTEM.md
if [ ! -f "$PROJECT/docs/POSTMORTEM.md" ]; then
    echo "❌ Faltan docs/POSTMORTEM.md"; ERRORS=$((ERRORS+1))
else echo "✅ docs/POSTMORTEM.md presente"; fi

# 6. assets/LICENCIAS.md
if [ ! -f "$PROJECT/assets/LICENCIAS.md" ]; then
    echo "❌ Faltan assets/LICENCIAS.md"; ERRORS=$((ERRORS+1))
else echo "✅ assets/LICENCIAS.md presente"; fi

# 7. tests/ (al menos un archivo)
if [ ! -d "$PROJECT/tests" ] || [ -z "$(ls -A "$PROJECT/tests" 2>/dev/null)" ]; then
    echo "❌ Faltan tests/ o está vacío"; ERRORS=$((ERRORS+1))
else echo "✅ tests/ presente con contenido"; fi

# 8. Resumen
echo ""
if [ "$ERRORS" -eq 0 ]; then
    echo "✅ Verificación finalizada: no hay errores críticos"
else
    echo "❌ Verificación finalizada: $ERRORS error(s) crítico(s) – corregirlos antes de entregar"
fi
exit $ERRORS
```

---

## 8. Próximos pasos para el usuario

1. **Leer** `29_proyecto_final/CONSIGNA.md` completo.  
2. **Seguir** `29_proyecto_final/GUIA_DE_DESARROLLO.md` para planificar los hitos.  
3. **Usar** las plantillas (`PLANTILLAS/`) para diseño, ADR y post‑mortem.  
4. **Compilar** y probar con `./29_proyecto_final/verificar_entrega.sh`.  
5. **Entregar** los archivos obligatorios indicados en `ENTREGABLES.md`.  

---

## 9. Referencias cruzadas en el curso

- **TEMAS.md** – al final se añade la sección “Módulo 29 — Proyecto Final”.  
- **PROGRESO.md** – se actualiza la tabla para que el módulo 29 aparezca como “Documentado (consigna)”.  
- **LEEME_PRIMERO.md** – indica que el próximo paso es construir el módulo 29.  
- **DECISIONES.md** – registra las decisiones tomadas para el módulo 29 (alcance, entregables, regla de oro, verificación).  

--- 

**¡Listo!** ya tenés un único `README.md` con toda la información necesaria para iniciar y completar el proyecto final del curso. Si necesitás alguna modificación o querés que elimine los archivos `md` individuales, decílo.