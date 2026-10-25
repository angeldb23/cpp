/* =============================================================================
   MÓDULO 25 - LECCIÓN 08: empaquetado de assets — formato pack, TOC e índice
   =============================================================================
   TEMA 5 (parte 1): Empaquetado de assets: archivo pack (zip/tar/custom),
   índice, streaming desde pack, verificación de integridad (CRC32/xxHash)
   - 2 lecciones. Acá: el FORMATO.

   ¿POR QUÉ UN PACK?
   -----------------
   Un juego chico tiene literalmente 1.000+ archivos. Un solo .pak:
   - Menos I/O: abrir 1 archivo una vez vs mil fopen (inodes, antivirus
     escaneando, cache del disco).
   - Streaming con seek: cada lump vive en un offset del archivo: se puede
     leer la PORCIÓN que importa (substream virtual) sin descomprimir todo.
   - Orden de carga exportable: el orden de la TOC es el orden de carga.

   TAR vs ZIP vs CUSTOM (sin depender de librerías)
   ------------------------------------------------
   - TAR: sin compresión, encabezados de 512 bytes, index linear; random
     access = buscar "scan" desde el principio (o pedir un índice extra).
   - ZIP: con compresión y central directory al FINAL del archivo (el
     "End of Central Directory" se busca desde el final del archivo). Cada
     entry lleva su propio CRC32: la MISMA fórmula que implementamos en la
     lección 09.
   - CUSTOM: lo más simple para un juego: tu TOC al principio + offsets
     directos + (opcional) compresión por chunk. En la práctica todos los
     motores terminan con un pack propio con un "cooker" (módulo 28:
     herramienta CLI que produce el binario final).
   ============================================================================= */
#include "include/f25.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

/* ---------------------------- Formato binario ----------------------------- */
namespace {

/* Layout del pack (little endian):
      magic     "VPAK"                  4 bytes
      version   1                      4 bytes
      count     cantidad de lumps      4 bytes
      TOC count entradas:
        nameLen u32, name (nameLen bytes), offset u64, size u64
      DA: los lumps concatenados, CADA UNO alineado a 16 bytes (padding con
      ceros entre lumps). Los offsets de la TOC apuntan al inicio del dato.
   La TOC va ANTES de los datos, pero sus offsets dependen del tamaño de la
   propia TOC (que depende del largo de los nombres). Por eso el escritor
   hace 3 momentos: 1) computa tocSize, 2) computa offsets, 3) serializa. */
inline constexpr std::uint32_t kMagicPack = 0x4B415056; /* "VPAK" LE */
inline constexpr std::uint32_t kVersionPack = 1;
inline constexpr std::size_t kAlineacion = 16;

struct EntradaPack {
    std::string nombre;
    std::uint64_t offset = 0; /* desde el INICIO del archivo */
    std::uint64_t size = 0;
};

void pushU32(std::vector<std::uint8_t>& v, std::uint32_t x) {
    v.push_back(static_cast<std::uint8_t>(x));
    v.push_back(static_cast<std::uint8_t>(x >> 8));
    v.push_back(static_cast<std::uint8_t>(x >> 16));
    v.push_back(static_cast<std::uint8_t>(x >> 24));
}
void pushU64(std::vector<std::uint8_t>& v, std::uint64_t x) {
    pushU32(v, static_cast<std::uint32_t>(x));
    pushU32(v, static_cast<std::uint32_t>(x >> 32));
}
void pushStr(std::vector<std::uint8_t>& v, const std::string& s) {
    pushU32(v, static_cast<std::uint32_t>(s.size()));
    for (char c : s) { v.push_back(static_cast<std::uint8_t>(c)); }
}
std::size_t alinear(std::size_t cursor) {
    const std::size_t resto = cursor % kAlineacion;
    return resto == 0 ? cursor : cursor + (kAlineacion - resto);
}

/* PASO 1: el "cooker" recibe (nombre, datos) y arma el binario completo. */
class PackWriter {
public:
    void addLump(std::string nombre, std::vector<std::uint8_t> datos) {
        lumps_.push_back(Lump{ std::move(nombre), std::move(datos), 0 });
    }

    std::vector<std::uint8_t> build() {
        /* 1) serializo SOLO el TOC (sin offsets) para conocer su tamaño. */
        std::vector<std::uint8_t> toc;
        for (const Lump& l : lumps_) {
            pushStr(toc, l.nombre);
            pushU64(toc, 0); /* offset (se computa abajo) */
            pushU64(toc, 0); /* size */
        }
        /* 2) offsets: cada lump alineado después de header+TOC. */
        std::size_t cursor = alinear(12 + toc.size());
        for (Lump& l : lumps_) {
            cursor = alinear(cursor);
            l.offset = cursor;
            cursor += l.datos.size();
        }
        /* 3) serialización final: header + TOC (offsets reales) + data. */
        std::vector<std::uint8_t> out;
        out.reserve(cursor);
        pushU32(out, kMagicPack);
        pushU32(out, kVersionPack);
        pushU32(out, static_cast<std::uint32_t>(lumps_.size()));
        for (const Lump& l : lumps_) {
            pushStr(out, l.nombre);
            pushU64(out, l.offset);
            pushU64(out, l.datos.size());
        }
        /* padding hasta el primer dato */
        out.resize(alinear(out.size()), 0);
        for (const Lump& l : lumps_) {
            out.resize(alinear(out.size()), 0); /* re-alinear entre lumps */
            out.insert(out.end(), l.datos.begin(), l.datos.end());
        }
        return out;
    }

