#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "projectile.h"
#include "player.h"
#include "enemy.h"
#include "xp.h"
#include "upgrade.h"
#include "particle.h"
#include "unlocks.h"
#include "leaderboard.h"
#include "character.h"
#include "achievement.h"
#include "coop.h"
#include <stdbool.h>

typedef enum GameState {
    STATE_MENU,
    STATE_MODE_SELECT,     // 1 Player / 2 Players selection
    STATE_SETTINGS,        // Settings menu
    STATE_LEADERBOARD,     // High scores display
    STATE_ACHIEVEMENTS,    // Achievements display
    STATE_CHARACTER_SELECT,// Character selection screen (P1)
    STATE_CHARACTER_SELECT_P2, // Character selection screen (P2 in co-op)
    STATE_STARTING,        // "Get Ready" transition screen
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_LEVELUP,
    STATE_GAMEOVER
} GameState;

// Settings that persist to disk
typedef struct GameSettings {
    float musicVolume;          // 0.0 - 1.0
    float sfxVolume;            // 0.0 - 1.0
    bool screenShakeEnabled;
    bool crtEnabled;
    bool chromaticEnabled;      // Chromatic aberration on low health
} GameSettings;

typedef struct GameData {
    GameState state;
    float gameTime;
    int score;
    bool isPaused;
    Player player;
    ProjectilePool projectiles;
    EnemyPool enemies;
    XPPool xp;
    ParticlePool particles;
    float spawnTimer;
    UpgradeType upgradeOptions[3];
    Camera2D camera;
    float shakeIntensity;
    float shakeDuration;
    // Post-processing (ping-pong textures for shader chaining)
    RenderTexture2D renderTarget;
    RenderTexture2D renderTarget2;
    Shader bloomShader;
    Shader crtShader;
    Shader chromaticShader;
    int bloomIntensityLoc;
    int crtTimeLoc;
    int chromaticIntensityLoc;
    int chromaticTimeLoc;
    bool shadersEnabled;
    bool crtEnabled;
    float chromaticIntensity;  // Current chromatic aberration intensity (0-1)
    // Phase 9: Menu & Polish
    int highScore;    // Best score across sessions (persisted to file)
    int killCount;    // Enemies killed this run
    // Phase 10: Score multiplier
    float scoreMultiplier;     // Current score multiplier (resets on damage)
    float timeSinceLastHit;    // Seconds since player last took damage
    // Quick wins: Game feel
    int hitstopFrames;         // Frames to freeze game (hitstop effect)
    float timeScale;           // Time multiplier for slow-mo (1.0 = normal)
    float tutorialTimer;       // Time elapsed since game start (for tutorial)
    // Impact frames (bright flash on explosions)
    Vector2 impactPos;         // Position of impact flash
    int impactFrames;          // Frames remaining for impact flash (0 = none)
    float impactRadius;        // Radius of impact flash
    // Starting transition
    float transitionTimer;     // Timer for "Get Ready" screen
    float fadeAlpha;           // Fade overlay alpha (0-1)
    // Settings
    GameSettings settings;     // Persisted game settings
    int settingsSelection;     // Current settings menu item (0-3)
    // Boss system
    float bossSpawnTimer;      // Time until next boss spawn
    int bossCount;             // Number of bosses defeated this run
    float bossWarningTimer;    // Warning display timer before boss spawn
    bool bossWarningActive;    // Show "BOSS INCOMING" warning
    // Permanent unlocks
    UnlockData unlocks;        // Persistent unlock data
    int bossKillsThisRun;      // Boss kills this run (for unlock tracking)
    // Leaderboard
    Leaderboard leaderboard;   // Top 10 high scores
    int leaderboardPosition;   // Position this run placed (-1 if didn't qualify)
    // Character selection
    CharacterType selectedCharacter;  // Currently selected character (P1)
    CharacterType selectedCharacterP2;// P2 character (co-op only)
    int characterSelection;           // Cursor position in character select
    // Achievement system
    AchievementData achievements;          // Persistent achievement data
    AchievementType pendingAchievement;    // Achievement to display (-1 if none)
    float achievementDisplayTimer;         // Time to show achievement popup
    int achievementSelection;              // Cursor position in achievements screen
    // Co-op mode
    GameMode gameMode;                     // Solo or Co-op
    CoopState coop;                        // Co-op state (players, cameras, etc.)
    int modeSelection;                     // 0 = 1 Player, 1 = 2 Players in mode select
} GameData;

void GameInit(GameData *game);
void GameInitShaders(GameData *game);
void GameCleanupShaders(GameData *game);
void GameUpdate(GameData *game, float dt);
void GameDraw(GameData *game);
void TriggerScreenShake(GameData *game, float intensity, float duration);
void TriggerImpactFrame(GameData *game, Vector2 pos, float radius);

#endif
