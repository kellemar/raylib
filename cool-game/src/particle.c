#include "particle.h"
#include <math.h>
#include <stdlib.h>

void ParticlePoolInit(ParticlePool *pool)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        pool->particles[i].active = false;
        pool->particles[i].activeIndex = -1;
        pool->freeIndices[i] = i;
    }
    pool->count = 0;
    pool->freeCount = MAX_PARTICLES;
}

static Particle* ParticleSpawn(ParticlePool *pool, Vector2 pos, Vector2 vel, Color color, float size, float lifetime)
{
    if (pool->freeCount <= 0) return NULL;

    int index = pool->freeIndices[pool->freeCount - 1];
    pool->freeCount--;

    Particle *p = &pool->particles[index];
    p->pos = pos;
    p->vel = vel;
    p->color = color;
    p->size = size;
    p->lifetime = lifetime;
    p->maxLifetime = lifetime;
    p->active = true;
    p->activeIndex = pool->count;
    pool->activeIndices[pool->count] = index;
    pool->count++;
    return p;
}

static void ParticleDeactivate(ParticlePool *pool, int index)
{
    if (index < 0 || index >= MAX_PARTICLES) return;
    if (!pool->particles[index].active) return;

    int removeSlot = pool->particles[index].activeIndex;
    int lastIndex = pool->activeIndices[pool->count - 1];

    pool->activeIndices[removeSlot] = lastIndex;
    pool->particles[lastIndex].activeIndex = removeSlot;
    pool->count--;

    pool->particles[index].active = false;
    pool->particles[index].activeIndex = -1;
    pool->freeIndices[pool->freeCount] = index;
    pool->freeCount++;
}

void ParticlePoolUpdate(ParticlePool *pool, float dt)
{
    for (int i = 0; i < pool->count; )
    {
        int index = pool->activeIndices[i];
        Particle *p = &pool->particles[index];
        if (!p->active)
        {
            ParticleDeactivate(pool, index);
            continue;
        }

        p->pos.x += p->vel.x * dt;
        p->pos.y += p->vel.y * dt;

        p->vel.x *= 0.98f;
        p->vel.y *= 0.98f;

        p->lifetime -= dt;

        if (p->lifetime <= 0.0f)
        {
            ParticleDeactivate(pool, index);
            continue;
        }

        i++;
    }
}

void ParticlePoolDraw(ParticlePool *pool, Rectangle view)
{
    for (int i = 0; i < pool->count; i++)
    {
        Particle *p = &pool->particles[pool->activeIndices[i]];
        if (!p->active) continue;

        float lifeRatio = p->lifetime / p->maxLifetime;
        unsigned char alpha = (unsigned char)(255.0f * lifeRatio);

        Color drawColor = p->color;
        drawColor.a = alpha;

        float drawSize = p->size * (0.5f + 0.5f * lifeRatio);

        if (p->pos.x + drawSize < view.x || p->pos.x - drawSize > view.x + view.width ||
            p->pos.y + drawSize < view.y || p->pos.y - drawSize > view.y + view.height)
        {
            continue;
        }

        DrawCircleV(p->pos, drawSize, drawColor);
    }
}

void SpawnExplosion(ParticlePool *pool, Vector2 pos, Color color, int count)
{
    for (int i = 0; i < count; i++)
    {
        float angle = ((float)(rand() % 360)) * (3.14159265f / 180.0f);
        float speed = 100.0f + (float)(rand() % 200);

        Vector2 vel = {
            cosf(angle) * speed,
            sinf(angle) * speed
        };

        Color particleColor = color;
        int variation = (rand() % 60) - 30;
        int r = (int)particleColor.r + variation;
        int g = (int)particleColor.g + variation;
        particleColor.r = (unsigned char)(r > 255 ? 255 : (r < 0 ? 0 : r));
        particleColor.g = (unsigned char)(g > 255 ? 255 : (g < 0 ? 0 : g));

        float size = 3.0f + (float)(rand() % 5);
        float lifetime = 0.3f + (float)(rand() % 100) / 200.0f;

        ParticleSpawn(pool, pos, vel, particleColor, size, lifetime);
    }
}

