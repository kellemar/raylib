#include "minunit.h"
#include <string.h>
#include <math.h>
#include <stdbool.h>

// Mock raylib types
typedef struct Vector2 {
    float x;
    float y;
} Vector2;

typedef struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} Color;

// Mock screen constants
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

// Co-op constants (must match coop.h)
#define MAX_COOP_PLAYERS 2
#define REVIVE_RANGE 80.0f
#define REVIVE_TIME 3.0f
#define COOP_SPAWN_MULTIPLIER 1.75f
#define COOP_HEALTH_MULTIPLIER 1.3f
#define COOP_BOSS_HEALTH_MULTIPLIER 1.5f
#define REVIVE_GRACE_PERIOD 0.5f
#define VIEWPORT_WIDTH (SCREEN_WIDTH / 2)
#define VIEWPORT_HEIGHT SCREEN_HEIGHT

// Game mode enum
typedef enum GameMode {
    GAME_MODE_SOLO,
    GAME_MODE_COOP
} GameMode;

typedef enum InputDevice {
    INPUT_KEYBOARD_P1,
    INPUT_KEYBOARD_P2,
    INPUT_GAMEPAD_0,
    INPUT_GAMEPAD_1
} InputDevice;

// Mock weapon structure
typedef struct Weapon {
    float fireRate;
    float cooldown;
} Weapon;

// Mock player structure (simplified for testing)
typedef struct Player {
    Vector2 pos;
    Vector2 vel;
    float speed;
    float health;
    float maxHealth;
    float armor;
    float radius;
    int level;
    int xp;
    int xpToNextLevel;
    float xpMultiplier;
    bool alive;
    float invincibilityTimer;
    Weapon weapon;
} Player;

// Revive state
typedef struct ReviveState {
    bool needsRevive;
    float reviveProgress;
    Vector2 deathPos;
    int reviveCount;
} ReviveState;

// Co-op player wrapper
typedef struct CoopPlayer {
    Player player;
    ReviveState revive;
    InputDevice inputDevice;
    int playerIndex;
} CoopPlayer;

// Mock camera structure (no actual rendering needed for tests)
typedef struct CoopCamera {
    Vector2 target;
    Vector2 offset;
    int viewportId;
} CoopCamera;

// Co-op state
typedef struct CoopState {
    CoopPlayer players[MAX_COOP_PLAYERS];
    int playerCount;
    int sharedXP;
    int sharedLevel;
    int sharedXPToNextLevel;
    int upgradeSelector;
    bool levelUpPending;
    CoopCamera cameras[MAX_COOP_PLAYERS];
    float graceTimer;
    bool bothDead;
} CoopState;

// XP formula (must match coop.c)
static int GetCoopXPForLevel(int level)
{
    return 10 * level * level;
}

// Vector math utilities
static float Vector2Distance(Vector2 a, Vector2 b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}

