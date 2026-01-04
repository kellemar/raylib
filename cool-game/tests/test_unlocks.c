#include "minunit.h"
#include <string.h>

// Define weapon types for testing (matching weapon.h)
typedef enum WeaponType {
    WEAPON_PULSE_CANNON,
    WEAPON_SPREAD_SHOT,
    WEAPON_HOMING_MISSILE,
    WEAPON_LIGHTNING,
    WEAPON_ORBIT_SHIELD,
    WEAPON_FLAMETHROWER,
    WEAPON_FREEZE_RAY,
    WEAPON_BLACK_HOLE,
    WEAPON_BASE_COUNT
} WeaponType;

// UnlockData structure (matching unlocks.h)
#define UNLOCKS_VERSION 1
#define META_UPGRADE_MAX_LEVEL 5
#define META_UPGRADE_COST_BASE 1000
#define META_UPGRADE_COST_MULT 2

typedef struct UnlockData {
    int version;
    unsigned int unlockedWeapons;
    unsigned int unlockedCharacters;
    int metaSpeed;
    int metaHealth;
    int metaDamage;
    int metaXP;
    int metaMagnet;
    int totalKills;
    int totalBossKills;
    int totalScore;
    int gamesPlayed;
    int highestLevel;
    float longestSurvival;
} UnlockData;

static void UnlocksInit(UnlockData *unlocks)
{
    memset(unlocks, 0, sizeof(UnlockData));
    unlocks->version = UNLOCKS_VERSION;
    unlocks->unlockedWeapons = (1 << WEAPON_PULSE_CANNON);
    unlocks->unlockedCharacters = (1 << 0);
}

static int UnlocksHasWeapon(UnlockData *unlocks, WeaponType weapon)
{
    if (weapon < 0 || weapon >= WEAPON_BASE_COUNT) return 0;
    return (unlocks->unlockedWeapons & (1 << weapon)) != 0;
}

static int UnlocksHasCharacter(UnlockData *unlocks, int characterId)
{
    if (characterId < 0 || characterId >= 8) return 0;
    return (unlocks->unlockedCharacters & (1 << characterId)) != 0;
}

static void UnlocksUnlockWeapon(UnlockData *unlocks, WeaponType weapon)
{
    if (weapon < 0 || weapon >= WEAPON_BASE_COUNT) return;
    unlocks->unlockedWeapons |= (1 << weapon);
}

static void UnlocksUnlockCharacter(UnlockData *unlocks, int characterId)
{
    if (characterId < 0 || characterId >= 8) return;
    unlocks->unlockedCharacters |= (1 << characterId);
}

static void UnlocksAddRunStats(UnlockData *unlocks, int kills, int bossKills,
                               int score, int level, float survivalTime)
{
    unlocks->totalKills += kills;
    unlocks->totalBossKills += bossKills;
    unlocks->totalScore += score;
    unlocks->gamesPlayed++;
    if (level > unlocks->highestLevel) unlocks->highestLevel = level;
    if (survivalTime > unlocks->longestSurvival) unlocks->longestSurvival = survivalTime;
}

static float UnlocksGetSpeedBonus(UnlockData *unlocks)
{
    return 1.0f + (unlocks->metaSpeed * 0.02f);
}

static float UnlocksGetHealthBonus(UnlockData *unlocks)
{
    return (float)(unlocks->metaHealth * 10);
}

static float UnlocksGetDamageBonus(UnlockData *unlocks)
{
    return 1.0f + (unlocks->metaDamage * 0.05f);
}

static int UnlocksGetMetaUpgradeCost(int currentLevel)
{
    if (currentLevel >= META_UPGRADE_MAX_LEVEL) return -1;
    int cost = META_UPGRADE_COST_BASE;
    for (int i = 0; i < currentLevel; i++) cost *= META_UPGRADE_COST_MULT;
    return cost;
}

