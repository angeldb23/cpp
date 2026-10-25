/* =============================================================================
   MÓDULO 25 - LECCIÓN 09: streaming desde el pack (VFS custom) + CRC32
   =============================================================================
   TEMA 5 (parte 2): índice, streaming desde pack y verificación de
   integridad (CRC32/xxHash).

   STREAMING DENTRO DEL PACK: NO ABRIR LUMPS COMO ARCHIVOS
   -------------------------------------------------------
   El audio (una música OGG de 40 s) NO se descomprime de golpe: se leen
   PORCIONES del lump bajo demanda. La solución: el decoder de miniaudio
   pide bytes "como si fuera un archivo", y un VFS CUSTOM le hace de
   "archivo virtual apuntando a un offset dentro del pack".

   El patrón REAL de miniaudio (verificado en el header 0.11.25):
   - Un "VFS" es un struct cuyo PRIMER miembro es ma_vfs_callbacks (así lo
     hace ma_default_vfs en el propio header: las funciones estáticas
      castean pVFS a ma_vfs_callbacks*).
       struct PackVFS {
           ma_vfs_callbacks cb;   // SIEMPRE el primer miembro
           PackReader* pack;      // estado propio de la fábrica
       };
     Callbacks a implementar: onOpen (resuelve la entry del pack: el "file"
     es un handle con lump + cursor), onRead (copia desde el buffer del
     pack), onSeek/onTell (cursor dentro del lump), onInfo (sizeInBytes),
     onClose. (onWrite queda sin usar: los packs son de solo lectura.)
   - Se instala: ma_resource_manager_config_init() + config.pVFS = &vfs
     (así lo muestra la discusión del resource manager en el header), y
     toda la API de ma_sound funciona "como si hubiera archivos sueltos".
   ¿Por qué VFS y no ma_decoder_init_memory? Con init_memory hay que copiar
   el lump completo (o descargar la fuente); el VFS streama DIRECTO del
   pack en RAM (o del mmap del .pak en disco).

   INTEGRIDAD: CRC32
   -----------------
   CRC32 = división polinómica del mensaje. Es el checksum estándar que usa
   ZIP (por entry), PNG, GZIP y todo el ecosystem del storage. Tabla con el
   polinomio reflejanizado 0xEDB88320 y el update por byte:
       crc arranca en 0xFFFFFFFF
       crc = (crc >> 8) ^ tabla[(crc ^ byte) & 0xFF]
       el valor final se XOR-ea con 0xFFFFFFFF
   Vector de prueba universal ("123456789", el del RFC) = 0xCBF43926.
   El pack de la lección 08 lo usa: el writer computa el CRC por lump y el
   loader lo recalcula y COMPARA. Si no matchea: error limpio ("asset
   corrupto: no cargar"), antes que devolver memoria tocada al mixer.

   xxHash (mención honstesta del topic): es el hash rápido "moderno" para
   packs gigantes (repo Cyan4973/xxHash; hay CMakeLists). NO lo portamos a
   mano en esta lección: es más fácil de equivartarse que CRC32 y en clase
   hace menos falta. Si tu motor lo quiere, FetchContent al tagreal.
   ============================================================================= */
#include "include/f25.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <vector>

/* --------------------------------- CRC32 --------------------------------- */
namespace {

class Crc32 {
public:
    static const std::array<std::uint32_t, 256>& tabla() {
        static const std::array<std::uint32_t, 256> t = crearTabla();
        return t;
    }

    /* Streaming: update por chunk (llamable en varios pedazos concat). */
    void update(const std::uint8_t* data, std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            crc_ = (crc_ >> 8) ^ tabla()[(crc_ ^ data[i]) & 0xFFu];
        }
        arranco_en_0 = true;
    }

    std::uint32_t final() const { return crc_ ^ 0xFFFFFFFFu; }

    /* One-shot: todo en una pasita (el caso típico del writer). */
    static std::uint32_t deUnaVez(const std::uint8_t* data, std::size_t n) {
        Crc32 c;
        c.update(data, n);
        return c.final();
    }

private:
    static std::array<std::uint32_t, 256> crearTabla() {
        std::array<std::uint32_t, 256> t{};
        for (std::uint32_t i = 0; i < 256; ++i) {
            std::uint32_t c = i;
            for (int bit = 0; bit < 8; ++bit) {
                c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            }
            t[i] = c;
        }
        return t;
    }

    std::uint32_t crc_ = 0xFFFFFFFFu;
    bool arranco_en_0 = false; /* (comprobación del estado inicial) */
};

/* Chunked stream: dos updates al UN mismo crc diario igual que one-shot. */
inline void chunkedStreamDemo() {
    const std::uint8_t datos[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    Crc32 uno;
    uno.update(datos, sizeof(datos));

    Crc32 dos;
    dos.update(datos, 3);       /* primer pedazo: 1..3 */
    dos.update(datos + 3, 5);   /* segundo: 4..8 */
    std::printf("CRC32 chunked: 0x%08X vs one-shot 0x%08X, mismos = %s\n",
                dos.final(), uno.final(),
                (dos.final() == uno.final()) ? "SI" : "NO");
}

} // namespace

/* --------------------------- Modo con miniaudio -------------------------- */
#if DEMO_MINIAUDIO
#include "miniaudio.h"

/* El handle de un "archivo abierto dentro del pack". */
struct PackArchivo {
    const std::uint8_t* datos;
    std::size_t size;
    std::size_t cursor;
};

