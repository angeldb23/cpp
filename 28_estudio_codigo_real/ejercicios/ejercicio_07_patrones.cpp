/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * EJERCICIO 07: Patrones Recurrentes — Arena Allocator y Opaque Handles
 *
 * OBJETIVO: Implementar dos patrones que aparecen en GLM, miniaudio, Box2D,
 * ImGui y SDL3 (lección 09):
 *   1. Arena / bump allocator: un solo bloque, allocations por bump del cursor
 *      y free = reset de todo el arena (frame allocators, scratch buffers).
 *   2. Opaque handle + function table: el header solo expone un puntero
 *      incompleto y una tabla de funciones versionada (ABI estable).
 *
 * Todo es lógica pura: no abre ventanas ni depende de librerías externas.
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

    // TODO: devolver un puntero alineado a `alignment` (potencia de 2) y
    // avanzar el cursor. Si no entra, devolver nullptr SIN avanzar.
    void* alloc(std::size_t size, std::size_t alignment) {
        (void)size; (void)alignment;
        return nullptr;
    }

    // TODO: liberar todo de una vez.
    void reset() {
    }

    std::size_t used() const { return cursor; }
    std::size_t capacity() const { return storage.size(); }
};

// ============================================================================
// 2. OPAQUE HANDLE + FUNCTION TABLE VERSIONADA
// ============================================================================

// La versión se codifica como major en los 16 bits altos y minor en los bajos.
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

// TODO: crear el handle opaco con su implementación y su tabla de funciones.
Engine* engine_create(int base) {
    (void)base;
    return nullptr;
}

// TODO: usar la tabla de funciones (no llamar a la implementación directo).
int engine_add(Engine* engine, int value) {
    (void)engine; (void)value;
    return 0;
}

// TODO: devolver la versión de la API del handle, o 0 si es nulo.
std::uint32_t engine_version(const Engine* engine) {
    (void)engine;
    return 0;
}

// TODO: destruir implementación y handle. Debe tolerar nullptr.
void engine_destroy(Engine* engine) {
    (void)engine;
}

// TODO: compatibilidad ABI tipo semver: mismo major y minor del plugin
// menor o igual al del host.
bool abi_compatible(std::uint32_t host_version, std::uint32_t plugin_version) {
    (void)host_version; (void)plugin_version;
    return false;
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
    ASSERT_EQ(arena.used(), usado); // no avanza si falla

    void* c = arena.alloc(8, 8);
    ASSERT_TRUE(c != nullptr);      // lo que quedaba sigue usable
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
    engine_destroy(nullptr); // debe tolerar nullptr
}

TEST_CASE(TestABICompatible) {
    std::uint32_t host = make_version(2, 3);

    ASSERT_TRUE(abi_compatible(host, make_version(2, 3)));
    ASSERT_TRUE(abi_compatible(host, make_version(2, 1)));
    ASSERT_FALSE(abi_compatible(host, make_version(2, 4))); // plugin más nuevo
    ASSERT_FALSE(abi_compatible(host, make_version(3, 0))); // major distinto
    ASSERT_FALSE(abi_compatible(host, make_version(1, 9)));
}

int main() { return RUN_ALL_TESTS(); }
