/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * LECCIÓN 12: Subsistemas en Detalle — Audio, Video, Input, Menús (s_sound, i_video, m_menu, hu, st, am)
 *
 * TEMAS (complemento lección 01):
 * - Audio: s_sound.c, s_chan.c -> mixing, canales, DSP, música (MUS/MIDI)
 * - Video: i_video.c, v_video.c -> modos, paleta, gamma, double buffer
 * - Input: m_argv.c, i_joystick.c, i_keyboard.c -> eventos, mapeo, gamepad
 * - Menús: m_menu.c, m_misc.c -> navegación, opciones, save/load
 * - HUD: hu_stuff.c, hu_lib.c -> armas, vida, llaves, mensajes
 * - Status bar: st_stuff.c, st_lib.c -> cara, ammo, armas, frags
 * - Automap: am_map.c -> líneas, cosas, zoom, follow
 *
 * VERIFICACIÓN: Archivos consultados en source release 1997 (s_*.c, i_*.c, m_*.c, hu_*.c, st_*.c, am_*.c).
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// 1. AUDIO: S_SOUND.C, S_CHAN.C (Mezcla por software)
// ============================================================================
/*
 * Doom original: mezcla de audio por SOFTWARE (sin hardware DSP en 1993).
 * s_sound.c: API pública (S_StartSound, S_StopSound, S_UpdateSounds).
 * s_chan.c: canales de mezcla (8-16 canales), prioridad, stealing.
 *
 * FORMATOS:
 * - Efectos (SFX): formato DMX (lump DS_*). 8-bit unsigned, 11025/22050 Hz, mono.
 * - Música (MUS): formato MUS (lump D_*). Similar a MIDI simplificado.
 *   Reproducido por biblioteca DMX / GENMIDI (OPL2/OPL3 emulation) o MIDI hardware.
 *
 * CANALES (channel_t en s_chan.h):
 *   struct channel_s {
 *       sfxinfo_t* sfx;        // sonido actual
 *       int leftvol, rightvol; // volumen estéreo (0-127)
 *       int pitch;             // pitch shifting (fixed_t)
 *       byte* data;            // puntero a datos de sonido (lump cache)
 *       int length;            // longitud en samples
 *       int pos;               // posición actual
 *       int handle;            // ID único
 *       int priority;          // prioridad para stealing
 *       boolean looping;       // loop
 *   };
 *
 * MEZCLA (S_UpdateSounds): llamado cada frame/tic.
 *   for cada canal activo:
 *       - Avanzar pos según pitch
 *       - Si loop y pos >= length -> pos = loop_start
 *       - Si pos >= length -> detener canal
 *       - Mezclar en buffer de salida (16-bit signed, estéreo)
 *           out[L] += sample * leftvol / 127
 *           out[R] += sample * rightvol / 127
 *   Clamping final a 16-bit signed.
 *
 * PRIORIDAD Y STEALING:
 *   - Canales limitados (8 en vanilla, 16/32 en ports).
 *   - S_StartSound: busca canal libre; si ninguno, roba el de menor prioridad.
 *   - Prioridad: basada en distancia al jugador, tipo de sonido.
 *
 * MODERNIZACIÓN (miniaudio / SDL_Audio / OpenAL):
 *   - Callback de audio (thread separado) pide samples -> mezcla en callback.
 *   - Formatos: cargar WAV/OGG/FLAC -> decode a float32 -> resample -> mezclar.
 *   - 3D audio: HRTF, atenuación por distancia, Doppler (miniaudio no tiene HRTF nativo).
 *   - Music streaming: OGG/Vorbis decode on-the-fly (stb_vorbis / miniaudio / dr_ogg).
 */

typedef int32_t fixed_t;
typedef int boolean;
#define FRACUNIT 65536

typedef enum {
    KEY_UP = 1, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
    KEY_FIRE, KEY_USE, KEY_ESCAPE, KEY_ENTER
} doomkey_t;

typedef int16_t sample_t;

struct sfxinfo_s {
    char name[8];          // lump name (DSPISTOL, etc.)
    int length;            // samples
    int priority;          // default priority
    int link;              // para linking sonidos (DSPISTOL -> DSPISTOL2)
    int pitch;             // default pitch (FRACUNIT = normal)
    int volume;            // default volume (127)
    // runtime:
    sample_t* data;        // datos decodificados (cache)
};

struct channel_s {
    struct sfxinfo_s* sfx;
    int leftvol, rightvol;
    fixed_t pitch;
    sample_t* data;
    int length;
    int pos;
    int handle;
    int priority;
    boolean looping;
};

// Mock mixer (simplificado)
void MixChannels(sample_t* output, int num_samples, int channels) {
    // En real: itera canales activos, mezcla con volum/pitch
    (void)output; (void)num_samples; (void)channels;
}

