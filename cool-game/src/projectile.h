#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "raylib.h"
#include "types.h"
#include <stdbool.h>

typedef struct Projectile {
    Vector2 pos;
    Vector2 vel;
    float radius;
    float damage;
    float lifetime;
    int weaponType;
    bool pierce;
    bool active;
} Projectile;

typedef struct ProjectilePool {
    Projectile projectiles[MAX_PROJECTILES];
    int count;
} ProjectilePool;

void ProjectilePoolInit(ProjectilePool *pool);
void ProjectilePoolUpdate(ProjectilePool *pool, float dt);
void ProjectilePoolDraw(ProjectilePool *pool);
Projectile* ProjectileSpawn(ProjectilePool *pool, Vector2 pos, Vector2 vel, float damage, float radius, float lifetime);

#endif
