#include "particle.h"
#include <math.h>
#include <stdlib.h>

void ParticlePoolInit(ParticlePool *pool)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        pool->particles[i].active = false;
    }
    pool->count = 0;
}

static Particle* ParticleSpawn(ParticlePool *pool, Vector2 pos, Vector2 vel, Color color, float size, float lifetime)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (!pool->particles[i].active)
        {
            Particle *p = &pool->particles[i];
            p->pos = pos;
            p->vel = vel;
            p->color = color;
            p->size = size;
            p->lifetime = lifetime;
            p->maxLifetime = lifetime;
            p->active = true;
            pool->count++;
            return p;
        }
    }
    return NULL;
}

void ParticlePoolUpdate(ParticlePool *pool, float dt)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        Particle *p = &pool->particles[i];
        if (!p->active) continue;

        p->pos.x += p->vel.x * dt;
        p->pos.y += p->vel.y * dt;

        p->vel.x *= 0.98f;
        p->vel.y *= 0.98f;

        p->lifetime -= dt;

        if (p->lifetime <= 0.0f)
        {
            p->active = false;
            pool->count--;
        }
    }
}

void ParticlePoolDraw(ParticlePool *pool)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        Particle *p = &pool->particles[i];
        if (!p->active) continue;

        float lifeRatio = p->lifetime / p->maxLifetime;
        unsigned char alpha = (unsigned char)(255.0f * lifeRatio);

        Color drawColor = p->color;
        drawColor.a = alpha;

        float drawSize = p->size * (0.5f + 0.5f * lifeRatio);

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