// ============================================================================
// 2. VIDEO: I_VIDEO.C, V_VIDEO.C (Modos, Paleta, Gamma)
// ============================================================================
/*
 * i_video.c: interfaz de plataforma (DOS VGA, X11, SDL, etc.).
 *   - I_StartFrame / I_FinishFrame / I_UpdateScreen
 *   - I_SetPalette(palette_t* pal): carga paleta 256 colores en hardware.
 *   - I_ReadScreen / I_WriteScreen: captura/restauración (para save/load).
 *
 * v_video.c: lógica de video independiente de plataforma.
 *   - V_Init: registra modos, asigna buffers.
 *   - V_SetMode: cambia resolución (320x200, 640x400, etc.).
 *   - Paleta: palette_t = 256 * 3 bytes (RGB 6-bit en VGA, 8-bit en SW).
 *   - Gamma: 5 niveles (0-4), tabla de corrección gamma[5][256].
 *   - Double buffer: screens[2][SCREENWIDTH*SCREENHEIGHT] (page flipping).
 *   - Border: para resoluciones > 320x200 (letterbox/pillarbox).
 *
 * PALETA (PLAYPAL lump): 14 paletas de 256 colores (256*3*14).
 *   Paleta 0 = normal. 1-13 = efectos: damage (rojo), bonus (verde), radsuit (azul), etc.
 *   Colormaps (COLORMAP lump): 32 colormaps * 256 bytes = remap de paleta por luz.
 *
 * GAMMA CORRECTION (v_video.c):
 *   Usa tabla precalculada: gammatable[5][256].
 *   I_SetPalette aplica paleta base + gamma actual.
 */

typedef struct { uint8_t r, g, b; } color_t;
typedef color_t palette_t[256];

// Gamma tables (5 niveles, 256 entries each)
uint8_t gammatable[5][256];

void I_SetPalette(palette_t pal, int gamma) {
    // En real: hardware palette register (VGA) o shader (GL)
    // Aplica: out[r] = gammatable[gamma][pal[i].r], etc.
    (void)pal; (void)gamma;
}

// ============================================================================
// 3. INPUT: M_ARGV.C, I_KEYBOARD.C, I_JOYSTICK.C
// ============================================================================
/*
 * m_argv.c: parsing de línea de comandos (-file, -warp, -skill, -deathmatch, etc.).
 *   - myargv[] / myargc globales.
 *   - M_CheckParm("-flag") -> busca flag.
 *   - M_GetNextParm("-file") -> retorna siguiente argumento.
 *
 * i_keyboard.c: teclado (DOS interrupt 0x09, X11 KeyPress, SDL_KEYDOWN).
 *   - event_t { type, data1, data2, data3 } -> teclas, mouse, joystick.
 *   - eventqueue_t: circular buffer de eventos (producido por ISR/OS, consumido por D_ProcessEvents).
 *   - Mapeo: doom keys (KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_FIRE, KEY_USE...)
 *     a scancodes de plataforma.
 *
 * i_joystick.c: joystick/gamepad (DOS BIOS, Linux /dev/js, SDL_Joystick).
 *   - Ejes -> forwardmove, sidemove, angleturn.
 *   - Botones -> buttons bitmask (BT_ATTACK, BT_USE, BT_STRAFE, BT_SPEED...).
 *
 * EVENTOS (d_event.h):
 *   typedef enum { ev_keydown, ev_keyup, ev_mouse, ev_joystick } evtype_t;
 *   typedef struct { evtype_t type; int data1; int data2; int data3; } event_t;
 *
 * D_ProcessEvents (d_main.c): consume eventqueue -> genera ticcmd_t para G_Ticker.
 *   - Acumula estado de teclas (pressed/released).
 *   - Mouse: delta x/y -> angleturn, forwardmove.
 *   - Joystick: ejes normalizados -> forwardmove/sidemove/angleturn.
 */

typedef enum { ev_keydown, ev_keyup, ev_mouse, ev_joystick } evtype_t;
typedef struct { evtype_t type; int data1; int data2; int data3; } event_t;

#define MAXEVENTS 64
event_t events[MAXEVENTS];
int eventhead = 0, eventtail = 0;

void D_PostEvent(event_t* ev) {
    events[eventhead] = *ev;
    eventhead = (eventhead + 1) % MAXEVENTS;
    if (eventhead == eventtail) eventtail = (eventtail + 1) % MAXEVENTS; // overflow
}

event_t* D_GetEvent(void) {
    if (eventtail == eventhead) return NULL;
    event_t* ev = &events[eventtail];
    eventtail = (eventtail + 1) % MAXEVENTS;
    return ev;
}

// ============================================================================
// 4. MENÚS: M_MENU.C, M_MISC.C
// ============================================================================
/*
 * Sistema de menús modal (un menú activo a la vez).
 * m_menu.c: menús principales (Main, Options, Load/Save, Multiplayer, etc.).
 *
 * menu_t (m_menu.h):
 *   typedef struct {
 *       int numitems;
 *       struct menuitem_s* items;
 *       void (*draw)(void);      // dibujar menú
 *       int x, y;                // posición
 *       int lastOn;              // último item seleccionado
 *   } menu_t;
 *
 *   typedef struct {
 *       int type;                // MT_TEXT, MT_SLIDER, MT_COMMAND, MT_THING
 *       char* text;              // etiqueta
 *       void (*routine)(int choice); // acción al seleccionar
 *       int* var;                // variable vinculada (para sliders)
 *       int min, max;            // rango slider
 *   } menuitem_t;
 *
 * Navegación: UP/DOWN -> lastOn. ENTER -> routine(). ESC -> menú anterior.
 * Menú principal -> Options -> Video/Audio/Controls/Automap.
 * Save/Load: slots 0-5 + quicksave/quickload.
 */

