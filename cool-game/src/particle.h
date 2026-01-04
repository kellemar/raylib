#ifndef PARTICLE_H
#define PARTICLE_H

#include "raylib.h"
#include "types.h"
#include <stdbool.h>

typedef struct Particle {
    Vector2 pos;
    Vector2 vel;
    Color color;
    float size;
    float lifetime;
    float maxLifetime;
    bool active;
} Particle;

typedef struct ParticlePool {
    Particle particles[MAX_PARTICLES];
    int count;
} ParticlePool;

void ParticlePoolInit(ParticlePool *pool);
void ParticlePoolUpdate(ParticlePool *pool, float dt);
void ParticlePoolDraw(ParticlePool *pool);
void SpawnExplosion(ParticlePool *pool, Vector2 pos, Color color, int count);
void SpawnHitParticles(ParticlePool *pool, Vector2 pos, Color color, int count);

#endif
