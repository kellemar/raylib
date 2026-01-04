#include "weapon.h"
#include "audio.h"
#include "types.h"
#include "raymath.h"
#include <math.h>
#include <stdlib.h>

// Weapon color palette for each type
static const Color WEAPON_COLORS[WEAPON_COUNT] = {
    { 255, 255, 50, 255 },   // PULSE_CANNON - Yellow
    { 255, 150, 50, 255 },   // SPREAD_SHOT - Orange
    { 255, 50, 150, 255 },   // HOMING_MISSILE - Pink
    { 100, 200, 255, 255 },  // LIGHTNING - Light Blue
    { 50, 255, 255, 255 },   // ORBIT_SHIELD - Cyan
    { 255, 100, 50, 255 },   // FLAMETHROWER - Red-Orange
    { 150, 200, 255, 255 },  // FREEZE_RAY - Ice Blue
    { 150, 50, 200, 255 },   // BLACK_HOLE - Purple
};

static const char* WEAPON_NAMES[WEAPON_COUNT] = {
    "Pulse Cannon",
    "Spread Shot",
    "Homing Missiles",
    "Lightning",
    "Orbit Shield",
    "Flamethrower",
    "Freeze Ray",
    "Black Hole",
};

void WeaponInit(Weapon *weapon, WeaponType type)
{
    weapon->type = type;
    weapon->cooldown = 0.0f;
    weapon->level = 1;
    weapon->pierce = false;
    weapon->chainCount = 0;
    weapon->orbitSpawnAngle = 0.0f;

    switch (type)
    {
        case WEAPON_PULSE_CANNON:
            weapon->damage = 10.0f;
            weapon->fireRate = 5.0f;
            weapon->projectileSpeed = 500.0f;
            weapon->projectileRadius = 5.0f;
            weapon->projectileLifetime = 2.0f;
            weapon->projectileCount = 1;
            weapon->spreadAngle = 0.15f;
            break;

        case WEAPON_SPREAD_SHOT:
            weapon->damage = 7.0f;
            weapon->fireRate = 3.0f;
            weapon->projectileSpeed = 450.0f;
            weapon->projectileRadius = 4.0f;
            weapon->projectileLifetime = 1.5f;
            weapon->projectileCount = 3;
            weapon->spreadAngle = 0.25f;  // Wider spread
            break;

        case WEAPON_HOMING_MISSILE:
            weapon->damage = 20.0f;
            weapon->fireRate = 2.0f;
            weapon->projectileSpeed = 250.0f;  // Slower
            weapon->projectileRadius = 6.0f;
            weapon->projectileLifetime = 4.0f;  // Longer life to seek
            weapon->projectileCount = 1;
            weapon->spreadAngle = 0.3f;
            break;

        case WEAPON_LIGHTNING:
            weapon->damage = 8.0f;
            weapon->fireRate = 4.0f;
            weapon->projectileSpeed = 600.0f;
            weapon->projectileRadius = 4.0f;
            weapon->projectileLifetime = 0.8f;
            weapon->projectileCount = 1;
            weapon->spreadAngle = 0.1f;
            weapon->chainCount = 3;  // Chains to 3 enemies
            break;

        case WEAPON_ORBIT_SHIELD:
            weapon->damage = 15.0f;
            weapon->fireRate = 0.5f;  // Slow spawn rate
            weapon->projectileSpeed = 0.0f;  // Doesn't move linearly
            weapon->projectileRadius = 8.0f;
            weapon->projectileLifetime = 8.0f;  // Long duration
            weapon->projectileCount = 1;
            weapon->spreadAngle = 0.0f;
            weapon->pierce = true;  // Orbiting projectiles pierce
            break;

        case WEAPON_FLAMETHROWER:
            weapon->damage = 3.0f;  // Low damage but rapid
            weapon->fireRate = 20.0f;  // Very fast
            weapon->projectileSpeed = 350.0f;
            weapon->projectileRadius = 6.0f;
            weapon->projectileLifetime = 0.4f;  // Short range
            weapon->projectileCount = 1;
            weapon->spreadAngle = 0.4f;  // Wide cone
            break;

        case WEAPON_FREEZE_RAY:
            weapon->damage = 5.0f;
            weapon->fireRate = 6.0f;
            weapon->projectileSpeed = 400.0f;
            weapon->projectileRadius = 5.0f;
            weapon->projectileLifetime = 1.5f;
            weapon->projectileCount = 1;
            weapon->spreadAngle = 0.1f;
            break;

        case WEAPON_BLACK_HOLE:
            weapon->damage = 5.0f;  // Low direct damage
            weapon->fireRate = 0.3f;  // Very slow
            weapon->projectileSpeed = 100.0f;  // Slow moving
            weapon->projectileRadius = 15.0f;  // Large
            weapon->projectileLifetime = 5.0f;
            weapon->projectileCount = 1;
            weapon->spreadAngle = 0.0f;
            break;

        default:
            // Fallback to pulse cannon stats
            weapon->damage = 10.0f;
            weapon->fireRate = 5.0f;
            weapon->projectileSpeed = 500.0f;
            weapon->projectileRadius = 5.0f;
            weapon->projectileLifetime = 2.0f;
            weapon->projectileCount = 1;
            weapon->spreadAngle = 0.15f;
            break;
    }
}

