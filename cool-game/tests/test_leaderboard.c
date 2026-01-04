#include "minunit.h"
#include <string.h>

// Leaderboard structures (matching leaderboard.h)
#define LEADERBOARD_MAX_ENTRIES 10
#define LEADERBOARD_VERSION 1

typedef struct LeaderboardEntry {
    int score;
    int level;
    int kills;
    float survivalTime;
    int day;
    int month;
    int year;
} LeaderboardEntry;

typedef struct Leaderboard {
    int version;
    int entryCount;
    LeaderboardEntry entries[LEADERBOARD_MAX_ENTRIES];
} Leaderboard;

// Test implementations (matching leaderboard.c)
static void LeaderboardInit(Leaderboard *lb)
{
    memset(lb, 0, sizeof(Leaderboard));
    lb->version = LEADERBOARD_VERSION;
    lb->entryCount = 0;
}

static int LeaderboardAddEntry(Leaderboard *lb, int score, int level, int kills, float survivalTime)
{
    // Find insertion position (entries sorted by score descending)
    int insertPos = -1;
    for (int i = 0; i < lb->entryCount && i < LEADERBOARD_MAX_ENTRIES; i++)
    {
        if (score > lb->entries[i].score)
        {
            insertPos = i;
            break;
        }
    }

    // If not found a position but board not full, add at end
    if (insertPos == -1 && lb->entryCount < LEADERBOARD_MAX_ENTRIES)
    {
        insertPos = lb->entryCount;
    }

    // Score didn't make the cut
    if (insertPos == -1)
    {
        return -1;
    }

    // Shift entries down to make room
    for (int i = LEADERBOARD_MAX_ENTRIES - 1; i > insertPos; i--)
    {
        lb->entries[i] = lb->entries[i - 1];
    }

    // Insert new entry (skip date for tests)
    lb->entries[insertPos].score = score;
    lb->entries[insertPos].level = level;
    lb->entries[insertPos].kills = kills;
    lb->entries[insertPos].survivalTime = survivalTime;
    lb->entries[insertPos].day = 1;
    lb->entries[insertPos].month = 1;
    lb->entries[insertPos].year = 2026;

    // Update count
    if (lb->entryCount < LEADERBOARD_MAX_ENTRIES)
    {
        lb->entryCount++;
    }

    return insertPos;
}

static int LeaderboardIsHighScore(Leaderboard *lb, int score)
{
    if (lb->entryCount < LEADERBOARD_MAX_ENTRIES)
    {
        return 1;
    }
    return score > lb->entries[LEADERBOARD_MAX_ENTRIES - 1].score;
}

static int LeaderboardGetMinScore(Leaderboard *lb)
{
    if (lb->entryCount < LEADERBOARD_MAX_ENTRIES)
    {
        return -1;
    }
    return lb->entries[LEADERBOARD_MAX_ENTRIES - 1].score;
}

static LeaderboardEntry* LeaderboardGetEntry(Leaderboard *lb, int position)
{
    if (position < 0 || position >= lb->entryCount)
    {
        return NULL;
    }
    return &lb->entries[position];
}

static int LeaderboardGetHighScore(Leaderboard *lb)
{
    if (lb->entryCount == 0)
    {
        return 0;
    }
    return lb->entries[0].score;
}

// Tests

static const char* test_leaderboard_init(void)
{
    Leaderboard lb;
    LeaderboardInit(&lb);

    mu_assert_int_eq(LEADERBOARD_VERSION, lb.version);
    mu_assert_int_eq(0, lb.entryCount);
    mu_assert_int_eq(0, LeaderboardGetHighScore(&lb));
    return 0;
}

static const char* test_leaderboard_add_first_entry(void)
{
    Leaderboard lb;
    LeaderboardInit(&lb);

    int pos = LeaderboardAddEntry(&lb, 1000, 5, 50, 120.0f);

    mu_assert_int_eq(0, pos);
    mu_assert_int_eq(1, lb.entryCount);
    mu_assert_int_eq(1000, lb.entries[0].score);
    mu_assert_int_eq(5, lb.entries[0].level);
    mu_assert_int_eq(50, lb.entries[0].kills);
    mu_assert_float_eq(120.0f, lb.entries[0].survivalTime);
    return 0;
}

static const char* test_leaderboard_sorting(void)
{
    Leaderboard lb;
    LeaderboardInit(&lb);

    // Add entries out of order
    LeaderboardAddEntry(&lb, 500, 3, 30, 60.0f);
    LeaderboardAddEntry(&lb, 1000, 5, 50, 120.0f);
    LeaderboardAddEntry(&lb, 750, 4, 40, 90.0f);

    // Should be sorted descending
    mu_assert_int_eq(3, lb.entryCount);
    mu_assert_int_eq(1000, lb.entries[0].score);
    mu_assert_int_eq(750, lb.entries[1].score);
    mu_assert_int_eq(500, lb.entries[2].score);
    return 0;
}

