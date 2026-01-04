#include "enemy.h"
#include "raymath.h"
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

void EnemyPoolInit(EnemyPool *pool)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        pool->enemies[i].active = false;
    }
    pool->count = 0;
}

Enemy* EnemySpawn(EnemyPool *pool, EnemyType type, Vector2 pos)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!pool->enemies[i].active)
        {
            Enemy *e = &pool->enemies[i];
            e->pos = pos;
            e->vel = (Vector2){ 0.0f, 0.0f };
            e->type = type;
            e->active = true;

            switch (type)
            {
                case ENEMY_CHASER:
                    e->radius = 12.0f;
                    e->speed = 100.0f;
                    e->baseSpeed = 100.0f;
                    e->health = 30.0f;
                    e->maxHealth = 30.0f;
                    e->damage = 10.0f;
                    e->xpValue = 1;
                    e->orbitAngle = 0.0f;
                    e->orbitDistance = 0.0f;
                    e->splitCount = 0;
                    e->hitFlashTimer = 0.0f;
                    e->slowTimer = 0.0f;
                    e->slowAmount = 0.0f;
                    e->isElite = false;
                    break;

                case ENEMY_ORBITER:
                    e->radius = 15.0f;
                    e->speed = 80.0f;
                    e->baseSpeed = 80.0f;
                    e->health = 50.0f;
                    e->maxHealth = 50.0f;
                    e->damage = 15.0f;
                    e->xpValue = 2;
                    e->orbitAngle = (float)(rand() % 360) * DEG2RAD;
                    e->orbitDistance = 200.0f + (float)(rand() % 100);
                    e->splitCount = 0;
                    e->hitFlashTimer = 0.0f;
                    e->slowTimer = 0.0f;
                    e->slowAmount = 0.0f;
                    e->isElite = false;
                    break;

                case ENEMY_SPLITTER:
                    e->radius = 20.0f;
                    e->speed = 60.0f;
                    e->baseSpeed = 60.0f;
                    e->health = 80.0f;
                    e->maxHealth = 80.0f;
                    e->damage = 20.0f;
                    e->xpValue = 3;
                    e->orbitAngle = 0.0f;
                    e->orbitDistance = 0.0f;
                    e->splitCount = 2;
                    e->hitFlashTimer = 0.0f;
                    e->slowTimer = 0.0f;
                    e->slowAmount = 0.0f;
                    e->isElite = false;
                    break;

                default:
                    e->radius = 12.0f;
                    e->speed = 100.0f;
                    e->baseSpeed = 100.0f;
                    e->health = 30.0f;
                    e->maxHealth = 30.0f;
                    e->damage = 10.0f;
                    e->xpValue = 1;
                    e->orbitAngle = 0.0f;
                    e->orbitDistance = 0.0f;
                    e->splitCount = 0;
                    e->hitFlashTimer = 0.0f;
                    e->slowTimer = 0.0f;
                    e->slowAmount = 0.0f;
                    e->isElite = false;
                    break;
            }

            pool->count++;
            return e;
        }
    }
    return NULL;
}

Enemy* EnemySpawnSplitterChild(EnemyPool *pool, Vector2 pos, int splitCount, float radius, float health)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!pool->enemies[i].active)
        {
            Enemy *e = &pool->enemies[i];
            e->pos = pos;
            e->vel = (Vector2){ 0.0f, 0.0f };
            e->type = ENEMY_SPLITTER;
            e->active = true;
            e->radius = radius;
            e->speed = 60.0f + (2 - splitCount) * 15.0f;
            e->baseSpeed = e->speed;
            e->health = health;
            e->maxHealth = health;
            e->damage = 15.0f + (2 - splitCount) * 2.5f;
            e->xpValue = (splitCount > 0) ? 1 : 2;
            e->orbitAngle = 0.0f;
            e->orbitDistance = 0.0f;
            e->splitCount = splitCount;
            e->hitFlashTimer = 0.0f;
            e->slowTimer = 0.0f;
            e->slowAmount = 0.0f;
            e->isElite = false;  // Splitter children are never elite

            pool->count++;
            return e;
        }
    }
    return NULL;
}

