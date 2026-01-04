#include "leaderboard.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

void LeaderboardInit(Leaderboard *lb)
{
    memset(lb, 0, sizeof(Leaderboard));
    lb->version = LEADERBOARD_VERSION;
    lb->entryCount = 0;
}

void LeaderboardLoad(Leaderboard *lb)
{
    FILE *file = fopen(LEADERBOARD_FILE, "rb");
    if (file != NULL)
    {
        if (fread(lb, sizeof(Leaderboard), 1, file) != 1)
        {
            // Read failed, initialize defaults
            LeaderboardInit(lb);
        }
        else if (lb->version != LEADERBOARD_VERSION)
        {
            // Version mismatch, reset
            LeaderboardInit(lb);
        }
        fclose(file);
    }
    else
    {
        // No file exists, initialize defaults
        LeaderboardInit(lb);
    }
}

void LeaderboardSave(Leaderboard *lb)
{
    FILE *file = fopen(LEADERBOARD_FILE, "wb");
    if (file != NULL)
    {
        fwrite(lb, sizeof(Leaderboard), 1, file);
        fclose(file);
    }
}

int LeaderboardAddEntry(Leaderboard *lb, int score, int level, int kills, float survivalTime)
{
    // Get current date
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

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

    // Insert new entry
    lb->entries[insertPos].score = score;
    lb->entries[insertPos].level = level;
    lb->entries[insertPos].kills = kills;
    lb->entries[insertPos].survivalTime = survivalTime;
    lb->entries[insertPos].day = tm_info->tm_mday;
    lb->entries[insertPos].month = tm_info->tm_mon + 1;
    lb->entries[insertPos].year = tm_info->tm_year + 1900;

    // Update count
    if (lb->entryCount < LEADERBOARD_MAX_ENTRIES)
    {
        lb->entryCount++;
    }

    return insertPos;
}

bool LeaderboardIsHighScore(Leaderboard *lb, int score)
{
    // If board not full, any score qualifies
    if (lb->entryCount < LEADERBOARD_MAX_ENTRIES)
    {
        return true;
    }

    // Check if score beats the lowest entry
    return score > lb->entries[LEADERBOARD_MAX_ENTRIES - 1].score;
}

int LeaderboardGetMinScore(Leaderboard *lb)
{
    if (lb->entryCount < LEADERBOARD_MAX_ENTRIES)
    {
        return -1;  // Board not full
    }
    return lb->entries[LEADERBOARD_MAX_ENTRIES - 1].score;
}

LeaderboardEntry* LeaderboardGetEntry(Leaderboard *lb, int position)
{
    if (position < 0 || position >= lb->entryCount)
    {
        return NULL;
    }
    return &lb->entries[position];
}

int LeaderboardGetHighScore(Leaderboard *lb)
{
    if (lb->entryCount == 0)
    {
        return 0;
    }
    return lb->entries[0].score;
}
