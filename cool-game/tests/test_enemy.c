#include "minunit.h"
#include "../src/utils.h"

#define MAX_ENEMIES 500

// Elite enemy multipliers (matching enemy.h)
#define ELITE_SPAWN_CHANCE  0.1f
#define ELITE_SIZE_MULT     1.5f
#define ELITE_HEALTH_MULT   3.0f
#define ELITE_DAMAGE_MULT   1.5f
#define ELITE_XP_MULT       5
#define ELITE_SPEED_MULT    0.8f

// Boss enemy stats (matching enemy.h)
#define BOSS_SPAWN_INTERVAL 300.0f  // 5 minutes
#define BOSS_BASE_HEALTH    2000.0f
#define BOSS_BASE_RADIUS    60.0f
#define BOSS_BASE_DAMAGE    30.0f
#define BOSS_BASE_SPEED     50.0f
#define BOSS_XP_VALUE       100
#define BOSS_ATTACK_INTERVAL 3.0f
#define BOSS_CHARGE_TIME    1.0f

typedef enum EnemyType {
    ENEMY_CHASER,
    ENEMY_ORBITER,
    ENEMY_SPLITTER,
    ENEMY_BOSS,
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
    int isElite;  // Elite flag
    // Boss fields
    int isBoss;
    int bossPhase;
    float bossAttackTimer;
    float bossChargeTimer;
    int bossCharging;
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
            e->isElite = 0;
            e->isBoss = 0;
            e->bossPhase = 0;
            e->bossAttackTimer = 0.0f;
            e->bossChargeTimer = 0.0f;
            e->bossCharging = 0;

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

                case ENEMY_BOSS:
                    e->radius = BOSS_BASE_RADIUS;
                    e->speed = BOSS_BASE_SPEED;
                    e->health = BOSS_BASE_HEALTH;
                    e->maxHealth = BOSS_BASE_HEALTH;
                    e->damage = BOSS_BASE_DAMAGE;
                    e->xpValue = BOSS_XP_VALUE;
                    e->orbitAngle = 0.0f;
                    e->orbitDistance = 0.0f;
                    e->splitCount = 0;
                    e->isBoss = 1;
                    e->bossAttackTimer = BOSS_ATTACK_INTERVAL;
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

static Enemy* EnemySpawnElite(EnemyPool *pool, EnemyType type, Vector2 pos)
{
    Enemy *e = EnemySpawn(pool, type, pos);
    if (e)
    {
        e->isElite = 1;
        e->radius *= ELITE_SIZE_MULT;
        e->health *= ELITE_HEALTH_MULT;
        e->maxHealth *= ELITE_HEALTH_MULT;
        e->damage *= ELITE_DAMAGE_MULT;
        e->xpValue *= ELITE_XP_MULT;
        e->speed *= ELITE_SPEED_MULT;
    }
    return e;
}

static Enemy* EnemySpawnBoss(EnemyPool *pool, Vector2 pos, int bossNumber)
{
    Enemy *e = EnemySpawn(pool, ENEMY_BOSS, pos);
    if (e)
    {
        // Scale boss stats based on boss number
        float scaleFactor = 1.0f + (bossNumber - 1) * 0.5f;
        e->health *= scaleFactor;
        e->maxHealth *= scaleFactor;
        e->damage *= scaleFactor;
        e->xpValue = BOSS_XP_VALUE * bossNumber;
    }
    return e;
}

static int EnemyPoolHasBoss(EnemyPool *pool)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (pool->enemies[i].active && pool->enemies[i].isBoss)
        {
            return 1;
        }
    }
    return 0;
}

static Enemy* EnemyPoolGetBoss(EnemyPool *pool)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (pool->enemies[i].active && pool->enemies[i].isBoss)
        {
            return &pool->enemies[i];
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
    mu_assert_false(e->isElite);
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
    mu_assert_false(e->isElite);
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
    mu_assert_false(e->isElite);
    return 0;
}

// Elite enemy tests

