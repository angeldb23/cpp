/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * LECCIÓN 08: WAD/Lumps y Zone Memory — Sistema de Archivos y Allocador (w_wad.h/c, z_zone.h/c)
 *
 * TEMAS:
 * - Formato WAD: header, directory, lumps (datos crudos)
 * - Namespaces: S_ (sprites), P_ (patches), F_ (flats), M_ (music), etc.
 * - W_CacheLumpNum / W_CacheLumpName: cache con reference counting y purge
 * - Zone Memory (zone.h/c): malloc/free con tags (PU_STATIC, PU_LEVEL, PU_CACHE, PU_LEVSPEC)
 *   Bloqueo (lock), purga (purge), cambio de tag, memory stats.
 * - Lump cache: zone blocks con tag PU_CACHE, purgables bajo presión de memoria.
 * - Lumps internos vs externos (PWADs), lump lookup lineal + hash (source ports).
 *
 * VERIFICACIÓN: w_wad.h/c, z_zone.h/c, doomdata.h del source release 1997.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// 1. FORMATO WAD (w_wad.h)
// ============================================================================
/*
 * WAD = "Where's All the Data". Archivo contenedor simple.
 *
 * HEADER (12 bytes):
 *   char  identification[4];   // "IWAD" o "PWAD"
 *   int   numlumps;            // número de lumps
 *   int   infotableofs;        // offset a directory (lumpinfo_t[])
 *
 * DIRECTORY (lumpinfo_t por lump, 16 bytes cada uno):
 *   typedef struct {
 *       int   filepos;         // offset en archivo
 *       int   size;            // tamaño en bytes
 *       char  name[8];         // nombre (8 chars, padded con NUL, case-insensitive)
 *   } lumpinfo_t;
 *
 * LUMPS: datos crudos secuenciales (sin compresión en original).
 *   - Flats (floor/ceiling): 64x64 = 4096 bytes raw (paleta indices)
 *   - Patches (sprites/texturas): formato patch_t (width, height, offsets, columns)
 *   - Sprites: múltiples patches por frame
 *   - Texturas compuestas (TEXTURE1/2): definen combinación de patches
 *   - Flats (F_ namespace): raw 64x64
 *   - Sonidos (DS_): formato DMX (luego WAV/RAW en ports)
 *   - Música (D_): MUS format (MIDI-like)
 *   - Mapas: THINGS, LINEDEFS, SIDEDEFS, VERTEXES, SEGS, SSECTORS, NODES, SECTORS, REJECT, BLOCKMAP
 *
 * IWAD = Internal WAD (doom.wad, doom2.wad): datos base del juego.
 * PWAD = Patch WAD: addons/mods, cargados sobre IWAD (override lumps por nombre).
 * Múltiples PWADs: se buscan en orden inverso de carga (último gana).
 */

#define LUMP_NAME_SIZE 8

typedef struct {
    char identification[4];  // "IWAD" o "PWAD"
    int32_t numlumps;
    int32_t infotableofs;
} wadheader_t;

typedef struct {
    int32_t filepos;
    int32_t size;
    char name[LUMP_NAME_SIZE];
} lumpinfo_t;

// ============================================================================
// 2. NAMESPACES (w_wad.h)
// ============================================================================
/*
 * Los nombres de lump tienen prefijos (namespaces) para evitar colisiones
 * y acelerar búsqueda por tipo:
 *
 *   S_  = Sprites (patches de sprites: TROOA0, TROOB0...)
 *   P_  = Patches (gráficos sueltos: texturas, HUD, fonts)
 *   F_  = Flats (pisos/techos: 64x64 raw)
 *   M_  = Music (D_ en algunos: D_RUNNIN, D_STALKS...)
 *   D_  = Music (Doom usa D_ para música, M_ en Heretic)
 *   DS_ = Digital Sounds (efectos: DSPISTOL, DSSGHT...)
 *   TX_ = Textures compuestas (TEXTURE1, TEXTURE2)
 *   PNAMES = lista de nombres de patches para texturas
 *   (sin prefijo) = Map lumps (THINGS, LINEDEFS, etc.), PLAYPAL, COLORMAP, ENDOOM...
 *
 * Búsqueda: W_CheckNumForName(name) busca en TODOS los WADs cargados (PWADs primero).
 * W_GetNumForName(name) = W_CheckNumForName + error si no existe.
 */

