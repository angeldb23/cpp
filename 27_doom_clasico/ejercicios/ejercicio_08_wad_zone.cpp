/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * EJERCICIO 08: WAD/Lumps y Zone Memory — Formato WAD, Namespaces, Z_Malloc
 *
 * BASADO EN: Lección 08 (wad_zone)
 *
 * OBJETIVO: Implementar estructuras WAD, namespaces y zone memory allocator.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/test_framework.hpp"

// ============================================================================
// FORMATO WAD (w_wad.h)
// ============================================================================

typedef struct {
    char identification[4];
    int32_t numlumps;
    int32_t infotableofs;
} wadheader_t;

typedef struct {
    int32_t filepos;
    int32_t size;
    char name[8];
} lumpinfo_t;

#define LUMP_NAME_SIZE 8

// ============================================================================
// NAMESPACES
// ============================================================================

typedef enum {
    NS_SPRITES = 0,
    NS_PATCHES = 1,
    NS_FLATS = 2,
    NS_COLORMAP = 3,
    NS_TEXTURE = 4,
    NS_SOUNDS = 5,
    NS_MUSIC = 6,
    NS_MAP = 7,
    NS_OTHER = 8
} namespace_t;

// ============================================================================
// ZONE MEMORY (z_zone.h)
// ============================================================================

#define PU_STATIC 1
#define PU_LEVEL 2
#define PU_LEVSPEC 3
#define PU_CACHE 4

#define ZONEID 0x1d4a11
#define MIN_FRAGMENT 64

typedef struct memblock_s {
    int size;
    void* user;
    int tag;
    int id;
    struct memblock_s* next;
    struct memblock_s* prev;
} memblock_t;

memblock_t* mainzone = NULL;

void Z_Init(int size) {
    mainzone = (memblock_t*)malloc(size);
    mainzone->size = size;
    mainzone->user = (uint8_t*)mainzone + sizeof(memblock_t);
    mainzone->tag = 0;  // FREE block initially
    mainzone->id = ZONEID;
    mainzone->next = mainzone->prev = mainzone;
    int* guard = (int*)((uint8_t*)mainzone + size - 4);
    *guard = ZONEID;
}

void* Z_Malloc(int size, int tag, void** ptr) {
    // TODO: reservar bloque en mainzone, fragmentar si rem >= MIN_FRAGMENT y asignar tag
    (void)size; (void)tag; (void)ptr;
    return NULL;
}

void Z_Free(void* ptr) {
    if (!ptr) return;
    memblock_t* block = (memblock_t*)((uint8_t*)ptr - sizeof(memblock_t));
    if (block->id != ZONEID) return;
    int* guard = (int*)((uint8_t*)block + block->size - 4);
    if (*guard != ZONEID) return;
    block->tag = 0;
    block->id = 0;
    if (block->prev->tag == 0) {
        block->prev->size += block->size;
        block->prev->next = block->next;
        block->next->prev = block->prev;
        block = block->prev;
    }
    if (block->next->tag == 0) {
        block->size += block->next->size;
        block->next = block->next->next;
        block->next->prev = block;
    }
}

void Z_ChangeTag(void* ptr, int tag) {
    memblock_t* block = (memblock_t*)((uint8_t*)ptr - sizeof(memblock_t));
    if (block->id == ZONEID) block->tag = tag;
}

void Z_FreeTags(int tag, int lowtag) {
    memblock_t* rover = mainzone->next;
    while (rover != mainzone) {
        memblock_t* next = rover->next;
        if (rover->tag == tag && tag >= lowtag) {
            rover->tag = 0;
            rover->id = 0;
            if (rover->prev->tag == 0) {
                rover->prev->size += rover->size;
                rover->prev->next = rover->next;
                rover->next->prev = rover->prev;
            }
            if (rover->next->tag == 0) {
                rover->size += rover->next->size;
                rover->next = rover->next->next;
                rover->next->prev = rover;
            }
        }
        rover = next;
    }
}

// ============================================================================
// LUMP CACHE
// ============================================================================

typedef struct {
    void* data;
    int size;
    int tag;
} lumpcache_t;

lumpcache_t* lumpcache = NULL;
int numlumps_total = 0;

void* W_CacheLumpNum(int lumpnum, int tag) {
    if (lumpnum < 0 || lumpnum >= numlumps_total) return NULL;
    lumpcache_t* lc = &lumpcache[lumpnum];
    if (!lc->data) {
        lc->size = 1024;
        lc->data = malloc(lc->size);
        lc->tag = PU_CACHE;
    }
    if (tag != PU_CACHE && lc->tag == PU_CACHE) {
        lc->tag = tag;
    }
    return lc->data;
}

// ============================================================================
// TESTS (NO MODIFICAR)
// ============================================================================

TEST_CASE(TestWADStructures) {
    ASSERT_EQ(sizeof(wadheader_t), 12);
    ASSERT_EQ(sizeof(lumpinfo_t), 16);
    ASSERT_EQ(LUMP_NAME_SIZE, 8);
}

TEST_CASE(TestNamespaces) {
    ASSERT_EQ(NS_SPRITES, 0);
    ASSERT_EQ(NS_PATCHES, 1);
    ASSERT_EQ(NS_FLATS, 2);
    ASSERT_EQ(NS_COLORMAP, 3);
    ASSERT_EQ(NS_TEXTURE, 4);
    ASSERT_EQ(NS_SOUNDS, 5);
    ASSERT_EQ(NS_MUSIC, 6);
    ASSERT_EQ(NS_MAP, 7);
    ASSERT_EQ(NS_OTHER, 8);
}

TEST_CASE(TestZoneTags) {
    ASSERT_EQ(PU_STATIC, 1);
    ASSERT_EQ(PU_LEVEL, 2);
    ASSERT_EQ(PU_LEVSPEC, 3);
    ASSERT_EQ(PU_CACHE, 4);
    ASSERT_EQ(ZONEID, 0x1d4a11);
    ASSERT_EQ(MIN_FRAGMENT, 64);
}

TEST_CASE(TestMemblock) {
    ASSERT_TRUE(sizeof(memblock_t) >= 24);
}

TEST_CASE(TestZ_Init_Malloc_Free) {
    Z_Init(65536);
    void* p = NULL;
    void* ptr = Z_Malloc(1024, PU_LEVEL, &p);
    ASSERT_TRUE(ptr != NULL);
    ASSERT_EQ(ptr, p);
    Z_Free(ptr);
    ASSERT_EQ(mainzone->next->tag, 0);
}

TEST_CASE(TestLumpCacheTags) {
    ASSERT_EQ(PU_CACHE, 4);
}

int main() {
    return RUN_ALL_TESTS();
}