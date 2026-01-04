#include "minunit.h"
#include "../src/utils.h"
#include <stdbool.h>

#define MAX_PROJECTILES 1000

typedef struct Projectile {
    Vector2 pos;
    Vector2 vel;
    float radius;
    float damage;
    float lifetime;
    int weaponType;
    bool pierce;
    bool active;
} Projectile;

typedef struct ProjectilePool {
    Projectile projectiles[MAX_PROJECTILES];
    int count;
} ProjectilePool;

static void ProjectilePoolInit(ProjectilePool *pool)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        pool->projectiles[i].active = false;
    }
    pool->count = 0;
}

static Projectile* ProjectileSpawn(ProjectilePool *pool, Vector2 pos, Vector2 vel, float damage, float radius, float lifetime)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        if (!pool->projectiles[i].active)
        {
            Projectile *p = &pool->projectiles[i];
            p->pos = pos;
            p->vel = vel;
            p->damage = damage;
            p->radius = radius;
            p->lifetime = lifetime;
            p->weaponType = 0;
            p->pierce = false;
            p->active = true;
            pool->count++;
            return p;
        }
    }
    return NULL;
}

static void ProjectilePoolUpdate(ProjectilePool *pool, float dt)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        Projectile *p = &pool->projectiles[i];
        if (!p->active) continue;

        p->pos.x += p->vel.x * dt;
        p->pos.y += p->vel.y * dt;
        p->lifetime -= dt;

        if (p->lifetime <= 0.0f)
        {
            p->active = false;
            pool->count--;
        }
    }
}

// Tests

static const char* test_projectile_pool_init(void)
{
    ProjectilePool pool;
    ProjectilePoolInit(&pool);
    mu_assert_int_eq(0, pool.count);
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        mu_assert_false(pool.projectiles[i].active);
    }
    return 0;
}

static const char* test_projectile_spawn_single(void)
{
    ProjectilePool pool;
    ProjectilePoolInit(&pool);

    Vector2 pos = { 100.0f, 200.0f };
    Vector2 vel = { 500.0f, 0.0f };
    Projectile *p = ProjectileSpawn(&pool, pos, vel, 10.0f, 5.0f, 2.0f);

    mu_assert(p != NULL, "ProjectileSpawn should return non-null");
    mu_assert_int_eq(1, pool.count);
    mu_assert_float_eq(100.0f, p->pos.x);
    mu_assert_float_eq(200.0f, p->pos.y);
    mu_assert_float_eq(500.0f, p->vel.x);
    mu_assert_float_eq(0.0f, p->vel.y);
    mu_assert_float_eq(10.0f, p->damage);
    mu_assert_float_eq(5.0f, p->radius);
    mu_assert_float_eq(2.0f, p->lifetime);
    mu_assert_true(p->active);
    mu_assert_false(p->pierce);
    return 0;
}

static const char* test_projectile_spawn_multiple(void)
{
    ProjectilePool pool;
    ProjectilePoolInit(&pool);

    for (int i = 0; i < 50; i++)
    {
        Vector2 pos = { (float)i * 10.0f, 0.0f };
        Vector2 vel = { 100.0f, 0.0f };
        ProjectileSpawn(&pool, pos, vel, 10.0f, 5.0f, 2.0f);
    }

    mu_assert_int_eq(50, pool.count);
    return 0;
}

static const char* test_projectile_pool_full(void)
{
    ProjectilePool pool;
    ProjectilePoolInit(&pool);

    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        Vector2 pos = { 0.0f, 0.0f };
        Vector2 vel = { 100.0f, 0.0f };
        ProjectileSpawn(&pool, pos, vel, 10.0f, 5.0f, 2.0f);
    }

    mu_assert_int_eq(MAX_PROJECTILES, pool.count);

    Vector2 pos = { 0.0f, 0.0f };
    Vector2 vel = { 100.0f, 0.0f };
    Projectile *p = ProjectileSpawn(&pool, pos, vel, 10.0f, 5.0f, 2.0f);
    mu_assert(p == NULL, "Pool full should return null");
    mu_assert_int_eq(MAX_PROJECTILES, pool.count);
    return 0;
}

