#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "projectile.h"
#include "player.h"
#include "enemy.h"
#include "xp.h"
#include "upgrade.h"
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
    float spawnTimer;
    UpgradeType upgradeOptions[3];
} GameData;

void GameInit(GameData *game);
void GameUpdate(GameData *game, float dt);
void GameDraw(GameData *game);

#endif