Enemy* EnemySpawnElite(EnemyPool *pool, EnemyType type, Vector2 pos)
{
    Enemy *e = EnemySpawn(pool, type, pos);
    if (e)
    {
        // Apply elite multipliers
        e->isElite = true;
        e->radius *= ELITE_SIZE_MULT;
        e->health *= ELITE_HEALTH_MULT;
        e->maxHealth *= ELITE_HEALTH_MULT;
        e->damage *= ELITE_DAMAGE_MULT;
        e->xpValue *= ELITE_XP_MULT;
        e->speed *= ELITE_SPEED_MULT;
        e->baseSpeed *= ELITE_SPEED_MULT;
    }
    return e;
}

void EnemyApplySlow(Enemy *enemy, float amount, float duration)
{
    if (!enemy || !enemy->active) return;

    // Validate inputs
    if (amount <= 0.0f || duration <= 0.0f) return;

    // Clamp slow amount to valid range (0.0 - 1.0)
    if (amount > 1.0f) amount = 1.0f;

    // Apply new slow if stronger or refresh existing
    if (amount >= enemy->slowAmount || enemy->slowTimer <= 0.0f)
    {
        enemy->slowAmount = amount;
        enemy->slowTimer = duration;
        enemy->speed = enemy->baseSpeed * (1.0f - amount);
    }
    else if (duration > enemy->slowTimer)
    {
        // Just refresh duration if new slow isn't stronger
        enemy->slowTimer = duration;
    }
}

Enemy* EnemyFindNearest(EnemyPool *pool, Vector2 pos, float maxDistance)
{
    Enemy *nearest = NULL;
    float nearestDist = maxDistance * maxDistance;  // Use squared distance for efficiency

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        Enemy *e = &pool->enemies[i];
        if (!e->active) continue;

        float dx = e->pos.x - pos.x;
        float dy = e->pos.y - pos.y;
        float distSq = dx * dx + dy * dy;

        if (distSq < nearestDist)
        {
            nearestDist = distSq;
            nearest = e;
        }
    }

    return nearest;
}

void EnemyPoolUpdate(EnemyPool *pool, Vector2 playerPos, float dt)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        Enemy *e = &pool->enemies[i];
        if (!e->active) continue;

        // Update hit flash timer
        if (e->hitFlashTimer > 0.0f)
        {
            e->hitFlashTimer -= dt;
        }

        // Update slow effect
        if (e->slowTimer > 0.0f)
        {
            e->slowTimer -= dt;
            if (e->slowTimer <= 0.0f)
            {
                // Slow expired, restore speed
                e->slowAmount = 0.0f;
                e->speed = e->baseSpeed;
            }
        }

        switch (e->type)
        {
            case ENEMY_CHASER:
            {
                Vector2 toPlayer = Vector2Subtract(playerPos, e->pos);
                float distance = Vector2Length(toPlayer);
                if (distance > 0.0f)
                {
                    Vector2 dir = Vector2Scale(toPlayer, 1.0f / distance);
                    e->vel = Vector2Scale(dir, e->speed);
                }
                break;
            }

            case ENEMY_ORBITER:
            {
                e->orbitAngle += e->speed * 0.01f * dt;
                e->orbitDistance -= 10.0f * dt;
                if (e->orbitDistance < 50.0f) e->orbitDistance = 50.0f;

                float targetX = playerPos.x + cosf(e->orbitAngle) * e->orbitDistance;
                float targetY = playerPos.y + sinf(e->orbitAngle) * e->orbitDistance;
                Vector2 targetPos = { targetX, targetY };

                Vector2 toTarget = Vector2Subtract(targetPos, e->pos);
                float dist = Vector2Length(toTarget);
                if (dist > 5.0f)
                {
                    Vector2 dir = Vector2Scale(toTarget, 1.0f / dist);
                    e->vel = Vector2Scale(dir, e->speed * 2.0f);
                }
                else
                {
                    e->vel = (Vector2){ 0.0f, 0.0f };
                }
                break;
            }

            case ENEMY_SPLITTER:
            {
                Vector2 toPlayer = Vector2Subtract(playerPos, e->pos);
                float distance = Vector2Length(toPlayer);
                if (distance > 0.0f)
                {
                    Vector2 dir = Vector2Scale(toPlayer, 1.0f / distance);
                    e->vel = Vector2Scale(dir, e->speed);
                }
                break;
            }

            default:
            {
                Vector2 toPlayer = Vector2Subtract(playerPos, e->pos);
                float distance = Vector2Length(toPlayer);
                if (distance > 0.0f)
                {
                    Vector2 dir = Vector2Scale(toPlayer, 1.0f / distance);
                    e->vel = Vector2Scale(dir, e->speed);
                }
                break;
            }
        }

        e->pos = Vector2Add(e->pos, Vector2Scale(e->vel, dt));
    }
}

