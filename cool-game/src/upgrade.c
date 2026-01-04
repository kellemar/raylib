#include "upgrade.h"
#include <stdlib.h>

static const Upgrade UPGRADE_DEFINITIONS[UPGRADE_COUNT] = {
    { UPGRADE_DAMAGE, "Power Up", "+25% Damage" },
    { UPGRADE_FIRE_RATE, "Rapid Fire", "+20% Fire Rate" },
    { UPGRADE_PROJECTILE_COUNT, "Multi Shot", "+1 Projectile" },
    { UPGRADE_SPEED, "Swift Feet", "+10% Move Speed" },
    { UPGRADE_MAX_HEALTH, "Vitality", "+20 Max HP" },
    { UPGRADE_MAGNET, "Magnetism", "+50% Pickup Range" }
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
