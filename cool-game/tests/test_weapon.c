#include "minunit.h"
#include "../src/utils.h"
#include <stdbool.h>

typedef enum WeaponType {
    WEAPON_PULSE_CANNON,
    WEAPON_COUNT
} WeaponType;

typedef struct Weapon {
    WeaponType type;
    float damage;
    float fireRate;
    float projectileSpeed;
    float projectileRadius;
    float projectileLifetime;
    int projectileCount;
    float cooldown;
    int level;
} Weapon;

static void WeaponInit(Weapon *weapon, WeaponType type)
{
    weapon->type = type;
    weapon->cooldown = 0.0f;
    weapon->level = 1;

    switch (type)
    {
        case WEAPON_PULSE_CANNON:
        default:
            weapon->damage = 10.0f;
            weapon->fireRate = 5.0f;
            weapon->projectileSpeed = 500.0f;
            weapon->projectileRadius = 5.0f;
            weapon->projectileLifetime = 2.0f;
            weapon->projectileCount = 1;
            break;
    }
}

static void WeaponUpdate(Weapon *weapon, float dt)
{
    if (weapon->cooldown > 0.0f)
    {
        weapon->cooldown -= dt;
        if (weapon->cooldown < 0.0f) weapon->cooldown = 0.0f;
    }
}

static bool WeaponCanFire(Weapon *weapon)
{
    return weapon->cooldown <= 0.0f;
}

// Tests

static const char* test_weapon_init_pulse_cannon(void)
{
    Weapon weapon;
    WeaponInit(&weapon, WEAPON_PULSE_CANNON);

    mu_assert_int_eq(WEAPON_PULSE_CANNON, weapon.type);
    mu_assert_float_eq(10.0f, weapon.damage);
    mu_assert_float_eq(5.0f, weapon.fireRate);
    mu_assert_float_eq(500.0f, weapon.projectileSpeed);
    mu_assert_float_eq(5.0f, weapon.projectileRadius);
    mu_assert_float_eq(2.0f, weapon.projectileLifetime);
    mu_assert_int_eq(1, weapon.projectileCount);
    mu_assert_float_eq(0.0f, weapon.cooldown);
    mu_assert_int_eq(1, weapon.level);
    return 0;
}

static const char* test_weapon_can_fire_initially(void)
{
    Weapon weapon;
    WeaponInit(&weapon, WEAPON_PULSE_CANNON);

    mu_assert_true(WeaponCanFire(&weapon));
    return 0;
}

static const char* test_weapon_cannot_fire_on_cooldown(void)
{
    Weapon weapon;
    WeaponInit(&weapon, WEAPON_PULSE_CANNON);
    weapon.cooldown = 0.5f;

    mu_assert_false(WeaponCanFire(&weapon));
    return 0;
}

static const char* test_weapon_update_reduces_cooldown(void)
{
    Weapon weapon;
    WeaponInit(&weapon, WEAPON_PULSE_CANNON);
    weapon.cooldown = 1.0f;

    WeaponUpdate(&weapon, 0.3f);

    mu_assert_float_eq(0.7f, weapon.cooldown);
    mu_assert_false(WeaponCanFire(&weapon));
    return 0;
}

static const char* test_weapon_cooldown_clamps_to_zero(void)
{
    Weapon weapon;
    WeaponInit(&weapon, WEAPON_PULSE_CANNON);
    weapon.cooldown = 0.2f;

    WeaponUpdate(&weapon, 0.5f);

    mu_assert_float_eq(0.0f, weapon.cooldown);
    mu_assert_true(WeaponCanFire(&weapon));
    return 0;
}

static const char* test_weapon_cooldown_multiple_updates(void)
{
    Weapon weapon;
    WeaponInit(&weapon, WEAPON_PULSE_CANNON);
    weapon.cooldown = 1.0f;

    WeaponUpdate(&weapon, 0.3f);
    mu_assert_false(WeaponCanFire(&weapon));

    WeaponUpdate(&weapon, 0.3f);
    mu_assert_false(WeaponCanFire(&weapon));

    WeaponUpdate(&weapon, 0.3f);
    mu_assert_false(WeaponCanFire(&weapon));

    WeaponUpdate(&weapon, 0.3f);
    mu_assert_true(WeaponCanFire(&weapon));
    return 0;
}

static const char* test_weapon_update_no_effect_when_zero(void)
{
    Weapon weapon;
    WeaponInit(&weapon, WEAPON_PULSE_CANNON);
    weapon.cooldown = 0.0f;

    WeaponUpdate(&weapon, 0.5f);

    mu_assert_float_eq(0.0f, weapon.cooldown);
    mu_assert_true(WeaponCanFire(&weapon));
    return 0;
}

static const char* test_weapon_fire_rate_meaning(void)
{
    // Fire rate of 5.0 means 5 shots per second
    // So cooldown after firing should be 1.0 / 5.0 = 0.2 seconds
    Weapon weapon;
    WeaponInit(&weapon, WEAPON_PULSE_CANNON);

    float expectedCooldown = 1.0f / weapon.fireRate;
    mu_assert_float_eq(0.2f, expectedCooldown);
    return 0;
}

static const char* test_weapon_stats_can_be_modified(void)
{
    Weapon weapon;
    WeaponInit(&weapon, WEAPON_PULSE_CANNON);

    // Simulate upgrades
    weapon.damage *= 1.25f;
    weapon.fireRate *= 1.2f;
    weapon.projectileCount += 1;

    mu_assert_float_eq(12.5f, weapon.damage);
    mu_assert_float_eq(6.0f, weapon.fireRate);
    mu_assert_int_eq(2, weapon.projectileCount);
    return 0;
}

const char* run_weapon_tests(void)
{
    mu_run_test(test_weapon_init_pulse_cannon);
    mu_run_test(test_weapon_can_fire_initially);
    mu_run_test(test_weapon_cannot_fire_on_cooldown);
    mu_run_test(test_weapon_update_reduces_cooldown);
    mu_run_test(test_weapon_cooldown_clamps_to_zero);
    mu_run_test(test_weapon_cooldown_multiple_updates);
    mu_run_test(test_weapon_update_no_effect_when_zero);
    mu_run_test(test_weapon_fire_rate_meaning);
    mu_run_test(test_weapon_stats_can_be_modified);
    return 0;
}