static float Vector2DistanceSqr(Vector2 a, Vector2 b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

// ============================================================================
// Implementation functions to test (matching coop.c logic)
// ============================================================================

static void CoopStateInit(CoopState *coop, GameMode mode)
{
    memset(coop, 0, sizeof(CoopState));

    coop->playerCount = (mode == GAME_MODE_COOP) ? 2 : 1;
    coop->sharedXP = 0;
    coop->sharedLevel = 1;
    coop->sharedXPToNextLevel = GetCoopXPForLevel(1);
    coop->upgradeSelector = 0;
    coop->levelUpPending = false;
    coop->graceTimer = 0.0f;
    coop->bothDead = false;

    for (int i = 0; i < MAX_COOP_PLAYERS; i++)
    {
        coop->players[i].playerIndex = i;
        coop->players[i].revive.needsRevive = false;
        coop->players[i].revive.reviveProgress = 0.0f;
        coop->players[i].revive.deathPos = (Vector2){ 0.0f, 0.0f };
        coop->players[i].revive.reviveCount = 0;
    }

    coop->players[0].inputDevice = INPUT_KEYBOARD_P1;
    if (mode == GAME_MODE_COOP)
    {
        coop->players[1].inputDevice = INPUT_KEYBOARD_P2;
    }
}

static void CoopInitPlayerStats(CoopState *coop)
{
    // Initialize P1
    coop->players[0].player.pos = (Vector2){ SCREEN_WIDTH / 2.0f - 100.0f, SCREEN_HEIGHT / 2.0f };
    coop->players[0].player.health = 100.0f;
    coop->players[0].player.maxHealth = 100.0f;
    coop->players[0].player.alive = true;
    coop->players[0].player.xpMultiplier = 1.0f;
    coop->players[0].revive.needsRevive = false;
    coop->players[0].revive.reviveProgress = 0.0f;
    coop->players[0].revive.reviveCount = 0;

    if (coop->playerCount == 2)
    {
        // Initialize P2
        coop->players[1].player.pos = (Vector2){ SCREEN_WIDTH / 2.0f + 100.0f, SCREEN_HEIGHT / 2.0f };
        coop->players[1].player.health = 100.0f;
        coop->players[1].player.maxHealth = 100.0f;
        coop->players[1].player.alive = true;
        coop->players[1].player.xpMultiplier = 1.0f;
        coop->players[1].revive.needsRevive = false;
        coop->players[1].revive.reviveProgress = 0.0f;
        coop->players[1].revive.reviveCount = 0;
    }

    // Reset shared progression
    coop->sharedXP = 0;
    coop->sharedLevel = 1;
    coop->sharedXPToNextLevel = GetCoopXPForLevel(1);
    coop->upgradeSelector = 0;
    coop->levelUpPending = false;
    coop->graceTimer = 0.0f;
    coop->bothDead = false;
}

static Player* CoopGetPlayer(CoopState *coop, int index)
{
    if (index < 0 || index >= coop->playerCount) return NULL;
    return &coop->players[index].player;
}

static int CoopGetAlivePlayerCount(CoopState *coop)
{
    int count = 0;
    for (int i = 0; i < coop->playerCount; i++)
    {
        if (coop->players[i].player.alive && !coop->players[i].revive.needsRevive)
        {
            count++;
        }
    }
    return count;
}

static bool CoopIsPlayerAlive(CoopState *coop, int index)
{
    if (index < 0 || index >= coop->playerCount) return false;
    return coop->players[index].player.alive && !coop->players[index].revive.needsRevive;
}

static bool CoopCheckReviveProximity(CoopState *coop, int aliveIndex, int deadIndex)
{
    if (aliveIndex < 0 || aliveIndex >= coop->playerCount) return false;
    if (deadIndex < 0 || deadIndex >= coop->playerCount) return false;

    float dist = Vector2Distance(
        coop->players[aliveIndex].player.pos,
        coop->players[deadIndex].revive.deathPos
    );

    return dist <= REVIVE_RANGE;
}

static void CoopRespawnPlayer(CoopState *coop, int playerIndex)
{
    if (playerIndex < 0 || playerIndex >= coop->playerCount) return;

    CoopPlayer *cp = &coop->players[playerIndex];
    Player *player = &cp->player;

    float baseHpPercent = 0.5f - (float)cp->revive.reviveCount * 0.1f;
    if (baseHpPercent < 0.25f) baseHpPercent = 0.25f;

    player->health = player->maxHealth * baseHpPercent;
    player->alive = true;
    player->pos = cp->revive.deathPos;
    player->invincibilityTimer = 2.0f;

    cp->revive.needsRevive = false;
    cp->revive.reviveProgress = 0.0f;
    cp->revive.reviveCount++;
}

static bool CoopCheckTotalPartyKill(CoopState *coop, float dt)
{
    if (coop->playerCount < 2) return false;

    bool allDead = true;
    for (int i = 0; i < coop->playerCount; i++)
    {
        if (!coop->players[i].revive.needsRevive)
        {
            allDead = false;
            break;
        }
    }

    if (allDead)
    {
        if (!coop->bothDead)
        {
            coop->bothDead = true;
            coop->graceTimer = REVIVE_GRACE_PERIOD;
        }
        else
        {
            coop->graceTimer -= dt;
            if (coop->graceTimer <= 0.0f)
            {
                return true;
            }
        }
    }
    else
    {
        coop->bothDead = false;
        coop->graceTimer = 0.0f;
    }

    return false;
}

static void CoopAddXP(CoopState *coop, int amount)
{
    float mult = 1.0f;
    for (int i = 0; i < coop->playerCount; i++)
    {
        if (!coop->players[i].revive.needsRevive)
        {
            mult = coop->players[i].player.xpMultiplier;
            break;
        }
    }

    coop->sharedXP += (int)(amount * mult);

    for (int i = 0; i < coop->playerCount; i++)
    {
        coop->players[i].player.xp = coop->sharedXP;
        coop->players[i].player.xpToNextLevel = coop->sharedXPToNextLevel;
        coop->players[i].player.level = coop->sharedLevel;
    }
}

static bool CoopCheckLevelUp(CoopState *coop)
{
    if (coop->sharedXP >= coop->sharedXPToNextLevel)
    {
        coop->sharedLevel++;
        coop->sharedXPToNextLevel = GetCoopXPForLevel(coop->sharedLevel);
        coop->levelUpPending = true;

        for (int i = 0; i < coop->playerCount; i++)
        {
            coop->players[i].player.level = coop->sharedLevel;
            coop->players[i].player.xpToNextLevel = coop->sharedXPToNextLevel;
        }

        return true;
    }
    return false;
}

static void CoopApplyUpgrade(CoopState *coop)
{
    // Alternate selector for next upgrade
    coop->upgradeSelector = (coop->upgradeSelector + 1) % coop->playerCount;
    coop->levelUpPending = false;
}

static float CoopGetSpawnMultiplier(CoopState *coop)
{
    return (coop->playerCount == 2) ? COOP_SPAWN_MULTIPLIER : 1.0f;
}

static float CoopGetHealthMultiplier(CoopState *coop)
{
    return (coop->playerCount == 2) ? COOP_HEALTH_MULTIPLIER : 1.0f;
}

static float CoopGetBossHealthMultiplier(CoopState *coop)
{
    return (coop->playerCount == 2) ? COOP_BOSS_HEALTH_MULTIPLIER : 1.0f;
}

static Vector2 CoopGetNearestPlayerPos(CoopState *coop, Vector2 fromPos)
{
    float minDist = 999999.0f;
    Vector2 nearestPos = coop->players[0].player.pos;

    for (int i = 0; i < coop->playerCount; i++)
    {
        if (coop->players[i].revive.needsRevive) continue;

        float dist = Vector2DistanceSqr(fromPos, coop->players[i].player.pos);
        if (dist < minDist)
        {
            minDist = dist;
            nearestPos = coop->players[i].player.pos;
        }
    }

    return nearestPos;
}

static int CoopGetNearestPlayerIndex(CoopState *coop, Vector2 fromPos)
{
    float minDist = 999999.0f;
    int nearestIndex = 0;

    for (int i = 0; i < coop->playerCount; i++)
    {
        if (coop->players[i].revive.needsRevive) continue;

        float dist = Vector2DistanceSqr(fromPos, coop->players[i].player.pos);
        if (dist < minDist)
        {
            minDist = dist;
            nearestIndex = i;
        }
    }

    return nearestIndex;
}

// ============================================================================
// Unit Tests
// ============================================================================

// --- Initialization Tests ---

static const char* test_coop_state_init_solo(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_SOLO);

    mu_assert_int_eq(1, coop.playerCount);
    mu_assert_int_eq(0, coop.sharedXP);
    mu_assert_int_eq(1, coop.sharedLevel);
    mu_assert_int_eq(10, coop.sharedXPToNextLevel);  // 10 * 1 * 1 = 10
    mu_assert_int_eq(0, coop.upgradeSelector);
    mu_assert_false(coop.levelUpPending);
    mu_assert_false(coop.bothDead);
    mu_assert_float_eq(0.0f, coop.graceTimer);

    return 0;
}