static int UnlocksCheckNewUnlocks(UnlockData *unlocks)
{
    int newUnlock = 0;

    // Spread Shot: Kill 100 enemies
    if (unlocks->totalKills >= 100 && !UnlocksHasWeapon(unlocks, WEAPON_SPREAD_SHOT))
    {
        UnlocksUnlockWeapon(unlocks, WEAPON_SPREAD_SHOT);
        newUnlock = 1;
    }

    // Lightning: Reach level 10
    if (unlocks->highestLevel >= 10 && !UnlocksHasWeapon(unlocks, WEAPON_LIGHTNING))
    {
        UnlocksUnlockWeapon(unlocks, WEAPON_LIGHTNING);
        newUnlock = 1;
    }

    // Character 2: Play 5 games
    if (unlocks->gamesPlayed >= 5 && !UnlocksHasCharacter(unlocks, 1))
    {
        UnlocksUnlockCharacter(unlocks, 1);
        newUnlock = 1;
    }

    return newUnlock;
}

// Tests

static const char* test_unlocks_init(void)
{
    UnlockData unlocks;
    UnlocksInit(&unlocks);

    mu_assert_int_eq(UNLOCKS_VERSION, unlocks.version);
    mu_assert_true(UnlocksHasWeapon(&unlocks, WEAPON_PULSE_CANNON));
    mu_assert_false(UnlocksHasWeapon(&unlocks, WEAPON_SPREAD_SHOT));
    mu_assert_true(UnlocksHasCharacter(&unlocks, 0));
    mu_assert_false(UnlocksHasCharacter(&unlocks, 1));
    mu_assert_int_eq(0, unlocks.totalKills);
    mu_assert_int_eq(0, unlocks.gamesPlayed);
    return 0;
}

static const char* test_unlocks_weapon_unlock(void)
{
    UnlockData unlocks;
    UnlocksInit(&unlocks);

    mu_assert_false(UnlocksHasWeapon(&unlocks, WEAPON_SPREAD_SHOT));
    UnlocksUnlockWeapon(&unlocks, WEAPON_SPREAD_SHOT);
    mu_assert_true(UnlocksHasWeapon(&unlocks, WEAPON_SPREAD_SHOT));

    // Multiple weapons can be unlocked
    UnlocksUnlockWeapon(&unlocks, WEAPON_LIGHTNING);
    mu_assert_true(UnlocksHasWeapon(&unlocks, WEAPON_PULSE_CANNON));
    mu_assert_true(UnlocksHasWeapon(&unlocks, WEAPON_SPREAD_SHOT));
    mu_assert_true(UnlocksHasWeapon(&unlocks, WEAPON_LIGHTNING));
    return 0;
}

static const char* test_unlocks_character_unlock(void)
{
    UnlockData unlocks;
    UnlocksInit(&unlocks);

    mu_assert_true(UnlocksHasCharacter(&unlocks, 0));
    mu_assert_false(UnlocksHasCharacter(&unlocks, 1));
    mu_assert_false(UnlocksHasCharacter(&unlocks, 2));

    UnlocksUnlockCharacter(&unlocks, 1);
    mu_assert_true(UnlocksHasCharacter(&unlocks, 1));
    UnlocksUnlockCharacter(&unlocks, 2);
    mu_assert_true(UnlocksHasCharacter(&unlocks, 2));
    return 0;
}

static const char* test_unlocks_add_run_stats(void)
{
    UnlockData unlocks;
    UnlocksInit(&unlocks);

    UnlocksAddRunStats(&unlocks, 50, 1, 1000, 5, 120.0f);
    mu_assert_int_eq(50, unlocks.totalKills);
    mu_assert_int_eq(1, unlocks.totalBossKills);
    mu_assert_int_eq(1000, unlocks.totalScore);
    mu_assert_int_eq(1, unlocks.gamesPlayed);
    mu_assert_int_eq(5, unlocks.highestLevel);
    mu_assert_float_eq(120.0f, unlocks.longestSurvival);

    // Add another run
    UnlocksAddRunStats(&unlocks, 30, 0, 500, 3, 90.0f);
    mu_assert_int_eq(80, unlocks.totalKills);  // 50 + 30
    mu_assert_int_eq(1, unlocks.totalBossKills);  // 1 + 0
    mu_assert_int_eq(1500, unlocks.totalScore);  // 1000 + 500
    mu_assert_int_eq(2, unlocks.gamesPlayed);
    mu_assert_int_eq(5, unlocks.highestLevel);  // Still 5 (3 < 5)
    mu_assert_float_eq(120.0f, unlocks.longestSurvival);  // Still 120 (90 < 120)
    return 0;
}

