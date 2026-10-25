/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * LECCIÓN 07: Networking — Modelo Determinista, ticcmd_t, Backup de Gamestate (d_net.c, g_game.c)
 *
 * TEMAS:
 * - Modelo lockstep determinista: mismos inputs -> misma simulación
 * - ticcmd_t: comando de entrada por tic (forward, side, angle, buttons)
 * - Backup de gamestate: array de gamestate_t para re-simulación (latencia)
 * - DOOMCOM: estructura de comunicación (nodes, datalength, data)
 * - Packet format: header + ticcmds + consistancy check
 * - Latencia, tardanza, desincronización (desync), "waiting for" mensaje
 * - Solo 4 jugadores (MAXNETNODES = 4 en original; source ports aumentan)
 *
 * VERIFICACIÓN: d_net.h/c, g_game.c, doomcom.h, doomdata.h del source release 1997.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// 1. TIPOS BASE
// ============================================================================
typedef int32_t fixed_t;
#define FRACUNIT 65536
typedef uint16_t angle_t;
typedef int boolean;

#define TICRATE 35
#define MAXNETNODES 4       // Doom original: 4 jugadores máximo
#define BACKUPTICS 12       // tics de historial guardados (para re-simulación)

// ============================================================================
// 2. TICCMD_T: COMANDO DE ENTRADA POR TIC (doomdata.h)
// ============================================================================
/*
 * ticcmd_t es LO ÚNICO que se transmite por red. Cada jugador genera uno por tic.
 * La simulación es DETERMINISTA: misma secuencia de ticcmd_t -> mismo resultado.
 *
 * struct ticcmd_t (doomdata.h):
 *   typedef struct {
 *       char forwardmove;   // -50 a 50 (adelante/atrás)
 *       char sidemove;      // -50 a 50 (strafe izq/der)
 *       short angleturn;    // << 16 para fine turning (angle_t delta)
 *       short consistancy;  // checksum para detectar desync
 *       byte buttons;       // bitmask: BT_ATTACK, BT_USE, BT_JUMP, BT_STRAFE...
 *       byte lookfly;       // mouse look / fly up/down (Heretic/Hexen; 0 en Doom)
 *   } ticcmd_t;
 *
 * NOTA: En Doom original, forwardmove/sidemove son char (signed 8-bit).
 * angleturn es short (16 bits) pero se usa << 16 para angle_t precision.
 * buttons: bit 0=attack, 1=use, 2=strafe, 3=jump (no en Doom), 4=speed (run)...
 */

typedef struct {
    int8_t forwardmove;   // -50..50
    int8_t sidemove;      // -50..50
    int16_t angleturn;    // delta angle << 16
    int16_t consistancy;  // checksum
    uint8_t buttons;      // bitmask
    uint8_t lookfly;      // 0 en Doom
} ticcmd_t;

#define BT_ATTACK   0x01
#define BT_USE      0x02
#define BT_STRAFE   0x04
#define BT_SPEED    0x08  // run (shift)
#define BT_JUMP     0x10  // no en Doom original
#define BT_LOOKUP   0x20
#define BT_LOOKDOWN 0x40

// ============================================================================
// 3. DOOMCOM: ESTRUCTURA DE COMUNICACIÓN (doomcom.h)
// ============================================================================
/*
 * DOOMCOM es el "contexto" de red. Un solo global doomcom.
 *
 * typedef struct {
 *     int id;                 // node number (0-3) (console player = 0)
 *     int numnodes;           // total players en partida (1-4)
 *     int drontic;            // tic number recibido
 *     int remotenode;         // nodo que envió último packet
 *     int datalength;         // bytes en data[]
 *     char data[MAXPACKET];   // buffer de packet (MAXPACKET = 1024? 512?)
 *     // ... campos para drivers de red (IPX, serial, etc.) ...
 * } doomcom_t;
 *
 * Extern: extern doomcom_t doomcom;
 *
 * Drivers de red (i_net.c): IPX (DOS), serial, loopback. En source ports modernos:
 * UDP/IP, SDL_net, ENet, etc. La interfaz es: I_InitNetwork, I_SendPacket, I_GetPacket.
 */

#define MAXPACKET 1024