struct PackVFS {
    ma_vfs_callbacks cb;        /* PRIMER miembro: patrón ma_default_vfs */
    const std::uint8_t* packFile; /* buffer del pack (lo real es PackReader*) */
    std::size_t packSize;
    std::size_t offsetLump;     /* el lump que "abre" = offset del pack */
    std::size_t sizeLump;
};

static ma_result vfsOpen(ma_vfs* pVFS, const char* pFilePath,
                         ma_uint32 openMode, ma_vfs_file* pFile) {
    auto* vfs = reinterpret_cast<PackVFS*>(pVFS);
    if ((openMode & MA_OPEN_MODE_WRITE) != 0) {
        return MA_ACCESS_DENIED; /* el pack es de solo lectura */
    }
    (void)pFilePath; /* un PackReader real: lookup del nombre en la TOC */
    auto* archivo = new PackArchivo{ vfs->packFile + vfs->offsetLump,
                                     vfs->sizeLump, 0 };
    *pFile = reinterpret_cast<ma_vfs_file>(archivo);
    return MA_SUCCESS;
}

static ma_result vfsRead(ma_vfs* pVFS, ma_vfs_file archivo, void* pDst,
                         size_t sizeInBytes, size_t* pBytesRead) {
    (void)pVFS;
    auto* a = reinterpret_cast<PackArchivo*>(archivo);
    const std::size_t disponible = a->size - a->cursor;
    const std::size_t n = sizeInBytes < disponible ? sizeInBytes : disponible;
    if (n > 0) {
        for (std::size_t i = 0; i < n; ++i) {
            static_cast<std::uint8_t*>(pDst)[i] = a->datos[a->cursor + i];
        }
        a->cursor += n;
    }
    if (pBytesRead) { *pBytesRead = n; }
    return n < sizeInBytes ? MA_END_OF_FILE : MA_SUCCESS;
}

static ma_result vfsSeek(ma_vfs* pVFS, ma_vfs_file archivo, ma_int64 offset,
                         ma_seek_origin origin) {
    (void)pVFS;
    auto* a = reinterpret_cast<PackArchivo*>(archivo);
    if (origin == ma_seek_origin_start) {
        a->cursor = static_cast<std::size_t>(offset > 0 ? offset : 0);
    } else if (origin == ma_seek_origin_current) {
        a->cursor += static_cast<std::size_t>(offset > 0 ? offset : 0);
    } else {
        return MA_INVALID_ARGS; /* ma_seek_origin_end no lo usan los decoders */
    }
    if (a->cursor > a->size) { return MA_BAD_SEEK; }
    return MA_SUCCESS;
}

static ma_result vfsTell(ma_vfs* pVFS, ma_vfs_file archivo, ma_int64* pCursor) {
    (void)pVFS;
    auto* a = reinterpret_cast<PackArchivo*>(archivo);
    if (pCursor) { *pCursor = static_cast<ma_int64>(a->cursor); }
    return MA_SUCCESS;
}

static ma_result vfsInfo(ma_vfs* pVFS, ma_vfs_file archivo,
                         ma_file_info* pInfo) {
    (void)pVFS;
    auto* a = reinterpret_cast<PackArchivo*>(archivo);
    if (pInfo) { pInfo->sizeInBytes = a->size; }
    return MA_SUCCESS;
}

static ma_result vfsClose(ma_vfs* pVFS, ma_vfs_file archivo) {
    (void)pVFS;
    delete reinterpret_cast<PackArchivo*>(archivo);
    return MA_SUCCESS;
}

int demoVfsReal() {
    ma_resource_manager rm;
    ma_resource_manager_config cfg = ma_resource_manager_config_init();
    PackVFS vfs;
    vfs.cb.onOpen = vfsOpen;
    vfs.cb.onClose = vfsClose;
    vfs.cb.onRead = vfsRead;
    vfs.cb.onSeek = vfsSeek;
    vfs.cb.onTell = vfsTell;
    vfs.cb.onInfo = vfsInfo;
    vfs.cb.onOpenW = nullptr;              /* la versión char alcanza */
    vfs.cb.onWrite = nullptr;              /* el pack es read-only */
    cfg.pVFS = &vfs;
    if (ma_resource_manager_init(&cfg, &rm) != MA_SUCCESS) {
        return 1;
    }
    std::printf("resource manager OK con VFS custom: los ma_sound leen del\n"
                "pack vía (onOpen / onRead / onSeek) sin tocar el disco.\n");
    ma_resource_manager_uninit(&rm);
    return 0;
}
#endif /* DEMO_MINIAUDIO */

int main() {
    std::printf("M25 / L09 - streaming desde pack + CRC32\n");
    std::printf("========================================\n");

    /* 1. El vector de prueba universal del CRC32. */
    const std::uint8_t rfc[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    const std::uint32_t v = Crc32::deUnaVez(rfc, sizeof(rfc));
    std::printf("CRC32(\"123456789\") = 0x%08X (esperado 0x%08X) %s\n", v,
                0xCBF43926u, (v == 0xCBF43926u) ? "OK" : "MAL");

    /* 2. Streaming por chunks == one-shot. */
    chunkedStreamDemo();

    /* 3. Integridad: un byte corrupto se detecta. */
    std::vector<std::uint8_t> lump = { 10, 20, 30, 40 };
    const std::uint32_t ok = Crc32::deUnaVez(lump.data(), lump.size());
    lump[2] = 31; /* corrupción */
    const std::uint32_t bad = Crc32::deUnaVez(lump.data(), lump.size());
    std::printf("crc lump integro = 0x%08X; corrupto = 0x%08X; detectado = %s\n",
                ok, bad, (ok != bad) ? "SI" : "NO");

#if DEMO_MINIAUDIO
    demoVfsReal();
#endif
    return 0;
}