static const char* test_coop_state_init_coop(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);

    mu_assert_int_eq(2, coop.playerCount);
    mu_assert_int_eq(0, coop.sharedXP);
    mu_assert_int_eq(1, coop.sharedLevel);
    mu_assert_int_eq(INPUT_KEYBOARD_P1, coop.players[0].inputDevice);
    mu_assert_int_eq(INPUT_KEYBOARD_P2, coop.players[1].inputDevice);
    mu_assert_int_eq(0, coop.players[0].playerIndex);
    mu_assert_int_eq(1, coop.players[1].playerIndex);

    return 0;
}

static const char* test_coop_init_players(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    // P1 should be on left
    mu_assert(coop.players[0].player.pos.x < SCREEN_WIDTH / 2.0f,
              "P1 should be on left side of screen");

    // P2 should be on right
    mu_assert(coop.players[1].player.pos.x > SCREEN_WIDTH / 2.0f,
              "P2 should be on right side of screen");

    // Both should be alive
    mu_assert_true(coop.players[0].player.alive);
    mu_assert_true(coop.players[1].player.alive);

    // Both should have full health
    mu_assert_float_eq(100.0f, coop.players[0].player.health);
    mu_assert_float_eq(100.0f, coop.players[1].player.health);

    return 0;
}

// --- Player Access Tests ---

