#include "minunit.h"
#include "../src/utils.h"
#include <stdbool.h>

// Weapon types matching weapon.h
typedef enum WeaponType {
    // Base weapons
    WEAPON_PULSE_CANNON,
    WEAPON_SPREAD_SHOT,
    WEAPON_HOMING_MISSILE,
    WEAPON_LIGHTNING,
    WEAPON_ORBIT_SHIELD,
    WEAPON_FLAMETHROWER,
    WEAPON_FREEZE_RAY,
    WEAPON_BLACK_HOLE,
    WEAPON_BASE_COUNT,
    // Evolved weapons
    WEAPON_MEGA_CANNON = WEAPON_BASE_COUNT,
    WEAPON_CIRCLE_BURST,
    WEAPON_SWARM,
    WEAPON_TESLA_COIL,
    WEAPON_BLADE_DANCER,
    WEAPON_INFERNO,
    WEAPON_BLIZZARD,
    WEAPON_SINGULARITY,
    WEAPON_COUNT
} WeaponType;

#define WEAPON_MAX_LEVEL 5

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

// Evolution helper functions (matching weapon.c)
static bool WeaponIsEvolved(WeaponType type)
{
    return type >= WEAPON_BASE_COUNT && type < WEAPON_COUNT;
}

static bool WeaponCanEvolve(Weapon *weapon, bool hasCatalyst)
{
    if (weapon->level < WEAPON_MAX_LEVEL) return false;
    if (weapon->type >= WEAPON_BASE_COUNT) return false;
    return hasCatalyst;
}

static WeaponType WeaponGetEvolvedType(WeaponType baseType)
{
    switch (baseType)
    {
        case WEAPON_PULSE_CANNON:  return WEAPON_MEGA_CANNON;
        case WEAPON_SPREAD_SHOT:   return WEAPON_CIRCLE_BURST;
        case WEAPON_HOMING_MISSILE: return WEAPON_SWARM;
        case WEAPON_LIGHTNING:     return WEAPON_TESLA_COIL;
        case WEAPON_ORBIT_SHIELD:  return WEAPON_BLADE_DANCER;
        case WEAPON_FLAMETHROWER:  return WEAPON_INFERNO;
        case WEAPON_FREEZE_RAY:    return WEAPON_BLIZZARD;
        case WEAPON_BLACK_HOLE:    return WEAPON_SINGULARITY;
        default: return baseType;
    }
}

static WeaponType WeaponGetBaseType(WeaponType evolvedType)
{
    switch (evolvedType)
    {
        case WEAPON_MEGA_CANNON:   return WEAPON_PULSE_CANNON;
        case WEAPON_CIRCLE_BURST:  return WEAPON_SPREAD_SHOT;
        case WEAPON_SWARM:         return WEAPON_HOMING_MISSILE;
        case WEAPON_TESLA_COIL:    return WEAPON_LIGHTNING;
        case WEAPON_BLADE_DANCER:  return WEAPON_ORBIT_SHIELD;
        case WEAPON_INFERNO:       return WEAPON_FLAMETHROWER;
        case WEAPON_BLIZZARD:      return WEAPON_FREEZE_RAY;
        case WEAPON_SINGULARITY:   return WEAPON_BLACK_HOLE;
        default: return evolvedType;
    }
}

static void WeaponLevelUp(Weapon *weapon)
{
    if (weapon->level < WEAPON_MAX_LEVEL)
    {
        weapon->level++;
        weapon->damage *= 1.1f;
        weapon->fireRate *= 1.05f;
    }
}