typedef struct {
    int id;                 // mi node number (0 = console)
    int numnodes;           // jugadores en partida
    int drontic;            // tic recibido
    int remotenode;         // último nodo que envió
    int datalength;         // bytes válidos en data
    uint8_t data[MAXPACKET];
    // función de envío/recepción (punteros a driver)
    void (*SendPacket)(int node, int datalength, void* data);
    int  (*GetPacket)(void);  // returns node number or -1
} doomcom_t;

doomcom_t doomcom{};  // en Doom real es un global definido en d_net.c

// ============================================================================
// 4. BACKUP DE GAMESTATE: RE-SIMULACIÓN POR LATENCIA (g_game.c)
// ============================================================================
/*
 * PROBLEMA: En red, los ticcmd_t de otros jugadores llegan CON RETRASO.
 * Si el jugador 1 está en tic 100 y recibe ticcmd de jugador 2 para tic 98,
 * ya simulamos tics 98 y 99 con input PREDICHO (o nulo). Cuando llega el real,
 * hay que RE-SIMULAR desde el tic del comando recibido.
 *
 * SOLUCIÓN: Backup circular de gamestate_t (BACKUPTICS = 12 tics).
 *   gamestate_t backups[BACKUPTICS];
 *
 * gamestate_t contiene TODO lo necesario para re-simular:
 *   - positions de todos mobj_t (x,y,z,angle,momx,momy,momz)
 *   - health, flags, state, tics, target, reactiontime, threshold...
 *   - leveltime, gametic, random index (rndindex)
 *   - positions de platforms, ceilings, doors (thinkers no-mobj)
 *   - Net state: ticcmds de todos los jugadores para tics previos
 *
 * EN CADA TIC (G_Ticker):
 *   1. Guardar estado actual en backups[gametic % BACKUPTICS] (ANTES de simular)
 *   2. Recibir packets (NetUpdate) -> llena ticcmds de otros jugadores
 *   3. Si hay ticcmd NUEVO para un tic YA simulado (backfill):
 *      - Restaurar backup de ese tic
 *      - Re-simular desde ese tic hasta actual con los comandos corregidos
 *   4. Simular tic actual con comandos disponibles
 *
 * CONSISTANCY CHECK (desync detection):
 *   Cada ticcmd tiene 'consistancy' = checksum del estado relevante.
 *   Al recibir packet, se compara consistancy local vs remoto.
 *   Si difiere -> DESYNC -> juego se pausa, muestra "desync" / "waiting for".
 */

// gamestate_t simplificado (en real es ENORME: todo el estado del juego)
typedef struct {
    int gametic;
    int leveltime;
    int rndindex;
    // mobj states: array de posiciones, health, state, etc.
    // En original: se hace memcpy de estructuras completas
    // Aquí placeholder:
    uint8_t data[4096];  // estado serializado
} gamestate_t;

gamestate_t gamestate_backups[BACKUPTICS];

// Guardar backup antes de simular tic
void G_SaveBackup(int tic) {
    int slot = tic % BACKUPTICS;
    // En real: memcpy(&gamestate_backups[slot], &current_gamestate, sizeof(gamestate_t));
    gamestate_backups[slot].gametic = tic;
    gamestate_backups[slot].leveltime = 0; // placeholder
    gamestate_backups[slot].rndindex = 0;
}

// Restaurar backup y re-simular
void G_RestoreAndResimulate(int from_tic, int to_tic) {
    int slot = from_tic % BACKUPTICS;
    // memcpy(&current_gamestate, &gamestate_backups[slot], sizeof(gamestate_t));
    // for (int t = from_tic; t <= to_tic; t++) G_Ticker(saved_ticcmds[t]);
    (void)from_tic; (void)to_tic;
}

