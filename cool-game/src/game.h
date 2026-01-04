#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "projectile.h"
#include "player.h"
#include "enemy.h"
#include "xp.h"
#include "upgrade.h"
#include "particle.h"
#include <stdbool.h>

typedef enum GameState {
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_LEVELUP,
    STATE_GAMEOVER
} GameState;

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
    int bloomIntensityLoc;
    int crtTimeLoc;
    bool shadersEnabled;
    bool crtEnabled;
    // Phase 9: Menu & Polish
    int highScore;    // Best score across sessions (persisted to file)
    int killCount;    // Enemies killed this run
    // Phase 10: Score multiplier
    float scoreMultiplier;     // Current score multiplier (resets on damage)
    float timeSinceLastHit;    // Seconds since player last took damage
} GameData;

void GameInit(GameData *game);
void GameInitShaders(GameData *game);
void GameCleanupShaders(GameData *game);
void GameUpdate(GameData *game, float dt);
void GameDraw(GameData *game);
void TriggerScreenShake(GameData *game, float intensity, float duration);

#endif
