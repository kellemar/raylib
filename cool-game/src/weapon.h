#ifndef WEAPON_H
#define WEAPON_H

#include "raylib.h"
#include "projectile.h"
#include <stdbool.h>

typedef enum WeaponType {
    WEAPON_PULSE_CANNON,
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
} Weapon;

void WeaponInit(Weapon *weapon, WeaponType type);
void WeaponUpdate(Weapon *weapon, float dt);
bool WeaponCanFire(Weapon *weapon);
void WeaponFire(Weapon *weapon, ProjectilePool *pool, Vector2 pos, Vector2 dir);

#endif