static const char* test_coop_get_player_valid(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    Player *p1 = CoopGetPlayer(&coop, 0);
    Player *p2 = CoopGetPlayer(&coop, 1);

    mu_assert(p1 != NULL, "P1 should not be NULL");
    mu_assert(p2 != NULL, "P2 should not be NULL");
    mu_assert(p1 != p2, "P1 and P2 should be different");

    return 0;
}

static const char* test_coop_get_player_invalid(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_SOLO);

    Player *invalid = CoopGetPlayer(&coop, 1);  // Solo only has P1
    mu_assert(invalid == NULL, "Invalid index should return NULL");

    invalid = CoopGetPlayer(&coop, -1);
    mu_assert(invalid == NULL, "Negative index should return NULL");

    invalid = CoopGetPlayer(&coop, 100);
    mu_assert(invalid == NULL, "Out of range index should return NULL");

    return 0;
}

// --- Alive Player Tests ---

static const char* test_coop_alive_count_all_alive(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    mu_assert_int_eq(2, CoopGetAlivePlayerCount(&coop));
    mu_assert_true(CoopIsPlayerAlive(&coop, 0));
    mu_assert_true(CoopIsPlayerAlive(&coop, 1));

    return 0;
}

static const char* test_coop_alive_count_one_dead(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    // Kill P2
    coop.players[1].revive.needsRevive = true;

    mu_assert_int_eq(1, CoopGetAlivePlayerCount(&coop));
    mu_assert_true(CoopIsPlayerAlive(&coop, 0));
    mu_assert_false(CoopIsPlayerAlive(&coop, 1));

    return 0;
}

static const char* test_coop_alive_count_both_dead(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    // Kill both
    coop.players[0].revive.needsRevive = true;
    coop.players[1].revive.needsRevive = true;

    mu_assert_int_eq(0, CoopGetAlivePlayerCount(&coop));
    mu_assert_false(CoopIsPlayerAlive(&coop, 0));
    mu_assert_false(CoopIsPlayerAlive(&coop, 1));

    return 0;
}

static const char* test_coop_is_player_alive_invalid_index(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    mu_assert_false(CoopIsPlayerAlive(&coop, -1));
    mu_assert_false(CoopIsPlayerAlive(&coop, 100));

    return 0;
}

// --- Revive Mechanics Tests ---

static const char* test_coop_revive_proximity_in_range(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    // P2 dies
    coop.players[1].revive.needsRevive = true;
    coop.players[1].revive.deathPos = coop.players[1].player.pos;

    // Move P1 close to P2's death position (within REVIVE_RANGE = 80)
    coop.players[0].player.pos = coop.players[1].revive.deathPos;
    coop.players[0].player.pos.x += 50.0f;  // 50 < 80, should be in range

    mu_assert_true(CoopCheckReviveProximity(&coop, 0, 1));

    return 0;
}

static const char* test_coop_revive_proximity_out_of_range(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    // P2 dies at their position
    coop.players[1].revive.needsRevive = true;
    coop.players[1].revive.deathPos = coop.players[1].player.pos;

    // P1 is far away (100 > 80)
    coop.players[0].player.pos = coop.players[1].revive.deathPos;
    coop.players[0].player.pos.x += 100.0f;

    mu_assert_false(CoopCheckReviveProximity(&coop, 0, 1));

    return 0;
}

static const char* test_coop_respawn_player_hp(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    // P2 dies
    coop.players[1].player.alive = false;
    coop.players[1].revive.needsRevive = true;
    coop.players[1].revive.deathPos = (Vector2){ 500.0f, 300.0f };
    coop.players[1].revive.reviveCount = 0;

    // First respawn: 50% HP
    CoopRespawnPlayer(&coop, 1);

    mu_assert_true(coop.players[1].player.alive);
    mu_assert_float_eq(50.0f, coop.players[1].player.health);  // 50% of 100
    mu_assert_int_eq(1, coop.players[1].revive.reviveCount);
    mu_assert_false(coop.players[1].revive.needsRevive);

    return 0;
}

