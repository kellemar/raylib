#include "minunit.h"
#include "../src/utils.h"

#define MAX_ENEMIES 500

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
    int active;
    float orbitAngle;
    float orbitDistance;
    int splitCount;
} Enemy;

typedef struct EnemyPool {
    Enemy enemies[MAX_ENEMIES];
    int count;
} EnemyPool;

static void EnemyPoolInit(EnemyPool *pool)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        pool->enemies[i].active = 0;
    }
    pool->count = 0;
}

static Enemy* EnemySpawn(EnemyPool *pool, EnemyType type, Vector2 pos)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!pool->enemies[i].active)
        {
            Enemy *e = &pool->enemies[i];
            e->pos = pos;
            e->vel = (Vector2){ 0.0f, 0.0f };
            e->type = type;
            e->active = 1;

            switch (type)
            {
                case ENEMY_CHASER:
                    e->radius = 12.0f;
                    e->speed = 100.0f;
                    e->health = 30.0f;
                    e->maxHealth = 30.0f;
                    e->damage = 10.0f;
                    e->xpValue = 1;
                    e->orbitAngle = 0.0f;
                    e->orbitDistance = 0.0f;
                    e->splitCount = 0;
                    break;

                case ENEMY_ORBITER:
                    e->radius = 15.0f;
                    e->speed = 80.0f;
                    e->health = 50.0f;
                    e->maxHealth = 50.0f;
                    e->damage = 15.0f;
                    e->xpValue = 2;
                    e->orbitAngle = 0.0f;
                    e->orbitDistance = 200.0f;
                    e->splitCount = 0;
                    break;

                case ENEMY_SPLITTER:
                    e->radius = 20.0f;
                    e->speed = 60.0f;
                    e->health = 80.0f;
                    e->maxHealth = 80.0f;
                    e->damage = 20.0f;
                    e->xpValue = 3;
                    e->orbitAngle = 0.0f;
                    e->orbitDistance = 0.0f;
                    e->splitCount = 2;
                    break;

                default:
                    e->radius = 12.0f;
                    e->speed = 100.0f;
                    e->health = 30.0f;
                    e->maxHealth = 30.0f;
                    e->damage = 10.0f;
                    e->xpValue = 1;
                    e->orbitAngle = 0.0f;
                    e->orbitDistance = 0.0f;
                    e->splitCount = 0;
                    break;
            }

            pool->count++;
            return e;
        }
    }
    return 0;
}

static const char* test_enemy_pool_init(void)
{
    EnemyPool pool;
    EnemyPoolInit(&pool);
    mu_assert_int_eq(0, pool.count);
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        mu_assert_false(pool.enemies[i].active);
    }
    return 0;
}

static const char* test_enemy_spawn_single(void)
{
    EnemyPool pool;
    EnemyPoolInit(&pool);

    Vector2 pos = { 100.0f, 200.0f };
    Enemy *e = EnemySpawn(&pool, ENEMY_CHASER, pos);

    mu_assert(e != 0, "EnemySpawn should return non-null");
    mu_assert_int_eq(1, pool.count);
    mu_assert_float_eq(100.0f, e->pos.x);
    mu_assert_float_eq(200.0f, e->pos.y);
    mu_assert_true(e->active);
    return 0;
}

static const char* test_enemy_spawn_chaser_stats(void)
{
    EnemyPool pool;
    EnemyPoolInit(&pool);

    Vector2 pos = { 0.0f, 0.0f };
    Enemy *e = EnemySpawn(&pool, ENEMY_CHASER, pos);

    mu_assert_float_eq(12.0f, e->radius);
    mu_assert_float_eq(100.0f, e->speed);
    mu_assert_float_eq(30.0f, e->health);
    mu_assert_float_eq(30.0f, e->maxHealth);
    mu_assert_float_eq(10.0f, e->damage);
    mu_assert_int_eq(1, e->xpValue);
    return 0;
}

static const char* test_enemy_spawn_multiple(void)
{
    EnemyPool pool;
    EnemyPoolInit(&pool);

    for (int i = 0; i < 10; i++)
    {
        Vector2 pos = { (float)i * 10.0f, 0.0f };
        EnemySpawn(&pool, ENEMY_CHASER, pos);
    }

    mu_assert_int_eq(10, pool.count);
    return 0;
}

static const char* test_enemy_pool_full(void)
{
    EnemyPool pool;
    EnemyPoolInit(&pool);

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        Vector2 pos = { 0.0f, 0.0f };
        EnemySpawn(&pool, ENEMY_CHASER, pos);
    }

    mu_assert_int_eq(MAX_ENEMIES, pool.count);

    Vector2 pos = { 0.0f, 0.0f };
    Enemy *e = EnemySpawn(&pool, ENEMY_CHASER, pos);
    mu_assert(e == 0, "Pool full should return null");
    mu_assert_int_eq(MAX_ENEMIES, pool.count);
    return 0;
}

static const char* test_enemy_reuse_slot(void)
{
    EnemyPool pool;
    EnemyPoolInit(&pool);

    Vector2 pos1 = { 100.0f, 100.0f };
    Enemy *e1 = EnemySpawn(&pool, ENEMY_CHASER, pos1);
    mu_assert_int_eq(1, pool.count);

    e1->active = 0;
    pool.count--;
    mu_assert_int_eq(0, pool.count);

    Vector2 pos2 = { 200.0f, 200.0f };
    Enemy *e2 = EnemySpawn(&pool, ENEMY_CHASER, pos2);
    mu_assert_int_eq(1, pool.count);
    mu_assert(e1 == e2, "Should reuse first slot");
    mu_assert_float_eq(200.0f, e2->pos.x);
    return 0;
}

static const char* test_enemy_spawn_orbiter_stats(void)
{
    EnemyPool pool;
    EnemyPoolInit(&pool);

    Vector2 pos = { 0.0f, 0.0f };
    Enemy *e = EnemySpawn(&pool, ENEMY_ORBITER, pos);

    mu_assert_float_eq(15.0f, e->radius);
    mu_assert_float_eq(80.0f, e->speed);
    mu_assert_float_eq(50.0f, e->health);
    mu_assert_float_eq(50.0f, e->maxHealth);
    mu_assert_float_eq(15.0f, e->damage);
    mu_assert_int_eq(2, e->xpValue);
    mu_assert_float_eq(200.0f, e->orbitDistance);
    mu_assert_int_eq(0, e->splitCount);
    return 0;
}

static const char* test_enemy_spawn_splitter_stats(void)
{
    EnemyPool pool;
    EnemyPoolInit(&pool);

    Vector2 pos = { 0.0f, 0.0f };
    Enemy *e = EnemySpawn(&pool, ENEMY_SPLITTER, pos);

    mu_assert_float_eq(20.0f, e->radius);
    mu_assert_float_eq(60.0f, e->speed);
    mu_assert_float_eq(80.0f, e->health);
    mu_assert_float_eq(80.0f, e->maxHealth);
    mu_assert_float_eq(20.0f, e->damage);
    mu_assert_int_eq(3, e->xpValue);
    mu_assert_int_eq(2, e->splitCount);
    return 0;
}

const char* run_enemy_tests(void)
{
    mu_run_test(test_enemy_pool_init);
    mu_run_test(test_enemy_spawn_single);
    mu_run_test(test_enemy_spawn_chaser_stats);
    mu_run_test(test_enemy_spawn_orbiter_stats);
    mu_run_test(test_enemy_spawn_splitter_stats);
    mu_run_test(test_enemy_spawn_multiple);
    mu_run_test(test_enemy_pool_full);
    mu_run_test(test_enemy_reuse_slot);
    return 0;
}
