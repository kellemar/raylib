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
                    e->health = 30.0f;
                    e->maxHealth = 30.0f;
                    e->damage = 10.0f;
                    e->xpValue = 1;
                    e->orbitAngle = 0.0f;
                    e->orbitDistance = 0.0f;
                    e->splitCount = 0;
                    break;

                case ENEMY_ORBITER:
                    e->radius = 15.0f;
                    e->speed = 80.0f;
                    e->health = 50.0f;
                    e->maxHealth = 50.0f;
                    e->damage = 15.0f;
                    e->xpValue = 2;
                    e->orbitAngle = (float)(rand() % 360) * DEG2RAD;
                    e->orbitDistance = 200.0f + (float)(rand() % 100);
                    e->splitCount = 0;
                    break;

                case ENEMY_SPLITTER:
                    e->radius = 20.0f;
                    e->speed = 60.0f;
                    e->health = 80.0f;
                    e->maxHealth = 80.0f;
                    e->damage = 20.0f;
                    e->xpValue = 3;
                    e->orbitAngle = 0.0f;
                    e->orbitDistance = 0.0f;
                    e->splitCount = 2;
                    break;

                default:
                    e->radius = 12.0f;
                    e->speed = 100.0f;
                    e->health = 30.0f;
                    e->maxHealth = 30.0f;
                    e->damage = 10.0f;
                    e->xpValue = 1;
                    e->orbitAngle = 0.0f;
                    e->orbitDistance = 0.0f;
                    e->splitCount = 0;
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
            e->health = health;
            e->maxHealth = health;
            e->damage = 15.0f + (2 - splitCount) * 2.5f;
            e->xpValue = (splitCount > 0) ? 1 : 2;
            e->orbitAngle = 0.0f;
            e->orbitDistance = 0.0f;
            e->splitCount = splitCount;

            pool->count++;
            return e;
        }
    }
    return NULL;
}

void EnemyPoolUpdate(EnemyPool *pool, Vector2 playerPos, float dt)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        Enemy *e = &pool->enemies[i];
        if (!e->active) continue;

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

        switch (e->type)
        {
            case ENEMY_CHASER:
                DrawCircleV(e->pos, e->radius, NEON_RED);
                DrawCircleV(e->pos, e->radius * 0.6f, NEON_ORANGE);
                break;

            case ENEMY_ORBITER:
                DrawCircleV(e->pos, e->radius, NEON_CYAN);
                DrawCircleV(e->pos, e->radius * 0.6f, NEON_PINK);
                DrawCircleLinesV(e->pos, e->radius + 3.0f, NEON_CYAN);
                break;

            case ENEMY_SPLITTER:
                DrawCircleV(e->pos, e->radius, NEON_YELLOW);
                DrawCircleV(e->pos, e->radius * 0.7f, NEON_GREEN);
                DrawCircleV(e->pos, e->radius * 0.4f, NEON_YELLOW);
                break;

            default:
                DrawCircleV(e->pos, e->radius, NEON_RED);
                DrawCircleV(e->pos, e->radius * 0.6f, NEON_ORANGE);
                break;
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
