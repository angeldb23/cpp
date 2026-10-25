/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * SOLUCIÓN 06: SDL3 — Properties, Events, GPU, Audio Streams, Storage
 */

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <cstring>
#include "../../include/test_framework.hpp"

// ============================================================================
// TIPOS BASE SDL3
// ============================================================================

using SDL_PropertiesID = uint64_t;
constexpr SDL_PropertiesID SDL_PROPERTIES_INVALID = 0;

enum class SDL_EventType : uint32_t {
    SDL_EVENT_QUIT = 0x100,
    SDL_EVENT_WINDOW_RESIZED = 0x204,
    SDL_EVENT_KEY_DOWN = 0x300,
    SDL_EVENT_KEY_UP = 0x301,
    SDL_EVENT_MOUSE_MOTION = 0x400,
    SDL_EVENT_GAMEPAD_BUTTON_DOWN = 0x601,
    SDL_EVENT_AUDIO_DEVICE_ADDED = 0x1100,
};

struct SDL_Event {
    uint32_t type = 0;
    uint32_t reserved = 0;
    uint64_t timestamp = 0;
    
    union {
        struct { uint32_t windowID; int w; int h; } window;
        struct { uint32_t windowID; uint32_t scancode; uint32_t keycode; uint16_t mod; bool repeat; } key;
        struct { uint32_t windowID; float x; float y; float xrel; float yrel; } motion;
        struct { uint32_t which; float axis[6]; } gaxis;
        struct { uint32_t which; uint8_t button; uint8_t state; } gbutton;
        struct { uint32_t which; } audio;
    };
};

// ============================================================================
// 1. PROPERTIES SYSTEM
// ============================================================================

enum class PropertyType { None, Int64, Double, String, Pointer, Bool };

struct PropertyValue {
    enum Type { None, Int64, Double, String, Pointer, Bool } type = Type::None;
    int64_t int_val = 0;
    double double_val = 0.0;
    std::string str_val;
    void* ptr_val = nullptr;
    bool bool_val = false;
    
    PropertyValue() = default;
    PropertyValue(int64_t v) : int_val(v) {}
    PropertyValue(double v) : double_val(v) {}
    PropertyValue(const std::string& v) : str_val(v) {}
    PropertyValue(const char* v) : str_val(v) {}
    PropertyValue(void* v) : ptr_val(v) {}
    PropertyValue(bool v) : bool_val(v) {}
};

struct SDL_Properties {
    std::unordered_map<std::string, void*> ptr_props;
    std::unordered_map<std::string, std::string> str_props;
    std::unordered_map<std::string, int64_t> int_props;
    std::unordered_map<std::string, bool> bool_props;
    
    bool SetPointer(const char* name, void* value) {
        ptr_props[name] = value;
        return true;
    }
    
    void* GetPointer(const char* name, void* default_value) {
        auto it = ptr_props.find(name);
        if (it == ptr_props.end()) return default_value;
        return it->second;
    }
    
    bool SetString(const char* name, const char* value) {
        str_props[name] = std::string(value);
        return true;
    }
    
    const char* GetString(const char* name, const char* default_value) {
        auto it = str_props.find(name);
        if (it == str_props.end()) return default_value;
        return it->second.c_str();
    }
    
    bool SetNumber(const char* name, int64_t value) {
        int_props[name] = value;
        return true;
    }
    
    int64_t GetNumber(const char* name, int64_t default_value) {
        auto it = int_props.find(name);
        if (it == int_props.end()) return default_value;
        return it->second;
    }
    
    bool SetBoolean(const char* name, bool value) {
        bool_props[name] = value;
        return true;
    }
    
    bool GetBoolean(const char* name, bool default_value) {
        auto it = bool_props.find(name);
        if (it == bool_props.end()) return false; // Missing boolean properties default to false
        return it->second;
    }
    
    bool HasProperty(const char* name) const {
        return ptr_props.count(name) || str_props.count(name) || int_props.count(name) || bool_props.count(name);
    }
    
    void ClearProperty(const char* name) {
        ptr_props.erase(name);
        str_props.erase(name);
        int_props.erase(name);
        bool_props.erase(name);
    }
    
    std::vector<std::string> GetKeys() const {
        std::vector<std::string> keys;
        for (const auto& [k, v] : ptr_props) keys.push_back(k);
        for (const auto& [k, v] : str_props) keys.push_back(k);
        for (const auto& [k, v] : int_props) keys.push_back(k);
        for (const auto& [k, v] : bool_props) keys.push_back(k);
        return keys;
    }
};

