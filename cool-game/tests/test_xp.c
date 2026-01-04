#include "minunit.h"
#include "../src/utils.h"
#include <stdbool.h>
#include <math.h>

#define MAX_XP_CRYSTALS 300
#define XP_CRYSTAL_RADIUS 6.0f
#define XP_CRYSTAL_LIFETIME 30.0f

typedef struct XPCrystal {
    Vector2 pos;
    int value;
    float radius;
    float lifetime;
    bool active;
} XPCrystal;

typedef struct XPPool {
    XPCrystal crystals[MAX_XP_CRYSTALS];
    int count;
} XPPool;

static void XPPoolInit(XPPool *pool)
{
    for (int i = 0; i < MAX_XP_CRYSTALS; i++)
    {
        pool->crystals[i].active = false;
    }
    pool->count = 0;
}

static void XPSpawn(XPPool *pool, Vector2 pos, int value)
{
    for (int i = 0; i < MAX_XP_CRYSTALS; i++)
    {
        XPCrystal *xp = &pool->crystals[i];
        if (!xp->active)
        {
            xp->pos = pos;
            xp->value = value;
            xp->radius = XP_CRYSTAL_RADIUS;
            xp->lifetime = XP_CRYSTAL_LIFETIME;
            xp->active = true;
            pool->count++;
            return;
        }
    }
}

static void XPPoolUpdate(XPPool *pool, float dt)
{
    for (int i = 0; i < MAX_XP_CRYSTALS; i++)
    {
        XPCrystal *xp = &pool->crystals[i];
        if (!xp->active) continue;

        xp->lifetime -= dt;
        if (xp->lifetime <= 0.0f)
        {
            xp->active = false;
            pool->count--;
        }
    }
}

static int XPCollect(XPPool *pool, Vector2 playerPos, float collectRadius)
{
    int totalCollected = 0;

    for (int i = 0; i < MAX_XP_CRYSTALS; i++)
    {
        XPCrystal *xp = &pool->crystals[i];
        if (!xp->active) continue;

        float distSq = Vector2DistanceSq(xp->pos, playerPos);
        if (distSq < collectRadius * collectRadius)
        {
            totalCollected += xp->value;
            xp->active = false;
            pool->count--;
        }
    }

    return totalCollected;
}

// Tests

static const char* test_xp_pool_init(void)
{
    XPPool pool;
    XPPoolInit(&pool);
    mu_assert_int_eq(0, pool.count);
    for (int i = 0; i < MAX_XP_CRYSTALS; i++)
    {
        mu_assert_false(pool.crystals[i].active);
    }
    return 0;
}

static const char* test_xp_spawn_single(void)
{
    XPPool pool;
    XPPoolInit(&pool);

    Vector2 pos = { 100.0f, 200.0f };
    XPSpawn(&pool, pos, 5);

    mu_assert_int_eq(1, pool.count);
    mu_assert_true(pool.crystals[0].active);
    mu_assert_float_eq(100.0f, pool.crystals[0].pos.x);
    mu_assert_float_eq(200.0f, pool.crystals[0].pos.y);
    mu_assert_int_eq(5, pool.crystals[0].value);
    mu_assert_float_eq(XP_CRYSTAL_RADIUS, pool.crystals[0].radius);
    mu_assert_float_eq(XP_CRYSTAL_LIFETIME, pool.crystals[0].lifetime);
    return 0;
}

static const char* test_xp_spawn_multiple(void)
{
    XPPool pool;
    XPPoolInit(&pool);

    for (int i = 0; i < 20; i++)
    {
        Vector2 pos = { (float)i * 10.0f, 0.0f };
        XPSpawn(&pool, pos, i + 1);
    }

    mu_assert_int_eq(20, pool.count);
    return 0;
}

static const char* test_xp_pool_full(void)
{
    XPPool pool;
    XPPoolInit(&pool);

    for (int i = 0; i < MAX_XP_CRYSTALS; i++)
    {
        Vector2 pos = { 0.0f, 0.0f };
        XPSpawn(&pool, pos, 1);
    }

    mu_assert_int_eq(MAX_XP_CRYSTALS, pool.count);

    // Try to spawn one more
    Vector2 pos = { 500.0f, 500.0f };
    XPSpawn(&pool, pos, 99);

    // Count should not change
    mu_assert_int_eq(MAX_XP_CRYSTALS, pool.count);
    return 0;
}

static const char* test_xp_collect_single(void)
{
    XPPool pool;
    XPPoolInit(&pool);

    Vector2 xpPos = { 100.0f, 100.0f };
    XPSpawn(&pool, xpPos, 5);

    Vector2 playerPos = { 105.0f, 100.0f };  // 5 units away
    int collected = XPCollect(&pool, playerPos, 10.0f);  // 10 unit radius

    mu_assert_int_eq(5, collected);
    mu_assert_int_eq(0, pool.count);
    mu_assert_false(pool.crystals[0].active);
    return 0;
}