static const char* test_projectile_reuse_slot(void)
{
    ProjectilePool pool;
    ProjectilePoolInit(&pool);

    Vector2 pos1 = { 100.0f, 100.0f };
    Vector2 vel = { 100.0f, 0.0f };
    Projectile *p1 = ProjectileSpawn(&pool, pos1, vel, 10.0f, 5.0f, 2.0f);
    mu_assert_int_eq(1, pool.count);

    p1->active = false;
    pool.count--;
    mu_assert_int_eq(0, pool.count);

    Vector2 pos2 = { 200.0f, 200.0f };
    Projectile *p2 = ProjectileSpawn(&pool, pos2, vel, 10.0f, 5.0f, 2.0f);
    mu_assert_int_eq(1, pool.count);
    mu_assert(p1 == p2, "Should reuse first slot");
    mu_assert_float_eq(200.0f, p2->pos.x);
    return 0;
}

static const char* test_projectile_update_movement(void)
{
    ProjectilePool pool;
    ProjectilePoolInit(&pool);

    Vector2 pos = { 100.0f, 100.0f };
    Vector2 vel = { 200.0f, 100.0f };
    Projectile *p = ProjectileSpawn(&pool, pos, vel, 10.0f, 5.0f, 2.0f);

    ProjectilePoolUpdate(&pool, 0.5f);

    mu_assert_float_eq(200.0f, p->pos.x);  // 100 + 200*0.5
    mu_assert_float_eq(150.0f, p->pos.y);  // 100 + 100*0.5
    mu_assert_float_eq(1.5f, p->lifetime); // 2.0 - 0.5
    mu_assert_true(p->active);
    return 0;
}

static const char* test_projectile_expires_on_lifetime(void)
{
    ProjectilePool pool;
    ProjectilePoolInit(&pool);

    Vector2 pos = { 100.0f, 100.0f };
    Vector2 vel = { 200.0f, 0.0f };
    Projectile *p = ProjectileSpawn(&pool, pos, vel, 10.0f, 5.0f, 1.0f);
    mu_assert_int_eq(1, pool.count);

    ProjectilePoolUpdate(&pool, 1.5f);

    mu_assert_false(p->active);
    mu_assert_int_eq(0, pool.count);
    return 0;
}

static const char* test_projectile_update_multiple(void)
{
    ProjectilePool pool;
    ProjectilePoolInit(&pool);

    Vector2 pos1 = { 0.0f, 0.0f };
    Vector2 vel1 = { 100.0f, 0.0f };
    Projectile *p1 = ProjectileSpawn(&pool, pos1, vel1, 10.0f, 5.0f, 0.5f);

    Vector2 pos2 = { 0.0f, 0.0f };
    Vector2 vel2 = { 0.0f, 100.0f };
    Projectile *p2 = ProjectileSpawn(&pool, pos2, vel2, 10.0f, 5.0f, 2.0f);

    mu_assert_int_eq(2, pool.count);

    ProjectilePoolUpdate(&pool, 1.0f);

    // First projectile should be expired
    mu_assert_false(p1->active);
    // Second should still be active
    mu_assert_true(p2->active);
    mu_assert_float_eq(100.0f, p2->pos.y);
    mu_assert_int_eq(1, pool.count);
    return 0;
}

static const char* test_projectile_negative_velocity(void)
{
    ProjectilePool pool;
    ProjectilePoolInit(&pool);

    Vector2 pos = { 500.0f, 500.0f };
    Vector2 vel = { -100.0f, -50.0f };
    Projectile *p = ProjectileSpawn(&pool, pos, vel, 10.0f, 5.0f, 2.0f);

    ProjectilePoolUpdate(&pool, 1.0f);

    mu_assert_float_eq(400.0f, p->pos.x);  // 500 + (-100)*1.0
    mu_assert_float_eq(450.0f, p->pos.y);  // 500 + (-50)*1.0
    return 0;
}

const char* run_projectile_tests(void)
{
    mu_run_test(test_projectile_pool_init);
    mu_run_test(test_projectile_spawn_single);
    mu_run_test(test_projectile_spawn_multiple);
    mu_run_test(test_projectile_pool_full);
    mu_run_test(test_projectile_reuse_slot);
    mu_run_test(test_projectile_update_movement);
    mu_run_test(test_projectile_expires_on_lifetime);
    mu_run_test(test_projectile_update_multiple);
    mu_run_test(test_projectile_negative_velocity);
    return 0;
}
