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
    // Upgrade stats
    float armor;               // Flat damage reduction
    float regen;               // HP per second
    float regenTimer;          // Accumulator for regen
    float xpMultiplier;        // XP gain multiplier (1.0 = normal)
    float knockbackMultiplier; // Enemy knockback multiplier (1.0 = normal)
    float dashDamage;          // Damage dealt while dashing
    float vampirism;           // Lifesteal percentage (0.0 - 1.0)
    float slowAuraRadius;      // Radius of slow aura around player
    float slowAuraAmount;      // Slow intensity (0.0 - 1.0)
    // Evolution tracking
    unsigned int acquiredUpgrades;  // Bitfield of acquired upgrade types
} Player;

void PlayerInit(Player *player);
void PlayerUpdate(Player *player, float dt, ProjectilePool *projectiles, Camera2D camera);
void PlayerDraw(Player *player);
void PlayerTakeDamage(Player *player, float damage);
void PlayerSwitchWeapon(Player *player, WeaponType type);
void PlayerCycleWeapon(Player *player, int direction);  // +1 next, -1 prev

// Upgrade tracking for evolution
void PlayerMarkUpgradeAcquired(Player *player, int upgradeType);
bool PlayerHasUpgrade(Player *player, int upgradeType);
bool PlayerCanEvolveWeapon(Player *player);
void PlayerEvolveWeapon(Player *player);

#endif