// Namespaces como enum para claridad:
typedef enum {
    NS_SPRITES  = 0,  // S_
    NS_PATCHES  = 1,  // P_
    NS_FLATS    = 2,  // F_
    NS_COLORMAP = 3,  // (COLORMAP)
    NS_TEXTURE  = 4,  // TEXTURE1/2, PNAMES
    NS_SOUNDS   = 5,  // DS_
    NS_MUSIC    = 6,  // D_ / M_
    NS_MAP      = 7,  // THINGS, LINEDEFS, etc.
    NS_OTHER    = 8   // resto
} namespace_t;

// ============================================================================
// 3. LUMP CACHE: W_CacheLumpNum / W_CacheLumpName (w_wad.c)
// ============================================================================
/*
 * El sistema de cache de lumps USA el Zone Memory allocator.
 * Cada lump cacheado es un bloque zone con tag PU_CACHE.
 *
 * void* W_CacheLumpNum(int lumpnum, int tag) {
 *     lumpcache_t* lump = &lumpcache[lumpnum];
 *     if (!lump->data) {
 *         // No cacheado: leer de disco
 *         lumpinfo_t* info = &lumpinfo[lumpnum];
 *         lump->data = Z_Malloc(info->size, tag, &lump->data); // tag PU_CACHE o dado
 *         fread(lump->data, 1, info->size, wadfile);
 *     } else if (tag != PU_CACHE) {
 *         // Cambiar tag del bloque existente (Z_ChangeTag)
 *         Z_ChangeTag(lump->data, tag);
 *     }
 *     return lump->data;
 * }
 *
 * lumpcache_t (w_wad.h):
 *   typedef struct {
 *       void* data;       // puntero a datos (zone block) o NULL
 *       // ... campos para reference counting en algunos ports ...
 *   } lumpcache_t;
 *
 * PU_CACHE: bloque PURGABLE. Cuando Z_Malloc falla por falta de memoria,
 * Z_FreeTags(PU_CACHE, 0) libera bloques PU_CACHE no bloqueados (lock=0).
 *
 * PU_STATIC: nunca se libera (hasta exit). Para IWAD lumps críticos.
 * PU_LEVEL: se libera al cambiar de nivel (P_FreeLevelData).
 * PU_LEVSPEC: se libera al cambiar de nivel PERO persiste en savegames.
 *
 * W_Profile: estadísticas de uso de lumps (hits, misses, purges).
 */

// Tags de memoria (z_zone.h):
#define PU_STATIC    1  // nunca libre (IWAD critical lumps)
#define PU_LEVEL     2  // libre al cambiar nivel (map data, sprites nivel)
#define PU_LEVSPEC   3  // como LEVEL pero persiste en save (scripts)
#define PU_CACHE     4  // cache purgable (lumps no críticos)

// Lump cache entry
typedef struct {
    void* data;       // zone block pointer o NULL
    int size;         // tamaño del lump
    int tag;          // tag actual del zone block
} lumpcache_t;

// Array global de lumpcache (uno por lump en todos los WADs cargados)
lumpcache_t* lumpcache = nullptr;
int numlumps_total = 0;

// W_CacheLumpNum simplificado
void* W_CacheLumpNum(int lumpnum, int tag) {
    if (lumpnum < 0 || lumpnum >= numlumps_total) return NULL;
    lumpcache_t* lc = &lumpcache[lumpnum];
    if (!lc->data) {
        // Leer de disco (simplificado)
        // lumpinfo_t* info = &lumpinfo[lumpnum];
        // lc->data = Z_Malloc(info->size, PU_CACHE, &lc->data);
        // fread(lc->data, 1, info->size, wadfile);
        lc->size = 1024; // placeholder
        lc->data = malloc(lc->size); // placeholder sin zone
        lc->tag = PU_CACHE;
    }
    if (tag != PU_CACHE && lc->tag == PU_CACHE) {
        // Z_ChangeTag(lc->data, tag); // cambiar tag
        lc->tag = tag;
    }
    return lc->data;
}

