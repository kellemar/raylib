#include "enemy.h"
#include "raymath.h"
#include <stddef.h>

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
                default:
                    e->radius = 12.0f;
                    e->speed = 100.0f;
                    e->health = 30.0f;
                    e->maxHealth = 30.0f;
                    e->damage = 10.0f;
                    e->xpValue = 1;
                    break;
            }

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

        DrawCircleV(e->pos, e->radius, NEON_RED);
        DrawCircleV(e->pos, e->radius * 0.6f, NEON_ORANGE);

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
