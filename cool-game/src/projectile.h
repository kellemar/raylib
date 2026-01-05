#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "raylib.h"
#include "types.h"
#include <stdbool.h>

// Trail system for projectiles
#define TRAIL_MAX_POINTS 8
#define TRAIL_UPDATE_INTERVAL 0.016f  // ~60fps trail updates

// Forward declaration to avoid circular includes
struct EnemyPool;
struct EnemySpatialGrid;

// Projectile behavior types
typedef enum ProjectileBehavior {
    PROJ_BEHAVIOR_LINEAR,       // Standard straight-line movement
    PROJ_BEHAVIOR_HOMING,       // Seeks nearest enemy
    PROJ_BEHAVIOR_ORBIT,        // Orbits around owner position
    PROJ_BEHAVIOR_PULL,         // Pulls enemies toward it (black hole)
} ProjectileBehavior;

// Projectile effect flags (can be combined)
typedef enum ProjectileEffect {
    PROJ_EFFECT_NONE    = 0,
    PROJ_EFFECT_SLOW    = 1 << 0,   // Slows enemies on hit (freeze ray)
    PROJ_EFFECT_CHAIN   = 1 << 1,   // Chains to nearby enemies (lightning)
    PROJ_EFFECT_DOT     = 1 << 2,   // Damage over time (flamethrower)
} ProjectileEffect;

typedef struct Projectile {
    Vector2 pos;
    Vector2 vel;
    float radius;
    float damage;
    float lifetime;
    int weaponType;
    bool pierce;
    bool active;
    // Extended properties for weapon variety
    ProjectileBehavior behavior;
    int effects;                    // Bitmask of ProjectileEffect
    float homingStrength;           // Turn rate for homing projectiles
    float orbitAngle;               // Current angle for orbiting projectiles
    float orbitRadius;              // Distance from owner for orbiting
    float orbitSpeed;               // Angular velocity for orbiting
    Vector2 *ownerPos;              // Pointer to owner position (for orbit)
    float pullStrength;             // Force for black hole pull effect
    int chainCount;                 // Remaining chain jumps for lightning
    float slowAmount;               // Slow percentage (0.0 - 1.0)
    float slowDuration;             // How long the slow lasts
    Color color;                    // Projectile color for rendering
    int activeIndex;                // Index in active list for O(1) removal
    // Trail system
    Vector2 trailPoints[TRAIL_MAX_POINTS];  // Historical positions for trail
    int trailHead;                          // Circular buffer head index
    int trailCount;                         // Number of valid trail points
    float trailTimer;                       // Timer for trail point capture
} Projectile;

typedef struct ProjectilePool {
    Projectile projectiles[MAX_PROJECTILES];
    int activeIndices[MAX_PROJECTILES];
    int freeIndices[MAX_PROJECTILES];
    int freeCount;
    int count;
} ProjectilePool;

void ProjectilePoolInit(ProjectilePool *pool);
void ProjectilePoolUpdate(ProjectilePool *pool, float dt, struct EnemyPool *enemies, struct EnemySpatialGrid *grid);
void ProjectilePoolDraw(ProjectilePool *pool, Rectangle view);
Projectile* ProjectileSpawn(ProjectilePool *pool, Vector2 pos, Vector2 vel, float damage, float radius, float lifetime);
void ProjectileDeactivate(ProjectilePool *pool, int index);

// Extended spawn for weapon variety system
typedef struct ProjectileSpawnParams {
    Vector2 pos;
    Vector2 vel;
    float damage;
    float radius;
    float lifetime;
    int weaponType;
    bool pierce;
    ProjectileBehavior behavior;
    int effects;
    float homingStrength;
    float orbitAngle;
    float orbitRadius;
    float orbitSpeed;
    Vector2 *ownerPos;
    float pullStrength;
    int chainCount;
    float slowAmount;
    float slowDuration;
    Color color;
} ProjectileSpawnParams;

Projectile* ProjectileSpawnEx(ProjectilePool *pool, ProjectileSpawnParams *params);

#endif
