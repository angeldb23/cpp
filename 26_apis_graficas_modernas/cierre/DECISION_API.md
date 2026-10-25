# Módulo 26 — Decisión de API Gráfica (cierre)

> **Resumen ejecutivo**: para un motor indie 2D/3D ligero en Linux/Windows/macOS
> hoy (2026), la recomendación por defecto es **SDL_GPU + SPIR-V** como capa
> portable, con Vulkan "a mano" solo donde la abstracción no llega. WebGPU
> (via wgpu-native) es la apuesta a futuro para Web y para código que quiera
> una sola ruta de shaders (WGSL) en todas las plataformas.

---

## 1. Contexto de la máquina de desarrollo (fuente: `vulkaninfo --summary`)

| Campo | Valor |
|-------|-------|
| GPU | Intel UHD Graphics 600 (GLK 2 / Gemini Lake) |
| Driver | Mesa 24.x / ANV (Vulkan 1.4.354) |
| OpenGL máximo | 3.3 core profile (Mesa) / 4.6 compat (llvmpipe) |
| Wayland | sí, `VK_KHR_wayland_surface` presente |
| Extensiones clave | `VK_KHR_swapchain`, `VK_KHR_dynamic_rendering`, `VK_EXT_descriptor_indexing` |
| Herramientas | `glslangValidator`, `spirv-val`, `spirv-cross` instalados |

**Conclusión**: la GPU soporta Vulkan 1.4 **nativo** (no emulación), pero el
techo OpenGL real es 3.3. Cualquier camino OpenGL 4.x+ requiere llvmpipe
(CPU) → inviable para juegos en tiempo real.

---

## 2. Comparativa de las 3 APIs del módulo

| Criterio | Vulkan (manual) | SDL_GPU | WebGPU (wgpu-native) |
|----------|-----------------|---------|----------------------|
| **Curva de aprendizaje** | Muy alta (1000+ líneas de arranque) | Media (50-100 líneas) | Media-alta (WGSL + bind groups) |
| **Control fino** | Total | Alto (pipeline/buffers/texturas expuestos) | Medio (más opinado) |
| **Portabilidad** | Linux/Windows/Android/macOS (via MoltenVK) | **Todas** (SDL elige backend) | **Todas** (Vulkan/Metal/DX12/Web) |
| **Shaders** | SPIR-V (glslang) | SPIR-V / DXBC / DXIL / MSL / METALLIB | **WGSL** (único, `spirv-cross` -> WGSL) |
| **Sincronización** | Manual (semaphores/fences) | Implícita en command buffer | Implícita (cola única) |
| **Memoria** | VMA obligatorio | SDL gestiona (VMA interno) | wgpu gestiona |
| **Debug/Validación** | Capas VK_LAYER_KHRONOS | `debug_mode=true` en device | `WGPU_DEVICE_LOST` + callbacks |
| **Overhead CPU** | Mínimo (explícito) | Muy bajo (capa fina) | Bajo (diseño de cola única) |
| **Ecosistema 2026** | Maduro, docs extensas | SDL3 estable (3.4.18) | wgpu-native 22.x, WebGPU en navegadores |

---

## 3. Recomendación por caso de uso

### A. Motor indie 2D/3D multiplataforma (escritorio + posible Web)
**Elección: SDL_GPU + SPIR-V**
- Una sola base de código C++, SDL elige Vulkan/Metal/DX12 automáticamente.
- Los shaders se escriben en GLSL → `glslangValidator -V` → `.spv`.
- La lección 04/05 demuestra el flujo completo: device, pipeline, VBO,
  texturas, samplers, render pass, draw.
- El demo 1 (triángulo) y demo 2 (quad texturizado) son el *starter kit*
  real del motor.

### B. Motor que **requiere** features Vulkan específicas
(ray tracing, mesh shaders, descriptor indexing avanzado, multi-GPU
explícito, present modes exóticos)
**Elección: Vulkan manual + VMA**
- El demo 3 (instancia mínima) + lección 02/03/08/09 dan la base.
- VMA (`v3.4.0` tag verificado) gestiona memoria y budget (`VK_EXT_memory_budget`).
- Render graph (lección 09) para encadenar passes sin barreras manuales.

### C. Motor que **debe correr en navegador** (WASM) + escritorio
**Elección: WebGPU via wgpu-native (C API) + WGSL**
- Lección 06: bind groups = descriptor sets simplificados, `var<uniform>`
  y `var<storage>` mapean 1:1 a Vulkan/Metal/DX12.
- `spirv-cross --wgsl` convierte SPIR-V → WGSL en el build (lección 07).
- Feature `Immediates` (`var<immediate>`) para push constants estilo.

### D. Prototyping / tooling / editor embebido
**Elección: SDL_GPU** (menor fricción, mismo device que el motor).

---

## 4. Estrategia de shaders recomendada (lección 07 + 06)

```
GLSL (fuente única)
    │
    ├─ glslangValidator -V → SPIR-V (.spv)
    │       │
    │       ├─ SDL_GPU (Vulkan backend) → usa SPIR-V directo
    │       ├─ Vulkan manual → usa SPIR-V directo
    │       └─ spirv-cross --wgsl → WGSL para wgpu-native / WebGPU
    │
    └─ (opcional) spirv-cross --msl → MSL para Metal nativo si SDL_GPU
            elige backend Metal y no querés SPIR-V -> MSL runtime
```

**Un solo fuente GLSL**, tres salidas. El pipeline de build (CMake) lo hace
automático: `add_custom_command` con `glslangValidator` + `spirv-cross`.

---

## 5. Budget de draws/frame en esta GPU (lección 10 + M10 decisión)

| API | Techo draws/frame @ 60 FPS | Comentario |
|-----|----------------------------|------------|
| OpenGL 3.3 (fallback) | **500** | `glDrawArrays` + binding manual, CPU bound |
| OpenGL 3.3 + `iso_ready` (AZDO) | **1200** | `glMultiDrawArraysIndirect` + persistent mapping |
| Vulkan / SDL_GPU / WebGPU | **5000+** | GPU-driven, indirect draws, descriptor indexing |

`iso_ready` = extensiones `GL_ARB_multi_draw_indirect` +
`GL_ARB_buffer_storage` + `GL_ARB_shader_draw_parameters` presentes.
En esta máquina (Mesa 24.x / GL 3.3) **sí están** → el techo sube a 1200.

---

## 6. Decisión final del módulo

> **Para el curso y el motor base**: **SDL_GPU + SPIR-V** como default.
> Vulkan manual queda expuesto en `f26.hpp` helpers (`PipelineSpec`,
> `BudgetHeap`, `GpuCap`) para quien necesite bajar a el metal.
> WebGPU (wgpu-native) se compila como *feature opcional* (`-DWITH_WGPU=ON`)
> y comparte los mismos `.spv` → WGSL via `spirv-cross`.

**Próximo paso (M27)**: renderizador 2D sobre SDL_GPU (sprites, atlas,
texto SDF, batching) — todo probado en esta GPU real.

---

*Generado al cerrar M26 — datos de `vulkaninfo --summary` y headers
verificados contra SDL3 3.4.18, Vulkan-Headers v1.4.366, VMA v3.4.0,
wgpu-native 22.x.*