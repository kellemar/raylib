#include "projectile.h"
#include "raymath.h"
#include <stddef.h>
#include <math.h>

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
            // Initialize extended fields with defaults
            p->behavior = PROJ_BEHAVIOR_LINEAR;
            p->effects = PROJ_EFFECT_NONE;
            p->homingStrength = 0.0f;
            p->orbitAngle = 0.0f;
            p->orbitRadius = 0.0f;
            p->orbitSpeed = 0.0f;
            p->ownerPos = NULL;
            p->pullStrength = 0.0f;
            p->chainCount = 0;
            p->slowAmount = 0.0f;
            p->slowDuration = 0.0f;
            p->color = NEON_YELLOW;
            pool->count++;
            return p;
        }
    }
    return NULL;
}

Projectile* ProjectileSpawnEx(ProjectilePool *pool, ProjectileSpawnParams *params)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (!pool->projectiles[i].active)
        {
            Projectile *p = &pool->projectiles[i];
            p->pos = params->pos;
            p->vel = params->vel;
            p->damage = params->damage;
            p->radius = params->radius;
            p->lifetime = params->lifetime;
            p->weaponType = params->weaponType;
            p->pierce = params->pierce;
            p->active = true;
            p->behavior = params->behavior;
            p->effects = params->effects;
            p->homingStrength = params->homingStrength;
            p->orbitAngle = params->orbitAngle;
            p->orbitRadius = params->orbitRadius;
            p->orbitSpeed = params->orbitSpeed;
            p->ownerPos = params->ownerPos;
            p->pullStrength = params->pullStrength;
            p->chainCount = params->chainCount;
            p->slowAmount = params->slowAmount;
            p->slowDuration = params->slowDuration;
            p->color = params->color;
            pool->count++;
            return p;
        }
    }
    return NULL;
}

void ProjectilePoolUpdate(ProjectilePool *pool, float dt, Vector2 *nearestEnemyPos)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        Projectile *p = &pool->projectiles[i];
        if (!p->active) continue;

        switch (p->behavior)
        {
            case PROJ_BEHAVIOR_LINEAR:
                // Standard linear movement
                p->pos.x += p->vel.x * dt;
                p->pos.y += p->vel.y * dt;
                break;

            case PROJ_BEHAVIOR_HOMING:
            {
                // Move toward nearest enemy if available
                if (nearestEnemyPos != NULL)
                {
                    Vector2 toTarget = Vector2Subtract(*nearestEnemyPos, p->pos);
                    float dist = Vector2Length(toTarget);
                    if (dist > 1.0f)
                    {
                        Vector2 targetDir = Vector2Scale(toTarget, 1.0f / dist);
                        Vector2 currentDir = Vector2Normalize(p->vel);
                        float speed = Vector2Length(p->vel);

                        // Lerp toward target direction
                        Vector2 newDir = Vector2Lerp(currentDir, targetDir, p->homingStrength * dt);
                        newDir = Vector2Normalize(newDir);
                        p->vel = Vector2Scale(newDir, speed);
                    }
                }
                p->pos.x += p->vel.x * dt;
                p->pos.y += p->vel.y * dt;
                break;
            }

            case PROJ_BEHAVIOR_ORBIT:
            {
                // Orbit around owner position
                if (p->ownerPos != NULL)
                {
                    p->orbitAngle += p->orbitSpeed * dt;
                    p->pos.x = p->ownerPos->x + cosf(p->orbitAngle) * p->orbitRadius;
                    p->pos.y = p->ownerPos->y + sinf(p->orbitAngle) * p->orbitRadius;
                }
                break;
            }

            case PROJ_BEHAVIOR_PULL:
            {
                // Move slowly, pulling handled in collision check
                p->pos.x += p->vel.x * dt;
                p->pos.y += p->vel.y * dt;
                break;
            }
        }

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

        // Draw with projectile's color
        DrawCircleV(p->pos, p->radius, p->color);

        // Inner glow (brighter center)
        Color innerColor = p->color;
        innerColor.r = (unsigned char)fminf(255, innerColor.r + 100);
        innerColor.g = (unsigned char)fminf(255, innerColor.g + 100);
        innerColor.b = (unsigned char)fminf(255, innerColor.b + 100);
        DrawCircleV(p->pos, p->radius * 0.5f, innerColor);

        // Special effects based on behavior
        if (p->behavior == PROJ_BEHAVIOR_PULL)
        {
            // Draw pull radius indicator
            DrawCircleLines((int)p->pos.x, (int)p->pos.y, p->radius * 3.0f, Fade(p->color, 0.3f));
        }
    }
}
