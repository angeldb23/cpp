/* =============================================================================
   MÓDULO 17 - LECCIÓN 09: Tilemaps — Tiled JSON, Capas, Tileset, Índices GID
   =============================================================================
   TEMA 8 de TEMAS.md (1ra lección): Tilemaps

   QUÉ ES UN TILEMAP:
   Una grilla de tiles (celdas) donde cada celda apunta a un tile del tileset.
   Los niveles de los juegos 2D (super mario, stardew valley, hot…) son tilemaps:
   baratos de editar (una grilla visual), baratos de render (dibujar cuadrados),
   y rápidos de consultar (colisión O(1)).

   FORMATO TILED JSON (export del editor Tiled, map format json):
   {
     "width": 16, "height": 12,          <- grilla en CANTIDAD de tiles
     "tilewidth": 32, "tileheight": 32,  <- tamaño del tile en píxeles
     "layers": [
       { "name": "Fondo",  "data": [1,1,2,0,...] },   <- índices GID por fila
       { "name": "Sólidos","data": [...] }
     ],
     "tilesets": [ { "firstgid": 1, "image": "tiles.png",
                     "imagewidth": 256, "imageheight": 64,
                     "tilewidth": 32, "tileheight": 32 } ]
   }
   GID = Global Tile ID: identificador GLOBAL del tile a través de todos los
   tilesets del mapa. 1, 2, 3... apuntan al PRIMER tileset (firstgid=1);
   si hay más tilesets, cada uno "toma" un rango consecutivo a partir de su
   firstgid. El GID 0 = "vacío" (no dibuja nada).

   DETALLES IMPORTANTES:
   - data es una lista de fila mayor (row-major): la fila 0 son los valores
     [0, width), la fila 1 los [width, 2*width), etc.
   - Los flips de Tiled se marcan con los bits altos del GID:
        0x80000000 horizontal flip, 0x40000000 vertical flip,
        0x20000000 diagonal flip (rotación 90°). Para leer el tile "limpio":
        gid &= 0x1FFFFFFF ( los 29 bits bajos).
   - Un tilemap puede tener muchas capas (fondo, sólidos, decoración, transform).

   PARSER JSON:
   Al curso le gustaría que se use nlohmann/json (módulo 12/15), pero para que
   TODO compile online y autocontenido en ESTA lección, escribimos un parser
   JSON mínimo (líneas ~80-160, estilo mini::expected del módulo 13). Sirve
   para leer maps exportados de Tiled SIN dependencias.

   NOTA: Código compila sin raylib (stubs). El map JSON es embebido en texto
   para que la demo sea determinista (si quieres leer archivos, módulo 12).
   ============================================================================= */

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <cmath>
#include <cstdlib>
#include <sstream>

// -----------------------------------------------------------------------------
// Stubs raylib
// -----------------------------------------------------------------------------
struct Rectangle { float x, y, width, height; };
struct Texture2D { int id; int width; int height; };
inline void DrawTextureRec(Texture2D, Rectangle, Rectangle /*pos*/, ...) {}

// =============================================================================
// 1. Mini parser JSON (suficiente para Tiled export)
// =============================================================================
// Se guía por "qué valores peculiares preciso del tiled": objetos, arrays,
// números, strings, bools y null. Con esto no necesitas la lib full JSON.
//
// Diseño: árbol de nodos con union discriminante (variant-style) de baja
// fricción; un JsonValue se sabe consultando kind.

class JsonValue {
public:
    enum Kind { Null, Bool, Number, String, Array, Object };

    Kind kind = Null;
    bool b = false;
    double num = 0.0;
    std::string str;
    std::vector<JsonValue> arr;
    std::unordered_map<std::string, JsonValue> obj;

    bool Is(Kind k) const { return kind == k; }

    bool IsNumber() const { return kind == Number; }
    bool IsString() const { return kind == String; }
    bool IsArray()  const { return kind == Array; }

    // Accesos cómodos (como en nlohmann::json pero sin el template plano)
    double AsNumber(double def = 0.0) const { return IsNumber() ? num : def; }
    const std::string& AsString() const { return str; }

