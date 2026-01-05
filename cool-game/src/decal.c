#include "decal.h"
#include "raylib.h"
#include <math.h>
#include <stdlib.h>

void DecalPoolInit(DecalPool *pool)
{
    for (int i = 0; i < MAX_DECALS; i++)
    {
        pool->decals[i].active = false;
        pool->decals[i].activeIndex = -1;
        pool->freeIndices[i] = i;
    }
    pool->count = 0;
    pool->freeCount = MAX_DECALS;
}

static Decal* DecalSpawn(DecalPool *pool, Vector2 pos, float radius, DecalType type, Color color, float lifetime)
{
    if (pool->freeCount <= 0) return NULL;

    int index = pool->freeIndices[pool->freeCount - 1];
    pool->freeCount--;

    Decal *d = &pool->decals[index];
    d->pos = pos;
    d->radius = radius;
    d->type = type;
    d->color = color;
    d->lifetime = lifetime;
    d->maxLifetime = lifetime;
    d->rotation = ((float)(rand() % 360)) * (PI / 180.0f);
    d->active = true;
    d->activeIndex = pool->count;
    pool->activeIndices[pool->count] = index;
    pool->count++;
    return d;
}

static void DecalDeactivate(DecalPool *pool, int index)
{
    if (index < 0 || index >= MAX_DECALS) return;
    if (!pool->decals[index].active) return;

    int removeSlot = pool->decals[index].activeIndex;
    int lastIndex = pool->activeIndices[pool->count - 1];

    pool->activeIndices[removeSlot] = lastIndex;
    pool->decals[lastIndex].activeIndex = removeSlot;
    pool->count--;

    pool->decals[index].active = false;
    pool->decals[index].activeIndex = -1;
    pool->freeIndices[pool->freeCount] = index;
    pool->freeCount++;
}

void DecalPoolUpdate(DecalPool *pool, float dt)
{
    for (int i = 0; i < pool->count; )
    {
        int index = pool->activeIndices[i];
        Decal *d = &pool->decals[index];
        if (!d->active)
        {
            DecalDeactivate(pool, index);
            continue;
        }

        d->lifetime -= dt;

        if (d->lifetime <= 0.0f)
        {
            DecalDeactivate(pool, index);
            continue;
        }

        i++;
    }
}