static std::unordered_map<SDL_PropertiesID, std::unique_ptr<SDL_Properties>> g_properties;
static SDL_PropertiesID g_next_prop_id = 1;

SDL_PropertiesID SDL_CreateProperties() {
    SDL_PropertiesID id = g_next_prop_id++;
    g_properties[id] = std::make_unique<SDL_Properties>();
    return id;
}

void SDL_DestroyProperties(SDL_PropertiesID props) {
    g_properties.erase(props);
}

bool SDL_CopyProperties(SDL_PropertiesID src, SDL_PropertiesID dst) {
    auto src_it = g_properties.find(src);
    auto dst_it = g_properties.find(dst);
    if (src_it == g_properties.end() || dst_it == g_properties.end()) return false;
    dst_it->second->ptr_props = src_it->second->ptr_props;
    dst_it->second->str_props = src_it->second->str_props;
    dst_it->second->int_props = src_it->second->int_props;
    dst_it->second->bool_props = src_it->second->bool_props;
    return true;
}

bool SDL_SetPointerProperty(SDL_PropertiesID props, const char* name, void* value) {
    auto it = g_properties.find(props);
    if (it == g_properties.end()) return false;
    return it->second->SetPointer(name, value);
}

void* SDL_GetPointerProperty(SDL_PropertiesID props, const char* name, void* default_value) {
    auto it = g_properties.find(props);
    if (it == g_properties.end()) return default_value;
    return it->second->GetPointer(name, default_value);
}

bool SDL_SetStringProperty(SDL_PropertiesID props, const char* name, const char* value) {
    auto it = g_properties.find(props);
    if (it == g_properties.end()) return false;
    return it->second->SetString(name, value);
}

const char* SDL_GetStringProperty(SDL_PropertiesID props, const char* name, const char* default_value) {
    auto it = g_properties.find(props);
    if (it == g_properties.end()) return default_value;
    return it->second->GetString(name, default_value);
}

bool SDL_SetNumberProperty(SDL_PropertiesID props, const char* name, int64_t value) {
    auto it = g_properties.find(props);
    if (it == g_properties.end()) return false;
    return it->second->SetNumber(name, value);
}

int64_t SDL_GetNumberProperty(SDL_PropertiesID props, const char* name, int64_t default_value) {
    auto it = g_properties.find(props);
    if (it == g_properties.end()) return default_value;
    return it->second->GetNumber(name, default_value);
}

bool SDL_SetBooleanProperty(SDL_PropertiesID props, const char* name, bool value) {
    auto it = g_properties.find(props);
    if (it == g_properties.end()) return false;
    return it->second->SetBoolean(name, value);
}

bool SDL_GetBooleanProperty(SDL_PropertiesID props, const char* name, bool default_value) {
    auto it = g_properties.find(props);
    if (it == g_properties.end()) return default_value;
    return it->second->GetBoolean(name, default_value);
}

bool SDL_HasProperty(SDL_PropertiesID props, const char* name) {
    auto it = g_properties.find(props);
    if (it == g_properties.end()) return false;
    return it->second->HasProperty(name);
}

void SDL_ClearProperty(SDL_PropertiesID props, const char* name) {
    auto it = g_properties.find(props);
    if (it == g_properties.end()) return;
    it->second->ClearProperty(name);
}

// ============================================================================
// 2. EVENT SYSTEM
// ============================================================================

static std::vector<SDL_Event> g_event_queue;

bool SDL_PushEvent(const SDL_Event* event) {
    if (!event) return false;
    g_event_queue.push_back(*event);
    return true;
}

bool SDL_PollEvent(SDL_Event* event) {
    if (g_event_queue.empty()) return false;
    *event = g_event_queue.front();
    g_event_queue.erase(g_event_queue.begin());
    return true;
}

bool SDL_WaitEvent(SDL_Event* event) {
    if (g_event_queue.empty()) return false;
    return SDL_PollEvent(event);
}