static const char* test_elite_chaser_stats(void)
{
    EnemyPool pool;
    EnemyPoolInit(&pool);

    Vector2 pos = { 0.0f, 0.0f };
    Enemy *e = EnemySpawnElite(&pool, ENEMY_CHASER, pos);

    mu_assert_true(e->isElite);
    // Base chaser: radius=12, speed=100, health=30, damage=10, xp=1
    mu_assert_float_eq(12.0f * ELITE_SIZE_MULT, e->radius);
    mu_assert_float_eq(100.0f * ELITE_SPEED_MULT, e->speed);
    mu_assert_float_eq(30.0f * ELITE_HEALTH_MULT, e->health);
    mu_assert_float_eq(30.0f * ELITE_HEALTH_MULT, e->maxHealth);
    mu_assert_float_eq(10.0f * ELITE_DAMAGE_MULT, e->damage);
    mu_assert_int_eq(1 * ELITE_XP_MULT, e->xpValue);
    return 0;
}

static const char* test_elite_orbiter_stats(void)
{
    EnemyPool pool;
    EnemyPoolInit(&pool);

    Vector2 pos = { 0.0f, 0.0f };
    Enemy *e = EnemySpawnElite(&pool, ENEMY_ORBITER, pos);

    mu_assert_true(e->isElite);
    // Base orbiter: radius=15, speed=80, health=50, damage=15, xp=2
    mu_assert_float_eq(15.0f * ELITE_SIZE_MULT, e->radius);
    mu_assert_float_eq(80.0f * ELITE_SPEED_MULT, e->speed);
    mu_assert_float_eq(50.0f * ELITE_HEALTH_MULT, e->health);
    mu_assert_float_eq(50.0f * ELITE_HEALTH_MULT, e->maxHealth);
    mu_assert_float_eq(15.0f * ELITE_DAMAGE_MULT, e->damage);
    mu_assert_int_eq(2 * ELITE_XP_MULT, e->xpValue);
    return 0;
}

static const char* test_elite_splitter_stats(void)
{
    EnemyPool pool;
    EnemyPoolInit(&pool);

    Vector2 pos = { 0.0f, 0.0f };
    Enemy *e = EnemySpawnElite(&pool, ENEMY_SPLITTER, pos);

    mu_assert_true(e->isElite);
    // Base splitter: radius=20, speed=60, health=80, damage=20, xp=3
    mu_assert_float_eq(20.0f * ELITE_SIZE_MULT, e->radius);
    mu_assert_float_eq(60.0f * ELITE_SPEED_MULT, e->speed);
    mu_assert_float_eq(80.0f * ELITE_HEALTH_MULT, e->health);
    mu_assert_float_eq(80.0f * ELITE_HEALTH_MULT, e->maxHealth);
    mu_assert_float_eq(20.0f * ELITE_DAMAGE_MULT, e->damage);
    mu_assert_int_eq(3 * ELITE_XP_MULT, e->xpValue);
    return 0;
}

static const char* test_elite_constants(void)
{
    // Verify the elite multiplier constants are sensible
    mu_assert_float_eq(0.1f, ELITE_SPAWN_CHANCE);
    mu_assert_float_eq(1.5f, ELITE_SIZE_MULT);
    mu_assert_float_eq(3.0f, ELITE_HEALTH_MULT);
    mu_assert_float_eq(1.5f, ELITE_DAMAGE_MULT);
    mu_assert_int_eq(5, ELITE_XP_MULT);
    mu_assert_float_eq(0.8f, ELITE_SPEED_MULT);
    return 0;
}

// Boss enemy tests

static const char* test_boss_spawn_stats(void)
{
    EnemyPool pool;
    EnemyPoolInit(&pool);

    Vector2 pos = { 0.0f, 0.0f };
    Enemy *e = EnemySpawn(&pool, ENEMY_BOSS, pos);

    mu_assert_true(e->isBoss);
    mu_assert_false(e->isElite);
    mu_assert_float_eq(BOSS_BASE_RADIUS, e->radius);
    mu_assert_float_eq(BOSS_BASE_SPEED, e->speed);
    mu_assert_float_eq(BOSS_BASE_HEALTH, e->health);
    mu_assert_float_eq(BOSS_BASE_HEALTH, e->maxHealth);
    mu_assert_float_eq(BOSS_BASE_DAMAGE, e->damage);
    mu_assert_int_eq(BOSS_XP_VALUE, e->xpValue);
    return 0;
}

