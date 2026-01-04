#ifndef UNLOCKS_H
#define UNLOCKS_H

#include <stdbool.h>
#include "weapon.h"

// Persistent unlock data saved to disk
typedef struct UnlockData {
    // Version for save file compatibility
    int version;

    // Unlocked weapons (bitfield - each bit = one weapon type)
    unsigned int unlockedWeapons;

    // Unlocked characters (bitfield)
    unsigned int unlockedCharacters;

    // Meta upgrades (permanent bonuses, 0-5 levels each)
    int metaSpeed;          // +2% movement speed per level
    int metaHealth;         // +10 max health per level
    int metaDamage;         // +5% damage per level
    int metaXP;             // +5% XP gain per level
    int metaMagnet;         // +10% pickup range per level

    // Lifetime stats for unlock conditions
    int totalKills;         // Lifetime enemies killed
    int totalBossKills;     // Lifetime bosses killed
    int totalScore;         // Lifetime score accumulated
    int gamesPlayed;        // Total runs
    int highestLevel;       // Best player level achieved
    float longestSurvival;  // Longest survival time (seconds)
} UnlockData;

#define UNLOCKS_FILE "unlocks.dat"
#define UNLOCKS_VERSION 1
#define META_UPGRADE_MAX_LEVEL 5

// Initialize with defaults (pulse cannon + character 1 unlocked)
void UnlocksInit(UnlockData *unlocks);

// Save/load unlocks to disk
void UnlocksSave(UnlockData *unlocks);
void UnlocksLoad(UnlockData *unlocks);

// Check if specific content is unlocked
bool UnlocksHasWeapon(UnlockData *unlocks, WeaponType weapon);
bool UnlocksHasCharacter(UnlockData *unlocks, int characterId);

// Unlock specific content
void UnlocksUnlockWeapon(UnlockData *unlocks, WeaponType weapon);
void UnlocksUnlockCharacter(UnlockData *unlocks, int characterId);

// Add to lifetime stats (called at end of run)
void UnlocksAddRunStats(UnlockData *unlocks, int kills, int bossKills,
                        int score, int level, float survivalTime);

// Get meta bonus values
float UnlocksGetSpeedBonus(UnlockData *unlocks);      // Returns 1.0 + bonus
float UnlocksGetHealthBonus(UnlockData *unlocks);     // Returns flat health bonus
float UnlocksGetDamageBonus(UnlockData *unlocks);     // Returns 1.0 + bonus
float UnlocksGetXPBonus(UnlockData *unlocks);         // Returns 1.0 + bonus
float UnlocksGetMagnetBonus(UnlockData *unlocks);     // Returns 1.0 + bonus

// Upgrade meta stats (costs accumulated score)
#define META_UPGRADE_COST_BASE 1000
#define META_UPGRADE_COST_MULT 2     // Cost doubles each level
int UnlocksGetMetaUpgradeCost(int currentLevel);
bool UnlocksCanAffordMetaUpgrade(UnlockData *unlocks, int currentLevel);
void UnlocksPurchaseMetaUpgrade(UnlockData *unlocks, int *metaStat);

// Check for new unlocks based on stats (returns true if something new unlocked)
bool UnlocksCheckNewUnlocks(UnlockData *unlocks);

#endif