void WeaponUpdate(Weapon *weapon, float dt)
{
    if (weapon->cooldown > 0.0f)
    {
        weapon->cooldown -= dt;
        if (weapon->cooldown < 0.0f) weapon->cooldown = 0.0f;
    }
}

bool WeaponCanFire(Weapon *weapon)
{
    return weapon->cooldown <= 0.0f;
}

const char* WeaponGetName(WeaponType type)
{
    if (type >= 0 && type < WEAPON_COUNT)
    {
        return WEAPON_NAMES[type];
    }
    return "Unknown";
}

Color WeaponGetColor(WeaponType type)
{
    if (type >= 0 && type < WEAPON_COUNT)
    {
        return WEAPON_COLORS[type];
    }
    return NEON_YELLOW;
}

void WeaponFire(Weapon *weapon, ProjectilePool *pool, Vector2 pos, Vector2 dir, Vector2 *ownerPosPtr)
{
    if (!WeaponCanFire(weapon)) return;

    Color projColor = WeaponGetColor(weapon->type);

    switch (weapon->type)
    {
        case WEAPON_PULSE_CANNON:
        case WEAPON_SPREAD_SHOT:
        {
            // Multi-shot spread pattern
            float startAngle = -weapon->spreadAngle * (weapon->projectileCount - 1) / 2.0f;

            for (int i = 0; i < weapon->projectileCount; i++)
            {
                float angle = startAngle + i * weapon->spreadAngle;
                Vector2 rotDir = Vector2Rotate(dir, angle);
                Vector2 vel = Vector2Scale(rotDir, weapon->projectileSpeed);

                ProjectileSpawnParams params = {
                    .pos = pos,
                    .vel = vel,
                    .damage = weapon->damage,
                    .radius = weapon->projectileRadius,
                    .lifetime = weapon->projectileLifetime,
                    .weaponType = weapon->type,
                    .pierce = weapon->pierce,
                    .behavior = PROJ_BEHAVIOR_LINEAR,
                    .effects = PROJ_EFFECT_NONE,
                    .color = projColor,
                };
                ProjectileSpawnEx(pool, &params);
            }
            break;
        }

        case WEAPON_HOMING_MISSILE:
        {
            float startAngle = -weapon->spreadAngle * (weapon->projectileCount - 1) / 2.0f;

            for (int i = 0; i < weapon->projectileCount; i++)
            {
                float angle = startAngle + i * weapon->spreadAngle;
                Vector2 rotDir = Vector2Rotate(dir, angle);
                Vector2 vel = Vector2Scale(rotDir, weapon->projectileSpeed);

                ProjectileSpawnParams params = {
                    .pos = pos,
                    .vel = vel,
                    .damage = weapon->damage,
                    .radius = weapon->projectileRadius,
                    .lifetime = weapon->projectileLifetime,
                    .weaponType = weapon->type,
                    .pierce = false,
                    .behavior = PROJ_BEHAVIOR_HOMING,
                    .effects = PROJ_EFFECT_NONE,
                    .homingStrength = 5.0f,  // Turn rate
                    .color = projColor,
                };
                ProjectileSpawnEx(pool, &params);
            }
            break;
        }

        case WEAPON_LIGHTNING:
        {
            Vector2 vel = Vector2Scale(dir, weapon->projectileSpeed);

            ProjectileSpawnParams params = {
                .pos = pos,
                .vel = vel,
                .damage = weapon->damage,
                .radius = weapon->projectileRadius,
                .lifetime = weapon->projectileLifetime,
                .weaponType = weapon->type,
                .pierce = true,  // Lightning pierces
                .behavior = PROJ_BEHAVIOR_LINEAR,
                .effects = PROJ_EFFECT_CHAIN,
                .chainCount = weapon->chainCount,
                .color = projColor,
            };
            ProjectileSpawnEx(pool, &params);
            break;
        }

        case WEAPON_ORBIT_SHIELD:
        {
            for (int i = 0; i < weapon->projectileCount; i++)
            {
                float angle = weapon->orbitSpawnAngle + (i * 2.0f * PI / weapon->projectileCount);

                ProjectileSpawnParams params = {
                    .pos = pos,
                    .vel = (Vector2){ 0, 0 },
                    .damage = weapon->damage,
                    .radius = weapon->projectileRadius,
                    .lifetime = weapon->projectileLifetime,
                    .weaponType = weapon->type,
                    .pierce = true,
                    .behavior = PROJ_BEHAVIOR_ORBIT,
                    .effects = PROJ_EFFECT_NONE,
                    .orbitAngle = angle,
                    .orbitRadius = 60.0f,  // Distance from player
                    .orbitSpeed = 3.0f,    // Radians per second
                    .ownerPos = ownerPosPtr,
                    .color = projColor,
                };
                ProjectileSpawnEx(pool, &params);
            }
            weapon->orbitSpawnAngle += 0.5f;  // Offset next spawn
            break;
        }

        case WEAPON_FLAMETHROWER:
        {
            // Rapid fire with random spread in a cone
            for (int i = 0; i < weapon->projectileCount; i++)
            {
                float randomSpread = ((float)(rand() % 100) / 100.0f - 0.5f) * weapon->spreadAngle;
                Vector2 rotDir = Vector2Rotate(dir, randomSpread);
                // Vary speed slightly for organic feel
                float speedVariation = weapon->projectileSpeed * (0.8f + (float)(rand() % 40) / 100.0f);
                Vector2 vel = Vector2Scale(rotDir, speedVariation);

                ProjectileSpawnParams params = {
                    .pos = pos,
                    .vel = vel,
                    .damage = weapon->damage,
                    .radius = weapon->projectileRadius * (0.7f + (float)(rand() % 60) / 100.0f),
                    .lifetime = weapon->projectileLifetime,
                    .weaponType = weapon->type,
                    .pierce = false,
                    .behavior = PROJ_BEHAVIOR_LINEAR,
                    .effects = PROJ_EFFECT_DOT,
                    .color = projColor,
                };
                ProjectileSpawnEx(pool, &params);
            }
            break;
        }

        case WEAPON_FREEZE_RAY:
        {
            float startAngle = -weapon->spreadAngle * (weapon->projectileCount - 1) / 2.0f;

            for (int i = 0; i < weapon->projectileCount; i++)
            {
                float angle = startAngle + i * weapon->spreadAngle;
                Vector2 rotDir = Vector2Rotate(dir, angle);
                Vector2 vel = Vector2Scale(rotDir, weapon->projectileSpeed);

                ProjectileSpawnParams params = {
                    .pos = pos,
                    .vel = vel,
                    .damage = weapon->damage,
                    .radius = weapon->projectileRadius,
                    .lifetime = weapon->projectileLifetime,
                    .weaponType = weapon->type,
                    .pierce = false,
                    .behavior = PROJ_BEHAVIOR_LINEAR,
                    .effects = PROJ_EFFECT_SLOW,
                    .slowAmount = 0.5f,      // 50% slow
                    .slowDuration = 2.0f,    // 2 seconds
                    .color = projColor,
                };
                ProjectileSpawnEx(pool, &params);
            }
            break;
        }

        case WEAPON_BLACK_HOLE:
        {
            Vector2 vel = Vector2Scale(dir, weapon->projectileSpeed);

            ProjectileSpawnParams params = {
                .pos = pos,
                .vel = vel,
                .damage = weapon->damage,
                .radius = weapon->projectileRadius,
                .lifetime = weapon->projectileLifetime,
                .weaponType = weapon->type,
                .pierce = true,  // Black hole doesn't disappear on hit
                .behavior = PROJ_BEHAVIOR_PULL,
                .effects = PROJ_EFFECT_NONE,
                .pullStrength = 200.0f,  // Pull force
                .color = projColor,
            };
            ProjectileSpawnEx(pool, &params);
            break;
        }

        default:
            break;
    }

    PlayGameSound(SOUND_SHOOT);
    weapon->cooldown = 1.0f / weapon->fireRate;
}