// W_CacheLumpName: busca por nombre y llama W_CacheLumpNum
void* W_CacheLumpName(const char* name, int tag) {
    // int lumpnum = W_GetNumForName(name);
    // return W_CacheLumpNum(lumpnum, tag);
    (void)name; (void)tag;
    return NULL;
}

// ============================================================================
// 4. ZONE MEMORY ALLOCATOR (z_zone.h, z_zone.c)
// ============================================================================
/*
 * Zone Memory = allocador personalizado de Doom. Reemplaza malloc/free.
 * Razones en 1993:
 * - Control total de memoria (DOS: 4MB-16MB total, sin virtual memory)
 * - Tags para purga selectiva (nivel, cache, static)
 * - Bloqueo (lock) para evitar purga de bloques en uso
 * - Detección de overrun/underrun (guard bytes)
 * - Estadísticas de fragmentación, uso por tag
 *
 * ESTRUCTURA DE BLOQUE ZONE (memblock_t en z_zone.c):
 *   typedef struct memblock_s {
 *       int size;              // tamaño TOTAL del bloque (incluye header)
 *       void* user;            // puntero user data (justo después del header)
 *       int tag;               // PU_STATIC, PU_LEVEL, PU_CACHE, PU_LEVSPEC
 *       int id;                // ID único para debugging (0x1d4a11)
 *       struct memblock_s* next;
 *       struct memblock_s* prev;
 *       // ... guard bytes al final ...
 *   } memblock_t;
 *
 *   User pointer = (byte*)memblock + sizeof(memblock_t).
 *   Z_Malloc devuelve user pointer.
 *   Z_Free recibe user pointer, resta sizeof(memblock_t) para header.
 *
 * LISTA DOBLEMENTE ENLAZADA GLOBAL: all zones linked together.
 *   memblock_t* mainzone;  // cabecera de zona principal (todo el heap)
 *
 * Z_Init(size): reserva heap grande (malloc único del SO), crea mainzone.
 *   mainzone = (memblock_t*)malloc(size);
 *   mainzone->size = size;
 *   mainzone->tag = PU_STATIC;
 *   mainzone->next = mainzone->prev = mainzone; // lista circular
 *
 * Z_Malloc(size, tag, *ptr):
 *   - Recorre lista buscando bloque FREE (tag == 0) con size >= pedido + header.
 *   - Si no hay: Z_FreeTags(PU_CACHE, 0) para purgar cache, reintentar.
 *   - Split: si bloque sobrante > MIN_FRAGMENT, crear nuevo bloque free.
 *   - Marcar tag, *ptr = user pointer, lock = 0.
 *
 * Z_Free(ptr):
 *   - Encuentra header, verifica id (0x1d4a11), verifica guard bytes.
 *   - tag = 0 (free), merge con adyacentes free (coalesce).
 *
 * Z_ChangeTag(ptr, newtag): cambia tag del bloque.
 * Z_ChangeTag2(ptr, oldtag, newtag): verifica oldtag antes de cambiar.
 *
 * Z_FreeTags(tag, lowtag): libera TODOS los bloques con tag >= lowtag y tag == tag.
 *   Usado para: cambio de nivel (PU_LEVEL), presión memoria (PU_CACHE).
 *
 * Z_CheckHeap(): valida integridad (guards, links, tags). Llamado en debug.
 */

#define ZONEID 0x1d4a11
#define MIN_FRAGMENT 64  // mínimo tamaño para fragment libre

typedef struct memblock_s {
    int size;                    // total size including header
    void* user;                  // user data pointer (after header)
    int tag;                     // PU_STATIC, PU_LEVEL, PU_CACHE, PU_LEVSPEC
    int id;                      // ZONEID for validation
    struct memblock_s* next;
    struct memblock_s* prev;
    // guard bytes: 4 bytes at end of block = 0x1d4a11 (or pattern)
} memblock_t;

memblock_t* mainzone = NULL;