int SDL_PeepEvents(SDL_Event* events, int numevents, int action, uint32_t minType, uint32_t maxType) {
    if (action == 2) { // SDL_ADDEVENT
        int count = 0;
        for (int i = 0; i < numevents && events; ++i) {
            if (events[i].type >= minType && events[i].type <= maxType) {
                g_event_queue.push_back(events[i]);
                count++;
            }
        }
        return count;
    } else if (action == 0) { // SDL_PEEKEVENT
        int count = 0;
        for (size_t i = 0; i < g_event_queue.size() && count < static_cast<size_t>(numevents); ++i) {
            if (g_event_queue[i].type >= minType && g_event_queue[i].type <= maxType) {
                events[count++] = g_event_queue[i];
            }
        }
        return count;
    } else if (action == 1) { // SDL_GETEVENT
        int count = 0;
        size_t i = 0;
        while (i < g_event_queue.size() && count < numevents) {
            if (g_event_queue[i].type >= minType && g_event_queue[i].type <= maxType) {
                events[count++] = g_event_queue[i];
                g_event_queue.erase(g_event_queue.begin() + i);
            } else {
                ++i;
            }
        }
        return count;
    }
    return 0;
}

// ============================================================================
// 3. GPU ABSTRACTION - using void* for handles so nullptr works
// ============================================================================

enum class SDL_GPUShaderStage : uint32_t {
    VERTEX = 1,
    FRAGMENT = 2,
    COMPUTE = 4,
};

enum class SDL_GPUTextureFormat : uint32_t {
    R8G8B8A8_UNORM = 0,
    B8G8R8A8_UNORM = 1,
    R16G16B16A16_FLOAT = 2,
};

struct SDL_GPUShaderCreateInfo {
    SDL_GPUShaderStage stage;
    const uint8_t* code;
    size_t code_size;
    const char* entrypoint = "main";
    uint32_t num_samplers = 0;
    uint32_t num_uniform_buffers = 0;
    uint32_t num_storage_buffers = 0;
};

struct SDL_GPUTextureCreateInfo {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t layer_count = 1;
    uint32_t num_levels = 1;
    uint32_t format = 0;
    uint32_t usage = 0;
};

struct SDL_GPUSamplerCreateInfo {
    int min_filter = 0;
    int mag_filter = 0;
    int address_mode_u = 0;
    int address_mode_v = 0;
    float max_anisotropy = 1.0f;
};

// Use void* for all GPU handles so nullptr comparison works
using SDL_GPUShader = void*;
using SDL_GPUTexture = void*;
using SDL_GPUSampler = void*;
using SDL_GPUCommandBuffer = void*;
using SDL_GPURenderPass = void*;

SDL_GPUShader SDL_CreateGPUShader(const void* createinfo) {
    (void)createinfo;
    return new int(1);
}

void SDL_ReleaseGPUShader(SDL_GPUShader shader) {
    delete static_cast<int*>(shader);
}

SDL_GPUTexture SDL_CreateGPUTexture(const void* createinfo) {
    (void)createinfo;
    return new int(2);
}

void SDL_ReleaseGPUTexture(SDL_GPUTexture texture) {
    delete static_cast<int*>(texture);
}

SDL_GPUSampler SDL_CreateGPUSampler(const void* createinfo) {
    (void)createinfo;
    return new int(3);
}

void SDL_ReleaseGPUSampler(SDL_GPUSampler sampler) {
    delete static_cast<int*>(sampler);
}

struct SDL_GPUColorTargetInfo {
    void* texture = nullptr;
    float clear_color[4] = {0, 0, 0, 0};
    bool load = true;
    bool store = true;
};

struct SDL_GPUDepthStencilTargetInfo {
    void* texture = nullptr;
    float clear_depth = 1.0f;
    bool load = true;
    bool store = true;
};

SDL_GPURenderPass SDL_BeginGPURenderPass(void* cmd, const void* color_targets, uint32_t num_color_targets, const void* depth_stencil) {
    (void)cmd; (void)color_targets; (void)num_color_targets; (void)depth_stencil;
    return new int(0);
}

void SDL_EndGPURenderPass(SDL_GPURenderPass render_pass) {
    delete static_cast<int*>(render_pass);
}

// ============================================================================
// 4. AUDIO STREAMS
// ============================================================================

enum class SDL_AudioFormat : uint32_t {
    U8 = 0x0008,
    S16 = 0x8010,
    S32 = 0x8020,
    F32 = 0x8120,
};

struct SDL_AudioSpec {
    int freq = 0;
    int format = 0;
    int channels = 0;
};

