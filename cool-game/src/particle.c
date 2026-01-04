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
