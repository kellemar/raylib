#include "xp.h"
#include "utils.h"
#include <math.h>

#define XP_CRYSTAL_RADIUS   6.0f
#define XP_CRYSTAL_LIFETIME 30.0f
#define XP_MAGNET_SPEED     400.0f
#define XP_COLLECT_RADIUS   15.0f

void XPPoolInit(XPPool *pool)
{
    for (int i = 0; i < MAX_XP_CRYSTALS; i++)
    {
        pool->crystals[i].active = false;
    }
    pool->count = 0;
}

void XPPoolUpdate(XPPool *pool, Vector2 playerPos, float magnetRadius, float dt)
{
    for (int i = 0; i < MAX_XP_CRYSTALS; i++)
    {
        XPCrystal *xp = &pool->crystals[i];
        if (!xp->active) continue;

        xp->lifetime -= dt;
        if (xp->lifetime <= 0.0f)
        {
            xp->active = false;
            pool->count--;
            continue;
        }

        float distSq = Vector2DistanceSq(xp->pos, playerPos);
        if (distSq < magnetRadius * magnetRadius && distSq > 0.0f)
        {
            float dist = sqrtf(distSq);
            float dx = (playerPos.x - xp->pos.x) / dist;
            float dy = (playerPos.y - xp->pos.y) / dist;

            float speedMultiplier = 1.0f + (magnetRadius - dist) / magnetRadius;
            float speed = XP_MAGNET_SPEED * speedMultiplier;

            xp->pos.x += dx * speed * dt;
            xp->pos.y += dy * speed * dt;
        }
    }
}

void XPPoolDraw(XPPool *pool)
{
    for (int i = 0; i < MAX_XP_CRYSTALS; i++)
    {
        XPCrystal *xp = &pool->crystals[i];
        if (!xp->active) continue;

        float r = xp->radius;
        float pulse = 1.0f + 0.2f * sinf(xp->lifetime * 8.0f);
        float size = r * pulse;

        Color glowColor = (Color){ 50, 255, 100, 80 };
        DrawCircleV(xp->pos, size * 1.5f, glowColor);

        Vector2 top = { xp->pos.x, xp->pos.y - size };
        Vector2 bottom = { xp->pos.x, xp->pos.y + size };
        Vector2 left = { xp->pos.x - size, xp->pos.y };
        Vector2 right = { xp->pos.x + size, xp->pos.y };

        Color crystalColor = NEON_GREEN;
        DrawTriangle(top, left, xp->pos, crystalColor);
        DrawTriangle(top, xp->pos, right, crystalColor);
        DrawTriangle(xp->pos, left, bottom, crystalColor);
        DrawTriangle(xp->pos, bottom, right, crystalColor);

        DrawCircleV(xp->pos, size * 0.3f, NEON_WHITE);
    }
}

void XPSpawn(XPPool *pool, Vector2 pos, int value)
{
    for (int i = 0; i < MAX_XP_CRYSTALS; i++)
    {
        XPCrystal *xp = &pool->crystals[i];
        if (!xp->active)
        {
            xp->pos = pos;
            xp->value = value;
            xp->radius = XP_CRYSTAL_RADIUS;
            xp->lifetime = XP_CRYSTAL_LIFETIME;
            xp->active = true;
            pool->count++;
            return;
        }
    }
}

int XPCollect(XPPool *pool, Vector2 playerPos, float collectRadius)
{
    int totalCollected = 0;

    for (int i = 0; i < MAX_XP_CRYSTALS; i++)
    {
        XPCrystal *xp = &pool->crystals[i];
        if (!xp->active) continue;

        float distSq = Vector2DistanceSq(xp->pos, playerPos);
        if (distSq < collectRadius * collectRadius)
        {
            totalCollected += xp->value;
            xp->active = false;
            pool->count--;
        }
    }

    return totalCollected;
}
