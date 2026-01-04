#include "achievement.h"
#include <stdio.h>
#include <string.h>

// Achievement definitions
static const AchievementDef achievementDefs[ACHIEVEMENT_COUNT] = {
    // Combat achievements
    {
        .type = ACH_FIRST_BLOOD,
        .name = "First Blood",
        .description = "Kill your first enemy",
        .iconIndex = 0
    },
    {
        .type = ACH_CENTURION,
        .name = "Centurion",
        .description = "Kill 100 enemies in one run",
        .iconIndex = 1
    },
    {
        .type = ACH_SLAYER,
        .name = "Slayer",
        .description = "Kill 1000 enemies total",
        .iconIndex = 2
    },
    {
        .type = ACH_BOSS_HUNTER,
        .name = "Boss Hunter",
        .description = "Defeat your first boss",
        .iconIndex = 3
    },
    {
        .type = ACH_BOSS_SLAYER,
        .name = "Boss Slayer",
        .description = "Defeat 5 bosses total",
        .iconIndex = 4
    },
    // Survival achievements
    {
        .type = ACH_SURVIVOR,
        .name = "Survivor",
        .description = "Survive for 3 minutes",
        .iconIndex = 5
    },
    {
        .type = ACH_VETERAN,
        .name = "Veteran",
        .description = "Survive for 10 minutes",
        .iconIndex = 6
    },
    {
        .type = ACH_IMMORTAL,
        .name = "Immortal",
        .description = "No damage for 1 minute",
        .iconIndex = 7
    },
    // Progression achievements
    {
        .type = ACH_LEVEL_5,
        .name = "Rising Star",
        .description = "Reach level 5",
        .iconIndex = 8
    },
    {
        .type = ACH_LEVEL_10,
        .name = "Champion",
        .description = "Reach level 10",
        .iconIndex = 9
    },
    {
        .type = ACH_FULLY_EVOLVED,
        .name = "Fully Evolved",
        .description = "Evolve a weapon",
        .iconIndex = 10
    },
    {
        .type = ACH_COMPLETIONIST,
        .name = "Completionist",
        .description = "Unlock all characters",
        .iconIndex = 11
    }
};

AchievementDef GetAchievementDef(AchievementType type)
{
    if (type < 0 || type >= ACHIEVEMENT_COUNT)
    {
        return achievementDefs[0];
    }
    return achievementDefs[type];
}

bool AchievementIsEarned(AchievementData *data, AchievementType type)
{
    if (type < 0 || type >= ACHIEVEMENT_COUNT) return false;
    return (data->earned & (1u << type)) != 0;
}

bool AchievementEarn(AchievementData *data, AchievementType type)
{
    if (type < 0 || type >= ACHIEVEMENT_COUNT) return false;
    if (AchievementIsEarned(data, type)) return false;  // Already earned

    data->earned |= (1u << type);
    return true;  // Newly earned
}

void AchievementInit(AchievementData *data)
{
    memset(data, 0, sizeof(AchievementData));
    data->version = ACHIEVEMENT_VERSION;
}

void AchievementSave(AchievementData *data)
{
    FILE *file = fopen(ACHIEVEMENT_FILE, "wb");
    if (file != NULL)
    {
        fwrite(data, sizeof(AchievementData), 1, file);
        fclose(file);
    }
}

void AchievementLoad(AchievementData *data)
{
    FILE *file = fopen(ACHIEVEMENT_FILE, "rb");
    if (file != NULL)
    {
        if (fread(data, sizeof(AchievementData), 1, file) != 1)
        {
            AchievementInit(data);
        }
        else if (data->version != ACHIEVEMENT_VERSION)
        {
            AchievementInit(data);
        }
        fclose(file);
    }
    else
    {
        AchievementInit(data);
    }
}

int AchievementGetEarnedCount(AchievementData *data)
{
    int count = 0;
    for (int i = 0; i < ACHIEVEMENT_COUNT; i++)
    {
        if (AchievementIsEarned(data, (AchievementType)i))
        {
            count++;
        }
    }
    return count;
}