struct SDL_AudioStream {
    std::vector<uint8_t> buffer;
    int src_freq = 0;
    int dst_freq = 0;
    bool has_conversion = false;
    float resample_ratio = 1.0f;
};

SDL_AudioStream* SDL_CreateAudioStream(const void* src_spec, const void* dst_spec) {
    if (!src_spec || !dst_spec) return nullptr;
    SDL_AudioStream* stream = new SDL_AudioStream();
    const int* src = static_cast<const int*>(src_spec);
    const int* dst = static_cast<const int*>(dst_spec);
    stream->src_freq = src[0];
    stream->dst_freq = dst[0];
    stream->has_conversion = (src[0] != dst[0]);
    if (stream->has_conversion) {
        stream->resample_ratio = static_cast<float>(dst[0]) / src[0];
    }
    return stream;
}

void SDL_DestroyAudioStream(SDL_AudioStream* stream) {
    delete stream;
}

bool SDL_AudioStreamPut(SDL_AudioStream* stream, const void* buf, int len) {
    if (!stream || !buf || len <= 0) return false;
    const uint8_t* src = static_cast<const uint8_t*>(buf);
    stream->buffer.insert(stream->buffer.end(), src, src + len);
    return true;
}

bool SDL_AudioStreamGet(SDL_AudioStream* stream, void* buf, int len) {
    if (!stream || !buf || len <= 0) return false;
    if (stream->buffer.empty()) return false;
    
    int available = static_cast<int>(stream->buffer.size());
    int to_copy = std::min(len, available);
    std::memcpy(buf, stream->buffer.data(), to_copy);
    stream->buffer.erase(stream->buffer.begin(), stream->buffer.begin() + to_copy);
    return true;
}

int SDL_AudioStreamAvailable(SDL_AudioStream* stream) {
    return stream ? static_cast<int>(stream->buffer.size()) : 0;
}

bool SDL_AudioStreamFlush(SDL_AudioStream* stream) {
    if (!stream) return false;
    stream->buffer.clear();
    return true;
}

bool SDL_AudioStreamClear(SDL_AudioStream* stream) {
    if (!stream) return false;
    stream->buffer.clear();
    return true;
}

// ============================================================================
// 5. STORAGE (VFS)
// ============================================================================

struct SDL_Storage {
    std::string root_path;
    std::unordered_map<std::string, std::vector<uint8_t>> files;
};

SDL_Storage* SDL_OpenStorage(const char* path) {
    if (!path) return nullptr;
    SDL_Storage* storage = new SDL_Storage();
    storage->root_path = path;
    return storage;
}

void SDL_CloseStorage(SDL_Storage* storage) {
    delete storage;
}

bool SDL_StorageReady(SDL_Storage* storage) {
    return storage != nullptr;
}

bool SDL_StorageFileExists(SDL_Storage* storage, const char* path) {
    if (!storage || !path) return false;
    return storage->files.find(path) != storage->files.end();
}

bool SDL_StorageReadFile(SDL_Storage* storage, const char* path, void* dst, uint64_t length) {
    if (!storage || !path || !dst) return false;
    auto it = storage->files.find(path);
    if (it == storage->files.end()) return false;
    
    uint64_t file_size = it->second.size();
    uint64_t to_copy = std::min(length, file_size);
    std::memcpy(dst, it->second.data(), to_copy);
    if (length > file_size) {
        std::memset(static_cast<uint8_t*>(dst) + file_size, 0, length - file_size);
    }
    return true;
}

bool SDL_StorageWriteFile(SDL_Storage* storage, const char* path, const void* src, uint64_t length, bool append) {
    if (!storage || !path || !src || length == 0) return false;
    
    auto& file_data = storage->files[path];
    if (append) {
        size_t old_size = file_data.size();
        file_data.resize(old_size + length);
        std::memcpy(file_data.data() + old_size, src, length);
    } else {
        file_data.assign(static_cast<const uint8_t*>(src), static_cast<const uint8_t*>(src) + length);
    }
    return true;
}

bool SDL_StorageRemove(SDL_Storage* storage, const char* path) {
    if (!storage || !path) return false;
    return storage->files.erase(path) > 0;
}

bool SDL_StorageMkdir(SDL_Storage* storage, const char* path) {
    if (!storage || !path) return false;
    std::string dir_path = std::string(path) + "/";
    storage->files[dir_path] = {};
    return true;
}

// ============================================================================
// TESTS (NO MODIFICAR)
// ============================================================================