static const char* test_xp_collect_none_out_of_range(void)
{
    XPPool pool;
    XPPoolInit(&pool);

    Vector2 xpPos = { 100.0f, 100.0f };
    XPSpawn(&pool, xpPos, 5);

    Vector2 playerPos = { 200.0f, 200.0f };  // Far away
    int collected = XPCollect(&pool, playerPos, 10.0f);

    mu_assert_int_eq(0, collected);
    mu_assert_int_eq(1, pool.count);
    mu_assert_true(pool.crystals[0].active);
    return 0;
}

static const char* test_xp_collect_multiple(void)
{
    XPPool pool;
    XPPoolInit(&pool);

    // Spawn 3 crystals near player
    Vector2 pos1 = { 100.0f, 100.0f };
    Vector2 pos2 = { 105.0f, 100.0f };
    Vector2 pos3 = { 100.0f, 105.0f };
    XPSpawn(&pool, pos1, 1);
    XPSpawn(&pool, pos2, 2);
    XPSpawn(&pool, pos3, 3);

    // Spawn 1 crystal far away
    Vector2 posFar = { 500.0f, 500.0f };
    XPSpawn(&pool, posFar, 10);

    mu_assert_int_eq(4, pool.count);

    Vector2 playerPos = { 102.0f, 102.0f };
    int collected = XPCollect(&pool, playerPos, 15.0f);

    mu_assert_int_eq(6, collected);  // 1 + 2 + 3
    mu_assert_int_eq(1, pool.count); // Only far one remains
    return 0;
}

static const char* test_xp_collect_at_boundary(void)
{
    XPPool pool;
    XPPoolInit(&pool);

    Vector2 xpPos = { 110.0f, 100.0f };
    XPSpawn(&pool, xpPos, 5);

    Vector2 playerPos = { 100.0f, 100.0f };
    // XP is exactly 10 units away, collect radius is 10
    // distSq = 100, collectRadius^2 = 100, so distSq < collectRadius^2 is false
    int collected = XPCollect(&pool, playerPos, 10.0f);

    mu_assert_int_eq(0, collected);  // Exactly at boundary = not collected
    mu_assert_int_eq(1, pool.count);
    return 0;
}

static const char* test_xp_lifetime_expiration(void)
{
    XPPool pool;
    XPPoolInit(&pool);

    Vector2 pos = { 100.0f, 100.0f };
    XPSpawn(&pool, pos, 5);
    mu_assert_int_eq(1, pool.count);

    XPPoolUpdate(&pool, XP_CRYSTAL_LIFETIME + 1.0f);

    mu_assert_int_eq(0, pool.count);
    mu_assert_false(pool.crystals[0].active);
    return 0;
}

static const char* test_xp_lifetime_not_expired(void)
{
    XPPool pool;
    XPPoolInit(&pool);

    Vector2 pos = { 100.0f, 100.0f };
    XPSpawn(&pool, pos, 5);

    XPPoolUpdate(&pool, 10.0f);  // Not enough to expire

    mu_assert_int_eq(1, pool.count);
    mu_assert_true(pool.crystals[0].active);
    mu_assert_float_eq(XP_CRYSTAL_LIFETIME - 10.0f, pool.crystals[0].lifetime);
    return 0;
}

static const char* test_xp_reuse_slot(void)
{
    XPPool pool;
    XPPoolInit(&pool);

    Vector2 pos1 = { 100.0f, 100.0f };
    XPSpawn(&pool, pos1, 5);

    // Collect it
    Vector2 playerPos = { 100.0f, 100.0f };
    XPCollect(&pool, playerPos, 10.0f);
    mu_assert_int_eq(0, pool.count);

    // Spawn another
    Vector2 pos2 = { 200.0f, 200.0f };
    XPSpawn(&pool, pos2, 10);

    mu_assert_int_eq(1, pool.count);
    mu_assert_float_eq(200.0f, pool.crystals[0].pos.x);
    mu_assert_int_eq(10, pool.crystals[0].value);
    return 0;
}

static const char* test_xp_different_values(void)
{
    XPPool pool;
    XPPoolInit(&pool);

    Vector2 pos = { 100.0f, 100.0f };
    XPSpawn(&pool, pos, 1);

    pos = (Vector2){ 101.0f, 100.0f };
    XPSpawn(&pool, pos, 2);

    pos = (Vector2){ 102.0f, 100.0f };
    XPSpawn(&pool, pos, 3);

    mu_assert_int_eq(1, pool.crystals[0].value);
    mu_assert_int_eq(2, pool.crystals[1].value);
    mu_assert_int_eq(3, pool.crystals[2].value);
    return 0;
}

const char* run_xp_tests(void)
{
    mu_run_test(test_xp_pool_init);
    mu_run_test(test_xp_spawn_single);
    mu_run_test(test_xp_spawn_multiple);
    mu_run_test(test_xp_pool_full);
    mu_run_test(test_xp_collect_single);
    mu_run_test(test_xp_collect_none_out_of_range);
    mu_run_test(test_xp_collect_multiple);
    mu_run_test(test_xp_collect_at_boundary);
    mu_run_test(test_xp_lifetime_expiration);
    mu_run_test(test_xp_lifetime_not_expired);
    mu_run_test(test_xp_reuse_slot);
    mu_run_test(test_xp_different_values);
    return 0;
}