static const char* test_leaderboard_high_score(void)
{
    Leaderboard lb;
    LeaderboardInit(&lb);

    LeaderboardAddEntry(&lb, 500, 3, 30, 60.0f);
    mu_assert_int_eq(500, LeaderboardGetHighScore(&lb));

    LeaderboardAddEntry(&lb, 1000, 5, 50, 120.0f);
    mu_assert_int_eq(1000, LeaderboardGetHighScore(&lb));

    LeaderboardAddEntry(&lb, 750, 4, 40, 90.0f);
    mu_assert_int_eq(1000, LeaderboardGetHighScore(&lb));
    return 0;
}

static const char* test_leaderboard_max_entries(void)
{
    Leaderboard lb;
    LeaderboardInit(&lb);

    // Fill the leaderboard
    for (int i = 0; i < LEADERBOARD_MAX_ENTRIES; i++)
    {
        LeaderboardAddEntry(&lb, (i + 1) * 100, i + 1, i * 10, (float)(i * 30));
    }

    mu_assert_int_eq(LEADERBOARD_MAX_ENTRIES, lb.entryCount);

    // Try to add a score that doesn't make the cut
    int pos = LeaderboardAddEntry(&lb, 50, 1, 5, 10.0f);
    mu_assert_int_eq(-1, pos);
    mu_assert_int_eq(LEADERBOARD_MAX_ENTRIES, lb.entryCount);
    return 0;
}

static const char* test_leaderboard_displacement(void)
{
    Leaderboard lb;
    LeaderboardInit(&lb);

    // Fill with low scores
    for (int i = 0; i < LEADERBOARD_MAX_ENTRIES; i++)
    {
        LeaderboardAddEntry(&lb, 100, 1, 10, 30.0f);
    }

    // Add high score that displaces lowest
    int pos = LeaderboardAddEntry(&lb, 5000, 10, 100, 300.0f);
    mu_assert_int_eq(0, pos);
    mu_assert_int_eq(LEADERBOARD_MAX_ENTRIES, lb.entryCount);
    mu_assert_int_eq(5000, lb.entries[0].score);
    return 0;
}

static const char* test_leaderboard_is_high_score(void)
{
    Leaderboard lb;
    LeaderboardInit(&lb);

    // Empty board - any score qualifies
    mu_assert_true(LeaderboardIsHighScore(&lb, 1));
    mu_assert_true(LeaderboardIsHighScore(&lb, 0));

    // Fill board with score 100
    for (int i = 0; i < LEADERBOARD_MAX_ENTRIES; i++)
    {
        LeaderboardAddEntry(&lb, 100, 1, 10, 30.0f);
    }

    // Score above 100 qualifies
    mu_assert_true(LeaderboardIsHighScore(&lb, 101));
    // Score of 100 or below doesn't
    mu_assert_false(LeaderboardIsHighScore(&lb, 100));
    mu_assert_false(LeaderboardIsHighScore(&lb, 50));
    return 0;
}

static const char* test_leaderboard_get_min_score(void)
{
    Leaderboard lb;
    LeaderboardInit(&lb);

    // Not full - returns -1
    mu_assert_int_eq(-1, LeaderboardGetMinScore(&lb));

    LeaderboardAddEntry(&lb, 500, 3, 30, 60.0f);
    mu_assert_int_eq(-1, LeaderboardGetMinScore(&lb));

    // Fill the board
    for (int i = 0; i < LEADERBOARD_MAX_ENTRIES - 1; i++)
    {
        LeaderboardAddEntry(&lb, 100, 1, 10, 30.0f);
    }

    // Now full - should return lowest score
    mu_assert_int_eq(100, LeaderboardGetMinScore(&lb));
    return 0;
}

static const char* test_leaderboard_get_entry(void)
{
    Leaderboard lb;
    LeaderboardInit(&lb);

    // Empty - should return NULL
    mu_assert(LeaderboardGetEntry(&lb, 0) == NULL, "Entry 0 should be NULL on empty board");
    mu_assert(LeaderboardGetEntry(&lb, -1) == NULL, "Negative index should return NULL");

    LeaderboardAddEntry(&lb, 1000, 5, 50, 120.0f);

    LeaderboardEntry *entry = LeaderboardGetEntry(&lb, 0);
    mu_assert(entry != NULL, "Entry 0 should exist");
    mu_assert_int_eq(1000, entry->score);

    // Out of bounds
    mu_assert(LeaderboardGetEntry(&lb, 1) == NULL, "Entry 1 should be NULL");
    mu_assert(LeaderboardGetEntry(&lb, 10) == NULL, "Entry 10 should be NULL");
    return 0;
}

const char* run_leaderboard_tests(void)
{
    mu_run_test(test_leaderboard_init);
    mu_run_test(test_leaderboard_add_first_entry);
    mu_run_test(test_leaderboard_sorting);
    mu_run_test(test_leaderboard_high_score);
    mu_run_test(test_leaderboard_max_entries);
    mu_run_test(test_leaderboard_displacement);
    mu_run_test(test_leaderboard_is_high_score);
    mu_run_test(test_leaderboard_get_min_score);
    mu_run_test(test_leaderboard_get_entry);
    return 0;
}
