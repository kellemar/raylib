#ifndef UPGRADE_H
#define UPGRADE_H

#include "player.h"

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

Upgrade GetUpgradeDefinition(UpgradeType type);
void ApplyUpgrade(UpgradeType type, Player *player);
void GenerateRandomUpgrades(UpgradeType *options, int count);

#endif
