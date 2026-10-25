/*
 * MÓDULO 27: Doom Clásico — De C a C++
 * SOLUCIÓN 07: Networking — ticcmd_t, DOOMCOM, Backup Gamestate, Consistancy
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
// TICCMD_T
// ============================================================================

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
// DOOMCOM
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
// GAMESTATE BACKUP
// ============================================================================

typedef struct {
    int gametic;
    int leveltime;
    int rndindex;
    uint8_t data[4096];
} gamestate_t;

gamestate_t gamestate_backups[BACKUPTICS];

void G_SaveBackup(int tic) {
    int slot = tic % BACKUPTICS;
    gamestate_backups[slot].gametic = tic;
    gamestate_backups[slot].leveltime = 0;
    gamestate_backups[slot].rndindex = 0;
}

void G_RestoreAndResimulate(int from_tic, int to_tic) {
    (void)from_tic; (void)to_tic;
}

// ============================================================================
// PACKET
// ============================================================================

typedef struct {
    uint8_t checksum;
    uint8_t player;
    uint8_t ack;
    uint8_t numtics;
    ticcmd_t cmds[BACKUPTICS];
} doomdata_t;

// ============================================================================
// NETCMDS
// ============================================================================

ticcmd_t netcmds[MAXNETNODES][BACKUPTICS];

// ============================================================================
// CONSISTANCY
// ============================================================================

int ComputeConsistancy(int tic) {
    return tic * 12345;
}

void CheckDesync(int tic) {
    for (int node = 0; node < doomcom.numnodes; node++) {
        if (node == doomcom.id) continue;
        ticcmd_t* cmd = &netcmds[node][tic % BACKUPTICS];
        if (cmd->consistancy != ComputeConsistancy(tic)) {
            // DESYNC
        }
    }
}

// ============================================================================
// TESTS
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