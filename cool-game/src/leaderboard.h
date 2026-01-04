#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <stdbool.h>

#define LEADERBOARD_MAX_ENTRIES 10
#define LEADERBOARD_FILE "leaderboard.dat"
#define LEADERBOARD_VERSION 1

// Single leaderboard entry
typedef struct LeaderboardEntry {
    int score;
    int level;
    int kills;
    float survivalTime;
    int day;       // Day of month (1-31)
    int month;     // Month (1-12)
    int year;      // Year (e.g., 2026)
} LeaderboardEntry;

// Leaderboard data structure
typedef struct Leaderboard {
    int version;
    int entryCount;
    LeaderboardEntry entries[LEADERBOARD_MAX_ENTRIES];
} Leaderboard;

// Initialize leaderboard with default values
void LeaderboardInit(Leaderboard *lb);

// Load leaderboard from file
void LeaderboardLoad(Leaderboard *lb);

// Save leaderboard to file
void LeaderboardSave(Leaderboard *lb);

// Add a new entry to the leaderboard (returns position if made it, -1 if not)
int LeaderboardAddEntry(Leaderboard *lb, int score, int level, int kills, float survivalTime);

// Check if a score qualifies for the leaderboard
bool LeaderboardIsHighScore(Leaderboard *lb, int score);

// Get the minimum score needed to enter the leaderboard (-1 if board not full)
int LeaderboardGetMinScore(Leaderboard *lb);

// Get entry at position (0-indexed, returns NULL if invalid)
LeaderboardEntry* LeaderboardGetEntry(Leaderboard *lb, int position);

// Get the highest score (returns 0 if empty)
int LeaderboardGetHighScore(Leaderboard *lb);

#endif // LEADERBOARD_H
