#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "types.h"
#include <stdbool.h>

typedef enum EnemyType {
    ENEMY_CHASER,
    ENEMY_ORBITER,
    ENEMY_SPLITTER,
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
} Enemy;

// Elite enemy multipliers
#define ELITE_SPAWN_CHANCE  0.1f    // 10% chance to spawn as elite
#define ELITE_SIZE_MULT     1.5f    // 50% larger
#define ELITE_HEALTH_MULT   3.0f    // 3x health
#define ELITE_DAMAGE_MULT   1.5f    // 50% more damage
#define ELITE_XP_MULT       5       // 5x XP reward
#define ELITE_SPEED_MULT    0.8f    // Slightly slower (but tankier)

typedef struct EnemyPool {
    Enemy enemies[MAX_ENEMIES];
    int count;
} EnemyPool;

void EnemyPoolInit(EnemyPool *pool);
void EnemyPoolUpdate(EnemyPool *pool, Vector2 playerPos, float dt);
void EnemyPoolDraw(EnemyPool *pool);
Enemy* EnemySpawn(EnemyPool *pool, EnemyType type, Vector2 pos);
Enemy* EnemySpawnSplitterChild(EnemyPool *pool, Vector2 pos, int splitCount, float radius, float health);
void EnemyApplySlow(Enemy *enemy, float amount, float duration);
Enemy* EnemyFindNearest(EnemyPool *pool, Vector2 pos, float maxDistance);
Enemy* EnemySpawnElite(EnemyPool *pool, EnemyType type, Vector2 pos);

#endif
