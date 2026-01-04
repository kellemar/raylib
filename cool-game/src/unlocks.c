#include "unlocks.h"
#include <stdio.h>
#include <string.h>

void UnlocksInit(UnlockData *unlocks)
{
    memset(unlocks, 0, sizeof(UnlockData));
    unlocks->version = UNLOCKS_VERSION;

    // Start with pulse cannon unlocked
    unlocks->unlockedWeapons = (1 << WEAPON_PULSE_CANNON);

    // Start with character 0 unlocked
    unlocks->unlockedCharacters = (1 << 0);
}

void UnlocksSave(UnlockData *unlocks)
{
    FILE *file = fopen(UNLOCKS_FILE, "wb");
    if (file != NULL)
    {
        fwrite(unlocks, sizeof(UnlockData), 1, file);
        fclose(file);
    }
}

void UnlocksLoad(UnlockData *unlocks)
{
    FILE *file = fopen(UNLOCKS_FILE, "rb");
    if (file != NULL)
    {
        if (fread(unlocks, sizeof(UnlockData), 1, file) != 1)
        {
            // Read failed, initialize defaults
            UnlocksInit(unlocks);
        }
        else if (unlocks->version != UNLOCKS_VERSION)
        {
            // Version mismatch, reset to defaults
            UnlocksInit(unlocks);
        }
        fclose(file);
    }
    else
    {
        // No save file, initialize defaults
        UnlocksInit(unlocks);
    }
}

bool UnlocksHasWeapon(UnlockData *unlocks, WeaponType weapon)
{
    if (weapon < 0 || weapon >= WEAPON_BASE_COUNT) return false;
    return (unlocks->unlockedWeapons & (1 << weapon)) != 0;
}

bool UnlocksHasCharacter(UnlockData *unlocks, int characterId)
{
    if (characterId < 0 || characterId >= 8) return false;
    return (unlocks->unlockedCharacters & (1 << characterId)) != 0;
}

void UnlocksUnlockWeapon(UnlockData *unlocks, WeaponType weapon)
{
    if (weapon < 0 || weapon >= WEAPON_BASE_COUNT) return;
    unlocks->unlockedWeapons |= (1 << weapon);
}

void UnlocksUnlockCharacter(UnlockData *unlocks, int characterId)
{
    if (characterId < 0 || characterId >= 8) return;
    unlocks->unlockedCharacters |= (1 << characterId);
}

void UnlocksAddRunStats(UnlockData *unlocks, int kills, int bossKills,
                        int score, int level, float survivalTime)
{
    unlocks->totalKills += kills;
    unlocks->totalBossKills += bossKills;
    unlocks->totalScore += score;
    unlocks->gamesPlayed++;

    if (level > unlocks->highestLevel)
    {
        unlocks->highestLevel = level;
    }
    if (survivalTime > unlocks->longestSurvival)
    {
        unlocks->longestSurvival = survivalTime;
    }
}

float UnlocksGetSpeedBonus(UnlockData *unlocks)
{
    return 1.0f + (unlocks->metaSpeed * 0.02f);  // +2% per level
}

float UnlocksGetHealthBonus(UnlockData *unlocks)
{
    return (float)(unlocks->metaHealth * 10);   // +10 health per level
}

float UnlocksGetDamageBonus(UnlockData *unlocks)
{
    return 1.0f + (unlocks->metaDamage * 0.05f);  // +5% per level
}

float UnlocksGetXPBonus(UnlockData *unlocks)
{
    return 1.0f + (unlocks->metaXP * 0.05f);     // +5% per level
}

float UnlocksGetMagnetBonus(UnlockData *unlocks)
{
    return 1.0f + (unlocks->metaMagnet * 0.10f);  // +10% per level
}

