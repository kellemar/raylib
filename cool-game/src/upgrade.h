#ifndef UPGRADE_H
#define UPGRADE_H

#include "player.h"

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

Upgrade GetUpgradeDefinition(UpgradeType type);
void ApplyUpgrade(UpgradeType type, Player *player);
void GenerateRandomUpgrades(UpgradeType *options, int count);

#endif
