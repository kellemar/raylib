#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "types.h"
#include <stdbool.h>

typedef enum EnemyType {
    ENEMY_CHASER,
    ENEMY_ORBITER,
    ENEMY_SPLITTER,
    ENEMY_BOSS,         // Boss enemy (spawns every 5 minutes)
    ENEMY_TYPE_COUNT
} EnemyType;

typedef struct Enemy {
    Vector2 pos;
    Vector2 vel;
    float radius;
    float speed;
    float health;
    float maxHealth;
    float damage;
    EnemyType type;
    int xpValue;
    bool active;
    float orbitAngle;
    float orbitDistance;
    int splitCount;
    float hitFlashTimer;
    // Status effects
    float slowTimer;        // Remaining slow duration
    float slowAmount;       // Speed reduction (0.0 - 1.0)
    float baseSpeed;        // Original speed before slow
    // Elite status
    bool isElite;           // Elite enemies are larger and stronger
    // Boss specific
    bool isBoss;            // Boss enemy flag
    int bossPhase;          // Current boss attack phase (0-2)
    float bossAttackTimer;  // Time until next attack
    float bossChargeTimer;  // Charge-up before attack (anticipation)
    bool bossCharging;      // Is boss currently charging an attack
    int activeIndex;        // Index in active list for O(1) removal
} Enemy;

// Elite enemy multipliers
#define ELITE_SPAWN_CHANCE  0.1f    // 10% chance to spawn as elite
#define ELITE_SIZE_MULT     1.5f    // 50% larger
#define ELITE_HEALTH_MULT   3.0f    // 3x health
#define ELITE_DAMAGE_MULT   1.5f    // 50% more damage
#define ELITE_XP_MULT       5       // 5x XP reward
#define ELITE_SPEED_MULT    0.8f    // Slightly slower (but tankier)

// Boss enemy stats
#define BOSS_SPAWN_INTERVAL 60.0f  // 5 minutes (300 seconds)
#define BOSS_BASE_HEALTH    2000.0f // High health pool
#define BOSS_BASE_RADIUS    60.0f   // Large visual size
#define BOSS_BASE_DAMAGE    30.0f   // High contact damage
#define BOSS_BASE_SPEED     50.0f   // Slow but menacing
#define BOSS_XP_VALUE       100     // Massive XP reward
#define BOSS_ATTACK_INTERVAL 3.0f   // Seconds between attacks
#define BOSS_CHARGE_TIME    1.0f    // Anticipation before attack

typedef struct EnemyPool {
    Enemy enemies[MAX_ENEMIES];
    int activeIndices[MAX_ENEMIES];
    int freeIndices[MAX_ENEMIES];
    int freeCount;
    int count;
} EnemyPool;

// Spatial grid for enemy queries
#define ENEMY_SPATIAL_CELL_SIZE 128.0f
#define ENEMY_SPATIAL_BUCKETS   1024

typedef struct EnemySpatialGrid {
    int bucketHeads[ENEMY_SPATIAL_BUCKETS];
    int next[MAX_ENEMIES];
    int cellX[MAX_ENEMIES];
    int cellY[MAX_ENEMIES];
} EnemySpatialGrid;

typedef bool (*EnemySpatialVisit)(Enemy *enemy, int index, void *user);

void EnemyPoolInit(EnemyPool *pool);
void EnemyPoolUpdate(EnemyPool *pool, Vector2 playerPos, float dt);
void EnemyPoolDraw(EnemyPool *pool, Rectangle view);
Enemy* EnemySpawn(EnemyPool *pool, EnemyType type, Vector2 pos);
Enemy* EnemySpawnSplitterChild(EnemyPool *pool, Vector2 pos, int splitCount, float radius, float health);
void EnemyApplySlow(Enemy *enemy, float amount, float duration);
Enemy* EnemyFindNearest(EnemyPool *pool, Vector2 pos, float maxDistance);
Enemy* EnemySpawnElite(EnemyPool *pool, EnemyType type, Vector2 pos);
Enemy* EnemySpawnBoss(EnemyPool *pool, Vector2 pos, int bossNumber);
bool EnemyPoolHasBoss(EnemyPool *pool);
Enemy* EnemyPoolGetBoss(EnemyPool *pool);
void EnemyDeactivate(EnemyPool *pool, int index);
void EnemySpatialGridBuild(EnemySpatialGrid *grid, EnemyPool *pool);
void EnemySpatialGridForEachInRadius(EnemySpatialGrid *grid, EnemyPool *pool, Vector2 center, float radius, EnemySpatialVisit visit, void *user);
Enemy* EnemyFindNearestInGrid(EnemyPool *pool, EnemySpatialGrid *grid, Vector2 pos, float maxDistance);

#endif