static const char* test_coop_respawn_diminishing_hp(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    // Set revive count to 2 (already revived twice)
    coop.players[1].revive.reviveCount = 2;
    coop.players[1].revive.needsRevive = true;
    coop.players[1].player.alive = false;
    coop.players[1].revive.deathPos = (Vector2){ 500.0f, 300.0f };

    // Third respawn: 50% - 20% = 30% HP
    CoopRespawnPlayer(&coop, 1);

    mu_assert_float_eq(30.0f, coop.players[1].player.health);
    mu_assert_int_eq(3, coop.players[1].revive.reviveCount);

    return 0;
}

static const char* test_coop_respawn_minimum_hp(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    // Set revive count high (already revived many times)
    coop.players[1].revive.reviveCount = 10;
    coop.players[1].revive.needsRevive = true;
    coop.players[1].player.alive = false;
    coop.players[1].revive.deathPos = (Vector2){ 500.0f, 300.0f };

    // HP should be capped at minimum 25%
    CoopRespawnPlayer(&coop, 1);

    mu_assert_float_eq(25.0f, coop.players[1].player.health);

    return 0;
}

static const char* test_coop_respawn_invincibility(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    coop.players[1].revive.needsRevive = true;
    coop.players[1].player.alive = false;
    coop.players[1].revive.deathPos = (Vector2){ 500.0f, 300.0f };

    CoopRespawnPlayer(&coop, 1);

    mu_assert_float_eq(2.0f, coop.players[1].player.invincibilityTimer);

    return 0;
}

// --- Total Party Kill Tests ---

static const char* test_coop_tpk_not_triggered_one_alive(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    // Only P2 dead
    coop.players[1].revive.needsRevive = true;

    mu_assert_false(CoopCheckTotalPartyKill(&coop, 0.1f));
    mu_assert_false(coop.bothDead);

    return 0;
}

static const char* test_coop_tpk_grace_period(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    // Both dead
    coop.players[0].revive.needsRevive = true;
    coop.players[1].revive.needsRevive = true;

    // First check starts grace timer
    mu_assert_false(CoopCheckTotalPartyKill(&coop, 0.0f));
    mu_assert_true(coop.bothDead);
    mu_assert_float_eq(REVIVE_GRACE_PERIOD, coop.graceTimer);

    // Small tick shouldn't trigger TPK
    mu_assert_false(CoopCheckTotalPartyKill(&coop, 0.1f));

    // After grace period expires, TPK triggers
    mu_assert_true(CoopCheckTotalPartyKill(&coop, 1.0f));

    return 0;
}

static const char* test_coop_tpk_reset_on_revive(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    // Both dead
    coop.players[0].revive.needsRevive = true;
    coop.players[1].revive.needsRevive = true;
    CoopCheckTotalPartyKill(&coop, 0.0f);

    mu_assert_true(coop.bothDead);

    // P1 revived
    coop.players[0].revive.needsRevive = false;
    CoopCheckTotalPartyKill(&coop, 0.0f);

    mu_assert_false(coop.bothDead);
    mu_assert_float_eq(0.0f, coop.graceTimer);

    return 0;
}

static const char* test_coop_tpk_solo_mode(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_SOLO);
    CoopInitPlayerStats(&coop);

    // Solo mode should never TPK through this function
    coop.players[0].revive.needsRevive = true;

    mu_assert_false(CoopCheckTotalPartyKill(&coop, 1.0f));

    return 0;
}

// --- Shared XP Tests ---

static const char* test_coop_add_xp(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    CoopAddXP(&coop, 5);

    mu_assert_int_eq(5, coop.sharedXP);
    mu_assert_int_eq(5, coop.players[0].player.xp);
    mu_assert_int_eq(5, coop.players[1].player.xp);

    return 0;
}

static const char* test_coop_add_xp_multiplier(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    // Set XP multiplier on P1
    coop.players[0].player.xpMultiplier = 2.0f;

    CoopAddXP(&coop, 10);

    mu_assert_int_eq(20, coop.sharedXP);  // 10 * 2.0 = 20

    return 0;
}

static const char* test_coop_check_level_up(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    // Add enough XP to level up (need 10 for level 1)
    coop.sharedXP = 10;

    mu_assert_true(CoopCheckLevelUp(&coop));
    mu_assert_int_eq(2, coop.sharedLevel);
    mu_assert_int_eq(40, coop.sharedXPToNextLevel);  // 10 * 2 * 2 = 40
    mu_assert_true(coop.levelUpPending);

    // Both players should be synced
    mu_assert_int_eq(2, coop.players[0].player.level);
    mu_assert_int_eq(2, coop.players[1].player.level);

    return 0;
}