void SpawnHitParticles(ParticlePool *pool, Vector2 pos, Color color, int count)
{
    for (int i = 0; i < count; i++)
    {
        float angle = ((float)(rand() % 360)) * (3.14159265f / 180.0f);
        float speed = 50.0f + (float)(rand() % 100);

        Vector2 vel = {
            cosf(angle) * speed,
            sinf(angle) * speed
        };

        float size = 2.0f + (float)(rand() % 3);
        float lifetime = 0.15f + (float)(rand() % 50) / 200.0f;

        ParticleSpawn(pool, pos, vel, color, size, lifetime);
    }
}

void SpawnDeathExplosion(ParticlePool *pool, Vector2 pos, DeathExplosionType type, float radius)
{
    // Scale effect intensity based on enemy radius
    float scale = radius / 12.0f;  // 12.0f is default chaser radius
    if (scale < 0.5f) scale = 0.5f;
    if (scale > 3.0f) scale = 3.0f;

    switch (type)
    {
        case DEATH_EXPLOSION_CHASER:
        {
            // Fast burst explosion - red/orange with outward spray
            int count = (int)(20 * scale);
            for (int i = 0; i < count; i++)
            {
                float angle = ((float)(rand() % 360)) * (3.14159265f / 180.0f);
                float speed = 200.0f + (float)(rand() % 150);

                Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };

                // Alternate between red and orange
                Color particleColor = (i % 2 == 0) ? NEON_RED : NEON_ORANGE;
                int variation = (rand() % 40) - 20;
                particleColor.r = (unsigned char)fminf(255, fmaxf(0, particleColor.r + variation));

                float size = 4.0f + (float)(rand() % 4);
                float lifetime = 0.25f + (float)(rand() % 100) / 400.0f;

                ParticleSpawn(pool, pos, vel, particleColor, size, lifetime);
            }
            break;
        }

        case DEATH_EXPLOSION_ORBITER:
        {
            // Spiral ring explosion - cyan/pink particles in ring pattern
            int rings = 2;
            int particlesPerRing = 12;
            for (int ring = 0; ring < rings; ring++)
            {
                float ringSpeed = 120.0f + ring * 80.0f;
                float angleOffset = ring * 0.3f;  // Spiral offset

                for (int i = 0; i < particlesPerRing; i++)
                {
                    float angle = (float)i / particlesPerRing * 2.0f * 3.14159265f + angleOffset;
                    Vector2 vel = { cosf(angle) * ringSpeed, sinf(angle) * ringSpeed };

                    // Alternate cyan and pink
                    Color particleColor = (i % 2 == 0) ? NEON_CYAN : NEON_PINK;

                    float size = 5.0f + (float)(rand() % 3);
                    float lifetime = 0.4f + ring * 0.1f;

                    ParticleSpawn(pool, pos, vel, particleColor, size, lifetime);
                }
            }

            // Add some random sparkles
            for (int i = 0; i < 8; i++)
            {
                float angle = ((float)(rand() % 360)) * (3.14159265f / 180.0f);
                float speed = 50.0f + (float)(rand() % 100);
                Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };
                ParticleSpawn(pool, pos, vel, WHITE, 2.0f, 0.3f);
            }
            break;
        }

        case DEATH_EXPLOSION_SPLITTER:
        {
            // Shatter/fragment explosion - yellow/green with angular shards
            int count = 25;
            for (int i = 0; i < count; i++)
            {
                // Create angular "shard" directions (8 main directions with variation)
                int mainDir = i % 8;
                float baseAngle = mainDir * (3.14159265f / 4.0f);
                float angleVariation = ((float)(rand() % 60) - 30.0f) * (3.14159265f / 180.0f);
                float angle = baseAngle + angleVariation;

                float speed = 100.0f + (float)(rand() % 200);
                Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };

                // Yellow and green fragments
                Color particleColor = (rand() % 2 == 0) ? NEON_YELLOW : NEON_GREEN;

                // Larger, more angular particles for shards
                float size = 5.0f + (float)(rand() % 5);
                float lifetime = 0.35f + (float)(rand() % 100) / 300.0f;

                ParticleSpawn(pool, pos, vel, particleColor, size, lifetime);
            }

            // Inner core particles
            for (int i = 0; i < 6; i++)
            {
                float angle = ((float)(rand() % 360)) * (3.14159265f / 180.0f);
                Vector2 vel = { cosf(angle) * 40.0f, sinf(angle) * 40.0f };
                ParticleSpawn(pool, pos, vel, WHITE, 3.0f, 0.2f);
            }
            break;
        }

        case DEATH_EXPLOSION_BOSS:
        {
            // Massive multi-stage explosion - purple/magenta with multiple waves
            // Wave 1: Inner burst
            for (int i = 0; i < 30; i++)
            {
                float angle = ((float)(rand() % 360)) * (3.14159265f / 180.0f);
                float speed = 150.0f + (float)(rand() % 100);
                Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };

                Color particleColor = (Color){ 200, 50, 200, 255 };  // Magenta
                float size = 8.0f + (float)(rand() % 6);
                float lifetime = 0.5f + (float)(rand() % 100) / 200.0f;

                ParticleSpawn(pool, pos, vel, particleColor, size, lifetime);
            }

            // Wave 2: Outer ring
            int ringCount = 16;
            for (int i = 0; i < ringCount; i++)
            {
                float angle = (float)i / ringCount * 2.0f * 3.14159265f;
                float speed = 250.0f;
                Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };

                Color particleColor = (Color){ 128, 0, 128, 255 };  // Deep purple
                ParticleSpawn(pool, pos, vel, particleColor, 10.0f, 0.6f);
            }

            // Wave 3: Sparkle shower
            for (int i = 0; i < 40; i++)
            {
                float angle = ((float)(rand() % 360)) * (3.14159265f / 180.0f);
                float speed = 50.0f + (float)(rand() % 300);
                Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };

                Color particleColor = (rand() % 2 == 0) ? WHITE : (Color){ 255, 200, 255, 255 };
                float size = 3.0f + (float)(rand() % 4);
                float lifetime = 0.7f + (float)(rand() % 100) / 200.0f;

                ParticleSpawn(pool, pos, vel, particleColor, size, lifetime);
            }
            break;
        }

        case DEATH_EXPLOSION_ELITE:
        {
            // Golden burst with sparks - prestigious death
            int count = 30;
            for (int i = 0; i < count; i++)
            {
                float angle = ((float)(rand() % 360)) * (3.14159265f / 180.0f);
                float speed = 150.0f + (float)(rand() % 150);
                Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };

                // Gold color with variation
                Color particleColor = (Color){ 255, 215, 0, 255 };  // Gold
                int variation = (rand() % 60) - 30;
                particleColor.r = (unsigned char)fminf(255, fmaxf(200, particleColor.r + variation));
                particleColor.g = (unsigned char)fminf(255, fmaxf(150, particleColor.g + variation));

                float size = 6.0f + (float)(rand() % 5);
                float lifetime = 0.4f + (float)(rand() % 100) / 250.0f;

                ParticleSpawn(pool, pos, vel, particleColor, size, lifetime);
            }

            // White sparkle core
            for (int i = 0; i < 15; i++)
            {
                float angle = ((float)(rand() % 360)) * (3.14159265f / 180.0f);
                float speed = 80.0f + (float)(rand() % 120);
                Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };
                ParticleSpawn(pool, pos, vel, WHITE, 4.0f, 0.35f);
            }
            break;
        }
    }
}
