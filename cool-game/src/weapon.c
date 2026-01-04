#include "weapon.h"
#include "audio.h"
#include "types.h"
#include "raymath.h"
#include <math.h>
#include <stdlib.h>

// Weapon constants
#define ORBIT_RADIUS        60.0f   // Distance from player for orbit shield
#define ORBIT_SPEED         3.0f    // Radians per second for orbit shield
#define ORBIT_ANGLE_OFFSET  0.5f    // Angle offset between spawns
#define HOMING_TURN_RATE    5.0f    // Turn rate for homing missiles
#define FREEZE_SLOW_AMOUNT  0.5f    // 50% slow from freeze ray
#define FREEZE_SLOW_DURATION 2.0f   // Duration of freeze effect
#define BLACK_HOLE_PULL     200.0f  // Pull force for black hole

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

                ProjectileSpawnParams params = { 0 };
                params.pos = pos;
                params.vel = vel;
                params.damage = weapon->damage;
                params.radius = weapon->projectileRadius;
                params.lifetime = weapon->projectileLifetime;
                params.weaponType = weapon->type;
                params.pierce = weapon->pierce;
                params.behavior = PROJ_BEHAVIOR_LINEAR;
                params.effects = PROJ_EFFECT_NONE;
                params.color = projColor;
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

                ProjectileSpawnParams params = { 0 };
                params.pos = pos;
                params.vel = vel;
                params.damage = weapon->damage;
                params.radius = weapon->projectileRadius;
                params.lifetime = weapon->projectileLifetime;
                params.weaponType = weapon->type;
                params.pierce = false;
                params.behavior = PROJ_BEHAVIOR_HOMING;
                params.effects = PROJ_EFFECT_NONE;
                params.homingStrength = HOMING_TURN_RATE;
                params.color = projColor;
                ProjectileSpawnEx(pool, &params);
            }
            break;
        }

        case WEAPON_LIGHTNING:
        {
            Vector2 vel = Vector2Scale(dir, weapon->projectileSpeed);

            ProjectileSpawnParams params = { 0 };
            params.pos = pos;
            params.vel = vel;
            params.damage = weapon->damage;
            params.radius = weapon->projectileRadius;
            params.lifetime = weapon->projectileLifetime;
            params.weaponType = weapon->type;
            params.pierce = true;
            params.behavior = PROJ_BEHAVIOR_LINEAR;
            params.effects = PROJ_EFFECT_CHAIN;
            params.chainCount = weapon->chainCount;
            params.color = projColor;
            ProjectileSpawnEx(pool, &params);
            break;
        }

        case WEAPON_ORBIT_SHIELD:
        {
            for (int i = 0; i < weapon->projectileCount; i++)
            {
                float angle = weapon->orbitSpawnAngle + (i * 2.0f * PI / weapon->projectileCount);

                ProjectileSpawnParams params = { 0 };
                params.pos = pos;
                params.vel = (Vector2){ 0, 0 };
                params.damage = weapon->damage;
                params.radius = weapon->projectileRadius;
                params.lifetime = weapon->projectileLifetime;
                params.weaponType = weapon->type;
                params.pierce = true;
                params.behavior = PROJ_BEHAVIOR_ORBIT;
                params.effects = PROJ_EFFECT_NONE;
                params.orbitAngle = angle;
                params.orbitRadius = ORBIT_RADIUS;
                params.orbitSpeed = ORBIT_SPEED;
                params.ownerPos = ownerPosPtr;
                params.color = projColor;
                ProjectileSpawnEx(pool, &params);
            }
            weapon->orbitSpawnAngle += ORBIT_ANGLE_OFFSET;
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

                ProjectileSpawnParams params = { 0 };
                params.pos = pos;
                params.vel = vel;
                params.damage = weapon->damage;
                params.radius = weapon->projectileRadius * (0.7f + (float)(rand() % 60) / 100.0f);
                params.lifetime = weapon->projectileLifetime;
                params.weaponType = weapon->type;
                params.pierce = false;
                params.behavior = PROJ_BEHAVIOR_LINEAR;
                params.effects = PROJ_EFFECT_DOT;
                params.color = projColor;
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

                ProjectileSpawnParams params = { 0 };
                params.pos = pos;
                params.vel = vel;
                params.damage = weapon->damage;
                params.radius = weapon->projectileRadius;
                params.lifetime = weapon->projectileLifetime;
                params.weaponType = weapon->type;
                params.pierce = false;
                params.behavior = PROJ_BEHAVIOR_LINEAR;
                params.effects = PROJ_EFFECT_SLOW;
                params.slowAmount = FREEZE_SLOW_AMOUNT;
                params.slowDuration = FREEZE_SLOW_DURATION;
                params.color = projColor;
                ProjectileSpawnEx(pool, &params);
            }
            break;
        }

        case WEAPON_BLACK_HOLE:
        {
            Vector2 vel = Vector2Scale(dir, weapon->projectileSpeed);

            ProjectileSpawnParams params = { 0 };
            params.pos = pos;
            params.vel = vel;
            params.damage = weapon->damage;
            params.radius = weapon->projectileRadius;
            params.lifetime = weapon->projectileLifetime;
            params.weaponType = weapon->type;
            params.pierce = true;
            params.behavior = PROJ_BEHAVIOR_PULL;
            params.effects = PROJ_EFFECT_NONE;
            params.pullStrength = BLACK_HOLE_PULL;
            params.color = projColor;
            ProjectileSpawnEx(pool, &params);
            break;
        }

        default:
            break;
    }

    PlayGameSound(SOUND_SHOOT);
    weapon->cooldown = 1.0f / weapon->fireRate;
}
