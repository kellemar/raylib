#include "minunit.h"
#include "../src/utils.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Minimal definitions for testing upgrades

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

#define PLAYER_TRAIL_LENGTH 5

typedef struct Player {
    Vector2 pos;
    Vector2 vel;
    Vector2 aimDir;
    float radius;
    float speed;
    float health;
    float maxHealth;
    float invincibilityTimer;
    float magnetRadius;
    int level;
    int xp;
    int xpToNextLevel;
    bool alive;
    Weapon weapon;
    Vector2 trailPositions[PLAYER_TRAIL_LENGTH];
    float trailUpdateTimer;
} Player;

typedef enum UpgradeType {
    UPGRADE_DAMAGE,
    UPGRADE_FIRE_RATE,
    UPGRADE_PROJECTILE_COUNT,
    UPGRADE_SPEED,
    UPGRADE_MAX_HEALTH,
    UPGRADE_MAGNET,
    UPGRADE_COUNT
} UpgradeType;

typedef struct Upgrade {
    UpgradeType type;
    const char *name;
    const char *description;
} Upgrade;

static const Upgrade UPGRADE_DEFINITIONS[UPGRADE_COUNT] = {
    { UPGRADE_DAMAGE, "Power Up", "+25% Damage" },
    { UPGRADE_FIRE_RATE, "Rapid Fire", "+20% Fire Rate" },
    { UPGRADE_PROJECTILE_COUNT, "Multi Shot", "+1 Projectile" },
    { UPGRADE_SPEED, "Swift Feet", "+10% Move Speed" },
    { UPGRADE_MAX_HEALTH, "Vitality", "+20 Max HP" },
    { UPGRADE_MAGNET, "Magnetism", "+50% Pickup Range" }
};

static Upgrade GetUpgradeDefinition(UpgradeType type)
{
    if (type >= 0 && type < UPGRADE_COUNT)
    {
        return UPGRADE_DEFINITIONS[type];
    }
    return UPGRADE_DEFINITIONS[0];
}

static void ApplyUpgrade(UpgradeType type, Player *player)
{
    switch (type)
    {
        case UPGRADE_DAMAGE:
            player->weapon.damage *= 1.25f;
            break;

        case UPGRADE_FIRE_RATE:
            player->weapon.fireRate *= 1.2f;
            break;

        case UPGRADE_PROJECTILE_COUNT:
            player->weapon.projectileCount += 1;
            break;

        case UPGRADE_SPEED:
            player->speed *= 1.1f;
            break;

        case UPGRADE_MAX_HEALTH:
            player->maxHealth += 20.0f;
            player->health += 20.0f;
            break;

        case UPGRADE_MAGNET:
            player->magnetRadius *= 1.5f;
            break;

        default:
            break;
    }
}

static void GenerateRandomUpgrades(UpgradeType *options, int count)
{
    int used[UPGRADE_COUNT] = { 0 };
    int generated = 0;

    while (generated < count && generated < UPGRADE_COUNT)
    {
        int candidate = rand() % UPGRADE_COUNT;
        if (!used[candidate])
        {
            used[candidate] = 1;
            options[generated] = (UpgradeType)candidate;
            generated++;
        }
    }
}

static void InitTestPlayer(Player *player)
{
    player->pos = (Vector2){ 640.0f, 360.0f };
    player->vel = (Vector2){ 0.0f, 0.0f };
    player->aimDir = (Vector2){ 1.0f, 0.0f };
    player->radius = 15.0f;
    player->speed = 300.0f;
    player->health = 100.0f;
    player->maxHealth = 100.0f;
    player->invincibilityTimer = 0.0f;
    player->magnetRadius = 80.0f;
    player->level = 1;
    player->xp = 0;
    player->xpToNextLevel = 10;
    player->alive = true;

    player->weapon.type = WEAPON_PULSE_CANNON;
    player->weapon.damage = 10.0f;
    player->weapon.fireRate = 5.0f;
    player->weapon.projectileSpeed = 500.0f;
    player->weapon.projectileRadius = 5.0f;
    player->weapon.projectileLifetime = 2.0f;
    player->weapon.projectileCount = 1;
    player->weapon.cooldown = 0.0f;
    player->weapon.level = 1;
}