static const char* test_coop_no_level_up_insufficient_xp(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    coop.sharedXP = 5;  // Need 10 for level 1

    mu_assert_false(CoopCheckLevelUp(&coop));
    mu_assert_int_eq(1, coop.sharedLevel);
    mu_assert_false(coop.levelUpPending);

    return 0;
}

// --- Upgrade Selection Tests ---

static const char* test_coop_apply_upgrade_alternates(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    mu_assert_int_eq(0, coop.upgradeSelector);

    CoopApplyUpgrade(&coop);
    mu_assert_int_eq(1, coop.upgradeSelector);
    mu_assert_false(coop.levelUpPending);

    CoopApplyUpgrade(&coop);
    mu_assert_int_eq(0, coop.upgradeSelector);  // Wraps back to P1

    return 0;
}

static const char* test_coop_apply_upgrade_solo(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_SOLO);
    CoopInitPlayerStats(&coop);

    mu_assert_int_eq(0, coop.upgradeSelector);

    CoopApplyUpgrade(&coop);
    mu_assert_int_eq(0, coop.upgradeSelector);  // Solo always stays at 0

    return 0;
}

// --- Enemy Scaling Tests ---

static const char* test_coop_spawn_multiplier_solo(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_SOLO);

    mu_assert_float_eq(1.0f, CoopGetSpawnMultiplier(&coop));

    return 0;
}

static const char* test_coop_spawn_multiplier_coop(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);

    mu_assert_float_eq(COOP_SPAWN_MULTIPLIER, CoopGetSpawnMultiplier(&coop));

    return 0;
}

static const char* test_coop_health_multiplier_solo(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_SOLO);

    mu_assert_float_eq(1.0f, CoopGetHealthMultiplier(&coop));

    return 0;
}

static const char* test_coop_health_multiplier_coop(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);

    mu_assert_float_eq(COOP_HEALTH_MULTIPLIER, CoopGetHealthMultiplier(&coop));

    return 0;
}

static const char* test_coop_boss_health_multiplier(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);

    mu_assert_float_eq(COOP_BOSS_HEALTH_MULTIPLIER, CoopGetBossHealthMultiplier(&coop));

    return 0;
}

// --- Nearest Player Tests ---

static const char* test_coop_nearest_player_p1_closer(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    coop.players[0].player.pos = (Vector2){ 100.0f, 100.0f };
    coop.players[1].player.pos = (Vector2){ 500.0f, 500.0f };

    Vector2 enemyPos = (Vector2){ 150.0f, 100.0f };

    Vector2 nearest = CoopGetNearestPlayerPos(&coop, enemyPos);
    mu_assert_float_eq(100.0f, nearest.x);
    mu_assert_float_eq(100.0f, nearest.y);

    int nearestIdx = CoopGetNearestPlayerIndex(&coop, enemyPos);
    mu_assert_int_eq(0, nearestIdx);

    return 0;
}

static const char* test_coop_nearest_player_p2_closer(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    coop.players[0].player.pos = (Vector2){ 100.0f, 100.0f };
    coop.players[1].player.pos = (Vector2){ 500.0f, 500.0f };

    Vector2 enemyPos = (Vector2){ 480.0f, 480.0f };

    Vector2 nearest = CoopGetNearestPlayerPos(&coop, enemyPos);
    mu_assert_float_eq(500.0f, nearest.x);
    mu_assert_float_eq(500.0f, nearest.y);

    int nearestIdx = CoopGetNearestPlayerIndex(&coop, enemyPos);
    mu_assert_int_eq(1, nearestIdx);

    return 0;
}

static const char* test_coop_nearest_player_skips_dead(void)
{
    CoopState coop;
    CoopStateInit(&coop, GAME_MODE_COOP);
    CoopInitPlayerStats(&coop);

    coop.players[0].player.pos = (Vector2){ 100.0f, 100.0f };
    coop.players[1].player.pos = (Vector2){ 500.0f, 500.0f };

    // P1 is closer but dead
    coop.players[0].revive.needsRevive = true;

    Vector2 enemyPos = (Vector2){ 150.0f, 100.0f };

    Vector2 nearest = CoopGetNearestPlayerPos(&coop, enemyPos);
    mu_assert_float_eq(500.0f, nearest.x);  // Should get P2's position

    int nearestIdx = CoopGetNearestPlayerIndex(&coop, enemyPos);
    mu_assert_int_eq(1, nearestIdx);  // Should be P2

    return 0;
}

