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
    bool pierce;
    float spreadAngle;
    int chainCount;
    float orbitSpawnAngle;
    // Upgrade-based stats
    float critChance;
    float critMultiplier;
    bool doubleShot;
    int ricochetCount;
    bool explosive;
    float explosionRadius;
    float homingStrength;
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
    float dashCooldown;
    float dashTimer;
    bool isDashing;
    Vector2 dashDir;
    // Upgrade stats
    float armor;
    float regen;
    float regenTimer;
    float xpMultiplier;
    float knockbackMultiplier;
    float dashDamage;
    float vampirism;
    float slowAuraRadius;
    float slowAuraAmount;
} Player;

typedef enum UpgradeType {
    // Weapon upgrades
    UPGRADE_DAMAGE,
    UPGRADE_FIRE_RATE,
    UPGRADE_PROJECTILE_COUNT,
    UPGRADE_PIERCE,
    UPGRADE_RANGE,
    UPGRADE_PROJ_SIZE,
    UPGRADE_COOLDOWN,
    UPGRADE_CRIT_CHANCE,
    // Player upgrades
    UPGRADE_SPEED,
    UPGRADE_MAX_HEALTH,
    UPGRADE_MAGNET,
    UPGRADE_ARMOR,
    UPGRADE_REGEN,
    UPGRADE_DASH_DAMAGE,
    UPGRADE_XP_BOOST,
    UPGRADE_KNOCKBACK,
    // Special upgrades
    UPGRADE_DOUBLE_SHOT,
    UPGRADE_VAMPIRISM,
    UPGRADE_EXPLOSIVE,
    UPGRADE_RICOCHET,
    UPGRADE_HOMING_BOOST,
    UPGRADE_SLOW_AURA,
    UPGRADE_COUNT
} UpgradeType;

typedef enum UpgradeRarity {
    RARITY_COMMON,
    RARITY_UNCOMMON,
    RARITY_RARE
} UpgradeRarity;

typedef struct Upgrade {
    UpgradeType type;
    const char *name;
    const char *description;
    UpgradeRarity rarity;
} Upgrade;