static const char* test_unlocks_meta_bonuses(void)
{
    UnlockData unlocks;
    UnlocksInit(&unlocks);

    // Level 0 bonuses
    mu_assert_float_eq(1.0f, UnlocksGetSpeedBonus(&unlocks));
    mu_assert_float_eq(0.0f, UnlocksGetHealthBonus(&unlocks));
    mu_assert_float_eq(1.0f, UnlocksGetDamageBonus(&unlocks));

    // Add some meta upgrades
    unlocks.metaSpeed = 3;   // +6% speed
    unlocks.metaHealth = 2;  // +20 health
    unlocks.metaDamage = 1;  // +5% damage

    mu_assert_float_eq(1.06f, UnlocksGetSpeedBonus(&unlocks));
    mu_assert_float_eq(20.0f, UnlocksGetHealthBonus(&unlocks));
    mu_assert_float_eq(1.05f, UnlocksGetDamageBonus(&unlocks));
    return 0;
}

static const char* test_unlocks_meta_upgrade_cost(void)
{
    // Level 0 -> 1: base cost
    mu_assert_int_eq(1000, UnlocksGetMetaUpgradeCost(0));
    // Level 1 -> 2: base * 2
    mu_assert_int_eq(2000, UnlocksGetMetaUpgradeCost(1));
    // Level 2 -> 3: base * 4
    mu_assert_int_eq(4000, UnlocksGetMetaUpgradeCost(2));
    // Level 3 -> 4: base * 8
    mu_assert_int_eq(8000, UnlocksGetMetaUpgradeCost(3));
    // Level 4 -> 5: base * 16
    mu_assert_int_eq(16000, UnlocksGetMetaUpgradeCost(4));
    // Max level - cannot upgrade
    mu_assert_int_eq(-1, UnlocksGetMetaUpgradeCost(5));
    return 0;
}

static const char* test_unlocks_check_new_unlocks(void)
{
    UnlockData unlocks;
    UnlocksInit(&unlocks);

    // No unlocks yet
    mu_assert_false(UnlocksCheckNewUnlocks(&unlocks));

    // Kill 100 enemies - should unlock Spread Shot
    unlocks.totalKills = 100;
    mu_assert_true(UnlocksCheckNewUnlocks(&unlocks));
    mu_assert_true(UnlocksHasWeapon(&unlocks, WEAPON_SPREAD_SHOT));

    // Second check shouldn't find new unlocks
    mu_assert_false(UnlocksCheckNewUnlocks(&unlocks));

    // Reach level 10 - should unlock Lightning
    unlocks.highestLevel = 10;
    mu_assert_true(UnlocksCheckNewUnlocks(&unlocks));
    mu_assert_true(UnlocksHasWeapon(&unlocks, WEAPON_LIGHTNING));

    // Play 5 games - should unlock character 2
    unlocks.gamesPlayed = 5;
    mu_assert_true(UnlocksCheckNewUnlocks(&unlocks));
    mu_assert_true(UnlocksHasCharacter(&unlocks, 1));
    return 0;
}

static const char* test_unlocks_invalid_inputs(void)
{
    UnlockData unlocks;
    UnlocksInit(&unlocks);

    // Invalid weapon index
    mu_assert_false(UnlocksHasWeapon(&unlocks, -1));
    mu_assert_false(UnlocksHasWeapon(&unlocks, 100));

    // Invalid character index
    mu_assert_false(UnlocksHasCharacter(&unlocks, -1));
    mu_assert_false(UnlocksHasCharacter(&unlocks, 10));

    // Unlocking invalid indices should be safe (no crash)
    UnlocksUnlockWeapon(&unlocks, -1);
    UnlocksUnlockWeapon(&unlocks, 100);
    UnlocksUnlockCharacter(&unlocks, -1);
    UnlocksUnlockCharacter(&unlocks, 10);
    return 0;
}

const char* run_unlocks_tests(void)
{
    mu_run_test(test_unlocks_init);
    mu_run_test(test_unlocks_weapon_unlock);
    mu_run_test(test_unlocks_character_unlock);
    mu_run_test(test_unlocks_add_run_stats);
    mu_run_test(test_unlocks_meta_bonuses);
    mu_run_test(test_unlocks_meta_upgrade_cost);
    mu_run_test(test_unlocks_check_new_unlocks);
    mu_run_test(test_unlocks_invalid_inputs);
    return 0;
}