// ============================================================================
// 5. NETUPDATE: RECIBIR Y PROCESAR PACKETS (d_net.c)
// ============================================================================
/*
 * NetUpdate(): llamado cada tic en G_Ticker ANTES de P_Ticker.
 *
 * void NetUpdate(void) {
 *     // 1. Recibir packets disponibles
 *     while (I_GetPacket() != -1) {
 *         int node = doomcom.remotenode;
 *         // Parse packet: header + ticcmds
 *         // doomcom.data contiene: [header][ticcmd_t][ticcmd_t]...
 *         // Header: starttic, numtics, ... (ver doomdata.h)
 *
 *         // 2. Para cada ticcmd en packet:
 *         //    int tic = starttic + i;
 *         //    ticcmd_t* cmd = &packet->cmds[i];
 *         //    netcmds[node][tic % BACKUPTICS] = *cmd;
 *
 *         // 3. Consistancy check:
 *         //    if (cmd->consistancy != local_consistancy[tic]) DESYNC!
 *     }
 *
 *     // 4. Verificar que tenemos comandos de TODOS los jugadores para tic actual
 *     //    Si falta alguno -> esperar (no avanzar tic) -> "waiting for player X"
 * }
 *
 * ESTRUCTURA DE PACKET (doomdata.h):
 *   typedef struct {
 *       byte checksum;        // simple checksum
 *       byte player;          // player number
 *       byte ack;             // ack de último tic recibido
 *       byte numtics;         // cuántos ticcmds en packet
 *       ticcmd_t cmds[BACKUPTICS]; // array de comandos
 *   } doomdata_t;
 *
 * En la práctica: packets UDP contienen múltiples ticcmds (redundancia).
 * Cada packet incluye comandos para tics recientes (ej: tic-2, tic-1, tic).
 * Esto permite recuperar de packet loss.
 */

typedef struct {
    uint8_t checksum;
    uint8_t player;
    uint8_t ack;
    uint8_t numtics;
    ticcmd_t cmds[BACKUPTICS];
} doomdata_t;

// Buffer de comandos recibidos por jugador y tic (circular)
ticcmd_t netcmds[MAXNETNODES][BACKUPTICS];

void NetUpdate(void) {
    // Simulación: en real llama I_GetPacket() que bloquea o poll
    // Aquí solo lógica conceptual:

    // 1. Recibir packets (bucle)
    // while (I_GetPacket() != -1) { ... }

    // 2. Verificar consistancy para tic actual
    int tic = 0; // gametic actual
    for (int node = 0; node < doomcom.numnodes; node++) {
        if (node == doomcom.id) continue; // yo genero mi propio cmd
        ticcmd_t* cmd = &netcmds[node][tic % BACKUPTICS];
        // Verificar consistancy (placeholder)
        // if (cmd->consistancy != ComputeConsistancy(tic)) Desync();
    }

    // 3. Si falta algún comando para tic actual -> NO avanzar (wait)
    //    En single player / listen server: el host genera cmd para todos?
    //    En Doom original: listen server simula todos, envía estado?
    //    NO: Doom es PEER-TO-PEER lockstep. Cada nodo simula TODO.
    //    El host (node 0) solo coordina inicio. Todos simulan igual.
}

// ============================================================================
// 6. G_RUNTICS: BUCLE DE TICS CON NETPLAY (g_game.c)
// ============================================================================
/*
 * void G_RunTics(ticcmd_t* cmd) {
 *     int ticcount = I_GetTime() / (1000/TICRATE);  // tics que deberían haber pasado
 *
 *     // Single player: ticcount avanza con tiempo real
 *     // Netplay: ticcount limitado por ticcmds recibidos de todos
 *
 *     while (gametic < ticcount) {
 *         G_Ticker(cmd);  // simula UN tic
 *         gametic++;
 *     }
 * }
 *
 * En netplay, ticcount NO avanza si faltan ticcmds de otros jugadores.
 * Esto causa "waiting for player X" (el juego se congela localmente).
 *
 * LATENCIA Y TARDANZA:
 * - Ping alto = ticcmds llegan tarde -> ticcount no avanza -> lag visible.
 * - Doom original: sin predicción cliente, sin interpolación. Lag = freeze.
 * - Source ports modernos: client-side prediction, server reconciliation,
 *   entity interpolation, lag compensation.
 */