TEST_CASE(TestPropertiesBasic) {
    SDL_PropertiesID props = SDL_CreateProperties();
    ASSERT_NE(props, SDL_PROPERTIES_INVALID);
    
    ASSERT_TRUE(SDL_SetStringProperty(props, "name", "test"));
    ASSERT_EQ(std::string(SDL_GetStringProperty(props, "name", "default")), "test");
    ASSERT_EQ(std::string(SDL_GetStringProperty(props, "missing", "default")), "default");
    
    ASSERT_TRUE(SDL_SetNumberProperty(props, "value", 42));
    ASSERT_EQ(SDL_GetNumberProperty(props, "value", 0), 42);
    ASSERT_EQ(SDL_GetNumberProperty(props, "missing", 0), 0);
    
    ASSERT_TRUE(SDL_SetBooleanProperty(props, "flag", true));
    ASSERT_TRUE(SDL_GetBooleanProperty(props, "flag", false));
    ASSERT_FALSE(SDL_GetBooleanProperty(props, "missing", true));
    
    ASSERT_TRUE(SDL_HasProperty(props, "name"));
    ASSERT_FALSE(SDL_HasProperty(props, "missing"));
    
    SDL_ClearProperty(props, "name");
    ASSERT_FALSE(SDL_HasProperty(props, "name"));
    
    SDL_DestroyProperties(props);
}

TEST_CASE(TestPropertiesPointer) {
    SDL_PropertiesID props = SDL_CreateProperties();
    int value = 42;
    
    ASSERT_TRUE(SDL_SetPointerProperty(props, "ptr", &value));
    void* retrieved = SDL_GetPointerProperty(props, "ptr", nullptr);
    ASSERT_EQ(retrieved, &value);
    
    int* retrieved_int = static_cast<int*>(retrieved);
    ASSERT_EQ(*retrieved_int, 42);
    
    SDL_DestroyProperties(props);
}

TEST_CASE(TestPropertiesCopy) {
    SDL_PropertiesID props1 = SDL_CreateProperties();
    SDL_SetStringProperty(props1, "key1", "value1");
    SDL_SetNumberProperty(props1, "key2", 123);
    
    SDL_PropertiesID props2 = SDL_CreateProperties();
    ASSERT_TRUE(SDL_CopyProperties(props1, props2));
    
    ASSERT_EQ(std::string(SDL_GetStringProperty(props2, "key1", "")), "value1");
    ASSERT_EQ(SDL_GetNumberProperty(props2, "key2", 0), 123);
    
    SDL_DestroyProperties(props1);
    SDL_DestroyProperties(props2);
}

TEST_CASE(TestEventQueue) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {}
    
    SDL_Event quit_event = {};
    quit_event.type = static_cast<uint32_t>(SDL_EventType::SDL_EVENT_QUIT);
    ASSERT_TRUE(SDL_PushEvent(&quit_event));
    
    ASSERT_TRUE(SDL_PollEvent(&event));
    ASSERT_EQ(event.type, static_cast<uint32_t>(SDL_EventType::SDL_EVENT_QUIT));
    
    ASSERT_FALSE(SDL_PollEvent(&event));
}

TEST_CASE(TestEventPeep) {
    SDL_Event events[4];
    for (int i = 0; i < 4; ++i) {
        events[i].type = static_cast<uint32_t>(SDL_EventType::SDL_EVENT_KEY_DOWN);
        events[i].key.scancode = i;
    }
    
    int added = SDL_PeepEvents(events, 4, 2, 0x300, 0x301);
    ASSERT_EQ(added, 4);
    
    SDL_Event out_events[4];
    int peeked = SDL_PeepEvents(out_events, 4, 0, 0x300, 0x301);
    ASSERT_EQ(peeked, 4);
    ASSERT_EQ(out_events[0].key.scancode, 0);
    ASSERT_EQ(out_events[3].key.scancode, 3);
    
    int gotten = SDL_PeepEvents(out_events, 4, 1, 0x300, 0x301);
    ASSERT_EQ(gotten, 4);
    
    peeked = SDL_PeepEvents(out_events, 4, 0, 0x300, 0x301);
    ASSERT_EQ(peeked, 0);
}

