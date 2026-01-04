#ifndef WEAPON_H
#define WEAPON_H

#include "raylib.h"
#include "projectile.h"
#include <stdbool.h>

typedef enum WeaponType {
    WEAPON_PULSE_CANNON,    // Single shot, medium speed
    WEAPON_SPREAD_SHOT,     // 3-way spread
    WEAPON_HOMING_MISSILE,  // Slow, seeks enemies
    WEAPON_LIGHTNING,       // Chain to nearby enemies
    WEAPON_ORBIT_SHIELD,    // Rotating projectiles around player
    WEAPON_FLAMETHROWER,    // Rapid fire short range cone
    WEAPON_FREEZE_RAY,      // Slows enemies on hit
    WEAPON_BLACK_HOLE,      // Pulls enemies in
    WEAPON_COUNT
} WeaponType;

typedef struct Weapon {
    WeaponType type;
    float damage;
    float fireRate;
    float projectileSpeed;
    float projectileRadius;
    float projectileLifetime;
    int projectileCount;
    float cooldown;
    int level;
    bool pierce;            // Projectiles pass through enemies
    float spreadAngle;      // Angle between multi-shot projectiles
    int chainCount;         // For lightning - how many enemies to chain to
    float orbitSpawnAngle;  // For orbit shield - angle offset for next spawn
} Weapon;

void WeaponInit(Weapon *weapon, WeaponType type);
void WeaponUpdate(Weapon *weapon, float dt);
bool WeaponCanFire(Weapon *weapon);
void WeaponFire(Weapon *weapon, ProjectilePool *pool, Vector2 pos, Vector2 dir, Vector2 *ownerPosPtr);
const char* WeaponGetName(WeaponType type);
Color WeaponGetColor(WeaponType type);

#endif