void DecalPoolDraw(DecalPool *pool, Rectangle view)
{
    for (int i = 0; i < pool->count; i++)
    {
        Decal *d = &pool->decals[pool->activeIndices[i]];
        if (!d->active) continue;

        // View culling
        if (d->pos.x + d->radius < view.x || d->pos.x - d->radius > view.x + view.width ||
            d->pos.y + d->radius < view.y || d->pos.y - d->radius > view.y + view.height)
        {
            continue;
        }

        float lifeRatio = d->lifetime / d->maxLifetime;
        // Fade out in last 30% of lifetime
        float alpha = (lifeRatio < 0.3f) ? (lifeRatio / 0.3f) : 1.0f;
        alpha *= 0.6f;  // Base opacity

        switch (d->type)
        {
            case DECAL_BURN:
            {
                // Burn mark: dark center with orange/red edge
                Color outerColor = { 60, 30, 10, (unsigned char)(180 * alpha) };
                Color innerColor = { 20, 10, 5, (unsigned char)(200 * alpha) };
                Color glowColor = { 255, 100, 30, (unsigned char)(80 * alpha * lifeRatio) };

                // Draw soft glow if fresh
                if (lifeRatio > 0.7f)
                {
                    DrawCircleV(d->pos, d->radius * 1.3f, glowColor);
                }

                // Draw burn layers
                DrawCircleV(d->pos, d->radius, outerColor);
                DrawCircleV(d->pos, d->radius * 0.6f, innerColor);

                // Cracks/texture pattern
                for (int crack = 0; crack < 4; crack++)
                {
                    float angle = d->rotation + crack * (PI / 2.0f);
                    float len = d->radius * 0.8f;
                    Vector2 end = {
                        d->pos.x + cosf(angle) * len,
                        d->pos.y + sinf(angle) * len
                    };
                    DrawLineEx(d->pos, end, 2.0f, (Color){ 40, 20, 10, (unsigned char)(150 * alpha) });
                }
                break;
            }

            case DECAL_ICE:
            {
                // Ice patch: translucent blue/white crystalline
                Color iceColor = { 150, 220, 255, (unsigned char)(120 * alpha) };
                Color frostColor = { 200, 240, 255, (unsigned char)(80 * alpha) };
                Color sparkleColor = { 255, 255, 255, (unsigned char)(150 * alpha * lifeRatio) };

                // Main ice circle
                DrawCircleV(d->pos, d->radius, iceColor);

                // Frost edge
                DrawCircleLinesV(d->pos, d->radius * 1.1f, frostColor);
                DrawCircleLinesV(d->pos, d->radius * 0.8f, frostColor);

                // Crystal spikes
                for (int spike = 0; spike < 6; spike++)
                {
                    float angle = d->rotation + spike * (PI / 3.0f);
                    float len = d->radius * (0.4f + (spike % 2) * 0.3f);
                    Vector2 end = {
                        d->pos.x + cosf(angle) * len,
                        d->pos.y + sinf(angle) * len
                    };
                    DrawLineEx(d->pos, end, 1.5f, sparkleColor);
                }

                // Center highlight
                DrawCircleV(d->pos, d->radius * 0.3f, (Color){ 255, 255, 255, (unsigned char)(60 * alpha) });
                break;
            }

            case DECAL_SCORCH:
            {
                // Generic scorch: dark gray mark
                Color scorchColor = { 30, 30, 35, (unsigned char)(150 * alpha) };
                DrawCircleV(d->pos, d->radius, scorchColor);
                DrawCircleV(d->pos, d->radius * 0.5f, (Color){ 20, 20, 25, (unsigned char)(180 * alpha) });
                break;
            }

            case DECAL_PLASMA:
            {
                // Plasma mark: cyan/purple energy residue
                Color plasmaOuter = { 100, 50, 200, (unsigned char)(100 * alpha) };
                Color plasmaInner = { 50, 200, 255, (unsigned char)(120 * alpha) };
                Color plasmaCore = { 255, 255, 255, (unsigned char)(80 * alpha * lifeRatio) };

                DrawCircleV(d->pos, d->radius, plasmaOuter);
                DrawCircleV(d->pos, d->radius * 0.7f, plasmaInner);
                DrawCircleV(d->pos, d->radius * 0.3f, plasmaCore);

                // Energy tendrils
                for (int tendril = 0; tendril < 5; tendril++)
                {
                    float angle = d->rotation + tendril * (2.0f * PI / 5.0f);
                    float len = d->radius * 0.9f;
                    float wobble = sinf((float)GetTime() * 5.0f + tendril) * 3.0f;
                    Vector2 end = {
                        d->pos.x + cosf(angle) * (len + wobble),
                        d->pos.y + sinf(angle) * (len + wobble)
                    };
                    DrawLineEx(d->pos, end, 1.0f, plasmaInner);
                }
                break;
            }

            case DECAL_BLOOD:
            {
                // Blood splatter: enemy-colored with darker center
                Color bloodColor = d->color;
                bloodColor.a = (unsigned char)(140 * alpha);

                Color darkBlood = {
                    (unsigned char)(bloodColor.r * 0.4f),
                    (unsigned char)(bloodColor.g * 0.4f),
                    (unsigned char)(bloodColor.b * 0.4f),
                    (unsigned char)(160 * alpha)
                };

                // Main splatter
                DrawCircleV(d->pos, d->radius, bloodColor);
                DrawCircleV(d->pos, d->radius * 0.5f, darkBlood);

                // Random splatter spots
                for (int spot = 0; spot < 4; spot++)
                {
                    float angle = d->rotation + spot * (PI / 2.0f) + 0.3f;
                    float dist = d->radius * (0.8f + (spot % 2) * 0.4f);
                    Vector2 spotPos = {
                        d->pos.x + cosf(angle) * dist,
                        d->pos.y + sinf(angle) * dist
                    };
                    float spotSize = d->radius * (0.2f + (spot % 3) * 0.1f);
                    DrawCircleV(spotPos, spotSize, bloodColor);
                }
                break;
            }

            case DECAL_LIGHTNING:
            {
                // Lightning strike: bright white/yellow radiating lines
                Color lightningCore = { 255, 255, 200, (unsigned char)(200 * alpha * lifeRatio) };
                Color lightningGlow = { 255, 255, 100, (unsigned char)(100 * alpha) };
                Color lightningOuter = { 100, 100, 255, (unsigned char)(60 * alpha) };

                // Glow base
                DrawCircleV(d->pos, d->radius * 1.2f, lightningOuter);
                DrawCircleV(d->pos, d->radius * 0.8f, lightningGlow);
                DrawCircleV(d->pos, d->radius * 0.4f, lightningCore);

                // Branching lightning marks
                for (int branch = 0; branch < 8; branch++)
                {
                    float angle = d->rotation + branch * (PI / 4.0f);
                    float len = d->radius * (0.6f + (branch % 3) * 0.3f);

                    // Jagged line effect
                    Vector2 prev = d->pos;
                    int segments = 3;
                    for (int seg = 1; seg <= segments; seg++)
                    {
                        float segLen = len / segments;
                        float jitter = ((rand() % 10) - 5) * 0.5f;
                        Vector2 next = {
                            prev.x + cosf(angle) * segLen + jitter,
                            prev.y + sinf(angle) * segLen + jitter
                        };
                        DrawLineEx(prev, next, 1.5f, lightningCore);
                        prev = next;
                    }
                }
                break;
            }
        }
    }
}

void DecalSpawnBurn(DecalPool *pool, Vector2 pos, float radius)
{
    DecalSpawn(pool, pos, radius, DECAL_BURN, NEON_ORANGE, 8.0f);
}

void DecalSpawnIce(DecalPool *pool, Vector2 pos, float radius)
{
    DecalSpawn(pool, pos, radius, DECAL_ICE, NEON_CYAN, 6.0f);
}

void DecalSpawnScorch(DecalPool *pool, Vector2 pos, float radius)
{
    DecalSpawn(pool, pos, radius, DECAL_SCORCH, (Color){ 40, 40, 45, 255 }, 10.0f);
}

void DecalSpawnPlasma(DecalPool *pool, Vector2 pos, float radius)
{
    DecalSpawn(pool, pos, radius, DECAL_PLASMA, NEON_CYAN, 5.0f);
}

void DecalSpawnBlood(DecalPool *pool, Vector2 pos, float radius, Color color)
{
    DecalSpawn(pool, pos, radius, DECAL_BLOOD, color, 12.0f);
}

void DecalSpawnLightning(DecalPool *pool, Vector2 pos, float radius)
{
    DecalSpawn(pool, pos, radius, DECAL_LIGHTNING, NEON_YELLOW, 3.0f);
}
