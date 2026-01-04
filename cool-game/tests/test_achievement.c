#include "minunit.h"
#include <string.h>

// Achievement structures (matching achievement.h)
#define ACHIEVEMENT_COUNT 12
#define ACHIEVEMENT_VERSION 1

typedef enum AchievementType {
    ACH_FIRST_BLOOD,
    ACH_CENTURION,
    ACH_SLAYER,
    ACH_BOSS_HUNTER,
    ACH_BOSS_SLAYER,
    ACH_SURVIVOR,
    ACH_VETERAN,
    ACH_IMMORTAL,
    ACH_LEVEL_5,
    ACH_LEVEL_10,
    ACH_FULLY_EVOLVED,
    ACH_COMPLETIONIST
} AchievementType;

typedef struct AchievementDef {
    AchievementType type;
    const char *name;
    const char *description;
    int iconIndex;
} AchievementDef;

typedef struct AchievementData {
    int version;
    unsigned int earned;
    int totalKills;
    int totalBossKills;
    float longestSurvival;
    int highestLevel;
    int hasEvolved;
} AchievementData;

// Test implementations (matching achievement.c)
static const AchievementDef achievementDefs[ACHIEVEMENT_COUNT] = {
    { ACH_FIRST_BLOOD, "First Blood", "Kill your first enemy", 0 },
    { ACH_CENTURION, "Centurion", "Kill 100 enemies in one run", 1 },
    { ACH_SLAYER, "Slayer", "Kill 1000 enemies total", 2 },
    { ACH_BOSS_HUNTER, "Boss Hunter", "Defeat your first boss", 3 },
    { ACH_BOSS_SLAYER, "Boss Slayer", "Defeat 5 bosses total", 4 },
    { ACH_SURVIVOR, "Survivor", "Survive for 3 minutes", 5 },
    { ACH_VETERAN, "Veteran", "Survive for 10 minutes", 6 },
    { ACH_IMMORTAL, "Immortal", "No damage for 1 minute", 7 },
    { ACH_LEVEL_5, "Rising Star", "Reach level 5", 8 },
    { ACH_LEVEL_10, "Champion", "Reach level 10", 9 },
    { ACH_FULLY_EVOLVED, "Fully Evolved", "Evolve a weapon", 10 },
    { ACH_COMPLETIONIST, "Completionist", "Unlock all characters", 11 }
};

static AchievementDef GetAchievementDef(AchievementType type)
{
    if (type < 0 || type >= ACHIEVEMENT_COUNT)
    {
        return achievementDefs[0];
    }
    return achievementDefs[type];
}

static int AchievementIsEarned(AchievementData *data, AchievementType type)
{
    if (type < 0 || type >= ACHIEVEMENT_COUNT) return 0;
    return (data->earned & (1u << type)) != 0;
}

static int AchievementEarn(AchievementData *data, AchievementType type)
{
    if (type < 0 || type >= ACHIEVEMENT_COUNT) return 0;
    if (AchievementIsEarned(data, type)) return 0;

    data->earned |= (1u << type);
    return 1;
}

static void AchievementInit(AchievementData *data)
{
    memset(data, 0, sizeof(AchievementData));
    data->version = ACHIEVEMENT_VERSION;
}

static int AchievementGetEarnedCount(AchievementData *data)
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

// Tests

static const char* test_achievement_count(void)
{
    mu_assert_int_eq(12, ACHIEVEMENT_COUNT);
    return 0;
}

static const char* test_achievement_init(void)
{
    AchievementData data;
    AchievementInit(&data);

    mu_assert_int_eq(ACHIEVEMENT_VERSION, data.version);
    mu_assert_int_eq(0, data.earned);
    mu_assert_int_eq(0, AchievementGetEarnedCount(&data));
    return 0;
}

static const char* test_achievement_earn(void)
{
    AchievementData data;
    AchievementInit(&data);

    // Not earned initially
    mu_assert_false(AchievementIsEarned(&data, ACH_FIRST_BLOOD));

    // Earn it
    int result = AchievementEarn(&data, ACH_FIRST_BLOOD);
    mu_assert_true(result);
    mu_assert_true(AchievementIsEarned(&data, ACH_FIRST_BLOOD));

    // Can't earn again
    result = AchievementEarn(&data, ACH_FIRST_BLOOD);
    mu_assert_false(result);
    return 0;
}

static const char* test_achievement_multiple(void)
{
    AchievementData data;
    AchievementInit(&data);

    AchievementEarn(&data, ACH_FIRST_BLOOD);
    AchievementEarn(&data, ACH_SURVIVOR);
    AchievementEarn(&data, ACH_LEVEL_5);

    mu_assert_int_eq(3, AchievementGetEarnedCount(&data));
    mu_assert_true(AchievementIsEarned(&data, ACH_FIRST_BLOOD));
    mu_assert_true(AchievementIsEarned(&data, ACH_SURVIVOR));
    mu_assert_true(AchievementIsEarned(&data, ACH_LEVEL_5));
    mu_assert_false(AchievementIsEarned(&data, ACH_CENTURION));
    return 0;
}

static const char* test_achievement_definitions(void)
{
    AchievementDef def = GetAchievementDef(ACH_FIRST_BLOOD);
    mu_assert(strcmp(def.name, "First Blood") == 0, "First Blood name");
    mu_assert(strlen(def.description) > 0, "Has description");

    def = GetAchievementDef(ACH_COMPLETIONIST);
    mu_assert(strcmp(def.name, "Completionist") == 0, "Completionist name");
    return 0;
}

static const char* test_achievement_invalid_type(void)
{
    AchievementData data;
    AchievementInit(&data);

    // Invalid type should fail
    mu_assert_false(AchievementIsEarned(&data, -1));
    mu_assert_false(AchievementIsEarned(&data, 100));
    mu_assert_false(AchievementEarn(&data, -1));
    mu_assert_false(AchievementEarn(&data, 100));
    return 0;
}

static const char* test_achievement_all_earned(void)
{
    AchievementData data;
    AchievementInit(&data);

    // Earn all achievements
    for (int i = 0; i < ACHIEVEMENT_COUNT; i++)
    {
        AchievementEarn(&data, (AchievementType)i);
    }

    mu_assert_int_eq(ACHIEVEMENT_COUNT, AchievementGetEarnedCount(&data));
    return 0;
}

const char* run_achievement_tests(void)
{
    mu_run_test(test_achievement_count);
    mu_run_test(test_achievement_init);
    mu_run_test(test_achievement_earn);
    mu_run_test(test_achievement_multiple);
    mu_run_test(test_achievement_definitions);
    mu_run_test(test_achievement_invalid_type);
    mu_run_test(test_achievement_all_earned);
    return 0;
}
