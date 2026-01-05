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
    int activeIndex;
} Particle;

typedef struct ParticlePool {
    Particle particles[MAX_PARTICLES];
    int activeIndices[MAX_PARTICLES];
    int freeIndices[MAX_PARTICLES];
    int freeCount;
    int count;
} ParticlePool;

void ParticlePoolInit(ParticlePool *pool);
void ParticlePoolUpdate(ParticlePool *pool, float dt);
void ParticlePoolDraw(ParticlePool *pool, Rectangle view);
void SpawnExplosion(ParticlePool *pool, Vector2 pos, Color color, int count);
void SpawnHitParticles(ParticlePool *pool, Vector2 pos, Color color, int count);

// Death explosion types by enemy
typedef enum DeathExplosionType {
    DEATH_EXPLOSION_CHASER,     // Fast burst (red/orange)
    DEATH_EXPLOSION_ORBITER,    // Spiral ring (cyan/pink)
    DEATH_EXPLOSION_SPLITTER,   // Shatter fragments (yellow/green)
    DEATH_EXPLOSION_BOSS,       // Massive multi-stage (purple/magenta)
    DEATH_EXPLOSION_ELITE       // Golden burst with sparks
} DeathExplosionType;

void SpawnDeathExplosion(ParticlePool *pool, Vector2 pos, DeathExplosionType type, float radius);

#endif