void Z_Init(int size) {
    // mainzone es un centinela; el primer bloque libre vive justo después.
    mainzone = (memblock_t*)malloc(size);
    mainzone->size = sizeof(memblock_t);
    mainzone->user = nullptr;
    mainzone->tag = PU_STATIC;
    mainzone->id = ZONEID;

    memblock_t* libre = (memblock_t*)((uint8_t*)mainzone + sizeof(memblock_t));
    libre->size = size - static_cast<int>(sizeof(memblock_t));
    libre->user = (uint8_t*)libre + sizeof(memblock_t);
    libre->tag = 0;  // libre
    libre->id = ZONEID;
    libre->next = mainzone;
    libre->prev = mainzone;

    mainzone->next = libre;
    mainzone->prev = libre;

    int* guard = (int*)((uint8_t*)libre + libre->size - 4);
    *guard = ZONEID;
}

void* Z_Malloc(int size, int tag, void** ptr) {
    // Alinear size
    size = (size + 3) & ~3;
    int total = size + sizeof(memblock_t) + 4; // header + guard

    memblock_t* rover = mainzone->next;
    while (rover != mainzone) {
        if (rover->tag == 0 && rover->size >= total) {
            // Found free block
            int rem = rover->size - total;
            if (rem >= MIN_FRAGMENT) {
                // Split
                memblock_t* newblock = (memblock_t*)((uint8_t*)rover + total);
                newblock->size = rem;
                newblock->tag = 0;
                newblock->id = ZONEID;
                newblock->next = rover->next;
                newblock->prev = rover;
                rover->next->prev = newblock;
                rover->next = newblock;
                // guard for newblock
                int* guard = (int*)((uint8_t*)newblock + rem - 4);
                *guard = ZONEID;
                rover->size = total;
                // guard del bloque asignado (antes estaba al final del libre)
                int* guard_rover = (int*)((uint8_t*)rover + total - 4);
                *guard_rover = ZONEID;
            }
            rover->tag = tag;
            rover->id = ZONEID;
            rover->user = (uint8_t*)rover + sizeof(memblock_t);
            *ptr = rover->user;
            return rover->user;
        }
        rover = rover->next;
    }
    // No space: try purge cache
    // Z_FreeTags(PU_CACHE, 0);
    // retry...
    return NULL; // OOM
}

void Z_Free(void* ptr) {
    if (!ptr) return;
    memblock_t* block = (memblock_t*)((uint8_t*)ptr - sizeof(memblock_t));
    if (block->id != ZONEID) {
        // Corruption!
        return;
    }
    // Verify guard
    int* guard = (int*)((uint8_t*)block + block->size - 4);
    if (*guard != ZONEID) {
        // Underrun/overrun!
        return;
    }
    block->tag = 0; // free
    block->id = 0;
    // Coalesce with prev if free
    if (block->prev->tag == 0) {
        block->prev->size += block->size;
        block->prev->next = block->next;
        block->next->prev = block->prev;
        block = block->prev;
    }
    // Coalesce with next if free
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
            // Free this block (coalesce handled in Z_Free logic)
            rover->tag = 0;
            rover->id = 0;
            // Coalesce...
        }
        rover = next;
    }
}

// ============================================================================
// 5. LUMP LOOKUP: LINEAL VS HASH (w_wad.c)
// ============================================================================
/*
 * Doom original: búsqueda LINEAL en directory (numlumps ~ 2000-3000).
 *   int W_CheckNumForName(char* name) {
 *       for (int i = numlumps-1; i >= 0; i--) {  // reverse: PWADs first
 *           if (strnicmp(lumpinfo[i].name, name, 8) == 0) return i;
 *       }
 *       return -1;
 *   }
 *
 * Source ports modernos: HASH TABLE (ej: FNV-1a de nombre 8 chars -> índice).
 *   - Construir hash al cargar WAD.
 *   - Lookup O(1) promedio.
 *   - Manejar colisiones (chaining o open addressing).
 *
 * Namespaces: W_CheckNumForName(name) busca en todos.
 * W_CheckNumForNamePwad(name, pwadnum) busca solo en un PWAD específico.
 */

int W_CheckNumForName(const char* name) {
    // Búsqueda lineal inversa (PWADs cargados al final del array)
    // for (int i = numlumps_total - 1; i >= 0; i--) ...
    (void)name;
    return -1;
}