static void WeaponEvolve(Weapon *weapon)
{
    if (weapon->type >= WEAPON_BASE_COUNT) return;

    WeaponType evolvedType = WeaponGetEvolvedType(weapon->type);
    int prevLevel = weapon->level;

    weapon->type = evolvedType;
    weapon->level = prevLevel;
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

// Evolution tests

static const char* test_weapon_is_evolved_base_weapons(void)
{
    mu_assert_false(WeaponIsEvolved(WEAPON_PULSE_CANNON));
    mu_assert_false(WeaponIsEvolved(WEAPON_SPREAD_SHOT));
    mu_assert_false(WeaponIsEvolved(WEAPON_HOMING_MISSILE));
    mu_assert_false(WeaponIsEvolved(WEAPON_LIGHTNING));
    mu_assert_false(WeaponIsEvolved(WEAPON_ORBIT_SHIELD));
    mu_assert_false(WeaponIsEvolved(WEAPON_FLAMETHROWER));
    mu_assert_false(WeaponIsEvolved(WEAPON_FREEZE_RAY));
    mu_assert_false(WeaponIsEvolved(WEAPON_BLACK_HOLE));
    return 0;
}

static const char* test_weapon_is_evolved_evolved_weapons(void)
{
    mu_assert_true(WeaponIsEvolved(WEAPON_MEGA_CANNON));
    mu_assert_true(WeaponIsEvolved(WEAPON_CIRCLE_BURST));
    mu_assert_true(WeaponIsEvolved(WEAPON_SWARM));
    mu_assert_true(WeaponIsEvolved(WEAPON_TESLA_COIL));
    mu_assert_true(WeaponIsEvolved(WEAPON_BLADE_DANCER));
    mu_assert_true(WeaponIsEvolved(WEAPON_INFERNO));
    mu_assert_true(WeaponIsEvolved(WEAPON_BLIZZARD));
    mu_assert_true(WeaponIsEvolved(WEAPON_SINGULARITY));
    return 0;
}

static const char* test_weapon_can_evolve_not_max_level(void)
{
    Weapon weapon;
    WeaponInit(&weapon, WEAPON_PULSE_CANNON);
    weapon.level = 3;  // Not max level

    mu_assert_false(WeaponCanEvolve(&weapon, true));
    return 0;
}

static const char* test_weapon_can_evolve_no_catalyst(void)
{
    Weapon weapon;
    WeaponInit(&weapon, WEAPON_PULSE_CANNON);
    weapon.level = WEAPON_MAX_LEVEL;

    mu_assert_false(WeaponCanEvolve(&weapon, false));
    return 0;
}

static const char* test_weapon_can_evolve_already_evolved(void)
{
    Weapon weapon;
    weapon.type = WEAPON_MEGA_CANNON;
    weapon.level = WEAPON_MAX_LEVEL;

    mu_assert_false(WeaponCanEvolve(&weapon, true));
    return 0;
}

static const char* test_weapon_can_evolve_success(void)
{
    Weapon weapon;
    WeaponInit(&weapon, WEAPON_PULSE_CANNON);
    weapon.level = WEAPON_MAX_LEVEL;

    mu_assert_true(WeaponCanEvolve(&weapon, true));
    return 0;
}

static const char* test_weapon_get_evolved_type_all(void)
{
    mu_assert_int_eq(WEAPON_MEGA_CANNON, WeaponGetEvolvedType(WEAPON_PULSE_CANNON));
    mu_assert_int_eq(WEAPON_CIRCLE_BURST, WeaponGetEvolvedType(WEAPON_SPREAD_SHOT));
    mu_assert_int_eq(WEAPON_SWARM, WeaponGetEvolvedType(WEAPON_HOMING_MISSILE));
    mu_assert_int_eq(WEAPON_TESLA_COIL, WeaponGetEvolvedType(WEAPON_LIGHTNING));
    mu_assert_int_eq(WEAPON_BLADE_DANCER, WeaponGetEvolvedType(WEAPON_ORBIT_SHIELD));
    mu_assert_int_eq(WEAPON_INFERNO, WeaponGetEvolvedType(WEAPON_FLAMETHROWER));
    mu_assert_int_eq(WEAPON_BLIZZARD, WeaponGetEvolvedType(WEAPON_FREEZE_RAY));
    mu_assert_int_eq(WEAPON_SINGULARITY, WeaponGetEvolvedType(WEAPON_BLACK_HOLE));
    return 0;
}

static const char* test_weapon_get_base_type_all(void)
{
    mu_assert_int_eq(WEAPON_PULSE_CANNON, WeaponGetBaseType(WEAPON_MEGA_CANNON));
    mu_assert_int_eq(WEAPON_SPREAD_SHOT, WeaponGetBaseType(WEAPON_CIRCLE_BURST));
    mu_assert_int_eq(WEAPON_HOMING_MISSILE, WeaponGetBaseType(WEAPON_SWARM));
    mu_assert_int_eq(WEAPON_LIGHTNING, WeaponGetBaseType(WEAPON_TESLA_COIL));
    mu_assert_int_eq(WEAPON_ORBIT_SHIELD, WeaponGetBaseType(WEAPON_BLADE_DANCER));
    mu_assert_int_eq(WEAPON_FLAMETHROWER, WeaponGetBaseType(WEAPON_INFERNO));
    mu_assert_int_eq(WEAPON_FREEZE_RAY, WeaponGetBaseType(WEAPON_BLIZZARD));
    mu_assert_int_eq(WEAPON_BLACK_HOLE, WeaponGetBaseType(WEAPON_SINGULARITY));
    return 0;
}

static const char* test_weapon_level_up(void)
{
    Weapon weapon;
    WeaponInit(&weapon, WEAPON_PULSE_CANNON);

    mu_assert_int_eq(1, weapon.level);
    WeaponLevelUp(&weapon);
    mu_assert_int_eq(2, weapon.level);
    WeaponLevelUp(&weapon);
    mu_assert_int_eq(3, weapon.level);
    return 0;
}

static const char* test_weapon_level_up_caps_at_max(void)
{
    Weapon weapon;
    WeaponInit(&weapon, WEAPON_PULSE_CANNON);
    weapon.level = WEAPON_MAX_LEVEL;

    WeaponLevelUp(&weapon);

    mu_assert_int_eq(WEAPON_MAX_LEVEL, weapon.level);
    return 0;
}

static const char* test_weapon_evolve_changes_type(void)
{
    Weapon weapon;
    WeaponInit(&weapon, WEAPON_PULSE_CANNON);
    weapon.level = WEAPON_MAX_LEVEL;

    WeaponEvolve(&weapon);

    mu_assert_int_eq(WEAPON_MEGA_CANNON, weapon.type);
    return 0;
}

static const char* test_weapon_evolve_preserves_level(void)
{
    Weapon weapon;
    WeaponInit(&weapon, WEAPON_PULSE_CANNON);
    weapon.level = WEAPON_MAX_LEVEL;

    WeaponEvolve(&weapon);

    mu_assert_int_eq(WEAPON_MAX_LEVEL, weapon.level);
    return 0;
}

static const char* test_weapon_evolve_does_nothing_if_already_evolved(void)
{
    Weapon weapon;
    weapon.type = WEAPON_MEGA_CANNON;
    weapon.level = WEAPON_MAX_LEVEL;

    WeaponEvolve(&weapon);

    mu_assert_int_eq(WEAPON_MEGA_CANNON, weapon.type);
    return 0;
}

static const char* test_weapon_base_count_is_8(void)
{
    mu_assert_int_eq(8, WEAPON_BASE_COUNT);
    return 0;
}

static const char* test_weapon_total_count_is_16(void)
{
    mu_assert_int_eq(16, WEAPON_COUNT);
    return 0;
}

static const char* test_weapon_max_level_is_5(void)
{
    mu_assert_int_eq(5, WEAPON_MAX_LEVEL);
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
    // Evolution tests
    mu_run_test(test_weapon_is_evolved_base_weapons);
    mu_run_test(test_weapon_is_evolved_evolved_weapons);
    mu_run_test(test_weapon_can_evolve_not_max_level);
    mu_run_test(test_weapon_can_evolve_no_catalyst);
    mu_run_test(test_weapon_can_evolve_already_evolved);
    mu_run_test(test_weapon_can_evolve_success);
    mu_run_test(test_weapon_get_evolved_type_all);
    mu_run_test(test_weapon_get_base_type_all);
    mu_run_test(test_weapon_level_up);
    mu_run_test(test_weapon_level_up_caps_at_max);
    mu_run_test(test_weapon_evolve_changes_type);
    mu_run_test(test_weapon_evolve_preserves_level);
    mu_run_test(test_weapon_evolve_does_nothing_if_already_evolved);
    mu_run_test(test_weapon_base_count_is_8);
    mu_run_test(test_weapon_total_count_is_16);
    mu_run_test(test_weapon_max_level_is_5);
    return 0;
}
