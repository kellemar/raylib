#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "types.h"
#include <stdbool.h>

typedef enum EnemyType {
    ENEMY_CHASER,
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
} Enemy;

typedef struct EnemyPool {
    Enemy enemies[MAX_ENEMIES];
    int count;
} EnemyPool;

void EnemyPoolInit(EnemyPool *pool);
void EnemyPoolUpdate(EnemyPool *pool, Vector2 playerPos, float dt);
void EnemyPoolDraw(EnemyPool *pool);
Enemy* EnemySpawn(EnemyPool *pool, EnemyType type, Vector2 pos);

#endif
