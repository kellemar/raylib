#ifndef COOP_H
#define COOP_H

#include "raylib.h"
#include "types.h"
#include "player.h"
#include "upgrade.h"
#include <stdbool.h>

#define MAX_COOP_PLAYERS 2
#define REVIVE_RANGE 80.0f
#define REVIVE_TIME 3.0f
#define COOP_SPAWN_MULTIPLIER 1.75f
#define COOP_HEALTH_MULTIPLIER 1.3f
#define COOP_BOSS_HEALTH_MULTIPLIER 1.5f
#define REVIVE_GRACE_PERIOD 0.5f

// Split-screen viewport dimensions (half screen width)
#define VIEWPORT_WIDTH (SCREEN_WIDTH / 2)
#define VIEWPORT_HEIGHT SCREEN_HEIGHT

typedef enum GameMode {
    GAME_MODE_SOLO,
    GAME_MODE_COOP
} GameMode;

typedef enum InputDevice {
    INPUT_KEYBOARD_P1,      // WASD + Mouse
    INPUT_KEYBOARD_P2,      // Arrow keys + IJKL
    INPUT_GAMEPAD_0,        // First gamepad
    INPUT_GAMEPAD_1         // Second gamepad
} InputDevice;

// Revive state for dead players
typedef struct ReviveState {
    bool needsRevive;       // Player is dead and waiting for revive
    float reviveProgress;   // 0.0 to REVIVE_TIME (3.0 seconds)
    Vector2 deathPos;       // Where player died (ghost location)
    int reviveCount;        // How many times revived (affects respawn HP)
} ReviveState;

// Co-op player wrapper with additional co-op specific state
typedef struct CoopPlayer {
    Player player;
    ReviveState revive;
    InputDevice inputDevice;
    int playerIndex;        // 0 or 1
} CoopPlayer;

// Camera state for each viewport
typedef struct CoopCamera {
    Camera2D cam;
    RenderTexture2D viewport;
    Rectangle sourceRect;   // For drawing (flipped Y for OpenGL)
    Rectangle destRect;     // Where to draw on screen
} CoopCamera;

// Main co-op state structure
typedef struct CoopState {
    CoopPlayer players[MAX_COOP_PLAYERS];
    int playerCount;        // 1 for solo, 2 for co-op

    // Shared progression
    int sharedXP;
    int sharedLevel;
    int sharedXPToNextLevel;

    // Upgrade selection
    int upgradeSelector;    // Which player chooses next upgrade (0 or 1)
    bool levelUpPending;    // Level up waiting to be processed

    // Cameras
    CoopCamera cameras[MAX_COOP_PLAYERS];

    // Death/revive state
    float graceTimer;       // Grace period timer for total party kill
    bool bothDead;          // True when both players are dead
} CoopState;

// Co-op initialization and lifecycle
void CoopStateInit(CoopState *coop, GameMode mode);
void CoopStateCleanup(CoopState *coop);

// Player management
void CoopInitPlayers(CoopState *coop, CharacterType p1Char, CharacterType p2Char);
void CoopResetPlayers(CoopState *coop);
Player* CoopGetPlayer(CoopState *coop, int index);
int CoopGetAlivePlayerCount(CoopState *coop);
bool CoopIsPlayerAlive(CoopState *coop, int index);

// Input handling
void CoopUpdateInput(CoopState *coop, float dt, struct ProjectilePool *projectiles);
void CoopUpdatePlayerInput(CoopPlayer *cp, float dt, struct ProjectilePool *projectiles, Camera2D camera);

// Camera management
void CoopInitCameras(CoopState *coop);
void CoopUpdateCameras(CoopState *coop, float dt);
void CoopCleanupCameras(CoopState *coop);

// Revive mechanics
void CoopUpdateRevive(CoopState *coop, float dt);
bool CoopCheckReviveProximity(CoopState *coop, int aliveIndex, int deadIndex);
void CoopRespawnPlayer(CoopState *coop, int playerIndex);
bool CoopCheckTotalPartyKill(CoopState *coop, float dt);

// Shared XP and leveling
void CoopAddXP(CoopState *coop, int amount);
bool CoopCheckLevelUp(CoopState *coop);
void CoopApplyUpgrade(CoopState *coop, int upgradeType);

// Enemy scaling
float CoopGetSpawnMultiplier(CoopState *coop);
float CoopGetHealthMultiplier(CoopState *coop);
float CoopGetBossHealthMultiplier(CoopState *coop);

// Utility
Vector2 CoopGetNearestPlayerPos(CoopState *coop, Vector2 fromPos);
int CoopGetNearestPlayerIndex(CoopState *coop, Vector2 fromPos);

#endif
