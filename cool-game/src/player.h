#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include <stdbool.h>

typedef struct Player {
    Vector2 pos;
    Vector2 vel;
    Vector2 aimDir;
    float radius;
    float speed;
    float health;
    float maxHealth;
    float invincibilityTimer;
    int level;
    int xp;
    int xpToNextLevel;
    bool alive;
} Player;

void PlayerInit(Player *player);
void PlayerUpdate(Player *player, float dt);
void PlayerDraw(Player *player);
void PlayerTakeDamage(Player *player, float damage);

#endif