// ============================================================================
// 6. MAP LUMPS: ORDEN Y DEPENDENCIAS
// ============================================================================
/*
 * Un mapa (E1M1, MAP01) consiste en 10 lumps consecutivos (orden fijo):
 *   THINGS, LINEDEFS, SIDEDEFS, VERTEXES, SEGS, SSECTORS, NODES, SECTORS, REJECT, BLOCKMAP
 *
 * Carga en P_SetupLevel:
 *   1. THINGS -> spawn things (P_SpawnMapThings)
 *   2. LINEDEFS -> linedefs[] array
 *   3. SIDEDEFS -> sidedefs[] array
 *   4. VERTEXES -> vertexes[] array
 *   5. SEGS -> segs[] array
 *   6. SSECTORS -> subsectors[] array
 *   7. NODES -> nodes[] array (BSP)
 *   8. SECTORS -> sectors[] array
 *   9. REJECT -> reject matrix (PVS simplificado: sector-sector visibility)
 *   10. BLOCKMAP -> blockmap para colisión
 *
 * REJECT: matrix [numsectors][numsectors] de bytes (0=visible, 1=reject).
 *   Usado en P_CheckSight para early-out: si reject[sec1][sec2] -> no visible.
 *   Tamaño: numsectors^2 bytes. En mapas grandes puede ser grande (ej: 400 sectors = 160KB).
 *
 * BLOCKMAP: grid uniforme para colisión (ver lección 05).
 */

// ============================================================================
// 7. PWADs: CARGA Y OVERRIDE
// ============================================================================
/*
 * W_AddFile(filename): carga PWAD, añade lumps al final de lumpinfo[] y lumpcache[].
 *   - Abre archivo, lee header, lee directory.
 *   - Ajusta filepos sumando offset del archivo en disco (cada WAD tiene su FILE*).
 *   - PWADs pueden OVERRIDE lumps del IWAD por nombre (último gana por búsqueda inversa).
 *
 * Múltiples PWADs: -file pwad1.wad -file pwad2.wad
 *   pwad2 override pwad1 override IWAD.
 *
 * En source ports: WADs en subdirectorios, PK3 (ZIP), carga dinámica.
 */

// ============================================================================
// 8. TESTS DE WAD Y ZONE
// ============================================================================

int test_wad_zone(void) {
    int passed = 0, failed = 0;
    #define TEST(expr, msg) \
        do { if (expr) { passed++; } else { failed++; printf("FAIL: %s\n", msg); } } while(0)

    // WAD header
    TEST(sizeof(wadheader_t) == 12, "wadheader_t = 12 bytes");
    TEST(sizeof(lumpinfo_t) == 16, "lumpinfo_t = 16 bytes (4+4+8)");

    // Lump name padding
    char name[8] = "TEST";
    TEST(strlen(name) <= 8, "lump name <= 8 chars");

    // Zone tags
    TEST(PU_STATIC == 1 && PU_LEVEL == 2 && PU_LEVSPEC == 3 && PU_CACHE == 4,
         "zone tags values");

    // Memblock header size
    TEST(sizeof(memblock_t) >= 24, "memblock_t >= 24 bytes (6*4 + 2*ptr)");

    // Z_Init / Z_Malloc / Z_Free roundtrip
    Z_Init(65536);
    void* p = NULL;
    void* ptr = Z_Malloc(1024, PU_LEVEL, &p);
    TEST(ptr != NULL, "Z_Malloc succeeded");
    TEST(ptr == p, "Z_Malloc sets output ptr");
    Z_Free(ptr);
    // After free, block should be coalesced
    TEST(mainzone->next->tag == 0, "block freed and coalesced");

    // Lump cache tags
    TEST(PU_CACHE == 4, "PU_CACHE = 4");

    printf("WAD/Zone tests: %d pass, %d fail\n", passed, failed);
    return failed == 0 ? 0 : 1;
}

int main(void) {
    printf("Módulo 27 - Lección 08: WAD/Lumps y Zone Memory\n");
    return test_wad_zone();
}