#ifndef DECAL_H
#define DECAL_H

#include "raylib.h"
#include "types.h"
#include <stdbool.h>

#define MAX_DECALS 200

// Decal types for different visual effects
typedef enum DecalType {
    DECAL_BURN,         // Fire/explosion burn mark (orange/black)
    DECAL_ICE,          // Freeze ray ice patch (blue/white)
    DECAL_SCORCH,       // Generic weapon scorch (dark)
    DECAL_PLASMA,       // Plasma/energy mark (cyan/purple)
    DECAL_BLOOD,        // Enemy death splatter (enemy color)
    DECAL_LIGHTNING     // Lightning strike mark (white/yellow)
} DecalType;

typedef struct Decal {
    Vector2 pos;
    float radius;
    float lifetime;
    float maxLifetime;
    float rotation;     // Random rotation for variety
    DecalType type;
    Color color;
    bool active;
    int activeIndex;
} Decal;

typedef struct DecalPool {
    Decal decals[MAX_DECALS];
    int activeIndices[MAX_DECALS];
    int freeIndices[MAX_DECALS];
    int freeCount;
    int count;
} DecalPool;

void DecalPoolInit(DecalPool *pool);
void DecalPoolUpdate(DecalPool *pool, float dt);
void DecalPoolDraw(DecalPool *pool, Rectangle view);

// Spawn decals at position
void DecalSpawnBurn(DecalPool *pool, Vector2 pos, float radius);
void DecalSpawnIce(DecalPool *pool, Vector2 pos, float radius);
void DecalSpawnScorch(DecalPool *pool, Vector2 pos, float radius);
void DecalSpawnPlasma(DecalPool *pool, Vector2 pos, float radius);
void DecalSpawnBlood(DecalPool *pool, Vector2 pos, float radius, Color color);
void DecalSpawnLightning(DecalPool *pool, Vector2 pos, float radius);

#endif