static const Upgrade UPGRADE_DEFINITIONS[UPGRADE_COUNT] = {
    // Weapon upgrades
    { UPGRADE_DAMAGE, "Power Up", "+25% Damage", RARITY_COMMON },
    { UPGRADE_FIRE_RATE, "Rapid Fire", "+20% Fire Rate", RARITY_COMMON },
    { UPGRADE_PROJECTILE_COUNT, "Multi Shot", "+1 Projectile", RARITY_UNCOMMON },
    { UPGRADE_PIERCE, "Piercing", "Shots pierce enemies", RARITY_UNCOMMON },
    { UPGRADE_RANGE, "Long Range", "+30% Projectile Range", RARITY_COMMON },
    { UPGRADE_PROJ_SIZE, "Big Bullets", "+25% Projectile Size", RARITY_COMMON },
    { UPGRADE_COOLDOWN, "Quick Draw", "-15% Weapon Cooldown", RARITY_COMMON },
    { UPGRADE_CRIT_CHANCE, "Critical Eye", "+10% Crit Chance", RARITY_UNCOMMON },
    // Player upgrades
    { UPGRADE_SPEED, "Swift Feet", "+10% Move Speed", RARITY_COMMON },
    { UPGRADE_MAX_HEALTH, "Vitality", "+20 Max HP", RARITY_COMMON },
    { UPGRADE_MAGNET, "Magnetism", "+50% Pickup Range", RARITY_COMMON },
    { UPGRADE_ARMOR, "Tough Skin", "+5 Armor", RARITY_COMMON },
    { UPGRADE_REGEN, "Regeneration", "+1 HP per second", RARITY_UNCOMMON },
    { UPGRADE_DASH_DAMAGE, "Dash Strike", "Deal damage while dashing", RARITY_UNCOMMON },
    { UPGRADE_XP_BOOST, "Wisdom", "+25% XP Gain", RARITY_COMMON },
    { UPGRADE_KNOCKBACK, "Force Push", "+50% Knockback", RARITY_COMMON },
    // Special upgrades
    { UPGRADE_DOUBLE_SHOT, "Double Tap", "Fire twice per shot", RARITY_RARE },
    { UPGRADE_VAMPIRISM, "Vampirism", "1% Lifesteal on hit", RARITY_RARE },
    { UPGRADE_EXPLOSIVE, "Explosive Shots", "Shots explode on hit", RARITY_RARE },
    { UPGRADE_RICOCHET, "Ricochet", "Shots bounce once", RARITY_RARE },
    { UPGRADE_HOMING_BOOST, "Heat Seeker", "+100% Homing Strength", RARITY_UNCOMMON },
    { UPGRADE_SLOW_AURA, "Time Warp", "Slow nearby enemies", RARITY_RARE }
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
        // Weapon upgrades
        case UPGRADE_DAMAGE:
            player->weapon.damage *= 1.25f;
            break;

        case UPGRADE_FIRE_RATE:
            player->weapon.fireRate *= 1.2f;
            break;

        case UPGRADE_PROJECTILE_COUNT:
            player->weapon.projectileCount += 1;
            break;

        case UPGRADE_PIERCE:
            player->weapon.pierce = true;
            break;

        case UPGRADE_RANGE:
            player->weapon.projectileLifetime *= 1.3f;
            break;

        case UPGRADE_PROJ_SIZE:
            player->weapon.projectileRadius *= 1.25f;
            break;

        case UPGRADE_COOLDOWN:
            player->weapon.fireRate *= 1.15f;
            break;

        case UPGRADE_CRIT_CHANCE:
            player->weapon.critChance += 0.1f;
            if (player->weapon.critChance > 1.0f) player->weapon.critChance = 1.0f;
            break;

        // Player upgrades
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

        case UPGRADE_ARMOR:
            player->armor += 5.0f;
            break;

        case UPGRADE_REGEN:
            player->regen += 1.0f;
            break;

        case UPGRADE_DASH_DAMAGE:
            player->dashDamage += 25.0f;
            break;

        case UPGRADE_XP_BOOST:
            player->xpMultiplier *= 1.25f;
            break;

        case UPGRADE_KNOCKBACK:
            player->knockbackMultiplier *= 1.5f;
            break;

        // Special upgrades
        case UPGRADE_DOUBLE_SHOT:
            player->weapon.doubleShot = true;
            break;

        case UPGRADE_VAMPIRISM:
            player->vampirism += 0.01f;
            break;

        case UPGRADE_EXPLOSIVE:
            player->weapon.explosive = true;
            break;

        case UPGRADE_RICOCHET:
            player->weapon.ricochetCount += 1;
            break;

        case UPGRADE_HOMING_BOOST:
            player->weapon.homingStrength *= 2.0f;
            break;

        case UPGRADE_SLOW_AURA:
            player->slowAuraRadius = 100.0f;
            player->slowAuraAmount = 0.3f;
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
    player->weapon.pierce = false;
    player->weapon.critChance = 0.0f;
    player->weapon.critMultiplier = 2.0f;
    player->weapon.doubleShot = false;
    player->weapon.ricochetCount = 0;
    player->weapon.explosive = false;
    player->weapon.explosionRadius = 30.0f;
    player->weapon.homingStrength = 1.0f;

    // Upgrade stats
    player->armor = 0.0f;
    player->regen = 0.0f;
    player->regenTimer = 0.0f;
    player->xpMultiplier = 1.0f;
    player->knockbackMultiplier = 1.0f;
    player->dashDamage = 0.0f;
    player->vampirism = 0.0f;
    player->slowAuraRadius = 0.0f;
    player->slowAuraAmount = 0.0f;
}

// Tests

