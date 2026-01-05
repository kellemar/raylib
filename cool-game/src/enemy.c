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
        pool->enemies[i].activeIndex = -1;
        pool->freeIndices[i] = i;
    }
    pool->count = 0;
    pool->freeCount = MAX_ENEMIES;
}

Enemy* EnemySpawn(EnemyPool *pool, EnemyType type, Vector2 pos)
{
    if (pool->freeCount <= 0) return NULL;

    int index = pool->freeIndices[pool->freeCount - 1];
    pool->freeCount--;

    Enemy *e = &pool->enemies[index];
    e->pos = pos;
    e->vel = (Vector2){ 0.0f, 0.0f };
    e->type = type;
    e->active = true;
    e->activeIndex = pool->count;
    pool->activeIndices[pool->count] = index;
    pool->count++;

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
            e->isBoss = false;
            e->bossPhase = 0;
            e->bossAttackTimer = 0.0f;
            e->bossChargeTimer = 0.0f;
            e->bossCharging = false;
            e->spawnTimer = SPAWN_EFFECT_DURATION;
            e->spawnDuration = SPAWN_EFFECT_DURATION;
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
            e->isBoss = false;
            e->bossPhase = 0;
            e->bossAttackTimer = 0.0f;
            e->bossChargeTimer = 0.0f;
            e->bossCharging = false;
            e->spawnTimer = SPAWN_EFFECT_DURATION;
            e->spawnDuration = SPAWN_EFFECT_DURATION;
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
            e->isBoss = false;
            e->bossPhase = 0;
            e->bossAttackTimer = 0.0f;
            e->bossChargeTimer = 0.0f;
            e->bossCharging = false;
            e->spawnTimer = SPAWN_EFFECT_DURATION;
            e->spawnDuration = SPAWN_EFFECT_DURATION;
            break;

        case ENEMY_BOSS:
            e->radius = BOSS_BASE_RADIUS;
            e->speed = BOSS_BASE_SPEED;
            e->baseSpeed = BOSS_BASE_SPEED;
            e->health = BOSS_BASE_HEALTH;
            e->maxHealth = BOSS_BASE_HEALTH;
            e->damage = BOSS_BASE_DAMAGE;
            e->xpValue = BOSS_XP_VALUE;
            e->orbitAngle = 0.0f;
            e->orbitDistance = 0.0f;
            e->splitCount = 0;
            e->hitFlashTimer = 0.0f;
            e->slowTimer = 0.0f;
            e->slowAmount = 0.0f;
            e->isElite = false;
            e->isBoss = true;
            e->bossPhase = 0;
            e->bossAttackTimer = BOSS_ATTACK_INTERVAL;
            e->bossChargeTimer = 0.0f;
            e->bossCharging = false;
            e->spawnTimer = SPAWN_EFFECT_DURATION * 2.0f;  // Boss has longer spawn
            e->spawnDuration = SPAWN_EFFECT_DURATION * 2.0f;
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
            e->isBoss = false;
            e->bossPhase = 0;
            e->bossAttackTimer = 0.0f;
            e->bossChargeTimer = 0.0f;
            e->bossCharging = false;
            e->spawnTimer = SPAWN_EFFECT_DURATION;
            e->spawnDuration = SPAWN_EFFECT_DURATION;
            break;
    }

    return e;
}