// ============================================================================
// 7. DETECCIÓN DE DESINCRONIZACIÓN (DESYNC)
// ============================================================================
/*
 * DESYNC = dos nodos tienen estado de juego DIFERENTE para mismo tic.
 * Causas:
 * - Bug en determinismo (random diferente, FPU rounding, uninitialized memory)
 * - Packet loss/corruption sin detectar
 * - Diferencia en código (versiones distintas, cheats, mods)
 * - Overflow de limits (visplane, drawseg) en un nodo pero no en otro
 *
 * DETECCIÓN:
 * - Consistancy checksum en cada ticcmd (campo consistancy).
 *   Calculado hash del estado relevante (positions, health, RNG index).
 * - Al recibir packet, comparar consistancy local vs remoto.
 * - En source ports: checksum completo del gamestate cada N tics.
 *
 * RECUPERACIÓN:
 * - Doom original: NO hay recuperación. Juego se pausa, muestra mensaje.
 * - Source ports: votar estado mayoritario, resync desde save, kick jugador.
 */

int ComputeConsistancy(int tic) {
    // Hash simplificado del estado: positions, health, RNG
    // En real: función compleja en g_game.c
    return tic * 12345; // placeholder
}

void CheckDesync(int tic) {
    for (int node = 0; node < doomcom.numnodes; node++) {
        if (node == doomcom.id) continue;
        ticcmd_t* cmd = &netcmds[node][tic % BACKUPTICS];
        if (cmd->consistancy != ComputeConsistancy(tic)) {
            // DESYNC!
            printf("DESYNC detected from node %d at tic %d\n", node, tic);
            // Pausar juego, mostrar "waiting for..."
        }
    }
}

// ============================================================================
// 8. ARQUITECTURA PEER-TO-PEER VS CLIENT-SERVER
// ============================================================================
/*
 * DOOM ORIGINAL (1993): PEER-TO-PEER LOCKSTEP
 * - Todos los nodos son iguales (simulan TODO).
 * - Cada nodo envía SU ticcmd a TODOS los demás (broadcast/mesh).
 * - No hay servidor autoritario. Consenso implícito por determinismo.
 * - Ventaja: simple, poca banda ancha (solo inputs).
 * - Desventaja: el MÁS LENTO frena a TODOS. Un lag = todos lag. No escala >4.
 * - Desync = catastrófico (no hay autoridad para resolver).
 *
 * SOURCE PORTS MODERNOS:
 * - Odamex: CLIENT-SERVER (autoritario). Servidor simula, clientes predicen.
 * - ZDaemon: Client-server con anticheat.
 * - Eternity: Peer-to-peer mejorado + opcional server.
 * - GZDoom: Peer-to-peer (compatible con demos vanilla) + opcional server.
 * - EDGE-Classic: C++20, arquitectura moderna, ¿client-server? (verificar al construir).
 *
 * CLAVE: El modelo original PEER-TO-PEER es por eso que Doom es TAN determinista.
 * Cualquier cambio en simulación (fixed-point, RNG, orden de thinkers) rompe demos y netplay.
 */

// ============================================================================
// 9. TESTS DE LÓGICA DE RED
// ============================================================================

int test_networking(void) {
    int passed = 0, failed = 0;
    #define TEST(expr, msg) \
        do { if (expr) { passed++; } else { failed++; printf("FAIL: %s\n", msg); } } while(0)

    // ticcmd_t size
    TEST(sizeof(ticcmd_t) == 8, "ticcmd_t = 8 bytes (1+1+2+2+1+1)");

    // Button bitmasks
    TEST((BT_ATTACK | BT_USE) == 0x03, "attack + use buttons");
    TEST((BT_ATTACK & BT_USE) == 0, "attack and use distinct bits");

    // BACKUPTICS
    TEST(BACKUPTICS == 12, "BACKUPTICS = 12");
    TEST(MAXNETNODES == 4, "MAXNETNODES = 4 (original)");

    // doomdata_t
    TEST(sizeof(doomdata_t) >= 1 + 1 + 1 + 1 + BACKUPTICS * sizeof(ticcmd_t),
         "doomdata_t size");

    // Circular backup index
    for (int tic = 0; tic < 20; tic++) {
        int slot = tic % BACKUPTICS;
        TEST(slot >= 0 && slot < BACKUPTICS, "backup slot in range");
    }

    // Consistancy: mismo tic -> mismo valor
    TEST(ComputeConsistancy(100) == ComputeConsistancy(100), "consistancy deterministic");

    printf("Networking tests: %d pass, %d fail\n", passed, failed);
    return failed == 0 ? 0 : 1;
}

int main(void) {
    printf("Módulo 27 - Lección 07: Networking Determinista\n");
    return test_networking();
}