// --- XP Formula Tests ---

static const char* test_xp_formula_levels(void)
{
    // Level 1: 10 * 1 * 1 = 10
    mu_assert_int_eq(10, GetCoopXPForLevel(1));

    // Level 2: 10 * 2 * 2 = 40
    mu_assert_int_eq(40, GetCoopXPForLevel(2));

    // Level 3: 10 * 3 * 3 = 90
    mu_assert_int_eq(90, GetCoopXPForLevel(3));

    // Level 5: 10 * 5 * 5 = 250
    mu_assert_int_eq(250, GetCoopXPForLevel(5));

    // Level 10: 10 * 10 * 10 = 1000
    mu_assert_int_eq(1000, GetCoopXPForLevel(10));

    return 0;
}

// --- Constants Tests ---

static const char* test_coop_constants(void)
{
    mu_assert_int_eq(2, MAX_COOP_PLAYERS);
    mu_assert_float_eq(80.0f, REVIVE_RANGE);
    mu_assert_float_eq(3.0f, REVIVE_TIME);
    mu_assert_float_eq(1.75f, COOP_SPAWN_MULTIPLIER);
    mu_assert_float_eq(1.3f, COOP_HEALTH_MULTIPLIER);
    mu_assert_float_eq(1.5f, COOP_BOSS_HEALTH_MULTIPLIER);
    mu_assert_float_eq(0.5f, REVIVE_GRACE_PERIOD);
    mu_assert_int_eq(640, VIEWPORT_WIDTH);
    mu_assert_int_eq(720, VIEWPORT_HEIGHT);

    return 0;
}

// ============================================================================
// Test Suite Runner
// ============================================================================

const char* run_coop_tests(void)
{
    // Initialization tests
    mu_run_test(test_coop_state_init_solo);
    mu_run_test(test_coop_state_init_coop);
    mu_run_test(test_coop_init_players);

    // Player access tests
    mu_run_test(test_coop_get_player_valid);
    mu_run_test(test_coop_get_player_invalid);

    // Alive player tests
    mu_run_test(test_coop_alive_count_all_alive);
    mu_run_test(test_coop_alive_count_one_dead);
    mu_run_test(test_coop_alive_count_both_dead);
    mu_run_test(test_coop_is_player_alive_invalid_index);

    // Revive mechanics tests
    mu_run_test(test_coop_revive_proximity_in_range);
    mu_run_test(test_coop_revive_proximity_out_of_range);
    mu_run_test(test_coop_respawn_player_hp);
    mu_run_test(test_coop_respawn_diminishing_hp);
    mu_run_test(test_coop_respawn_minimum_hp);
    mu_run_test(test_coop_respawn_invincibility);

    // Total party kill tests
    mu_run_test(test_coop_tpk_not_triggered_one_alive);
    mu_run_test(test_coop_tpk_grace_period);
    mu_run_test(test_coop_tpk_reset_on_revive);
    mu_run_test(test_coop_tpk_solo_mode);

    // Shared XP tests
    mu_run_test(test_coop_add_xp);
    mu_run_test(test_coop_add_xp_multiplier);
    mu_run_test(test_coop_check_level_up);
    mu_run_test(test_coop_no_level_up_insufficient_xp);

    // Upgrade selection tests
    mu_run_test(test_coop_apply_upgrade_alternates);
    mu_run_test(test_coop_apply_upgrade_solo);

    // Enemy scaling tests
    mu_run_test(test_coop_spawn_multiplier_solo);
    mu_run_test(test_coop_spawn_multiplier_coop);
    mu_run_test(test_coop_health_multiplier_solo);
    mu_run_test(test_coop_health_multiplier_coop);
    mu_run_test(test_coop_boss_health_multiplier);

    // Nearest player tests
    mu_run_test(test_coop_nearest_player_p1_closer);
    mu_run_test(test_coop_nearest_player_p2_closer);
    mu_run_test(test_coop_nearest_player_skips_dead);

    // XP formula tests
    mu_run_test(test_xp_formula_levels);

    // Constants tests
    mu_run_test(test_coop_constants);

    return 0;
}