Enemy* EnemySpawnSplitterChild(EnemyPool *pool, Vector2 pos, int splitCount, float radius, float health)
{
    if (pool->freeCount <= 0) return NULL;

    int index = pool->freeIndices[pool->freeCount - 1];
    pool->freeCount--;

    Enemy *e = &pool->enemies[index];
    e->pos = pos;
    e->vel = (Vector2){ 0.0f, 0.0f };
    e->type = ENEMY_SPLITTER;
    e->active = true;
    e->activeIndex = pool->count;
    pool->activeIndices[pool->count] = index;
    pool->count++;

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
    e->isBoss = false;
    e->bossPhase = 0;
    e->bossAttackTimer = 0.0f;
    e->bossChargeTimer = 0.0f;
    e->bossCharging = false;
    e->spawnTimer = SPAWN_EFFECT_DURATION * 0.3f;  // Quick spawn for split children
    e->spawnDuration = SPAWN_EFFECT_DURATION * 0.3f;

    return e;
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

Enemy* EnemySpawnBoss(EnemyPool *pool, Vector2 pos, int bossNumber)
{
    Enemy *e = EnemySpawn(pool, ENEMY_BOSS, pos);
    if (e)
    {
        // Scale boss stats based on boss number (gets harder each time)
        float scaleFactor = 1.0f + (bossNumber - 1) * 0.5f;  // 50% stronger each boss
        e->health *= scaleFactor;
        e->maxHealth *= scaleFactor;
        e->damage *= scaleFactor;
        e->xpValue = BOSS_XP_VALUE * bossNumber;  // More XP for later bosses
    }
    return e;
}

bool EnemyPoolHasBoss(EnemyPool *pool)
{
    for (int i = 0; i < pool->count; i++)
    {
        Enemy *e = &pool->enemies[pool->activeIndices[i]];
        if (e->active && e->isBoss) return true;
    }
    return false;
}

Enemy* EnemyPoolGetBoss(EnemyPool *pool)
{
    for (int i = 0; i < pool->count; i++)
    {
        Enemy *e = &pool->enemies[pool->activeIndices[i]];
        if (e->active && e->isBoss) return e;
    }
    return NULL;
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

    for (int i = 0; i < pool->count; i++)
    {
        Enemy *e = &pool->enemies[pool->activeIndices[i]];
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

void EnemyDeactivate(EnemyPool *pool, int index)
{
    if (index < 0 || index >= MAX_ENEMIES) return;
    if (!pool->enemies[index].active) return;

    int removeSlot = pool->enemies[index].activeIndex;
    int lastIndex = pool->activeIndices[pool->count - 1];

    pool->activeIndices[removeSlot] = lastIndex;
    pool->enemies[lastIndex].activeIndex = removeSlot;
    pool->count--;

    pool->enemies[index].active = false;
    pool->enemies[index].activeIndex = -1;
    pool->freeIndices[pool->freeCount] = index;
    pool->freeCount++;
}

static int EnemySpatialCellCoord(float value)
{
    return (int)floorf(value / ENEMY_SPATIAL_CELL_SIZE);
}

static unsigned int EnemySpatialHash(int cellX, int cellY)
{
    unsigned int hash = (unsigned int)(cellX * 73856093) ^ (unsigned int)(cellY * 19349663);
    return hash & (ENEMY_SPATIAL_BUCKETS - 1);
}

void EnemySpatialGridBuild(EnemySpatialGrid *grid, EnemyPool *pool)
{
    for (int i = 0; i < ENEMY_SPATIAL_BUCKETS; i++)
    {
        grid->bucketHeads[i] = -1;
    }

    for (int i = 0; i < pool->count; i++)
    {
        int index = pool->activeIndices[i];
        Enemy *e = &pool->enemies[index];
        if (!e->active) continue;

        int cellX = EnemySpatialCellCoord(e->pos.x);
        int cellY = EnemySpatialCellCoord(e->pos.y);

        grid->cellX[index] = cellX;
        grid->cellY[index] = cellY;

        unsigned int hash = EnemySpatialHash(cellX, cellY);
        grid->next[index] = grid->bucketHeads[hash];
        grid->bucketHeads[hash] = index;
    }
}

void EnemySpatialGridForEachInRadius(EnemySpatialGrid *grid, EnemyPool *pool, Vector2 center, float radius, EnemySpatialVisit visit, void *user)
{
    float radiusSq = radius * radius;
    int minCellX = EnemySpatialCellCoord(center.x - radius);
    int maxCellX = EnemySpatialCellCoord(center.x + radius);
    int minCellY = EnemySpatialCellCoord(center.y - radius);
    int maxCellY = EnemySpatialCellCoord(center.y + radius);

    bool stop = false;

    for (int cellY = minCellY; cellY <= maxCellY && !stop; cellY++)
    {
        for (int cellX = minCellX; cellX <= maxCellX && !stop; cellX++)
        {
            unsigned int hash = EnemySpatialHash(cellX, cellY);
            for (int index = grid->bucketHeads[hash]; index != -1; index = grid->next[index])
            {
                if (grid->cellX[index] != cellX || grid->cellY[index] != cellY) continue;

                Enemy *e = &pool->enemies[index];
                if (!e->active) continue;

                float dx = e->pos.x - center.x;
                float dy = e->pos.y - center.y;
                float distSq = dx * dx + dy * dy;

                if (distSq <= radiusSq)
                {
                    if (!visit(e, index, user))
                    {
                        stop = true;
                        break;
                    }
                }
            }
        }
    }
}

Enemy* EnemyFindNearestInGrid(EnemyPool *pool, EnemySpatialGrid *grid, Vector2 pos, float maxDistance)
{
    Enemy *nearest = NULL;
    float nearestDist = maxDistance * maxDistance;

    int minCellX = EnemySpatialCellCoord(pos.x - maxDistance);
    int maxCellX = EnemySpatialCellCoord(pos.x + maxDistance);
    int minCellY = EnemySpatialCellCoord(pos.y - maxDistance);
    int maxCellY = EnemySpatialCellCoord(pos.y + maxDistance);

    for (int cellY = minCellY; cellY <= maxCellY; cellY++)
    {
        for (int cellX = minCellX; cellX <= maxCellX; cellX++)
        {
            unsigned int hash = EnemySpatialHash(cellX, cellY);
            for (int index = grid->bucketHeads[hash]; index != -1; index = grid->next[index])
            {
                if (grid->cellX[index] != cellX || grid->cellY[index] != cellY) continue;

                Enemy *e = &pool->enemies[index];
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
        }
    }

    return nearest;
}

void EnemyPoolUpdate(EnemyPool *pool, Vector2 playerPos, float dt)
{
    for (int i = 0; i < pool->count; i++)
    {
        Enemy *e = &pool->enemies[pool->activeIndices[i]];
        if (!e->active) continue;

        // Update spawn animation timer
        if (e->spawnTimer > 0.0f)
        {
            e->spawnTimer -= dt;
            continue;  // Don't move or act while spawning
        }

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

            case ENEMY_BOSS:
            {
                // Boss AI: Slowly chase player, periodically charge and dash
                Vector2 toPlayer = Vector2Subtract(playerPos, e->pos);
                float distance = Vector2Length(toPlayer);

                // Update boss attack timer
                if (!e->bossCharging)
                {
                    e->bossAttackTimer -= dt;
                    if (e->bossAttackTimer <= 0.0f)
                    {
                        // Start charging an attack
                        e->bossCharging = true;
                        e->bossChargeTimer = BOSS_CHARGE_TIME;
                        e->vel = (Vector2){ 0.0f, 0.0f };  // Stop while charging
                    }
                    else
                    {
                        // Normal movement: slowly chase player
                        if (distance > 0.0f)
                        {
                            Vector2 dir = Vector2Scale(toPlayer, 1.0f / distance);
                            e->vel = Vector2Scale(dir, e->speed);
                        }
                    }
                }
                else
                {
                    // Charging up attack
                    e->bossChargeTimer -= dt;
                    if (e->bossChargeTimer <= 0.0f)
                    {
                        // Execute attack: dash toward player
                        if (distance > 0.0f)
                        {
                            Vector2 dir = Vector2Scale(toPlayer, 1.0f / distance);
                            e->vel = Vector2Scale(dir, e->speed * 8.0f);  // Fast dash
                        }
                        e->bossCharging = false;
                        e->bossAttackTimer = BOSS_ATTACK_INTERVAL;
                        // Cycle through attack phases
                        e->bossPhase = (e->bossPhase + 1) % 3;
                    }
                    else
                    {
                        // Hold still while charging (with slight vibration)
                        float shake = sinf(e->bossChargeTimer * 50.0f) * 2.0f;
                        e->vel = (Vector2){ shake, shake };
                    }
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

void EnemyPoolDraw(EnemyPool *pool, Rectangle view)
{
    for (int i = 0; i < pool->count; i++)
    {
        Enemy *e = &pool->enemies[pool->activeIndices[i]];
        if (!e->active) continue;

        float cullRadius = e->radius;
        if (e->isBoss) cullRadius += 35.0f;
        else if (e->isElite) cullRadius += 12.0f;

        if (e->pos.x + cullRadius < view.x || e->pos.x - cullRadius > view.x + view.width ||
            e->pos.y + cullRadius < view.y || e->pos.y - cullRadius > view.y + view.height)
        {
            continue;
        }

        // Check if hit flash is active
        bool isFlashing = e->hitFlashTimer > 0.0f;
        bool isSlowed = e->slowTimer > 0.0f;
        bool isSpawning = e->spawnTimer > 0.0f;

        // Draw spawn effect (glitch/ripple animation)
        if (isSpawning)
        {
            float spawnProgress = 1.0f - (e->spawnTimer / e->spawnDuration);
            float time = (float)GetTime();

            // Glitch-style distortion rings
            for (int ring = 0; ring < 3; ring++)
            {
                float ringProgress = fmodf(spawnProgress + ring * 0.2f, 1.0f);
                float ringRadius = e->radius * (1.5f + ring * 0.8f) * ringProgress;
                float ringAlpha = (1.0f - ringProgress) * 0.6f;

                // Glitchy offset
                float glitchX = sinf(time * 20.0f + ring) * 3.0f * (1.0f - spawnProgress);
                float glitchY = cosf(time * 25.0f + ring * 2.0f) * 3.0f * (1.0f - spawnProgress);
                Vector2 glitchPos = { e->pos.x + glitchX, e->pos.y + glitchY };

                Color ringColor = e->isBoss ? (Color){ 180, 50, 180, (unsigned char)(255 * ringAlpha) }
                                            : (Color){ 255, 255, 255, (unsigned char)(255 * ringAlpha) };
                DrawCircleLinesV(glitchPos, ringRadius, ringColor);
            }

            // Central flickering silhouette
            float flicker = (sinf(time * 40.0f) > 0.0f) ? 1.0f : 0.5f;
            float silhouetteRadius = e->radius * spawnProgress * flicker;
            Color silhouetteColor = { 255, 255, 255, (unsigned char)(100 * spawnProgress) };
            DrawCircleV(e->pos, silhouetteRadius, silhouetteColor);

            // Scanline glitch effect (horizontal lines)
            if (spawnProgress < 0.8f)
            {
                int numLines = 5;
                for (int line = 0; line < numLines; line++)
                {
                    float lineY = e->pos.y - e->radius + (e->radius * 2.0f * line / numLines);
                    float lineOffset = sinf(time * 30.0f + line * 3.0f) * 8.0f * (1.0f - spawnProgress);
                    float lineLen = e->radius * 0.8f;
                    DrawLineEx(
                        (Vector2){ e->pos.x - lineLen + lineOffset, lineY },
                        (Vector2){ e->pos.x + lineLen + lineOffset, lineY },
                        2.0f,
                        (Color){ 255, 255, 255, (unsigned char)(80 * (1.0f - spawnProgress)) }
                    );
                }
            }

            continue;  // Don't draw the normal enemy yet
        }

        // Draw boss glow effect (behind enemy) - purple pulsing aura
        if (e->isBoss && !isFlashing)
        {
            float pulse = sinf((float)GetTime() * 3.0f) * 0.4f + 0.6f;
            float glowRadius = e->radius + 20.0f * pulse;

            // Outer dark purple glow
            DrawCircleV(e->pos, glowRadius + 15.0f, (Color){ 80, 0, 80, 40 });
            DrawCircleV(e->pos, glowRadius + 8.0f, (Color){ 128, 0, 128, 60 });
            DrawCircleV(e->pos, glowRadius, (Color){ 180, 50, 180, (unsigned char)(80 * pulse) });

            // Warning indicator when charging
            if (e->bossCharging)
            {
                // Flashing red warning
                float flashRate = (BOSS_CHARGE_TIME - e->bossChargeTimer) / BOSS_CHARGE_TIME;
                float flashIntensity = (sinf(flashRate * 30.0f) + 1.0f) * 0.5f;
                Color warningColor = (Color){ 255, 50, 50, (unsigned char)(200 * flashIntensity) };
                DrawCircleLinesV(e->pos, e->radius + 10.0f + flashRate * 20.0f, warningColor);
                DrawCircleLinesV(e->pos, e->radius + 15.0f + flashRate * 25.0f, warningColor);
            }
        }
        // Draw elite glow effect (behind enemy)
        else if (e->isElite && !isFlashing)
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

                case ENEMY_BOSS:
                    outerColor = (Color){ 128, 0, 128, 255 };   // Deep purple
                    innerColor = (Color){ 200, 50, 200, 255 };  // Bright magenta
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

            // Boss specific decorations
            if (e->isBoss && !isSlowed)
            {
                // Draw skull-like pattern or menacing eyes
                float eyeOffset = e->radius * 0.3f;
                float eyeRadius = e->radius * 0.15f;
                Vector2 leftEye = { e->pos.x - eyeOffset, e->pos.y - eyeOffset * 0.5f };
                Vector2 rightEye = { e->pos.x + eyeOffset, e->pos.y - eyeOffset * 0.5f };
                DrawCircleV(leftEye, eyeRadius, (Color){ 255, 0, 0, 255 });
                DrawCircleV(rightEye, eyeRadius, (Color){ 255, 0, 0, 255 });

                // Crown-like spikes on top
                for (int spike = 0; spike < 5; spike++)
                {
                    float angle = PI + (spike - 2) * 0.3f;
                    float spikeLen = e->radius * 0.4f;
                    Vector2 spikeEnd = {
                        e->pos.x + cosf(angle) * (e->radius + spikeLen),
                        e->pos.y + sinf(angle) * (e->radius + spikeLen)
                    };
                    Vector2 spikeBase = {
                        e->pos.x + cosf(angle) * e->radius,
                        e->pos.y + sinf(angle) * e->radius
                    };
                    DrawLineEx(spikeBase, spikeEnd, 3.0f, (Color){ 200, 50, 200, 255 });
                }

                // Multiple purple rings
                DrawCircleLinesV(e->pos, e->radius + 3.0f, (Color){ 180, 50, 180, 255 });
                DrawCircleLinesV(e->pos, e->radius + 6.0f, (Color){ 128, 0, 128, 200 });
                DrawCircleLinesV(e->pos, e->radius + 9.0f, (Color){ 80, 0, 80, 150 });
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
