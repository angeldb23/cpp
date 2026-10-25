/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * LECCIÓN 08: Estudio de SDL3 — C API, Subsystems, GPU Abstraction
 *
 * TEMAS:
 * - SDL3 vs SDL2: qué cambió realmente (API cleanup, naming, properties)
 * - Subsystems: Video, Audio, Input, Haptic, Sensor, Camera, GPU, Storage
* - Properties: sistema unificado key-value para objetos (ventanas, devices, etc.)
* - Main callbacks: SDL_AppInit/SDL_AppEvent/SDL_AppIterate/SDL_AppQuit
* - Window: creación, flags, HiDPI, fullscreen, grab, opacity, shape
* - Renderer: SDL_Renderer (2D), SDL_GPU (3D/compute), software fallback
* - Events: nuevo sistema (SDL_EventType, common fields, payload)
* - Input: keyboard scancodes/keycodes, mouse, gamepad, touch, pen
* - Gamepad: SDL_Gamepad (unificado), sensor, rumble, touchpad, lightbar
* - Audio: SDL_AudioStream (push/pull), device management, logical devices
* - GPU: SDL_GPUDevice, pipelines, buffers, textures, samplers, compute
* - Storage: SDL_Storage (abstract filesystem), title/user/base paths
* - Threading: SDL_Thread, SDL_Mutex, SDL_Condition, SDL_RWLock, TLS
* - Time: SDL_GetTicksNS, SDL_GetPerformanceCounter, high-res timers
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace modulo28_sdl3 {

// ============================================================================
// TIPOS BASE MÍNIMOS DE SDL3 (en SDL3 real viven en SDL_rect.h, SDL_gpu.h, ...)
// ============================================================================

using SDL_Scancode = std::uint32_t;
using SDL_Keycode = std::uint32_t;

struct SDL_Rect { int x = 0, y = 0, w = 0, h = 0; };
struct SDL_Point { int x = 0, y = 0; };
struct SDL_FRect { float x = 0, y = 0, w = 0, h = 0; };
struct SDL_FPoint { float x = 0, y = 0; };

enum SDL_FlipMode : std::uint32_t {
    SDL_FLIP_NONE = 0,
    SDL_FLIP_HORIZONTAL = 1,
    SDL_FLIP_VERTICAL = 2,
};

enum SDL_BlendMode : std::uint32_t {
    SDL_BLENDMODE_NONE = 0,
    SDL_BLENDMODE_BLEND = 1,
    SDL_BLENDMODE_ADD = 2,
    SDL_BLENDMODE_MOD = 3,
    SDL_BLENDMODE_MUL = 4,
};

enum SDL_ScaleMode : std::uint32_t {
    SDL_SCALEMODE_NEAREST = 0,
    SDL_SCALEMODE_LINEAR = 1,
};

typedef struct SDL_Surface SDL_Surface;
typedef struct SDL_Texture SDL_Texture;

struct SDL_Vertex {
    float position[2];
    float color[4];
    float tex_coord[2];
};

// Opaques de SDL_GPU (definidos una sola vez en SDL_gpu.h)
typedef struct SDL_GPUDevice SDL_GPUDevice;
typedef struct SDL_GPUCommandBuffer SDL_GPUCommandBuffer;
typedef struct SDL_GPUGraphicsPipeline SDL_GPUGraphicsPipeline;
typedef struct SDL_GPUComputePipeline SDL_GPUComputePipeline;
typedef struct SDL_GPUBuffer SDL_GPUBuffer;
typedef struct SDL_GPUTexture SDL_GPUTexture;
typedef struct SDL_GPUSampler SDL_GPUSampler;
typedef struct SDL_GPUShader SDL_GPUShader;
typedef struct SDL_GPURenderPass SDL_GPURenderPass;
typedef struct SDL_GPUComputePass SDL_GPUComputePass;
typedef struct SDL_GPUCopyPass SDL_GPUCopyPass;
typedef struct SDL_GPUFence SDL_GPUFence;
typedef struct SDL_GPUTransferBuffer SDL_GPUTransferBuffer;

enum SDL_GPUTextureType : std::uint32_t {
    SDL_GPU_TEXTURETYPE_2D = 0,
    SDL_GPU_TEXTURETYPE_2D_ARRAY = 1,
    SDL_GPU_TEXTURETYPE_3D = 2,
    SDL_GPU_TEXTURETYPE_CUBE = 3,
};

enum SDL_GPUSampleCount : std::uint32_t {
    SDL_GPU_SAMPLECOUNT_1 = 0,
    SDL_GPU_SAMPLECOUNT_2 = 1,
    SDL_GPU_SAMPLECOUNT_4 = 2,
    SDL_GPU_SAMPLECOUNT_8 = 3,
};

enum SDL_GPUSwapchainComposition : std::uint32_t {
    SDL_GPU_SWAPCHAINCOMPOSITION_SDR = 0,
    SDL_GPU_SWAPCHAINCOMPOSITION_HDR10_ST2084 = 1,
};

enum SDL_GPUPresentMode : std::uint32_t {
    SDL_GPU_PRESENTMODE_VSYNC = 0,
    SDL_GPU_PRESENTMODE_IMMEDIATE = 1,
    SDL_GPU_PRESENTMODE_MAILBOX = 2,
};

enum SDL_GPUBufferUsageFlags : std::uint32_t {
    SDL_GPU_BUFFERUSAGE_VERTEX = 0x1,
    SDL_GPU_BUFFERUSAGE_INDEX = 0x2,
    SDL_GPU_BUFFERUSAGE_INDIRECT = 0x4,
    SDL_GPU_BUFFERUSAGE_STORAGE = 0x8,
};

enum SDL_GPUTextureUsageFlags : std::uint32_t {
    SDL_GPU_TEXTUREUSAGE_SAMPLER = 0x1,
    SDL_GPU_TEXTUREUSAGE_COLOR_TARGET = 0x2,
    SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET = 0x4,
    SDL_GPU_TEXTUREUSAGE_STORAGE = 0x8,
};

enum SDL_GPUFilter : std::uint32_t {
    SDL_GPU_FILTER_NEAREST = 0,
    SDL_GPU_FILTER_LINEAR = 1,
};

enum SDL_GPUAddressMode : std::uint32_t {
    SDL_GPU_ADDRESSMODE_REPEAT = 0,
    SDL_GPU_ADDRESSMODE_CLAMP_TO_EDGE = 1,
    SDL_GPU_ADDRESSMODE_CLAMP_TO_BORDER = 2,
};

enum SDL_GPUCompareOp : std::uint32_t {
    SDL_GPU_COMPAREOP_INVALID = 0,
    SDL_GPU_COMPAREOP_NEVER = 1,
    SDL_GPU_COMPAREOP_LESS = 2,
    SDL_GPU_COMPAREOP_EQUAL = 3,
    SDL_GPU_COMPAREOP_LESS_OR_EQUAL = 4,
    SDL_GPU_COMPAREOP_ALWAYS = 5,
};

enum SDL_GPULoadOp : std::uint32_t {
    SDL_GPU_LOADOP_LOAD = 0,
    SDL_GPU_LOADOP_CLEAR = 1,
    SDL_GPU_LOADOP_DONT_CARE = 2,
};

enum SDL_GPUStoreOp : std::uint32_t {
    SDL_GPU_STOREOP_STORE = 0,
    SDL_GPU_STOREOP_DONT_CARE = 1,
    SDL_GPU_STOREOP_RESOLVE = 2,
};

enum SDL_GPUIndexElementSize : std::uint32_t {
    SDL_GPU_INDEXELEMENTSIZE_16BIT = 0,
    SDL_GPU_INDEXELEMENTSIZE_32BIT = 1,
};

struct SDL_GPUBufferBinding {
    SDL_GPUBuffer* buffer = nullptr;
    std::uint32_t offset = 0;
};

struct SDL_GPUTextureSamplerBinding {
    SDL_GPUTexture* texture = nullptr;
    SDL_GPUSampler* sampler = nullptr;
};

struct SDL_GPUTextureTransferInfo {
    SDL_GPUTransferBuffer* transfer_buffer = nullptr;
    std::uint32_t offset = 0;
    std::uint32_t pixels_per_row = 0;
    std::uint32_t rows_per_layer = 0;
};

struct SDL_GPUTextureRegion {
    SDL_GPUTexture* texture = nullptr;
    std::uint32_t mip_level = 0;
    std::uint32_t layer = 0;
    std::uint32_t x = 0, y = 0, z = 0;
    std::uint32_t w = 0, h = 0, d = 0;
};

struct SDL_GPUBufferLocation {
    SDL_GPUBuffer* buffer = nullptr;
    std::uint32_t offset = 0;
};

struct SDL_GPUStorageTextureReadWriteBinding {
    SDL_GPUTexture* texture = nullptr;
    std::uint32_t mip_level = 0;
    std::uint32_t layer = 0;
    bool cycle = false;
};

struct SDL_GPUStorageBufferReadWriteBinding {
    SDL_GPUBuffer* buffer = nullptr;
    bool cycle = false;
};


// ============================================================================
// SDL3 VS SDL2: CAMBIOS CLAVE
// ============================================================================

/*
 * Cambios principales SDL2 -> SDL3:
 * 
 * Naming:
 *   SDL_Window* -> SDL_Window* (igual)
 *   SDL_Renderer* -> SDL_Renderer* (igual)
 *   SDL_Texture* -> SDL_Texture* (igual)
 *   PERO: SDL_CreateWindowAndRenderer -> SDL_CreateWindowAndRenderer (nuevo)
 *   SDL_CreateWindow + SDL_CreateRenderer -> sigue existiendo
 * 
 * Properties System (NUEVO):
 *   SDL_SetPointerProperty(props, "key", ptr)
 *   SDL_GetPointerProperty(props, "key", default)
 *   SDL_SetStringProperty, SDL_GetStringProperty
 *   SDL_SetNumberProperty, SDL_GetNumberProperty
 *   SDL_HasProperty, SDL_ClearProperty
 *   Aplicado a: ventanas, renderers, texturas, devices, gamepads, etc.
 * 
 * Main Callbacks (NUEVO - opcional):
 *   SDL_AppInit(void** appstate, int argc, char* argv[]) -> SDL_AppResult
 *   SDL_AppEvent(void* appstate, SDL_Event* event) -> SDL_AppResult
 *   SDL_AppIterate(void* appstate) -> SDL_AppResult
 *   SDL_AppQuit(void* appstate, SDL_AppResult result)
 *   Permite apps sin main() propio, cross-platform consistente
 * 
 * Events:
 *   SDL_Event: union con type + timestamp + common fields (windowID, which)
 *   Payloads específicos: window, keyboard, mouse, gamepad, touch, pen, audio, sensor, camera, drag-drop
 *   SDL_PollEvent, SDL_WaitEvent, SDL_PeepEvents
 * 
 * Error Handling:
 *   SDL_GetError() -> const char* (thread-local)
 *   SDL_ClearError()
 *   SDL_SetError(fmt, ...) - printf-style
 *   La mayoría de funciones devuelven bool/int con 0/false = error
 */

// ============================================================================
// SUBSYSTEMS
// ============================================================================

enum SDL_InitFlags : uint32_t {
    SDL_INIT_VIDEO        = 0x00000020,
    SDL_INIT_AUDIO        = 0x00000100,
    SDL_INIT_JOYSTICK     = 0x00000200,
    SDL_INIT_HAPTIC       = 0x00001000,
    SDL_INIT_GAMEPAD      = 0x00002000,
    SDL_INIT_EVENTS       = 0x00004000,
    SDL_INIT_SENSOR       = 0x00008000,
    SDL_INIT_CAMERA       = 0x00010000,
    SDL_INIT_GPU          = 0x00020000,
    SDL_INIT_STORAGE      = 0x00040000,
};

bool SDL_Init(uint32_t flags);
bool SDL_InitSubSystem(uint32_t flags);
void SDL_QuitSubSystem(uint32_t flags);
void SDL_Quit();

// ============================================================================
// PROPERTIES SYSTEM
// ============================================================================

typedef struct SDL_PropertiesID SDL_PropertiesID;

SDL_PropertiesID SDL_CreateProperties();
void SDL_DestroyProperties(SDL_PropertiesID props);
bool SDL_CopyProperties(SDL_PropertiesID src, SDL_PropertiesID dst);

bool SDL_SetPointerProperty(SDL_PropertiesID props, const char* name, void* value);
void* SDL_GetPointerProperty(SDL_PropertiesID props, const char* name, void* default_value);
bool SDL_SetStringProperty(SDL_PropertiesID props, const char* name, const char* value);
const char* SDL_GetStringProperty(SDL_PropertiesID props, const char* name, const char* default_value);
bool SDL_SetNumberProperty(SDL_PropertiesID props, const char* name, int64_t value);
int64_t SDL_GetNumberProperty(SDL_PropertiesID props, const char* name, int64_t default_value);
bool SDL_SetBooleanProperty(SDL_PropertiesID props, const char* name, bool value);
bool SDL_GetBooleanProperty(SDL_PropertiesID props, const char* name, bool default_value);
bool SDL_HasProperty(SDL_PropertiesID props, const char* name);
void SDL_ClearProperty(SDL_PropertiesID props, const char* name);
const char** SDL_GetPropertyKeys(SDL_PropertiesID props, int* count);

// ============================================================================
// WINDOW
// ============================================================================

enum SDL_WindowFlags : uint32_t {
    SDL_WINDOW_FULLSCREEN         = 0x00000001,
    SDL_WINDOW_OPENGL             = 0x00000002,
    SDL_WINDOW_HIDDEN             = 0x00000008,
    SDL_WINDOW_BORDERLESS         = 0x00000010,
    SDL_WINDOW_RESIZABLE          = 0x00000020,
    SDL_WINDOW_MINIMIZED          = 0x00000040,
    SDL_WINDOW_MAXIMIZED          = 0x00000080,
    SDL_WINDOW_MOUSE_GRABBED      = 0x00000100,
    SDL_WINDOW_INPUT_FOCUS        = 0x00000200,
    SDL_WINDOW_MOUSE_FOCUS        = 0x00000400,
    SDL_WINDOW_EXTERNAL           = 0x00000800,
    SDL_WINDOW_MODAL              = 0x00001000,
    SDL_WINDOW_HIGH_PIXEL_DENSITY = 0x00002000,
    SDL_WINDOW_MOUSE_CAPTURE      = 0x00004000,
    SDL_WINDOW_ALWAYS_ON_TOP      = 0x00008000,
    SDL_WINDOW_UTILITY            = 0x00010000,
    SDL_WINDOW_TOOLTIP            = 0x00020000,
    SDL_WINDOW_POPUP_MENU         = 0x00040000,
    SDL_WINDOW_VULKAN             = 0x10000000,
    SDL_WINDOW_METAL              = 0x20000000,
    SDL_WINDOW_TRANSPARENT        = 0x40000000,
    SDL_WINDOW_NOT_FOCUSABLE      = 0x80000000,
};

typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;

SDL_Window* SDL_CreateWindow(const char* title, int w, int h, SDL_WindowFlags flags);
bool SDL_CreateWindowAndRenderer(const char* title, int w, int h, SDL_WindowFlags window_flags, SDL_Window** window, SDL_Renderer** renderer);
void SDL_DestroyWindow(SDL_Window* window);

const char* SDL_GetWindowTitle(SDL_Window* window);
void SDL_SetWindowTitle(SDL_Window* window, const char* title);
void SDL_GetWindowPosition(SDL_Window* window, int* x, int* y);
void SDL_SetWindowPosition(SDL_Window* window, int x, int y);
void SDL_GetWindowSize(SDL_Window* window, int* w, int* h);
bool SDL_SetWindowSize(SDL_Window* window, int w, int h);
void SDL_GetWindowSizeInPixels(SDL_Window* window, int* w, int* h);
float SDL_GetWindowPixelDensity(SDL_Window* window);
void SDL_SetWindowOpacity(SDL_Window* window, float opacity);
void SDL_GetWindowOpacity(SDL_Window* window, float* opacity);
void SDL_SetWindowModal(SDL_Window* window, bool modal);
bool SDL_GetWindowModal(SDL_Window* window);
void SDL_SetWindowFullscreen(SDL_Window* window, bool fullscreen);
bool SDL_GetWindowFullscreen(SDL_Window* window);
void SDL_SetWindowGrab(SDL_Window* window, bool grabbed);
bool SDL_GetWindowGrab(SDL_Window* window);
void SDL_SetWindowKeyboardGrab(SDL_Window* window, bool grabbed);
void SDL_SetWindowMouseGrab(SDL_Window* window, bool grabbed);
void SDL_SetWindowMinimumSize(SDL_Window* window, int min_w, int min_h);
void SDL_SetWindowMaximumSize(SDL_Window* window, int max_w, int max_h);
void SDL_GetWindowBordersSize(SDL_Window* window, int* top, int* left, int* bottom, int* right);
bool SDL_WindowHasSurface(SDL_Window* window);
void SDL_FlashWindow(SDL_Window* window, int operation);
void SDL_DestroyWindowSurface(SDL_Window* window);
void* SDL_GetWindowICCProfile(SDL_Window* window, size_t* size);
SDL_PropertiesID SDL_GetWindowProperties(SDL_Window* window);

// HiDPI
void SDL_GetDisplayUsableBounds(int displayID, SDL_Rect* rect);
float SDL_GetDisplayContentScale(int displayID);
int SDL_GetDisplayForPoint(const SDL_Point* point);
int SDL_GetDisplayForRect(const SDL_Rect* rect);
int SDL_GetDisplayForWindow(SDL_Window* window);

// ============================================================================
// RENDERER (2D)
// ============================================================================

enum SDL_RendererFlags : uint32_t {
    SDL_RENDERER_SOFTWARE     = 0x00000001,
    SDL_RENDERER_ACCELERATED  = 0x00000002,
    SDL_RENDERER_PRESENTVSYNC = 0x00000004,
    SDL_RENDERER_TARGETTEXTURE = 0x00000008,
};

typedef struct SDL_Renderer SDL_Renderer;

SDL_Renderer* SDL_CreateRenderer(SDL_Window* window, const char* name, SDL_RendererFlags flags);
void SDL_DestroyRenderer(SDL_Renderer* renderer);
SDL_Window* SDL_GetRenderWindow(SDL_Renderer* renderer);
const char* SDL_GetRendererName(SDL_Renderer* renderer);
SDL_PropertiesID SDL_GetRendererProperties(SDL_Renderer* renderer);

bool SDL_SetRenderDrawColor(SDL_Renderer* renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
bool SDL_GetRenderDrawColor(SDL_Renderer* renderer, uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a);
bool SDL_RenderClear(SDL_Renderer* renderer);
bool SDL_RenderPresent(SDL_Renderer* renderer);
bool SDL_RenderRect(SDL_Renderer* renderer, const SDL_FRect* rect);
bool SDL_RenderFillRect(SDL_Renderer* renderer, const SDL_FRect* rect);
bool SDL_RenderLine(SDL_Renderer* renderer, float x1, float y1, float x2, float y2);
bool SDL_RenderPoint(SDL_Renderer* renderer, float x, float y);
bool SDL_RenderGeometry(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Vertex* vertices, int num_vertices, const int* indices, int num_indices);
bool SDL_RenderGeometryRaw(SDL_Renderer* renderer, SDL_Texture* texture, const float* xy, int xy_stride, const uint8_t* color, int color_stride, const float* uv, int uv_stride, int num_vertices, const void* indices, int num_indices, int size_indices);
bool SDL_RenderTexture(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_FRect* srcrect, const SDL_FRect* dstrect);
bool SDL_RenderTextureRotated(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_FRect* srcrect, const SDL_FRect* dstrect, double angle, const SDL_FPoint* center, SDL_FlipMode flip);
bool SDL_RenderTextureAffine(SDL_Renderer* renderer, SDL_Texture* texture, const float* origin, const float* right, const float* down, const SDL_FRect* srcrect, SDL_FRect* dstrect);
bool SDL_RenderTextureTiled(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_FRect* srcrect, float scale, const SDL_FRect* dstrect);

// Target textures
SDL_Texture* SDL_CreateTexture(SDL_Renderer* renderer, uint32_t format, int access, int w, int h);
SDL_Texture* SDL_CreateTextureFromSurface(SDL_Renderer* renderer, SDL_Surface* surface);
void SDL_DestroyTexture(SDL_Texture* texture);
bool SDL_SetTextureColorMod(SDL_Texture* texture, uint8_t r, uint8_t g, uint8_t b);
bool SDL_SetTextureAlphaMod(SDL_Texture* texture, uint8_t alpha);
bool SDL_SetTextureBlendMode(SDL_Texture* texture, SDL_BlendMode blendMode);
bool SDL_SetTextureScaleMode(SDL_Texture* texture, SDL_ScaleMode scaleMode);
void SDL_GetTextureSize(SDL_Texture* texture, int* w, int* h);

// ============================================================================
// GPU ABSTRACTION (SDL_GPU) - NUEVO EN SDL3
// ============================================================================

/*
 * SDL_GPU: abstraction moderna sobre Vulkan/Metal/DX12
 * - Device: SDL_GPUDevice* (create con preferencias de backend)
 * - Pipeline: graphics (vertex+fragment) y compute
 * - Buffers: vertex, index, uniform, storage, indirect
 * - Textures: 2D, 3D, cube, array, multisample
 * - Samplers: filter, address mode, anisotropy, comparison
 * - Command Buffer: grab -> commands -> submit
 * - Passes: render (color+depth), compute, copy
 * - Swapchain: acquire -> render -> present
 * - Sync: fences, semaphores
 * - Transfer: upload/download buffers/textures
 * 
 * Backends: Vulkan, Metal, DirectX 12
 * Selección automática o forzada via SDL_GPUDeviceCreateInfo
 */

typedef struct SDL_GPUDevice SDL_GPUDevice;
typedef struct SDL_GPUCommandBuffer SDL_GPUCommandBuffer;
typedef struct SDL_GPUGraphicsPipeline SDL_GPUGraphicsPipeline;
typedef struct SDL_GPUComputePipeline SDL_GPUComputePipeline;
typedef struct SDL_GPUBuffer SDL_GPUBuffer;
typedef struct SDL_GPUTexture SDL_GPUTexture;
typedef struct SDL_GPUSampler SDL_GPUSampler;
typedef struct SDL_GPUShader SDL_GPUShader;
typedef struct SDL_GPURenderPass SDL_GPURenderPass;
typedef struct SDL_GPUComputePass SDL_GPUComputePass;
typedef struct SDL_GPUCopyPass SDL_GPUCopyPass;
typedef struct SDL_GPUFence SDL_GPUFence;

enum SDL_GPUShaderStage : uint32_t {
    SDL_GPU_SHADERSTAGE_VERTEX   = 0x00000001,
    SDL_GPU_SHADERSTAGE_FRAGMENT = 0x00000002,
    SDL_GPU_SHADERSTAGE_COMPUTE  = 0x00000004,
};

enum SDL_GPUTextureFormat : uint32_t {
    SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM = 0,
    SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM = 1,
    SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT = 2,
    SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT = 3,
    SDL_GPU_TEXTUREFORMAT_D32_FLOAT = 100,
    // ... muchos más
};

enum SDL_GPUPrimitiveType : uint32_t {
    SDL_GPU_PRIMITIVETYPE_TRIANGLELIST = 0,
    SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP = 1,
    SDL_GPU_PRIMITIVETYPE_LINELIST = 2,
    SDL_GPU_PRIMITIVETYPE_LINE_STRIP = 3,
    SDL_GPU_PRIMITIVETYPE_POINTLIST = 4,
};

struct SDL_GPUDeviceCreateInfo {
    bool debug_mode = false;
    bool prefer_low_power = false;
    uint32_t max_frames_in_flight = 2;
    // backend preference: 0=auto, 1=vulkan, 2=metal, 3=dx12
    int backend_preference = 0;
    const char* shader_language = "SPIRV"; // or "MSL", "DXIL"
    void* sdl_window = nullptr; // for swapchain
};

SDL_GPUDevice* SDL_CreateGPUDevice(const SDL_GPUDeviceCreateInfo* createinfo);
void SDL_DestroyGPUDevice(SDL_GPUDevice* device);
bool SDL_GPUDeviceSupportsTextureFormat(SDL_GPUDevice* device, SDL_GPUTextureFormat format, SDL_GPUTextureType type, SDL_GPUSampleCount sample_count);
bool SDL_GPUDeviceSupportsSampleCount(SDL_GPUDevice* device, SDL_GPUTextureFormat format, SDL_GPUSampleCount sample_count);

// Swapchain
bool SDL_ClaimWindowForGPUDevice(SDL_GPUDevice* device, SDL_Window* window);
void SDL_ReleaseWindowFromGPUDevice(SDL_GPUDevice* device, SDL_Window* window);
bool SDL_SetGPUSwapchainParameters(SDL_GPUDevice* device, SDL_Window* window, SDL_GPUTextureFormat format, SDL_GPUSwapchainComposition composition, SDL_GPUPresentMode present_mode);
bool SDL_AcquireGPUSwapchainTexture(SDL_GPUCommandBuffer* cmd, SDL_Window* window, SDL_GPUTexture** out_texture, uint32_t* out_width, uint32_t* out_height);

// Shaders
struct SDL_GPUShaderCreateInfo {
    SDL_GPUShaderStage stage;
    const uint8_t* code;
    size_t code_size;
    const char* entrypoint = "main";
    uint32_t num_samplers = 0;
    uint32_t num_uniform_buffers = 0;
    uint32_t num_storage_buffers = 0;
    uint32_t num_storage_textures = 0;
};

SDL_GPUShader* SDL_CreateGPUShader(SDL_GPUDevice* device, const SDL_GPUShaderCreateInfo* createinfo);
void SDL_ReleaseGPUShader(SDL_GPUDevice* device, SDL_GPUShader* shader);

// Pipelines
struct SDL_GPUGraphicsPipelineCreateInfo {
    SDL_GPUShader* vertex_shader;
    SDL_GPUShader* fragment_shader;
    SDL_GPUPrimitiveType primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    // vertex input, input assembly, rasterizer, multisample, depth stencil, color target
    uint32_t num_vertex_buffers = 0;
    // ... más campos
};

SDL_GPUGraphicsPipeline* SDL_CreateGPUGraphicsPipeline(SDL_GPUDevice* device, const struct SDL_GPUGraphicsPipelineCreateInfo* createinfo);
void SDL_ReleaseGPUGraphicsPipeline(SDL_GPUDevice* device, SDL_GPUGraphicsPipeline* pipeline);

// Buffers
struct SDL_GPUBufferCreateInfo {
    SDL_GPUBufferUsageFlags usage;
    uint32_t size;
    void* data = nullptr; // initial data
};

SDL_GPUBuffer* SDL_CreateGPUBuffer(SDL_GPUDevice* device, const struct SDL_GPUBufferCreateInfo* createinfo);
void SDL_ReleaseGPUBuffer(SDL_GPUDevice* device, SDL_GPUBuffer* buffer);
bool SDL_UploadToGPUBuffer(SDL_GPUTransferBuffer* transfer, SDL_GPUBuffer* buffer, uint32_t offset, uint32_t size, const void* data, bool cycle);
bool SDL_DownloadFromGPUBuffer(SDL_GPUTransferBuffer* transfer, SDL_GPUBuffer* buffer, uint32_t offset, uint32_t size, void* data, bool cycle);

// Textures
struct SDL_GPUTextureCreateInfo {
    SDL_GPUTextureType type = SDL_GPU_TEXTURETYPE_2D;
    SDL_GPUTextureFormat format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t layer_count_or_depth = 1;
    uint32_t num_levels = 1;
    SDL_GPUTextureUsageFlags usage;
    SDL_GPUSampleCount sample_count = SDL_GPU_SAMPLECOUNT_1;
};

SDL_GPUTexture* SDL_CreateGPUTexture(SDL_GPUDevice* device, const struct SDL_GPUTextureCreateInfo* createinfo);
void SDL_ReleaseGPUTexture(SDL_GPUDevice* device, SDL_GPUTexture* texture);

// Samplers
struct SDL_GPUSamplerCreateInfo {
    SDL_GPUFilter min_filter = SDL_GPU_FILTER_LINEAR;
    SDL_GPUFilter mag_filter = SDL_GPU_FILTER_LINEAR;
    SDL_GPUFilter mipmap_filter = SDL_GPU_FILTER_LINEAR;
    SDL_GPUAddressMode address_mode_u = SDL_GPU_ADDRESSMODE_REPEAT;
    SDL_GPUAddressMode address_mode_v = SDL_GPU_ADDRESSMODE_REPEAT;
    SDL_GPUAddressMode address_mode_w = SDL_GPU_ADDRESSMODE_REPEAT;
    float mip_lod_bias = 0.0f;
    float max_anisotropy = 1.0f;
    SDL_GPUCompareOp compare_op = SDL_GPU_COMPAREOP_INVALID;
    float min_lod = 0.0f;
    float max_lod = 1000.0f;
};

SDL_GPUSampler* SDL_CreateGPUSampler(SDL_GPUDevice* device, const struct SDL_GPUSamplerCreateInfo* createinfo);
void SDL_ReleaseGPUSampler(SDL_GPUDevice* device, SDL_GPUSampler* sampler);

// Command Buffer
SDL_GPUCommandBuffer* SDL_AcquireGPUCommandBuffer(SDL_GPUDevice* device);
bool SDL_SubmitGPUCommandBuffer(SDL_GPUCommandBuffer* cmd);
bool SDL_SubmitGPUCommandBufferAndAcquireFence(SDL_GPUCommandBuffer* cmd, SDL_GPUFence** out_fence);

// Render Pass
struct SDL_GPUColorTargetInfo {
    SDL_GPUTexture* texture;
    float clear_color[4] = {0, 0, 0, 0};
    bool load = true;
    bool store = true;
    SDL_GPULoadOp load_op = SDL_GPU_LOADOP_CLEAR;
    SDL_GPUStoreOp store_op = SDL_GPU_STOREOP_STORE;
};

struct SDL_GPUDepthStencilTargetInfo {
    SDL_GPUTexture* texture;
    float clear_depth = 1.0f;
    uint8_t clear_stencil = 0;
    bool load = true;
    bool store = true;
    SDL_GPULoadOp load_op = SDL_GPU_LOADOP_CLEAR;
    SDL_GPUStoreOp store_op = SDL_GPU_STOREOP_STORE;
};

SDL_GPURenderPass* SDL_BeginGPURenderPass(SDL_GPUCommandBuffer* cmd, const SDL_GPUColorTargetInfo* color_targets, uint32_t num_color_targets, const SDL_GPUDepthStencilTargetInfo* depth_stencil);
void SDL_BindGPUGraphicsPipeline(SDL_GPURenderPass* render_pass, SDL_GPUGraphicsPipeline* pipeline);
void SDL_BindGPUVertexBuffers(SDL_GPURenderPass* render_pass, uint32_t first_slot, const SDL_GPUBufferBinding* bindings, uint32_t num_bindings);
void SDL_BindGPUIndexBuffer(SDL_GPURenderPass* render_pass, const SDL_GPUBufferBinding* binding, SDL_GPUIndexElementSize index_size);
void SDL_BindGPUVertexSamplers(SDL_GPURenderPass* render_pass, uint32_t first_slot, const SDL_GPUTextureSamplerBinding* bindings, uint32_t num_bindings);
void SDL_BindGPUVertexStorageBuffers(SDL_GPURenderPass* render_pass, uint32_t first_slot, const SDL_GPUBufferBinding* bindings, uint32_t num_bindings);
void SDL_BindGPUFragmentSamplers(SDL_GPURenderPass* render_pass, uint32_t first_slot, const SDL_GPUTextureSamplerBinding* bindings, uint32_t num_bindings);
void SDL_BindGPUFragmentStorageBuffers(SDL_GPURenderPass* render_pass, uint32_t first_slot, const SDL_GPUBufferBinding* bindings, uint32_t num_bindings);
void SDL_DrawGPUPrimitives(SDL_GPURenderPass* render_pass, uint32_t num_vertices, uint32_t num_instances, uint32_t first_vertex, uint32_t first_instance);
void SDL_DrawGPUIndexedPrimitives(SDL_GPURenderPass* render_pass, uint32_t num_indices, uint32_t num_instances, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance);
void SDL_EndGPURenderPass(SDL_GPURenderPass* render_pass);

// Compute Pass
SDL_GPUComputePass* SDL_BeginGPUComputePass(SDL_GPUCommandBuffer* cmd, const SDL_GPUStorageTextureReadWriteBinding* storage_textures, uint32_t num_storage_textures, const SDL_GPUStorageBufferReadWriteBinding* storage_buffers, uint32_t num_storage_buffers);
void SDL_BindGPUComputePipeline(SDL_GPUComputePass* compute_pass, SDL_GPUComputePipeline* pipeline);
void SDL_BindGPUComputeSamplers(SDL_GPUComputePass* compute_pass, uint32_t first_slot, const SDL_GPUTextureSamplerBinding* bindings, uint32_t num_bindings);
void SDL_BindGPUComputeStorageBuffers(SDL_GPUComputePass* compute_pass, uint32_t first_slot, const SDL_GPUBufferBinding* bindings, uint32_t num_bindings);
void SDL_BindGPUComputeStorageTextures(SDL_GPUComputePass* compute_pass, uint32_t first_slot, const SDL_GPUTextureSamplerBinding* bindings, uint32_t num_bindings);
void SDL_DispatchGPUCompute(SDL_GPUComputePass* compute_pass, uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z);
void SDL_EndGPUComputePass(SDL_GPUComputePass* compute_pass);

// Copy Pass
SDL_GPUCopyPass* SDL_BeginGPUCopyPass(SDL_GPUCommandBuffer* cmd);
void SDL_UploadToGPUTexture(SDL_GPUCopyPass* copy_pass, const SDL_GPUTextureTransferInfo* source, const SDL_GPUTextureRegion* destination, bool cycle);
void SDL_DownloadFromGPUTexture(SDL_GPUCopyPass* copy_pass, const SDL_GPUTextureRegion* source, const SDL_GPUTextureTransferInfo* destination);
void SDL_CopyGPUBufferToBuffer(SDL_GPUCopyPass* copy_pass, const SDL_GPUBufferLocation* source, const SDL_GPUBufferLocation* destination, bool cycle);
void SDL_EndGPUCopyPass(SDL_GPUCopyPass* copy_pass);

// ============================================================================
// EVENTS
// ============================================================================

enum SDL_EventType : uint32_t {
    SDL_EVENT_FIRST           = 0,
    SDL_EVENT_QUIT            = 0x100,
    SDL_EVENT_TERMINATING     = 0x101,
    SDL_EVENT_LOW_MEMORY      = 0x102,
    SDL_EVENT_WILL_ENTER_BACKGROUND = 0x103,
    SDL_EVENT_DID_ENTER_BACKGROUND  = 0x104,
    SDL_EVENT_WILL_ENTER_FOREGROUND = 0x105,
    SDL_EVENT_DID_ENTER_FOREGROUND  = 0x106,
    SDL_EVENT_LOCALE_CHANGED  = 0x107,
    SDL_EVENT_SYSTEM_THEME_CHANGED  = 0x108,
    
    // Window events
    SDL_EVENT_WINDOW_SHOWN    = 0x200,
    SDL_EVENT_WINDOW_HIDDEN   = 0x201,
    SDL_EVENT_WINDOW_EXPOSED  = 0x202,
    SDL_EVENT_WINDOW_MOVED    = 0x203,
    SDL_EVENT_WINDOW_RESIZED  = 0x204,
    SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED = 0x205,
    SDL_WINDOW_METAL_VIEW_RESIZED = 0x206,
    SDL_EVENT_WINDOW_MINIMIZED = 0x207,
    SDL_EVENT_WINDOW_MAXIMIZED = 0x208,
    SDL_EVENT_WINDOW_RESTORED = 0x209,
    SDL_EVENT_WINDOW_ENTER_FULLSCREEN = 0x20A,
    SDL_EVENT_WINDOW_LEAVE_FULLSCREEN = 0x20B,
    SDL_EVENT_WINDOW_DESTROYED = 0x20C,
    SDL_EVENT_WINDOW_HIT_TEST = 0x20D,
    SDL_EVENT_WINDOW_ICC_PROF_CHANGED = 0x20E,
    SDL_EVENT_WINDOW_DISPLAY_CHANGED = 0x20F,
    SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED = 0x210,
    SDL_EVENT_WINDOW_SAFE_AREA_CHANGED = 0x211,
    SDL_EVENT_WINDOW_OCCLUDED = 0x212,
    SDL_EVENT_WINDOW_ENTER_PICTURE_IN_PICTURE = 0x213,
    SDL_EVENT_WINDOW_LEAVE_PICTURE_IN_PICTURE = 0x214,
    
    // Keyboard
    SDL_EVENT_KEY_DOWN        = 0x300,
    SDL_EVENT_KEY_UP          = 0x301,
    SDL_EVENT_TEXT_EDITING    = 0x302,
    SDL_EVENT_TEXT_INPUT      = 0x303,
    SDL_EVENT_KEYMAP_CHANGED  = 0x304,
    SDL_EVENT_TEXT_EDITING_CANDIDATES = 0x305,
    
    // Mouse
    SDL_EVENT_MOUSE_MOTION    = 0x400,
    SDL_EVENT_MOUSE_BUTTON_DOWN = 0x401,
    SDL_EVENT_MOUSE_BUTTON_UP   = 0x402,
    SDL_EVENT_MOUSE_WHEEL     = 0x403,
    
    // Gamepad
    SDL_EVENT_GAMEPAD_AXIS_MOTION = 0x600,
    SDL_EVENT_GAMEPAD_BUTTON_DOWN = 0x601,
    SDL_EVENT_GAMEPAD_BUTTON_UP   = 0x602,
    SDL_EVENT_GAMEPAD_ADDED      = 0x603,
    SDL_EVENT_GAMEPAD_REMOVED    = 0x604,
    SDL_EVENT_GAMEPAD_REMAPPED   = 0x605,
    SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN = 0x606,
    SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION = 0x607,
    SDL_EVENT_GAMEPAD_TOUCHPAD_UP   = 0x608,
    SDL_EVENT_GAMEPAD_SENSOR_UPDATE = 0x609,
    
    // Touch/Pen
    SDL_EVENT_FINGER_DOWN     = 0x700,
    SDL_EVENT_FINGER_UP       = 0x701,
    SDL_EVENT_FINGER_MOTION   = 0x702,
    SDL_EVENT_PEN_PROXIMITY_IN = 0x710,
    SDL_EVENT_PEN_PROXIMITY_OUT = 0x711,
    SDL_EVENT_PEN_DOWN        = 0x712,
    SDL_EVENT_PEN_UP          = 0x713,
    SDL_EVENT_PEN_MOTION      = 0x714,
    SDL_EVENT_PEN_BUTTON_DOWN = 0x715,
    SDL_EVENT_PEN_BUTTON_UP   = 0x716,
    
    // Drag-drop
    SDL_EVENT_DROP_BEGIN      = 0x1000,
    SDL_EVENT_DROP_FILE       = 0x1001,
    SDL_EVENT_DROP_TEXT       = 0x1002,
    SDL_EVENT_DROP_COMPLETE   = 0x1003,
    SDL_EVENT_DROP_POSITION   = 0x1004,
    
    // Audio/Clipboard
    SDL_EVENT_AUDIO_DEVICE_ADDED = 0x1100,
    SDL_EVENT_AUDIO_DEVICE_REMOVED = 0x1101,
    SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED = 0x1102,
    SDL_EVENT_CLIPBOARD_UPDATE = 0x1200,
};

struct SDL_Event {
    uint32_t type;
    uint32_t reserved;
    uint64_t timestamp;
    
    union {
        struct { uint32_t windowID; int data1; int data2; } window;
        struct { uint32_t windowID; SDL_Scancode scancode; SDL_Keycode key; uint16_t mod; bool repeat; } key;
        struct { uint32_t windowID; float x; float y; float xrel; float yrel; uint32_t button; uint32_t clicks; } motion;
        struct { uint32_t windowID; uint32_t which; uint8_t button; uint8_t state; uint8_t clicks; } button;
        struct { uint32_t windowID; float x; float y; float precise_x; float precise_y; } wheel;
        struct { uint32_t which; float axis[6]; } gaxis;
        struct { uint32_t which; uint8_t button; uint8_t state; } gbutton;
        struct { uint32_t which; int sensor; float data[3]; uint64_t timestamp; } gsensor;
        struct { uint32_t which; int fingerId; float x; float y; float pressure; } tfinger;
        struct { uint32_t which; int penId; float x; float y; float pressure; float rotation; float tilt_x; float tilt_y; } pen;
        struct { uint32_t windowID; const char* file; } drop;
        struct { uint32_t which; } audio;
        struct { uint64_t timestamp; } quit;
    };
};

bool SDL_PollEvent(SDL_Event* event);
bool SDL_WaitEvent(SDL_Event* event);
bool SDL_WaitEventTimeout(SDL_Event* event, int timeoutMS);
int SDL_PeepEvents(SDL_Event* events, int numevents, int action, uint32_t minType, uint32_t maxType);
bool SDL_PushEvent(SDL_Event* event);

// ============================================================================
// AUDIO (SDL_AudioStream)
// ============================================================================

/*
 * SDL3 Audio: basado en SDL_AudioStream (push/pull model)
 * - Dispositivo físico: SDL_AudioDeviceID
 * - Stream lógico: SDL_AudioStream (conversión formato, rate, channels)
 * - Callback O push manual: SDL_AudioStreamPut / SDL_AudioStreamGet
 * - Logical devices: múltiples streams -> un device físico
 * - Formatos: U8, S16, S32, F32 (planar/interleaved)
 * - Resampling: linear/cubic (configurable)
 */

typedef struct SDL_AudioStream SDL_AudioStream;
typedef uint32_t SDL_AudioDeviceID;

enum SDL_AudioFormat : uint32_t {
    SDL_AUDIO_U8     = 0x0008,
    SDL_AUDIO_S16    = 0x8010,
    SDL_AUDIO_S32    = 0x8020,
    SDL_AUDIO_F32    = 0x8120,
};

struct SDL_AudioSpec {
    int freq = 0;
    SDL_AudioFormat format = static_cast<SDL_AudioFormat>(0);
    int channels = 0;
};

SDL_AudioStream* SDL_CreateAudioStream(const SDL_AudioSpec* src_spec, const SDL_AudioSpec* dst_spec);
void SDL_DestroyAudioStream(SDL_AudioStream* stream);
bool SDL_AudioStreamPut(SDL_AudioStream* stream, const void* buf, int len);
bool SDL_AudioStreamGet(SDL_AudioStream* stream, void* buf, int len);
int SDL_AudioStreamAvailable(SDL_AudioStream* stream);
bool SDL_AudioStreamFlush(SDL_AudioStream* stream);
bool SDL_AudioStreamClear(SDL_AudioStream* stream);

bool SDL_OpenAudioDevice(SDL_AudioDeviceID devid, const SDL_AudioSpec* spec);
bool SDL_BindAudioStream(SDL_AudioDeviceID devid, SDL_AudioStream* stream);
void SDL_UnbindAudioStream(SDL_AudioStream* stream);
void SDL_CloseAudioDevice(SDL_AudioDeviceID devid);

// ============================================================================
// STORAGE (VFS ABSTRACTION)
// ============================================================================

/*
 * SDL_Storage: abstraction unificada sobre filesystem
 * - Título (read-only, app bundle): SDL_GetTitleStorage
 * - Usuario (read-write, prefs/saves): SDL_GetUserStorage
 * - Base (read-only, assets): SDL_GetBasePath -> SDL_OpenStorage
 * - Operaciones: read, write, enumerate, exists, remove, mkdir, stat
 * - Async: SDL_StorageReadFileAsync, callback al completar
 * - Cross-platform: Win32, POSIX, Android, iOS, WebAssembly
 */

typedef struct SDL_Storage SDL_Storage;

SDL_Storage* SDL_OpenStorage(const char* path);
SDL_Storage* SDL_OpenTitleStorage(const char* org, const char* app, const char* branch);
SDL_Storage* SDL_GetUserStorage(const char* org, const char* app);
void SDL_CloseStorage(SDL_Storage* storage);

bool SDL_StorageReady(SDL_Storage* storage);
bool SDL_StorageFileExists(SDL_Storage* storage, const char* path);
int64_t SDL_GetStorageFileSize(SDL_Storage* storage, const char* path);
bool SDL_StorageReadFile(SDL_Storage* storage, const char* path, void* dst, uint64_t length);
bool SDL_StorageWriteFile(SDL_Storage* storage, const char* path, const void* src, uint64_t length, bool append);
bool SDL_StorageRemove(SDL_Storage* storage, const char* path);
bool SDL_StorageMkdir(SDL_Storage* storage, const char* path);
typedef bool (*SDL_EnumerateDirectoryCallback)(void* userdata, const char* dirname, const char* fname);

bool SDL_StorageEnumerateDirectory(SDL_Storage* storage, const char* path, SDL_EnumerateDirectoryCallback callback, void* userdata);

// ============================================================================
// THREADING
// ============================================================================

typedef struct SDL_Thread SDL_Thread;
typedef struct SDL_Mutex SDL_Mutex;
typedef struct SDL_Condition SDL_Condition;
typedef struct SDL_RWLock SDL_RWLock;

SDL_Thread* SDL_CreateThread(int (*fn)(void*), const char* name, void* data);
int SDL_WaitThread(SDL_Thread* thread, int* status);
void SDL_DetachThread(SDL_Thread* thread);

SDL_Mutex* SDL_CreateMutex();
void SDL_DestroyMutex(SDL_Mutex* mutex);
bool SDL_LockMutex(SDL_Mutex* mutex);
bool SDL_TryLockMutex(SDL_Mutex* mutex);
bool SDL_UnlockMutex(SDL_Mutex* mutex);

SDL_Condition* SDL_CreateCondition();
void SDL_DestroyCondition(SDL_Condition* cond);
bool SDL_WaitCondition(SDL_Condition* cond, SDL_Mutex* mutex);
bool SDL_WaitConditionTimeout(SDL_Condition* cond, SDL_Mutex* mutex, int timeoutMS);
bool SDL_BroadcastCondition(SDL_Condition* cond);
bool SDL_SignalCondition(SDL_Condition* cond);

SDL_RWLock* SDL_CreateRWLock();
void SDL_DestroyRWLock(SDL_RWLock* rwlock);
bool SDL_LockRWLockForReading(SDL_RWLock* rwlock);
bool SDL_LockRWLockForWriting(SDL_RWLock* rwlock);
bool SDL_TryLockRWLockForReading(SDL_RWLock* rwlock);
bool SDL_TryLockRWLockForWriting(SDL_RWLock* rwlock);
bool SDL_UnlockRWLock(SDL_RWLock* rwlock);

// TLS
typedef uint32_t SDL_TLSID;
SDL_TLSID SDL_CreateTLS();
bool SDL_SetTLS(SDL_TLSID id, void* value);
void* SDL_GetTLS(SDL_TLSID id);
void SDL_DestroyTLS(SDL_TLSID id);

// ============================================================================
// TIME
// ============================================================================

uint64_t SDL_GetTicks();           // ms since init (32-bit wraparound)
uint64_t SDL_GetTicksNS();         // nanoseconds since init (no wraparound)
uint64_t SDL_GetPerformanceCounter();
uint64_t SDL_GetPerformanceFrequency();
void SDL_Delay(int ms);
void SDL_DelayNS(uint64_t ns);
void SDL_DelayPrecise(int ms);

// ============================================================================
// EJEMPLO MÍNIMO SDL3
// ============================================================================

/*
// main callbacks style (opcional)
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;
    SDL_Window* window = SDL_CreateWindow("SDL3 App", 800, 600, SDL_WINDOW_RESIZABLE);
    *appstate = window;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    SDL_Window* window = (SDL_Window*)appstate;
    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
    if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event->window.windowID == SDL_GetWindowID(window))
        return SDL_APP_SUCCESS;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    // render frame
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    SDL_Window* window = (SDL_Window*)appstate;
    SDL_DestroyWindow(window);
    SDL_Quit();
}
*/

} // namespace modulo28_sdl3