    const JsonValue* Find(const std::string& key) const {
        if (kind != Object) return nullptr;
        auto it = obj.find(key);
        return (it == obj.end()) ? nullptr : &it->second;
    }
};

class JsonParser {
    const char* s_;
    const char* end_;

public:
    // Parse completo (permitido: mapa json del tiled es pequeño)
    static bool Parse(const std::string& text, JsonValue& out) {
        JsonParser p(text.data(), text.data() + text.size());
        p.SkipWs();
        if (!p.ExpectValue(out)) return false;
        p.SkipWs();
        return true;
    }

private:
    JsonParser(const char* s, const char* e) : s_(s), end_(e) {}

    void SkipWs() {
        while (s_ < end_ && (*s_ == ' ' || *s_ == '\t' || *s_ == '\n' || *s_ == '\r')) ++s_;
    }

    bool Match(char c) {
        if (s_ < end_ && *s_ == c) { ++s_; return true; }
        return false;
    }

    bool Expect(char c) {
        SkipWs();
        if (!Match(c)) {
            std::cout << "JSON ERROR: se esperaba '" << c << "'\n";
            return false;
        }
        return true;
    }

    // Dispatch por primer carácter
    bool ExpectValue(JsonValue& v) {
        SkipWs();
        if (s_ >= end_) return false;
        char c = *s_;
        if (c == '{')  return ExpectObject(v);
        if (c == '[')  return ExpectArray(v);
        if (c == '"')  { v.kind = JsonValue::String; return ParseString(v.str); }
        if (c == 't')  { return ParseLiteral("true")  && (v = BoolTrue(), true); }
        if (c == 'f')  { return ParseLiteral("false") && (v = BoolFalse(), true); }
        if (c == 'n')  { return ParseLiteral("null")  && (v = NullValue(), true); }
        return ParseNumber(v);
    }

    bool ParseLiteral(const char* lit) {
        for (; *lit; ++lit) {
            if (s_ >= end_ || *s_ != *lit) return false;
            ++s_;
        }
        return true;
    }

    static JsonValue BoolTrue()  { JsonValue v; v.kind = JsonValue::Bool; v.b = true;  return v; }
    static JsonValue BoolFalse() { JsonValue v; v.kind = JsonValue::Bool; v.b = false; return v; }
    static JsonValue NullValue() { return JsonValue{}; }

    bool ParseString(std::string& out) {
        if (!Match('"')) return false;
        out.clear();
        while (s_ < end_ && *s_ != '"') {
            if (*s_ == '\\' && s_ + 1 < end_) {
                ++s_;  // skip escape char; para Tiled solo necesitamos \\, \" y \n
                if (*s_ == 'n') out += '\n'; else out += *s_;
            } else {
                out += *s_;
            }
            ++s_;
        }
        return Match('"');
    }

    bool ParseNumber(JsonValue& v) {
        char* endp = nullptr;
        double d = std::strtod(s_, &endp);
        if (endp == s_) return false;
        s_ = endp;
        v.kind = JsonValue::Number;
        v.num = d;
        return true;
    }

    bool ExpectObject(JsonValue& v) {
        v.kind = JsonValue::Object;
        if (!Expect('{')) return false;
        while (true) {
            SkipWs();                       // \n\t entre ',' y la próxima clave
            if (Match('}')) return true;    // incluye objeto vacío y cierre
            std::string key;
            if (!ParseString(key)) return false;
            if (!Expect(':')) return false;
            JsonValue val;
            if (!ExpectValue(val)) return false;
            v.obj[key] = std::move(val);
            SkipWs();
            if (Match(',')) continue;
            if (Match('}')) return true;
            std::cout << "JSON ERROR: se esperaba ',' o '}'\n";
            return false;
        }
    }

    bool ExpectArray(JsonValue& v) {
        v.kind = JsonValue::Array;
        if (!Expect('[')) return false;
        while (true) {
            SkipWs();
            if (Match(']')) return true;    // incluye array vacío y cierre
            JsonValue item;
            if (!ExpectValue(item)) return false;
            v.arr.push_back(std::move(item));
            SkipWs();
            if (Match(',')) continue;
            if (Match(']')) return true;
            std::cout << "JSON ERROR: se esperaba ',' o ']'\n";
            return false;
        }
    }
};