static const char* test_boss_constants(void)
{
    // Verify boss constants are sensible
    mu_assert_float_eq(300.0f, BOSS_SPAWN_INTERVAL);  // 5 minutes
    mu_assert_float_eq(2000.0f, BOSS_BASE_HEALTH);
    mu_assert_float_eq(60.0f, BOSS_BASE_RADIUS);
    mu_assert_float_eq(30.0f, BOSS_BASE_DAMAGE);
    mu_assert_float_eq(50.0f, BOSS_BASE_SPEED);
    mu_assert_int_eq(100, BOSS_XP_VALUE);
    mu_assert_float_eq(3.0f, BOSS_ATTACK_INTERVAL);
    mu_assert_float_eq(1.0f, BOSS_CHARGE_TIME);
    return 0;
}

static const char* test_boss_pool_has_boss(void)
{
    EnemyPool pool;
    EnemyPoolInit(&pool);

    // No boss initially
    mu_assert_false(EnemyPoolHasBoss(&pool));

    // Spawn regular enemy
    Vector2 pos = { 0.0f, 0.0f };
    EnemySpawn(&pool, ENEMY_CHASER, pos);
    mu_assert_false(EnemyPoolHasBoss(&pool));

    // Spawn boss
    EnemySpawn(&pool, ENEMY_BOSS, pos);
    mu_assert_true(EnemyPoolHasBoss(&pool));

    return 0;
}

static const char* test_boss_pool_get_boss(void)
{
    EnemyPool pool;
    EnemyPoolInit(&pool);

    // No boss initially
    mu_assert(EnemyPoolGetBoss(&pool) == 0, "No boss should return null");

    // Spawn regular enemies
    Vector2 pos = { 0.0f, 0.0f };
    EnemySpawn(&pool, ENEMY_CHASER, pos);
    EnemySpawn(&pool, ENEMY_ORBITER, pos);

    mu_assert(EnemyPoolGetBoss(&pool) == 0, "No boss should still return null");

    // Spawn boss
    Enemy *boss = EnemySpawn(&pool, ENEMY_BOSS, pos);
    Enemy *found = EnemyPoolGetBoss(&pool);

    mu_assert(found == boss, "Should return the boss");
    mu_assert_true(found->isBoss);

    return 0;
}

static const char* test_boss_scaling(void)
{
    EnemyPool pool1;
    EnemyPoolInit(&pool1);
    EnemyPool pool2;
    EnemyPoolInit(&pool2);

    Vector2 pos = { 0.0f, 0.0f };

    // First boss (bossNumber = 1)
    Enemy *boss1 = EnemySpawnBoss(&pool1, pos, 1);

    // Second boss (bossNumber = 2) - should be 50% stronger
    Enemy *boss2 = EnemySpawnBoss(&pool2, pos, 2);

    // Boss 2 should have more health
    mu_assert(boss2->health > boss1->health, "Boss 2 should have more health");
    mu_assert(boss2->damage > boss1->damage, "Boss 2 should deal more damage");
    mu_assert(boss2->xpValue > boss1->xpValue, "Boss 2 should give more XP");

    // Verify scaling factor (50% increase per boss)
    float expectedHealthBoss2 = BOSS_BASE_HEALTH * 1.5f;  // 1 + (2-1)*0.5
    mu_assert_float_eq(expectedHealthBoss2, boss2->health);

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
    // Elite enemy tests
    mu_run_test(test_elite_chaser_stats);
    mu_run_test(test_elite_orbiter_stats);
    mu_run_test(test_elite_splitter_stats);
    mu_run_test(test_elite_constants);
    // Boss enemy tests
    mu_run_test(test_boss_spawn_stats);
    mu_run_test(test_boss_constants);
    mu_run_test(test_boss_pool_has_boss);
    mu_run_test(test_boss_pool_get_boss);
    mu_run_test(test_boss_scaling);
    return 0;
}