static const char* test_upgrade_definition_damage(void)
{
    Upgrade u = GetUpgradeDefinition(UPGRADE_DAMAGE);
    mu_assert_int_eq(UPGRADE_DAMAGE, u.type);
    mu_assert(strcmp(u.name, "Power Up") == 0, "Expected 'Power Up'");
    mu_assert(strcmp(u.description, "+25% Damage") == 0, "Expected '+25% Damage'");
    mu_assert_int_eq(RARITY_COMMON, u.rarity);
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
    mu_assert_int_eq(RARITY_UNCOMMON, u.rarity);
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

static const char* test_upgrade_definition_rare(void)
{
    Upgrade u = GetUpgradeDefinition(UPGRADE_VAMPIRISM);
    mu_assert_int_eq(UPGRADE_VAMPIRISM, u.type);
    mu_assert(strcmp(u.name, "Vampirism") == 0, "Expected 'Vampirism'");
    mu_assert_int_eq(RARITY_RARE, u.rarity);
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

static const char* test_apply_pierce_upgrade(void)
{
    Player player;
    InitTestPlayer(&player);

    mu_assert(player.weapon.pierce == false, "Pierce should start false");
    ApplyUpgrade(UPGRADE_PIERCE, &player);
    mu_assert(player.weapon.pierce == true, "Pierce should be true after upgrade");
    return 0;
}

static const char* test_apply_armor_upgrade(void)
{
    Player player;
    InitTestPlayer(&player);

    float originalArmor = player.armor;
    ApplyUpgrade(UPGRADE_ARMOR, &player);

    mu_assert_float_eq(originalArmor + 5.0f, player.armor);
    return 0;
}

static const char* test_apply_regen_upgrade(void)
{
    Player player;
    InitTestPlayer(&player);

    float originalRegen = player.regen;
    ApplyUpgrade(UPGRADE_REGEN, &player);

    mu_assert_float_eq(originalRegen + 1.0f, player.regen);
    return 0;
}

static const char* test_apply_crit_chance_upgrade(void)
{
    Player player;
    InitTestPlayer(&player);

    float originalCrit = player.weapon.critChance;
    ApplyUpgrade(UPGRADE_CRIT_CHANCE, &player);

    mu_assert_float_eq(originalCrit + 0.1f, player.weapon.critChance);
    return 0;
}

static const char* test_apply_crit_chance_caps_at_100(void)
{
    Player player;
    InitTestPlayer(&player);
    player.weapon.critChance = 0.95f;

    ApplyUpgrade(UPGRADE_CRIT_CHANCE, &player);

    mu_assert_float_eq(1.0f, player.weapon.critChance);
    return 0;
}

static const char* test_apply_vampirism_upgrade(void)
{
    Player player;
    InitTestPlayer(&player);

    float originalVamp = player.vampirism;
    ApplyUpgrade(UPGRADE_VAMPIRISM, &player);

    mu_assert_float_eq(originalVamp + 0.01f, player.vampirism);
    return 0;
}

static const char* test_apply_xp_boost_upgrade(void)
{
    Player player;
    InitTestPlayer(&player);

    float originalMult = player.xpMultiplier;
    ApplyUpgrade(UPGRADE_XP_BOOST, &player);

    mu_assert_float_eq(originalMult * 1.25f, player.xpMultiplier);
    return 0;
}

static const char* test_apply_knockback_upgrade(void)
{
    Player player;
    InitTestPlayer(&player);

    float originalKB = player.knockbackMultiplier;
    ApplyUpgrade(UPGRADE_KNOCKBACK, &player);

    mu_assert_float_eq(originalKB * 1.5f, player.knockbackMultiplier);
    return 0;
}

static const char* test_apply_slow_aura_upgrade(void)
{
    Player player;
    InitTestPlayer(&player);

    mu_assert_float_eq(0.0f, player.slowAuraRadius);
    ApplyUpgrade(UPGRADE_SLOW_AURA, &player);

    mu_assert_float_eq(100.0f, player.slowAuraRadius);
    mu_assert_float_eq(0.3f, player.slowAuraAmount);
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

static const char* test_upgrade_count_is_22(void)
{
    mu_assert_int_eq(22, UPGRADE_COUNT);
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
    mu_run_test(test_upgrade_definition_rare);
    mu_run_test(test_apply_damage_upgrade);
    mu_run_test(test_apply_fire_rate_upgrade);
    mu_run_test(test_apply_projectile_count_upgrade);
    mu_run_test(test_apply_speed_upgrade);
    mu_run_test(test_apply_max_health_upgrade);
    mu_run_test(test_apply_magnet_upgrade);
    mu_run_test(test_apply_pierce_upgrade);
    mu_run_test(test_apply_armor_upgrade);
    mu_run_test(test_apply_regen_upgrade);
    mu_run_test(test_apply_crit_chance_upgrade);
    mu_run_test(test_apply_crit_chance_caps_at_100);
    mu_run_test(test_apply_vampirism_upgrade);
    mu_run_test(test_apply_xp_boost_upgrade);
    mu_run_test(test_apply_knockback_upgrade);
    mu_run_test(test_apply_slow_aura_upgrade);
    mu_run_test(test_apply_multiple_upgrades);
    mu_run_test(test_generate_random_upgrades_count);
    mu_run_test(test_generate_random_upgrades_unique);
    mu_run_test(test_generate_random_upgrades_max_count);
    mu_run_test(test_upgrade_count_is_22);
    return 0;
}