// =============================================================================
// 2. Estructuras de juego: Tileset, Layer, TileMap
// =============================================================================
struct Tileset {
    std::string imagePath;
    int imageWidth = 0, imageHeight = 0;
    int tileWidth = 32, tileHeight = 32;
    int columns = 1;             // derivado: imageWidth / tileWidth
    int firstGid = 1;
};

struct TileLayer {
    std::string name;
    int width = 0, height = 0;
    std::vector<int> gids;       // row-major, 0 = vacío
};

class TileMap {
public:
    int MapWidth()  const { return widthTiles_; }
    int MapHeight() const { return heightTiles_; }
    int TileWidth() const { return tileW_; }
    int TileHeight() const { return tileH_; }
    int PixelWidth() const { return widthTiles_ * tileW_; }

    TileMap(int wTiles, int hTiles, int tw, int th)
        : widthTiles_(wTiles), heightTiles_(hTiles), tileW_(tw), tileH_(th) {}

    Tileset& AddTileset(const Tileset& ts) { tilesets_.push_back(ts); return tilesets_.back(); }
    TileLayer& AddLayer(const std::string& name) {
        TileLayer layer;
        layer.name = name;
        layer.width = widthTiles_;
        layer.height = heightTiles_;
        layers_.push_back(std::move(layer));
        return layers_.back();
    }

    // GID -> tileset index (búsqueda simple; con 1 tileset es 0 siempre)
    // Con N tilesets se ordena por firstGid y se hace una lower_bound.
    const Tileset* TilesetForGid(int gid) const {
        const Tileset* best = nullptr;
        for (const auto& ts : tilesets_) {
            if (gid >= ts.firstGid && (best == nullptr || ts.firstGid > best->firstGid)) {
                best = &ts;
            }
        }
        return best;
    }

    // GID -> rect en el atlas del tileset correspondiente (para el DrawTexturePro)
    // Devuelve false si el GID es 0 (vacío) o no hay tileset que lo cubra.
    bool UvRectForGid(int gid, const Tileset*& ts, Rectangle& src) const {
        if (gid <= 0) return false;
        ts = TilesetForGid(gid);
        if (!ts) return false;

        int localIndex = gid - ts->firstGid;      // índice dentro del tileset
        int col = localIndex % ts->columns;
        int row = localIndex / ts->columns;
        src = Rectangle{
            float(col * ts->tileWidth), float(row * ts->tileHeight),
            float(ts->tileWidth), float(ts->tileHeight)
        };
        return true;
    }

    // Acceso lógico
    int GidAt(const TileLayer& layer, int tx, int ty) const {
        if (tx < 0 || ty < 0 || tx >= layer.width || ty >= layer.height) return 0;
        return layer.gids[ty * layer.width + tx];
    }

    Rectangle UvFromTileAtlas(const Tileset& ts, int localIndex) const {
        int col = localIndex % ts.columns;
        int row = localIndex / ts.columns;
        return Rectangle{ float(col * ts.tileWidth), float(row * ts.tileHeight),
                          float(ts.tileWidth), float(ts.tileHeight) };
    }

    size_t LayerCount() const { return layers_.size(); }
    const TileLayer& LayerAt(size_t i) const { return layers_[i]; }

private:
    int widthTiles_ = 0, heightTiles_ = 0;
    int tileW_ = 32, tileH_ = 32;
    std::vector<Tileset> tilesets_;
    std::vector<TileLayer> layers_;
};