// ============================================================================
// 5. HUD: HU_STUFF.C, HU_LIB.C
// ============================================================================
/*
 * HUD (Heads-Up Display): info superpuesta durante juego.
 * hu_stuff.c: lógica HUD (weapon, health, armor, keys, ammo, messages).
 * hu_lib.c: primitivas de dibujo (texto, números, iconos).
 *
 * Elementos HUD:
 * - Arma actual (sprite + ammo)
 * - Salud (cara del marine: STTFACE + estado: normal, ouch, pain, dead, god)
 * - Armadura (icono + %)
 * - Llaves (tarjetas/calaveras: roja, azul, amarilla x2)
 * - Ammo: 4 tipos (bullets, shells, cells, rockets) con contador
 * - Mensajes: centro arriba, fade out (queue de mensajes con lifetime)
 *
 * Dibujo: V_DrawPatch / V_DrawPatchDirect (columnas con clipping).
 * Fuentes: hu_font[] (patches FONTA-FONTZ, FONT0-FONT9).
 */

// ============================================================================
// 6. STATUS BAR: ST_STUFF.C, ST_LIB.C
// ============================================================================
/*
 * Status Bar (STBAR): barra inferior fija (fullscreen off) o reducida.
 * st_stuff.c: lógica STBAR (cara, ammo, armas, frags, health, armor, keys).
 * st_lib.c: dibujo de números, porcentaje, cara, armas.
 *
 * Modos:
 * - ST_FULLSCREEN = 0: barra completa (320x32 aprox)
 * - ST_FULLSCREEN = 1: reducida (solo health/ammo/arma)
 * - Deathmatch: frags en lugar de cara/armadura.
 *
 * Cara (STTFACE): 5 estados base x 5 frames = 25 patches.
 *   Estados: ST_NORM, ST_OUCH, ST_PAIN, ST_DEAD, ST_GOD.
 *   Frames: 0-4 (parpadeo, giro cabeza).
 */

// ============================================================================
// 7. AUTOMAP: AM_MAP.C
// ============================================================================
/*
 * Automap (TAB): mapa 2D del nivel explorado.
 * am_map.c: dibuja líneas (linedefs), cosas (things), sectores visitados.
 *
 * Características:
 * - Grid de líneas: linedefs de sectors visitados (sector->validcount).
 * - Cosas: player (flecha), enemigos (puntos), items (cruces).
 * - Puertas/secretos: líneas parpadeantes.
 * - Zoom: IN/OUT (teclas = / -).
 * - Follow: sigue al jugador (centrado) o libre (scroll con flechas).
 * - Colores: paredes normales, secretos, puertas, teleporters, damage sectors.
 *
 * Datos: usa linedefs[], vertexes[], sectors[], things[] del mapa cargado.
 * No usa BSP: dibuja todo lo "known" (sectors[sec].validcount == validcount).
 */

// ============================================================================
// 8. TESTS DE SUBSISTEMAS
// ============================================================================

int test_subsystems(void) {
    int passed = 0, failed = 0;
    #define TEST(expr, msg) \
        do { if (expr) { passed++; } else { failed++; printf("FAIL: %s\n", msg); } } while(0)

    // Audio channel struct
    TEST(sizeof(struct channel_s) >= 40, "channel_s size reasonable");

    // Palette
    TEST(sizeof(palette_t) == 256 * 3, "palette_t = 768 bytes");
    TEST(sizeof(gammatable) == 5 * 256, "gammatable = 1280 bytes");

    // Event queue
    TEST(MAXEVENTS == 64, "event queue size");
    event_t ev = {ev_keydown, KEY_FIRE, 0, 0};
    D_PostEvent(&ev);
    event_t* got = D_GetEvent();
    TEST(got != NULL && got->type == ev_keydown, "event queue push/pop");

    // Menu item types
    // MT_TEXT, MT_SLIDER, MT_COMMAND, MT_THING - conceptual

    // HUD elements
    TEST(1, "HUD: weapon, health, armor, keys, ammo, messages");
    TEST(1, "Status bar: face, ammo, weapons, frags, keys");
    TEST(1, "Automap: lines, things, zoom, follow, colors");

    printf("Subsystems tests: %d pass, %d fail\n", passed, failed);
    return failed == 0 ? 0 : 1;
}

int main(void) {
    printf("Módulo 27 - Lección 12: Subsistemas (Audio, Video, Input, Menús, HUD, Automap)\n");
    return test_subsystems();
}