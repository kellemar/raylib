#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "weapon.h"
#include <stdbool.h>

#define PLAYER_TRAIL_LENGTH 5

typedef struct Player {
    Vector2 pos;
    Vector2 vel;
    Vector2 aimDir;
    float radius;
    float speed;
    float health;
    float maxHealth;
    float invincibilityTimer;
    float magnetRadius;
    int level;
    int xp;
    int xpToNextLevel;
    bool alive;
    Weapon weapon;
    // Trail effect
    Vector2 trailPositions[PLAYER_TRAIL_LENGTH];
    float trailUpdateTimer;
    // Dash ability
    float dashCooldown;        // Time until dash available again
    float dashTimer;           // Time remaining in current dash
    bool isDashing;            // Currently performing dash
    Vector2 dashDir;           // Direction of dash
} Player;

void PlayerInit(Player *player);
void PlayerUpdate(Player *player, float dt, ProjectilePool *projectiles, Camera2D camera);
void PlayerDraw(Player *player);
void PlayerTakeDamage(Player *player, float damage);

#endif
