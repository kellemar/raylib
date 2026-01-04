#ifndef GAME_H
#define GAME_H

#include "raylib.h"
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
} GameData;

void GameInit(GameData *game);
void GameUpdate(GameData *game, float dt);
void GameDraw(GameData *game);

#endif