// =============================================================================
// 3. Importación TILED JSON -> TileMap
// =============================================================================
bool LoadTiledJson(const std::string& json, TileMap& out) {
    JsonValue root;
    if (!JsonParser::Parse(json, root)) return false;

    auto* w  = root.Find("width");
    auto* h  = root.Find("height");
    auto* tw = root.Find("tilewidth");
    auto* th = root.Find("tileheight");
    if (!w || !h || !tw || !th) {
        std::cout << "Tiled JSON: faltan width/height/tilewidth/tileheight\n";
        return false;
    }
    out = TileMap(int(w->num), int(h->num), int(tw->num), int(th->num));

    if (auto* tsets = root.Find("tilesets"); tsets && tsets->Is(JsonValue::Array)) {
        for (const auto& tset : tsets->arr) {
            Tileset ts;
            ts.firstGid = int(tset.Find("firstgid")->num);
            ts.imagePath = tset.Find("image")->str;
            ts.imageWidth  = int(tset.Find("imagewidth")->AsNumber(0));
            ts.imageHeight = int(tset.Find("imageheight")->AsNumber(0));
            ts.tileWidth  = int(tset.Find("tilewidth")->AsNumber(tw ? 32 : 32));
            ts.tileHeight = int(tset.Find("tileheight")->AsNumber(th ? 32 : 32));
            ts.columns = ts.imageWidth / ts.tileWidth;
            out.AddTileset(ts);
        }
    }

    if (auto* layers = root.Find("layers"); layers && layers->Is(JsonValue::Array)) {
        for (const auto& lyr : layers->arr) {
            auto* name = lyr.Find("name");
            auto* data = lyr.Find("data");
            if (!name || !data) continue;
            TileLayer& layer = out.AddLayer(name->str);
            layer.gids.reserve(data->arr.size());
            for (const auto& gid : data->arr) {
                layer.gids.push_back(int(gid.AsNumber(0)));
            }
        }
    }
    return true;
}

// -----------------------------------------------------------------------------
// Demo con un mini-mapa Tiled (16x4, 32x32) embebido
// -----------------------------------------------------------------------------
const char* kDemoMap = R"JSON(
{
  "width": 16, "height": 4, "tilewidth": 32, "tileheight": 32,
  "tilesets": [
    { "firstgid": 1, "image": "assets/tiles.png",
      "imagewidth": 192, "imageheight": 32, "tilewidth": 32, "tileheight": 32 }
  ],
  "layers": [
    { "name": "Fondo", "data": [
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 ] },
    { "name": "Sólidos", "data": [
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,3,0,0,0,0,0,7,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        2,2,2,0,0,0,4,4,4,4,4,0,0,5,5,5 ] }
  ]
}
)JSON";

void demo_tilemap() {
    std::cout << "=== Tilemaps: Tiled JSON ===\n\n";

    TileMap emptyMap(0, 0, 32, 32);
    if (!LoadTiledJson(kDemoMap, emptyMap)) {
        std::cout << "Error cargando el mapa\n";
        return;
    }
    std::cout << "Mapa: " << emptyMap.MapWidth() << "x" << emptyMap.MapHeight()
              << " tiles de " << emptyMap.TileWidth() << "px => "
              << emptyMap.PixelWidth() << "px de largo\n";

    // Capas cargadas
    for (size_t i = 0; i < emptyMap.LayerCount(); ++i) {
        std::cout << "Capa " << i << ": " << emptyMap.LayerAt(i).name
                  << " con " << emptyMap.LayerAt(i).gids.size() << " celdas\n";
    }

    // GID -> tileset: primer tileset tiene firstGid=1
    const Tileset* ts = nullptr;
    Rectangle src{};
    for (int gid : {1, 2, 3, 5, 7, 10}) {
        bool ok = emptyMap.UvRectForGid(gid, ts, src);
        std::cout << "GID " << gid << ": " << (ok ? "OK" : "vacío")
                  << " => rect(" << src.x << "," << src.y
                  << ",w=" << src.width << ",h=" << src.height << ")\n";
    }

    // Comprobación del grid: GID 2 en "Sólidos", fila 3 col 0, 4px plat de GID 5
    const TileLayer& solids = emptyMap.LayerAt(1);
    std::cout << "GID(0,3) en Sólidos   = " << emptyMap.GidAt(solids, 0, 3) << "\n";
    std::cout << "GID(5,1) en Sólidos   = " << emptyMap.GidAt(solids, 5, 1) << "\n";
    std::cout << "GID(11,1) en Sólidos  = " << emptyMap.GidAt(solids, 11, 1) << "\n";

    // Conteo de tiles sólidos (GID != 0)
    int solidsCount = 0;
    for (int gid : solids.gids) if (gid != 0) ++solidsCount;
    std::cout << "Tiles solidos: " << solidsCount << "\n";

    std::cout << "\n=== FIN LECCIÓN 09 ===\n";
}

int main() {
    demo_tilemap();
    return 0;
}