int UnlocksGetMetaUpgradeCost(int currentLevel)
{
    if (currentLevel >= META_UPGRADE_MAX_LEVEL) return -1;  // Max level reached

    int cost = META_UPGRADE_COST_BASE;
    for (int i = 0; i < currentLevel; i++)
    {
        cost *= META_UPGRADE_COST_MULT;
    }
    return cost;
}

bool UnlocksCanAffordMetaUpgrade(UnlockData *unlocks, int currentLevel)
{
    int cost = UnlocksGetMetaUpgradeCost(currentLevel);
    if (cost < 0) return false;  // Already max level
    return unlocks->totalScore >= cost;
}

void UnlocksPurchaseMetaUpgrade(UnlockData *unlocks, int *metaStat)
{
    if (*metaStat >= META_UPGRADE_MAX_LEVEL) return;

    int cost = UnlocksGetMetaUpgradeCost(*metaStat);
    if (cost < 0 || unlocks->totalScore < cost) return;

    unlocks->totalScore -= cost;
    (*metaStat)++;
}

bool UnlocksCheckNewUnlocks(UnlockData *unlocks)
{
    bool newUnlock = false;

    // Weapon unlocks based on lifetime stats
    // Spread Shot: Kill 100 enemies
    if (unlocks->totalKills >= 100 && !UnlocksHasWeapon(unlocks, WEAPON_SPREAD_SHOT))
    {
        UnlocksUnlockWeapon(unlocks, WEAPON_SPREAD_SHOT);
        newUnlock = true;
    }

    // Homing Missile: Kill 500 enemies
    if (unlocks->totalKills >= 500 && !UnlocksHasWeapon(unlocks, WEAPON_HOMING_MISSILE))
    {
        UnlocksUnlockWeapon(unlocks, WEAPON_HOMING_MISSILE);
        newUnlock = true;
    }

    // Lightning: Reach level 10
    if (unlocks->highestLevel >= 10 && !UnlocksHasWeapon(unlocks, WEAPON_LIGHTNING))
    {
        UnlocksUnlockWeapon(unlocks, WEAPON_LIGHTNING);
        newUnlock = true;
    }

    // Orbit Shield: Survive 3 minutes
    if (unlocks->longestSurvival >= 180.0f && !UnlocksHasWeapon(unlocks, WEAPON_ORBIT_SHIELD))
    {
        UnlocksUnlockWeapon(unlocks, WEAPON_ORBIT_SHIELD);
        newUnlock = true;
    }

    // Flamethrower: Kill 1 boss
    if (unlocks->totalBossKills >= 1 && !UnlocksHasWeapon(unlocks, WEAPON_FLAMETHROWER))
    {
        UnlocksUnlockWeapon(unlocks, WEAPON_FLAMETHROWER);
        newUnlock = true;
    }

    // Freeze Ray: Kill 3 bosses
    if (unlocks->totalBossKills >= 3 && !UnlocksHasWeapon(unlocks, WEAPON_FREEZE_RAY))
    {
        UnlocksUnlockWeapon(unlocks, WEAPON_FREEZE_RAY);
        newUnlock = true;
    }

    // Black Hole: Score 10000 lifetime points
    if (unlocks->totalScore >= 10000 && !UnlocksHasWeapon(unlocks, WEAPON_BLACK_HOLE))
    {
        UnlocksUnlockWeapon(unlocks, WEAPON_BLACK_HOLE);
        newUnlock = true;
    }

    // Character unlocks
    // Character 2 (Tank): Play 5 games
    if (unlocks->gamesPlayed >= 5 && !UnlocksHasCharacter(unlocks, 1))
    {
        UnlocksUnlockCharacter(unlocks, 1);
        newUnlock = true;
    }

    // Character 3 (Speedster): Survive 5 minutes
    if (unlocks->longestSurvival >= 300.0f && !UnlocksHasCharacter(unlocks, 2))
    {
        UnlocksUnlockCharacter(unlocks, 2);
        newUnlock = true;
    }

    return newUnlock;
}
