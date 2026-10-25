/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * SOLUCIÓN 07: Patrones Recurrentes — Arena Allocator y Opaque Handles
 *
 * Misma consigna que el ejercicio 07, con las implementaciones completas.
 */

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include "../../include/test_framework.hpp"

// ============================================================================
// 1. ARENA / BUMP ALLOCATOR
// ============================================================================

struct Arena {
    std::vector<std::byte> storage;
    std::size_t cursor = 0;

    explicit Arena(std::size_t capacity) : storage(capacity) {}

    void* alloc(std::size_t size, std::size_t alignment) {
        if (alignment == 0) alignment = 1;
        std::size_t aligned = (cursor + (alignment - 1)) & ~(alignment - 1);
        if (aligned > storage.size()) return nullptr;
        if (size > storage.size() - aligned) return nullptr;
        cursor = aligned + size;
        return storage.data() + aligned;
    }

    void reset() {
        cursor = 0;
    }

    std::size_t used() const { return cursor; }
    std::size_t capacity() const { return storage.size(); }
};

// ============================================================================
// 2. OPAQUE HANDLE + FUNCTION TABLE VERSIONADA
// ============================================================================

inline std::uint32_t make_version(std::uint32_t major, std::uint32_t minor) {
    return (major << 16) | (minor & 0xFFFFu);
}

struct EngineApi {
    std::uint32_t version;
    int (*add)(void* impl, int value);
    void (*destroy)(void* impl);
};

struct Engine {
    void* impl = nullptr;
    const EngineApi* api = nullptr;
};

struct EngineImpl {
    int value = 0;
};

// Implementaciones privadas: no se exponen en el "header" del módulo.
static int engine_impl_add(void* impl, int value) {
    auto* self = static_cast<EngineImpl*>(impl);
    return self->value + value;
}

static void engine_impl_destroy(void* impl) {
    delete static_cast<EngineImpl*>(impl);
}

Engine* engine_create(int base) {
    static const EngineApi api{
        make_version(1, 2),
        &engine_impl_add,
        &engine_impl_destroy
    };

    auto* impl = new EngineImpl{base};
    auto* engine = new Engine{impl, &api};
    return engine;
}

int engine_add(Engine* engine, int value) {
    if (engine == nullptr || engine->api == nullptr || engine->api->add == nullptr) {
        return 0;
    }
    return engine->api->add(engine->impl, value);
}

std::uint32_t engine_version(const Engine* engine) {
    if (engine == nullptr || engine->api == nullptr) return 0;
    return engine->api->version;
}

void engine_destroy(Engine* engine) {
    if (engine == nullptr) return;
    if (engine->api != nullptr && engine->api->destroy != nullptr) {
        engine->api->destroy(engine->impl);
    }
    delete engine;
}

bool abi_compatible(std::uint32_t host_version, std::uint32_t plugin_version) {
    std::uint32_t host_major = host_version >> 16;
    std::uint32_t plugin_major = plugin_version >> 16;
    std::uint32_t host_minor = host_version & 0xFFFFu;
    std::uint32_t plugin_minor = plugin_version & 0xFFFFu;
    return host_major == plugin_major && plugin_minor <= host_minor;
}

// ============================================================================
// TESTS (NO MODIFICAR)
// ============================================================================

TEST_CASE(TestArenaAlineacion) {
    Arena arena(256);

    void* a = arena.alloc(1, 1);
    ASSERT_TRUE(a != nullptr);
    ASSERT_EQ(reinterpret_cast<std::uintptr_t>(a) % 1, 0u);

    void* b = arena.alloc(4, 8);
    ASSERT_TRUE(b != nullptr);
    ASSERT_EQ(reinterpret_cast<std::uintptr_t>(b) % 8, 0u);

    void* c = arena.alloc(3, 16);
    ASSERT_TRUE(c != nullptr);
    ASSERT_EQ(reinterpret_cast<std::uintptr_t>(c) % 16, 0u);

    ASSERT_TRUE(reinterpret_cast<std::uintptr_t>(b) > reinterpret_cast<std::uintptr_t>(a));
    ASSERT_TRUE(reinterpret_cast<std::uintptr_t>(c) > reinterpret_cast<std::uintptr_t>(b));
    ASSERT_TRUE(arena.used() > 0);
    ASSERT_EQ(arena.capacity(), 256u);
}

TEST_CASE(TestArenaSinEspacio) {
    Arena arena(64);

    void* a = arena.alloc(40, 8);
    ASSERT_TRUE(a != nullptr);
    std::size_t usado = arena.used();

    void* b = arena.alloc(400, 8);
    ASSERT_TRUE(b == nullptr);
    ASSERT_EQ(arena.used(), usado);

    void* c = arena.alloc(8, 8);
    ASSERT_TRUE(c != nullptr);
}

TEST_CASE(TestArenaReset) {
    Arena arena(128);

    ASSERT_TRUE(arena.alloc(32, 8) != nullptr);
    ASSERT_TRUE(arena.alloc(16, 8) != nullptr);
    ASSERT_TRUE(arena.used() > 0);

    arena.reset();
    ASSERT_EQ(arena.used(), 0u);

    void* primero = arena.alloc(16, 8);
    ASSERT_TRUE(primero != nullptr);
    ASSERT_EQ(reinterpret_cast<std::uintptr_t>(primero) % 8, 0u);
}

TEST_CASE(TestEngineHandle) {
    Engine* engine = engine_create(10);
    ASSERT_TRUE(engine != nullptr);
    ASSERT_TRUE(engine->impl != nullptr);
    ASSERT_TRUE(engine->api != nullptr);

    ASSERT_EQ(engine_add(engine, 5), 15);
    ASSERT_EQ(engine_add(engine, -20), -10);

    engine_destroy(engine);
}

TEST_CASE(TestEngineVersion) {
    Engine* engine = engine_create(0);
    ASSERT_TRUE(engine != nullptr);

    std::uint32_t v = engine_version(engine);
    ASSERT_EQ(v, make_version(1, 2));
    ASSERT_EQ(engine_version(nullptr), 0u);

    engine_destroy(engine);
    engine_destroy(nullptr);
}

TEST_CASE(TestABICompatible) {
    std::uint32_t host = make_version(2, 3);

    ASSERT_TRUE(abi_compatible(host, make_version(2, 3)));
    ASSERT_TRUE(abi_compatible(host, make_version(2, 1)));
    ASSERT_FALSE(abi_compatible(host, make_version(2, 4)));
    ASSERT_FALSE(abi_compatible(host, make_version(3, 0)));
    ASSERT_FALSE(abi_compatible(host, make_version(1, 9)));
}

int main() { return RUN_ALL_TESTS(); }