// Tests

static const char* test_upgrade_definition_damage(void)
{
    Upgrade u = GetUpgradeDefinition(UPGRADE_DAMAGE);
    mu_assert_int_eq(UPGRADE_DAMAGE, u.type);
    mu_assert(strcmp(u.name, "Power Up") == 0, "Expected 'Power Up'");
    mu_assert(strcmp(u.description, "+25% Damage") == 0, "Expected '+25% Damage'");
    return 0;
}

static const char* test_upgrade_definition_fire_rate(void)
{
    Upgrade u = GetUpgradeDefinition(UPGRADE_FIRE_RATE);
    mu_assert_int_eq(UPGRADE_FIRE_RATE, u.type);
    mu_assert(strcmp(u.name, "Rapid Fire") == 0, "Expected 'Rapid Fire'");
    return 0;
}

static const char* test_upgrade_definition_projectile_count(void)
{
    Upgrade u = GetUpgradeDefinition(UPGRADE_PROJECTILE_COUNT);
    mu_assert_int_eq(UPGRADE_PROJECTILE_COUNT, u.type);
    mu_assert(strcmp(u.name, "Multi Shot") == 0, "Expected 'Multi Shot'");
    return 0;
}

static const char* test_upgrade_definition_speed(void)
{
    Upgrade u = GetUpgradeDefinition(UPGRADE_SPEED);
    mu_assert_int_eq(UPGRADE_SPEED, u.type);
    mu_assert(strcmp(u.name, "Swift Feet") == 0, "Expected 'Swift Feet'");
    return 0;
}

static const char* test_upgrade_definition_max_health(void)
{
    Upgrade u = GetUpgradeDefinition(UPGRADE_MAX_HEALTH);
    mu_assert_int_eq(UPGRADE_MAX_HEALTH, u.type);
    mu_assert(strcmp(u.name, "Vitality") == 0, "Expected 'Vitality'");
    return 0;
}

static const char* test_upgrade_definition_magnet(void)
{
    Upgrade u = GetUpgradeDefinition(UPGRADE_MAGNET);
    mu_assert_int_eq(UPGRADE_MAGNET, u.type);
    mu_assert(strcmp(u.name, "Magnetism") == 0, "Expected 'Magnetism'");
    return 0;
}

static const char* test_upgrade_definition_invalid_type(void)
{
    Upgrade u = GetUpgradeDefinition((UpgradeType)99);
    // Should return first upgrade as fallback
    mu_assert_int_eq(UPGRADE_DAMAGE, u.type);
    return 0;
}

static const char* test_apply_damage_upgrade(void)
{
    Player player;
    InitTestPlayer(&player);

    float originalDamage = player.weapon.damage;
    ApplyUpgrade(UPGRADE_DAMAGE, &player);

    mu_assert_float_eq(originalDamage * 1.25f, player.weapon.damage);
    return 0;
}

static const char* test_apply_fire_rate_upgrade(void)
{
    Player player;
    InitTestPlayer(&player);

    float originalRate = player.weapon.fireRate;
    ApplyUpgrade(UPGRADE_FIRE_RATE, &player);

    mu_assert_float_eq(originalRate * 1.2f, player.weapon.fireRate);
    return 0;
}

static const char* test_apply_projectile_count_upgrade(void)
{
    Player player;
    InitTestPlayer(&player);

    int originalCount = player.weapon.projectileCount;
    ApplyUpgrade(UPGRADE_PROJECTILE_COUNT, &player);

    mu_assert_int_eq(originalCount + 1, player.weapon.projectileCount);
    return 0;
}

