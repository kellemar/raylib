#include "projectile.h"
#include "enemy.h"
#include "raymath.h"
#include <stddef.h>
#include <math.h>

// Maximum distance for homing projectiles to acquire targets
#define HOMING_MAX_RANGE 500.0f

void ProjectilePoolInit(ProjectilePool *pool)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        pool->projectiles[i].active = false;
        pool->projectiles[i].activeIndex = -1;
        pool->freeIndices[i] = i;
    }
    pool->count = 0;
    pool->freeCount = MAX_PROJECTILES;
}

Projectile* ProjectileSpawn(ProjectilePool *pool, Vector2 pos, Vector2 vel, float damage, float radius, float lifetime)
{
    if (pool->freeCount <= 0) return NULL;

    int index = pool->freeIndices[pool->freeCount - 1];
    pool->freeCount--;

    Projectile *p = &pool->projectiles[index];
    p->pos = pos;
    p->vel = vel;
    p->damage = damage;
    p->radius = radius;
    p->lifetime = lifetime;
    p->weaponType = 0;
    p->pierce = false;
    p->active = true;
    p->activeIndex = pool->count;
    pool->activeIndices[pool->count] = index;
    pool->count++;
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
    // Initialize trail
    p->trailHead = 0;
    p->trailCount = 0;
    p->trailTimer = 0.0f;
    for (int t = 0; t < TRAIL_MAX_POINTS; t++)
    {
        p->trailPoints[t] = pos;
    }
    return p;
}

Projectile* ProjectileSpawnEx(ProjectilePool *pool, ProjectileSpawnParams *params)
{
    if (pool->freeCount <= 0) return NULL;

    int index = pool->freeIndices[pool->freeCount - 1];
    pool->freeCount--;

    Projectile *p = &pool->projectiles[index];
    p->pos = params->pos;
    p->vel = params->vel;
    p->damage = params->damage;
    p->radius = params->radius;
    p->lifetime = params->lifetime;
    p->weaponType = params->weaponType;
    p->pierce = params->pierce;
    p->active = true;
    p->activeIndex = pool->count;
    pool->activeIndices[pool->count] = index;
    pool->count++;
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
    // Initialize trail
    p->trailHead = 0;
    p->trailCount = 0;
    p->trailTimer = 0.0f;
    for (int t = 0; t < TRAIL_MAX_POINTS; t++)
    {
        p->trailPoints[t] = params->pos;
    }
    return p;
}

void ProjectileDeactivate(ProjectilePool *pool, int index)
{
    if (index < 0 || index >= MAX_PROJECTILES) return;
    if (!pool->projectiles[index].active) return;

    int removeSlot = pool->projectiles[index].activeIndex;
    int lastIndex = pool->activeIndices[pool->count - 1];

    pool->activeIndices[removeSlot] = lastIndex;
    pool->projectiles[lastIndex].activeIndex = removeSlot;
    pool->count--;

    pool->projectiles[index].active = false;
    pool->projectiles[index].activeIndex = -1;
    pool->freeIndices[pool->freeCount] = index;
    pool->freeCount++;
}

void ProjectilePoolUpdate(ProjectilePool *pool, float dt, struct EnemyPool *enemies, struct EnemySpatialGrid *grid)
{
    for (int i = 0; i < pool->count; )
    {
        int index = pool->activeIndices[i];
        Projectile *p = &pool->projectiles[index];
        if (!p->active)
        {
            ProjectileDeactivate(pool, index);
            continue;
        }

        switch (p->behavior)
        {
            case PROJ_BEHAVIOR_LINEAR:
                // Standard linear movement
                p->pos.x += p->vel.x * dt;
                p->pos.y += p->vel.y * dt;
                break;

            case PROJ_BEHAVIOR_HOMING:
            {
                // Each projectile finds its own nearest enemy
                Enemy *target = NULL;
                if (enemies != NULL)
                {
                    if (grid != NULL)
                    {
                        target = EnemyFindNearestInGrid(enemies, grid, p->pos, HOMING_MAX_RANGE);
                    }
                    else
                    {
                        target = EnemyFindNearest(enemies, p->pos, HOMING_MAX_RANGE);
                    }
                }

                if (target != NULL)
                {
                    Vector2 toTarget = Vector2Subtract(target->pos, p->pos);
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

        // Update trail (capture position at intervals)
        p->trailTimer += dt;
        if (p->trailTimer >= TRAIL_UPDATE_INTERVAL)
        {
            p->trailTimer = 0.0f;
            // Add current position to trail (circular buffer)
            p->trailPoints[p->trailHead] = p->pos;
            p->trailHead = (p->trailHead + 1) % TRAIL_MAX_POINTS;
            if (p->trailCount < TRAIL_MAX_POINTS)
            {
                p->trailCount++;
            }
        }

        p->lifetime -= dt;

        if (p->lifetime <= 0.0f)
        {
            ProjectileDeactivate(pool, index);
            continue;
        }

        i++;
    }
}

void ProjectilePoolDraw(ProjectilePool *pool, Rectangle view)
{
    for (int i = 0; i < pool->count; i++)
    {
        Projectile *p = &pool->projectiles[pool->activeIndices[i]];
        if (!p->active) continue;

        float cullRadius = p->radius;
        if (p->behavior == PROJ_BEHAVIOR_PULL)
        {
            cullRadius = p->radius * 3.0f;
        }
        if (p->pos.x + cullRadius < view.x || p->pos.x - cullRadius > view.x + view.width ||
            p->pos.y + cullRadius < view.y || p->pos.y - cullRadius > view.y + view.height)
        {
            continue;
        }

        // Draw glowing trail (behind projectile)
        if (p->trailCount > 1)
        {
            Vector2 prevPoint = p->pos;
            for (int t = 0; t < p->trailCount; t++)
            {
                // Read from circular buffer (newest to oldest)
                int idx = (p->trailHead - 1 - t + TRAIL_MAX_POINTS) % TRAIL_MAX_POINTS;
                Vector2 trailPoint = p->trailPoints[idx];

                // Fade based on age (older = more faded)
                float age = (float)(t + 1) / (float)TRAIL_MAX_POINTS;
                float alpha = (1.0f - age) * 0.6f;
                float thickness = p->radius * 2.0f * (1.0f - age * 0.7f);

                // Trail color with fade
                Color trailColor = p->color;
                trailColor.a = (unsigned char)(255 * alpha);

                // Draw line segment from previous to current trail point
                if (t > 0)
                {
                    DrawLineEx(prevPoint, trailPoint, thickness, trailColor);
                }
                prevPoint = trailPoint;
            }
        }

        // Draw with projectile's color
        DrawCircleV(p->pos, p->radius, p->color);

        // Inner glow (brighter center)
        Color innerColor = p->color;
        innerColor.r = (unsigned char)fminf(255, innerColor.r + 100);
        innerColor.g = (unsigned char)fminf(255, innerColor.g + 100);
        innerColor.b = (unsigned char)fminf(255, innerColor.b + 100);
        DrawCircleV(p->pos, p->radius * 0.5f, innerColor);

        // Outer glow halo
        DrawCircleV(p->pos, p->radius * 1.5f, Fade(p->color, 0.3f));

        // Special effects based on behavior
        if (p->behavior == PROJ_BEHAVIOR_PULL)
        {
            // Draw pull radius indicator
            DrawCircleLines((int)p->pos.x, (int)p->pos.y, p->radius * 3.0f, Fade(p->color, 0.3f));
        }
    }
}
