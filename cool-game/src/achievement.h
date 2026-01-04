#ifndef ACHIEVEMENT_H
#define ACHIEVEMENT_H

#include <stdbool.h>

#define ACHIEVEMENT_COUNT 12
#define ACHIEVEMENT_FILE "achievements.dat"
#define ACHIEVEMENT_VERSION 1

// Achievement types
typedef enum AchievementType {
    // Combat achievements
    ACH_FIRST_BLOOD,      // Kill your first enemy
    ACH_CENTURION,        // Kill 100 enemies in one run
    ACH_SLAYER,           // Kill 1000 enemies lifetime
    ACH_BOSS_HUNTER,      // Kill your first boss
    ACH_BOSS_SLAYER,      // Kill 5 bosses lifetime
    // Survival achievements
    ACH_SURVIVOR,         // Survive 3 minutes
    ACH_VETERAN,          // Survive 10 minutes
    ACH_IMMORTAL,         // Survive without taking damage for 1 minute
    // Progression achievements
    ACH_LEVEL_5,          // Reach level 5
    ACH_LEVEL_10,         // Reach level 10
    ACH_FULLY_EVOLVED,    // Evolve a weapon
    ACH_COMPLETIONIST     // Unlock all characters
} AchievementType;

// Achievement definition
typedef struct AchievementDef {
    AchievementType type;
    const char *name;
    const char *description;
    int iconIndex;        // For future icon support
} AchievementDef;

// Achievement state
typedef struct AchievementData {
    int version;
    unsigned int earned;  // Bitfield of earned achievements
    // Lifetime stats for achievement tracking
    int totalKills;
    int totalBossKills;
    float longestSurvival;
    int highestLevel;
    bool hasEvolved;
} AchievementData;

// Achievement notification queue
#define ACHIEVEMENT_QUEUE_SIZE 3
typedef struct AchievementNotification {
    AchievementType type;
    float timer;
    bool active;
} AchievementNotification;

// Get achievement definition
AchievementDef GetAchievementDef(AchievementType type);

// Check if achievement is earned
bool AchievementIsEarned(AchievementData *data, AchievementType type);

// Mark achievement as earned (returns true if newly earned)
bool AchievementEarn(AchievementData *data, AchievementType type);

// Initialize achievement data
void AchievementInit(AchievementData *data);

// Save/load achievements
void AchievementSave(AchievementData *data);
void AchievementLoad(AchievementData *data);

// Get earned count
int AchievementGetEarnedCount(AchievementData *data);

#endif // ACHIEVEMENT_H
