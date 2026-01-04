#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "weapon.h"
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
    Weapon weapon;
} Player;

void PlayerInit(Player *player);
void PlayerUpdate(Player *player, float dt, ProjectilePool *projectiles);
void PlayerDraw(Player *player);
void PlayerTakeDamage(Player *player, float damage);

#endif