void EnemyPoolDraw(EnemyPool *pool)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        Enemy *e = &pool->enemies[i];
        if (!e->active) continue;

        // Check if hit flash is active
        bool isFlashing = e->hitFlashTimer > 0.0f;
        bool isSlowed = e->slowTimer > 0.0f;

        // Draw elite glow effect (behind enemy)
        if (e->isElite && !isFlashing)
        {
            // Pulsing gold glow
            float pulse = sinf((float)GetTime() * 4.0f) * 0.3f + 0.7f;
            float glowRadius = e->radius + 8.0f * pulse;
            Color glowColor = (Color){ 255, 215, 0, (unsigned char)(100 * pulse) };  // Gold glow
            DrawCircleV(e->pos, glowRadius + 4.0f, (Color){ 255, 215, 0, 50 });
            DrawCircleV(e->pos, glowRadius, glowColor);
        }

        if (isFlashing)
        {
            // Draw white flash
            DrawCircleV(e->pos, e->radius, WHITE);
            DrawCircleV(e->pos, e->radius * 0.6f, (Color){ 255, 255, 255, 200 });
        }
        else
        {
            // Apply blue tint if slowed
            Color outerColor, innerColor;

            switch (e->type)
            {
                case ENEMY_CHASER:
                    outerColor = NEON_RED;
                    innerColor = NEON_ORANGE;
                    break;

                case ENEMY_ORBITER:
                    outerColor = NEON_CYAN;
                    innerColor = NEON_PINK;
                    break;

                case ENEMY_SPLITTER:
                    outerColor = NEON_YELLOW;
                    innerColor = NEON_GREEN;
                    break;

                default:
                    outerColor = NEON_RED;
                    innerColor = NEON_ORANGE;
                    break;
            }

            // Apply ice-blue tint when slowed
            if (isSlowed)
            {
                outerColor = (Color){ 150, 200, 255, 255 };  // Ice blue
                innerColor = (Color){ 200, 230, 255, 255 };  // Lighter ice blue
            }

            DrawCircleV(e->pos, e->radius, outerColor);
            DrawCircleV(e->pos, e->radius * 0.6f, innerColor);

            // Additional decorations for specific types
            if (!isSlowed)
            {
                if (e->type == ENEMY_ORBITER)
                {
                    DrawCircleLinesV(e->pos, e->radius + 3.0f, NEON_CYAN);
                }
                else if (e->type == ENEMY_SPLITTER)
                {
                    DrawCircleV(e->pos, e->radius * 0.4f, NEON_YELLOW);
                }
            }
            else
            {
                // Draw ice ring when slowed
                DrawCircleLinesV(e->pos, e->radius + 2.0f, (Color){ 150, 200, 255, 150 });
            }

            // Draw elite crown/border
            if (e->isElite)
            {
                DrawCircleLinesV(e->pos, e->radius + 2.0f, (Color){ 255, 215, 0, 255 });  // Gold border
                DrawCircleLinesV(e->pos, e->radius + 4.0f, (Color){ 255, 200, 50, 180 });  // Outer gold border
            }
        }

        if (e->health < e->maxHealth)
        {
            float healthBarWidth = e->radius * 2.0f;
            float healthBarHeight = 4.0f;
            float healthBarX = e->pos.x - healthBarWidth / 2.0f;
            float healthBarY = e->pos.y - e->radius - 8.0f;
            float healthRatio = e->health / e->maxHealth;

            DrawRectangle((int)healthBarX, (int)healthBarY, (int)healthBarWidth, (int)healthBarHeight, (Color){ 80, 20, 20, 255 });
            DrawRectangle((int)healthBarX, (int)healthBarY, (int)(healthBarWidth * healthRatio), (int)healthBarHeight, NEON_GREEN);
        }
    }
}
