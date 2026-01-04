#include "upgrade.h"
#include <stdlib.h>

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

Upgrade GetUpgradeDefinition(UpgradeType type)
{
    if (type >= 0 && type < UPGRADE_COUNT)
    {
        return UPGRADE_DEFINITIONS[type];
    }
    return UPGRADE_DEFINITIONS[0];
}

void ApplyUpgrade(UpgradeType type, Player *player)
{
    // Mark upgrade as acquired for evolution tracking
    PlayerMarkUpgradeAcquired(player, (int)type);

    switch (type)
    {
        // Weapon upgrades - also level up the weapon
        case UPGRADE_DAMAGE:
            player->weapon.damage *= 1.25f;
            WeaponLevelUp(&player->weapon);
            break;

        case UPGRADE_FIRE_RATE:
            player->weapon.fireRate *= 1.2f;
            WeaponLevelUp(&player->weapon);
            break;

        case UPGRADE_PROJECTILE_COUNT:
            player->weapon.projectileCount += 1;
            WeaponLevelUp(&player->weapon);
            break;

        case UPGRADE_PIERCE:
            player->weapon.pierce = true;
            WeaponLevelUp(&player->weapon);
            break;

        case UPGRADE_RANGE:
            player->weapon.projectileLifetime *= 1.3f;
            WeaponLevelUp(&player->weapon);
            break;

        case UPGRADE_PROJ_SIZE:
            player->weapon.projectileRadius *= 1.25f;
            WeaponLevelUp(&player->weapon);
            break;

        case UPGRADE_COOLDOWN:
            player->weapon.fireRate *= 1.15f;  // Faster fire rate = shorter cooldown
            WeaponLevelUp(&player->weapon);
            break;

        case UPGRADE_CRIT_CHANCE:
            player->weapon.critChance += 0.1f;
            if (player->weapon.critChance > 1.0f) player->weapon.critChance = 1.0f;
            WeaponLevelUp(&player->weapon);
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

void GenerateRandomUpgrades(UpgradeType *options, int count)
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

UpgradeType GetEvolutionCatalyst(WeaponType baseWeapon)
{
    switch (baseWeapon)
    {
        case WEAPON_PULSE_CANNON:  return UPGRADE_PIERCE;           // → Mega Cannon
        case WEAPON_SPREAD_SHOT:   return UPGRADE_PROJECTILE_COUNT; // → Circle Burst
        case WEAPON_HOMING_MISSILE: return UPGRADE_DOUBLE_SHOT;     // → Swarm
        case WEAPON_LIGHTNING:     return UPGRADE_CRIT_CHANCE;      // → Tesla Coil
        case WEAPON_ORBIT_SHIELD:  return UPGRADE_DAMAGE;           // → Blade Dancer
        case WEAPON_FLAMETHROWER:  return UPGRADE_RANGE;            // → Inferno
        case WEAPON_FREEZE_RAY:    return UPGRADE_SLOW_AURA;        // → Blizzard
        case WEAPON_BLACK_HOLE:    return UPGRADE_EXPLOSIVE;        // → Singularity
        default: return UPGRADE_DAMAGE;
    }
}

bool IsEvolutionCatalyst(UpgradeType upgrade, WeaponType weapon)
{
    return GetEvolutionCatalyst(weapon) == upgrade;
}