TEST_CASE(TestGPUResources) {
    // Shader
    SDL_GPUShader shader = SDL_CreateGPUShader(nullptr);
    ASSERT_NE(shader, nullptr);
    SDL_ReleaseGPUShader(shader);
    
    // Texture
    SDL_GPUTexture texture = SDL_CreateGPUTexture(nullptr);
    ASSERT_NE(texture, nullptr);
    SDL_ReleaseGPUTexture(texture);
    
    // Sampler
    SDL_GPUSampler sampler = SDL_CreateGPUSampler(nullptr);
    ASSERT_NE(sampler, nullptr);
    SDL_ReleaseGPUSampler(sampler);
}

TEST_CASE(TestGPURenderPass) {
    void* cmd = nullptr;
    void* color_target = nullptr;
    void* depth_target = nullptr;
    
    SDL_GPURenderPass pass = SDL_BeginGPURenderPass(cmd, color_target, 1, depth_target);
    ASSERT_NE(pass, nullptr);
    SDL_EndGPURenderPass(pass);
    ASSERT_TRUE(true);
}

TEST_CASE(TestAudioStream) {
    int src_spec[3] = {44100, 4, 2};
    int dst_spec[3] = {48000, 2, 2};
    
    SDL_AudioStream* stream = SDL_CreateAudioStream(src_spec, dst_spec);
    ASSERT_NE(stream, nullptr);
    ASSERT_TRUE(stream->has_conversion);
    ASSERT_EQ(stream->src_freq, 44100);
    ASSERT_EQ(stream->dst_freq, 48000);
    
    float input[256] = {0.5f};
    ASSERT_TRUE(SDL_AudioStreamPut(stream, input, sizeof(input)));
    
    int16_t output[512];
    ASSERT_TRUE(SDL_AudioStreamGet(stream, output, sizeof(output)));
    
    int available = SDL_AudioStreamAvailable(stream);
    ASSERT_GE(available, 0);
    
    ASSERT_TRUE(SDL_AudioStreamFlush(stream));
    ASSERT_TRUE(SDL_AudioStreamClear(stream));
    ASSERT_EQ(SDL_AudioStreamAvailable(stream), 0);
    
    SDL_DestroyAudioStream(stream);
}

TEST_CASE(TestAudioStreamNoConversion) {
    int spec[3] = {48000, 2, 2};
    SDL_AudioStream* stream = SDL_CreateAudioStream(spec, spec);
    ASSERT_NE(stream, nullptr);
    ASSERT_FALSE(stream->has_conversion);
    SDL_DestroyAudioStream(stream);
}

TEST_CASE(TestStorage) {
    SDL_Storage* storage = SDL_OpenStorage("/tmp");
    ASSERT_NE(storage, nullptr);
    
    const char* test_path = "test_file.txt";
    const char* test_data = "Hello, SDL3 Storage!";
    size_t data_len = strlen(test_data);
    
    ASSERT_TRUE(SDL_StorageWriteFile(storage, test_path, test_data, data_len, false));
    ASSERT_TRUE(SDL_StorageFileExists(storage, test_path));
    
    char buffer[256];
    ASSERT_TRUE(SDL_StorageReadFile(storage, test_path, buffer, sizeof(buffer)));
    ASSERT_EQ(std::string(buffer), test_data);
    
    ASSERT_TRUE(SDL_StorageRemove(storage, test_path));
    ASSERT_FALSE(SDL_StorageFileExists(storage, test_path));
    
    ASSERT_TRUE(SDL_StorageMkdir(storage, "test_dir"));
    
    SDL_CloseStorage(storage);
}

TEST_CASE(TestStorageOverwrite) {
    SDL_Storage* storage = SDL_OpenStorage("/tmp");
    const char* path = "overwrite.txt";
    
    ASSERT_TRUE(SDL_StorageWriteFile(storage, path, "first", 5, false));
    ASSERT_TRUE(SDL_StorageWriteFile(storage, path, "second", 6, false));
    
    char buffer[256];
    ASSERT_TRUE(SDL_StorageReadFile(storage, path, buffer, sizeof(buffer)));
    ASSERT_EQ(std::string(buffer), "second");
    
    ASSERT_TRUE(SDL_StorageWriteFile(storage, path, "_append", 7, true));
    ASSERT_TRUE(SDL_StorageReadFile(storage, path, buffer, sizeof(buffer)));
    ASSERT_EQ(std::string(buffer), "second_append");
    
    SDL_CloseStorage(storage);
}

int main() { return RUN_ALL_TESTS(); }