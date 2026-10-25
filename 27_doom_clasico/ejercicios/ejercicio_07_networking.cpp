/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * EJERCICIO 07: Networking — ticcmd_t, DOOMCOM, Backup Gamestate, Consistancy
 *
 * BASADO EN: Lección 07 (networking)
 *
 * OBJETIVO: Implementar estructuras de red: ticcmd_t, doomcom, backup circular.
 */

#include <stdint.h>
#include "../../include/test_framework.hpp"

// ============================================================================
// CONSTANTES
// ============================================================================

#define TICRATE 35
#define MAXNETNODES 4
#define BACKUPTICS 12
#define MAXPACKET 1024

// ============================================================================
// TICCMD_T (doomdata.h)
// ============================================================================

// TODO: ticcmd_t
typedef struct {
    int8_t forwardmove;
    int8_t sidemove;
    int16_t angleturn;
    int16_t consistancy;
    uint8_t buttons;
    uint8_t lookfly;
} ticcmd_t;

#define BT_ATTACK 0x01
#define BT_USE 0x02
#define BT_STRAFE 0x04
#define BT_SPEED 0x08

// ============================================================================
// DOOMCOM (doomcom.h)
// ============================================================================

typedef struct {
    int id;
    int numnodes;
    int drontic;
    int remotenode;
    int datalength;
    uint8_t data[MAXPACKET];
} doomcom_t;

doomcom_t doomcom = {0};

// ============================================================================
// GAMESTATE BACKUP (g_game.c)
// ============================================================================

typedef struct {
    int gametic;
    int leveltime;
    int rndindex;
    uint8_t data[4096];
} gamestate_t;

gamestate_t gamestate_backups[BACKUPTICS];

void G_SaveBackup(int tic) {}
void G_RestoreAndResimulate(int from_tic, int to_tic) {}

// ============================================================================
// PACKET (doomdata.h)
// ============================================================================

typedef struct {
    uint8_t checksum;
    uint8_t player;
    uint8_t ack;
    uint8_t numtics;
    ticcmd_t cmds[BACKUPTICS];
} doomdata_t;

// ============================================================================
// CONSISTANCY CHECK
// ============================================================================

int ComputeConsistancy(int tic) {
    // TODO: calcular consistencia para sincronización (tic * 12345)
    (void)tic;
    return 0;
}
void CheckDesync(int tic) { (void)tic; }

// ============================================================================
// NETCMDS BUFFER
// ============================================================================

ticcmd_t netcmds[4][12];

// ============================================================================
// TESTS (NO MODIFICAR)
// ============================================================================

TEST_CASE(TestTiccmdT) {
    ASSERT_EQ(sizeof(ticcmd_t), 8);
    ASSERT_EQ((BT_ATTACK|BT_USE), 0x03);
    ASSERT_EQ((BT_ATTACK&BT_USE), 0);
}

TEST_CASE(TestConstants) {
    ASSERT_EQ(MAXNETNODES, 4);
    ASSERT_EQ(BACKUPTICS, 12);
    ASSERT_EQ(MAXPACKET, 1024);
}

TEST_CASE(TestDoomdataT) {
    ASSERT_TRUE(sizeof(doomdata_t) >= 4 + BACKUPTICS * sizeof(ticcmd_t));
}

TEST_CASE(TestBackupCircular) {
    for (int tic = 0; tic < 20; tic++) {
        int slot = tic % BACKUPTICS;
        ASSERT_TRUE(slot >= 0 && slot < BACKUPTICS);
    }
}

TEST_CASE(TestConsistancy) {
    ASSERT_EQ(ComputeConsistancy(100), 1234500);
}

int main() {
    return RUN_ALL_TESTS();
}