    const std::vector<EntradaPack> toc() const {
        std::vector<EntradaPack> e;
        for (const Lump& l : lumps_) {
            e.push_back(EntradaPack{ l.nombre, l.offset, l.datos.size() });
        }
        return e;
    }

private:
    struct Lump {
        std::string nombre;
        std::vector<std::uint8_t> datos;
        std::uint64_t offset = 0;
    };
    std::vector<Lump> lumps_;
};

/* PASO 2: el lector (el motor juega con esto EN runtime). */
class PackReader {
public:
    bool open(const std::vector<std::uint8_t>& file) {
        file_ = &file;
        std::size_t p = 0;
        auto u32 = [&]() -> std::uint32_t {
            std::uint32_t x = static_cast<std::uint32_t>((*file_)[p])
                | (static_cast<std::uint32_t>((*file_)[p + 1]) << 8)
                | (static_cast<std::uint32_t>((*file_)[p + 2]) << 16)
                | (static_cast<std::uint32_t>((*file_)[p + 3]) << 24);
            p += 4;
            return x;
        };
        auto u64 = [&]() -> std::uint64_t { return u32() + (static_cast<std::uint64_t>(u32()) << 32); };
        if (u32() != kMagicPack) {
            return false;
        }
        if (u32() != kVersionPack) {
            return false;
        }
        const std::uint32_t count = u32();
        toc_.clear();
        for (std::uint32_t i = 0; i < count; ++i) {
            const std::uint32_t nameLen = u32();
            std::string nombre;
            nombre.resize(nameLen);
            for (std::uint32_t a = 0; a < nameLen; ++a) {
                nombre[a] = static_cast<char>((*file_)[p++]);
            }
            const std::uint64_t off = u64();
            const std::uint64_t sz = u64();
            toc_.push_back(EntradaPack{ nombre, off, sz });
        }
        return true;
    }

    /* Devuelve un PUNTERO dentro del propio archivo (zero-copy): el lump
       vive donde el pack ya lo tenía en memoria. */
    const std::uint8_t* lump(std::size_t& size, const std::string& nombre) const {
        for (const EntradaPack& e : toc_) {
            if (e.nombre == nombre) {
                size = static_cast<std::size_t>(e.size);
                return file_->data() + e.offset;
            }
        }
        size = 0;
        return nullptr;
    }

    const std::vector<EntradaPack>& toc() const { return toc_; }

private:
    const std::vector<std::uint8_t>* file_ = nullptr;
    std::vector<EntradaPack> toc_;
};

} // namespace

int main() {
    std::printf("M25 / L08 - pack de assets: formato y TOC\n");
    std::printf("=========================================\n");

    /* 1. Cocinar un pack con 3 lumps. */
    PackWriter w;
    w.addLump("sfx/paso.bin", { 1, 2, 3, 4, 5 });
    w.addLump("sfx/tiro.bin", { 9, 9, 9, 9, 9, 9 });
    std::vector<std::uint8_t> musicaAudio(64, 0xC3);
    w.addLump("music/menina.ogg", musicaAudio);
    const std::vector<std::uint8_t> file = w.build();
    std::printf("pack construido: %zu bytes totales\n", file.size());

    /* 2. Leer la TOC y confirmar offsets alineados. */
    PackReader r;
    if (!r.open(file)) {
        std::printf("ERROR: el pack no abre (magic/version)\n");
        return 1;
    }
    for (const EntradaPack& e : r.toc()) {
        std::printf("  TOC: %-18s offset=%llu size=%llu alineado=%s\n",
                    e.nombre.c_str(),
                    (unsigned long long)e.offset,
                    (unsigned long long)e.size,
                    (e.offset % kAlineacion == 0) ? "si" : "no");
    }

    /* 3. Zero-copy: el lector devuelve punteros DENTRO del file buffer. */
    std::size_t z = 0;
    const std::uint8_t* paso = r.lump(z, "sfx/paso.bin");
    if (paso != nullptr) {
        std::printf("lump sfx/paso.bin: size=%zu datos[0..2]=%u %u %u\n",
                    z, paso[0], paso[1], paso[2]);
    }
    std::size_t z2 = 0;
    std::printf("lump inexistente: %p (nullptr = 404 del asset)\n",
                static_cast<const void*>(r.lump(z2, "no/existe.bin")));

    (void)alinear;
    return 0;
}
