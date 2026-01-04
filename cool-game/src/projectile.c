#include "projectile.h"
#include <stddef.h>

void ProjectilePoolInit(ProjectilePool *pool)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        pool->projectiles[i].active = false;
    }
    pool->count = 0;
}

Projectile* ProjectileSpawn(ProjectilePool *pool, Vector2 pos, Vector2 vel, float damage, float radius, float lifetime)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (!pool->projectiles[i].active)
        {
            Projectile *p = &pool->projectiles[i];
            p->pos = pos;
            p->vel = vel;
            p->damage = damage;
            p->radius = radius;
            p->lifetime = lifetime;
            p->weaponType = 0;
            p->pierce = false;
            p->active = true;
            pool->count++;
            return p;
        }
    }
    return NULL;
}

void ProjectilePoolUpdate(ProjectilePool *pool, float dt)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        Projectile *p = &pool->projectiles[i];
        if (!p->active) continue;

        p->pos.x += p->vel.x * dt;
        p->pos.y += p->vel.y * dt;
        p->lifetime -= dt;

        if (p->lifetime <= 0.0f)
        {
            p->active = false;
            pool->count--;
        }
    }
}

void ProjectilePoolDraw(ProjectilePool *pool)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        Projectile *p = &pool->projectiles[i];
        if (!p->active) continue;

        DrawCircleV(p->pos, p->radius, NEON_YELLOW);
        DrawCircleV(p->pos, p->radius * 0.5f, NEON_WHITE);
    }
}