static const char* test_apply_speed_upgrade(void)
{
    Player player;
    InitTestPlayer(&player);

    float originalSpeed = player.speed;
    ApplyUpgrade(UPGRADE_SPEED, &player);

    mu_assert_float_eq(originalSpeed * 1.1f, player.speed);
    return 0;
}

static const char* test_apply_max_health_upgrade(void)
{
    Player player;
    InitTestPlayer(&player);

    float originalMaxHealth = player.maxHealth;
    float originalHealth = player.health;
    ApplyUpgrade(UPGRADE_MAX_HEALTH, &player);

    mu_assert_float_eq(originalMaxHealth + 20.0f, player.maxHealth);
    mu_assert_float_eq(originalHealth + 20.0f, player.health);
    return 0;
}

static const char* test_apply_magnet_upgrade(void)
{
    Player player;
    InitTestPlayer(&player);

    float originalRadius = player.magnetRadius;
    ApplyUpgrade(UPGRADE_MAGNET, &player);

    mu_assert_float_eq(originalRadius * 1.5f, player.magnetRadius);
    return 0;
}

static const char* test_apply_multiple_upgrades(void)
{
    Player player;
    InitTestPlayer(&player);

    // Apply damage upgrade twice
    ApplyUpgrade(UPGRADE_DAMAGE, &player);
    ApplyUpgrade(UPGRADE_DAMAGE, &player);

    mu_assert_float_eq(10.0f * 1.25f * 1.25f, player.weapon.damage);
    return 0;
}

static const char* test_generate_random_upgrades_count(void)
{
    srand(12345);
    UpgradeType options[3];
    GenerateRandomUpgrades(options, 3);

    // Just verify we got 3 valid upgrade types
    for (int i = 0; i < 3; i++)
    {
        mu_assert(options[i] >= 0 && options[i] < UPGRADE_COUNT, "Invalid upgrade type");
    }
    return 0;
}

static const char* test_generate_random_upgrades_unique(void)
{
    srand(12345);
    UpgradeType options[3];
    GenerateRandomUpgrades(options, 3);

    // Verify all 3 are unique
    mu_assert(options[0] != options[1], "Options 0 and 1 should be different");
    mu_assert(options[1] != options[2], "Options 1 and 2 should be different");
    mu_assert(options[0] != options[2], "Options 0 and 2 should be different");
    return 0;
}

static const char* test_generate_random_upgrades_max_count(void)
{
    srand(12345);
    UpgradeType options[UPGRADE_COUNT];
    GenerateRandomUpgrades(options, UPGRADE_COUNT);

    // All upgrade types should be present
    int seen[UPGRADE_COUNT] = { 0 };
    for (int i = 0; i < UPGRADE_COUNT; i++)
    {
        seen[options[i]] = 1;
    }
    for (int i = 0; i < UPGRADE_COUNT; i++)
    {
        mu_assert(seen[i] == 1, "All upgrade types should be present");
    }
    return 0;
}

const char* run_upgrade_tests(void)
{
    mu_run_test(test_upgrade_definition_damage);
    mu_run_test(test_upgrade_definition_fire_rate);
    mu_run_test(test_upgrade_definition_projectile_count);
    mu_run_test(test_upgrade_definition_speed);
    mu_run_test(test_upgrade_definition_max_health);
    mu_run_test(test_upgrade_definition_magnet);
    mu_run_test(test_upgrade_definition_invalid_type);
    mu_run_test(test_apply_damage_upgrade);
    mu_run_test(test_apply_fire_rate_upgrade);
    mu_run_test(test_apply_projectile_count_upgrade);
    mu_run_test(test_apply_speed_upgrade);
    mu_run_test(test_apply_max_health_upgrade);
    mu_run_test(test_apply_magnet_upgrade);
    mu_run_test(test_apply_multiple_upgrades);
    mu_run_test(test_generate_random_upgrades_count);
    mu_run_test(test_generate_random_upgrades_unique);
    mu_run_test(test_generate_random_upgrades_max_count);
    return 0;
}
