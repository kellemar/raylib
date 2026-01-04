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

// Weapon color palette for each type (base + evolved)
static const Color WEAPON_COLORS[WEAPON_COUNT] = {
    // Base weapons
    { 255, 255, 50, 255 },   // PULSE_CANNON - Yellow
    { 255, 150, 50, 255 },   // SPREAD_SHOT - Orange
    { 255, 50, 150, 255 },   // HOMING_MISSILE - Pink
    { 100, 200, 255, 255 },  // LIGHTNING - Light Blue
    { 50, 255, 255, 255 },   // ORBIT_SHIELD - Cyan
    { 255, 100, 50, 255 },   // FLAMETHROWER - Red-Orange
    { 150, 200, 255, 255 },  // FREEZE_RAY - Ice Blue
    { 150, 50, 200, 255 },   // BLACK_HOLE - Purple
    // Evolved weapons (brighter, more saturated)
    { 255, 255, 200, 255 },  // MEGA_CANNON - Bright Yellow-White
    { 255, 200, 100, 255 },  // CIRCLE_BURST - Bright Orange
    { 255, 100, 200, 255 },  // SWARM - Bright Pink
    { 200, 255, 255, 255 },  // TESLA_COIL - Bright Cyan
    { 100, 255, 200, 255 },  // BLADE_DANCER - Bright Teal
    { 255, 50, 50, 255 },    // INFERNO - Bright Red
    { 200, 230, 255, 255 },  // BLIZZARD - Bright Ice
    { 200, 100, 255, 255 },  // SINGULARITY - Bright Purple
};

static const char* WEAPON_NAMES[WEAPON_COUNT] = {
    // Base weapons
    "Pulse Cannon",
    "Spread Shot",
    "Homing Missiles",
    "Lightning",
    "Orbit Shield",
    "Flamethrower",
    "Freeze Ray",
    "Black Hole",
    // Evolved weapons
    "MEGA CANNON",
    "CIRCLE BURST",
    "SWARM",
    "TESLA COIL",
    "BLADE DANCER",
    "INFERNO",
    "BLIZZARD",
    "SINGULARITY",
};

void WeaponInit(Weapon *weapon, WeaponType type)
{
    weapon->type = type;
    weapon->cooldown = 0.0f;
    weapon->level = 1;
    weapon->pierce = false;
    weapon->chainCount = 0;
    weapon->orbitSpawnAngle = 0.0f;
    // Initialize upgrade-based stats
    weapon->critChance = 0.0f;
    weapon->critMultiplier = 2.0f;
    weapon->doubleShot = false;
    weapon->ricochetCount = 0;
    weapon->explosive = false;
    weapon->explosionRadius = 30.0f;
    weapon->homingStrength = 1.0f;

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

        // Evolved weapons - significantly more powerful
        case WEAPON_MEGA_CANNON:
            weapon->damage = 50.0f;  // Massive damage
            weapon->fireRate = 2.0f;
            weapon->projectileSpeed = 800.0f;  // Very fast
            weapon->projectileRadius = 15.0f;  // Huge beam
            weapon->projectileLifetime = 3.0f;
            weapon->projectileCount = 1;
            weapon->spreadAngle = 0.0f;
            weapon->pierce = true;  // Always pierces
            break;

        case WEAPON_CIRCLE_BURST:
            weapon->damage = 15.0f;
            weapon->fireRate = 1.5f;
            weapon->projectileSpeed = 400.0f;
            weapon->projectileRadius = 6.0f;
            weapon->projectileLifetime = 2.0f;
            weapon->projectileCount = 16;  // 360-degree burst
            weapon->spreadAngle = 0.3927f;  // 2*PI/16 = 22.5 degrees
            break;

        case WEAPON_SWARM:
            weapon->damage = 8.0f;
            weapon->fireRate = 3.0f;
            weapon->projectileSpeed = 300.0f;
            weapon->projectileRadius = 4.0f;
            weapon->projectileLifetime = 5.0f;
            weapon->projectileCount = 6;  // Multiple homing missiles
            weapon->spreadAngle = 0.5f;
            weapon->homingStrength = 2.0f;
            break;

        case WEAPON_TESLA_COIL:
            weapon->damage = 12.0f;
            weapon->fireRate = 8.0f;  // Very rapid
            weapon->projectileSpeed = 700.0f;
            weapon->projectileRadius = 5.0f;
            weapon->projectileLifetime = 1.2f;
            weapon->projectileCount = 3;  // Multiple lightning bolts
            weapon->spreadAngle = 0.3f;
            weapon->chainCount = 5;  // Chains to more enemies
            weapon->pierce = true;
            break;

        case WEAPON_BLADE_DANCER:
            weapon->damage = 25.0f;
            weapon->fireRate = 0.8f;
            weapon->projectileSpeed = 0.0f;
            weapon->projectileRadius = 10.0f;
            weapon->projectileLifetime = 12.0f;  // Long lasting
            weapon->projectileCount = 2;  // Spawn 2 at a time
            weapon->spreadAngle = 0.0f;
            weapon->pierce = true;
            break;

        case WEAPON_INFERNO:
            weapon->damage = 8.0f;
            weapon->fireRate = 30.0f;  // Extremely rapid
            weapon->projectileSpeed = 500.0f;
            weapon->projectileRadius = 10.0f;
            weapon->projectileLifetime = 0.8f;  // Longer range
            weapon->projectileCount = 2;  // Double flames
            weapon->spreadAngle = 0.6f;  // Wider cone
            break;

        case WEAPON_BLIZZARD:
            weapon->damage = 10.0f;
            weapon->fireRate = 10.0f;
            weapon->projectileSpeed = 350.0f;
            weapon->projectileRadius = 8.0f;
            weapon->projectileLifetime = 2.0f;
            weapon->projectileCount = 4;  // Multiple ice shards
            weapon->spreadAngle = 0.25f;
            break;

        case WEAPON_SINGULARITY:
            weapon->damage = 20.0f;  // High damage
            weapon->fireRate = 0.5f;
            weapon->projectileSpeed = 80.0f;  // Very slow
            weapon->projectileRadius = 25.0f;  // Massive
            weapon->projectileLifetime = 8.0f;  // Very long
            weapon->projectileCount = 1;
            weapon->spreadAngle = 0.0f;
            weapon->explosive = true;
            weapon->explosionRadius = 100.0f;
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
        case WEAPON_MEGA_CANNON:
        case WEAPON_CIRCLE_BURST:
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
        case WEAPON_SWARM:
        {
            float startAngle = -weapon->spreadAngle * (weapon->projectileCount - 1) / 2.0f;
            float homingRate = HOMING_TURN_RATE * weapon->homingStrength;

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
                params.homingStrength = homingRate;
                params.color = projColor;
                ProjectileSpawnEx(pool, &params);
            }
            break;
        }

        case WEAPON_LIGHTNING:
        case WEAPON_TESLA_COIL:
        {
            // Tesla Coil fires multiple lightning bolts, Lightning fires one
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
                params.effects = PROJ_EFFECT_CHAIN;
                params.chainCount = weapon->chainCount;
                params.color = projColor;
                ProjectileSpawnEx(pool, &params);
            }
            break;
        }

        case WEAPON_ORBIT_SHIELD:
        case WEAPON_BLADE_DANCER:
        {
            // Blade Dancer has faster orbit and more blades
            float orbitSpeed = (weapon->type == WEAPON_BLADE_DANCER) ? ORBIT_SPEED * 2.0f : ORBIT_SPEED;
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
                params.orbitSpeed = orbitSpeed;
                params.ownerPos = ownerPosPtr;
                params.color = projColor;
                ProjectileSpawnEx(pool, &params);
            }
            weapon->orbitSpawnAngle += ORBIT_ANGLE_OFFSET;
            break;
        }

        case WEAPON_FLAMETHROWER:
        case WEAPON_INFERNO:
        {
            // Rapid fire with random spread in a cone (Inferno is wider and stronger)
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
        case WEAPON_BLIZZARD:
        {
            // Blizzard fires multiple ice shards with stronger slow
            float startAngle = -weapon->spreadAngle * (weapon->projectileCount - 1) / 2.0f;
            float slowAmount = (weapon->type == WEAPON_BLIZZARD) ? 0.8f : FREEZE_SLOW_AMOUNT;
            float slowDuration = (weapon->type == WEAPON_BLIZZARD) ? 3.0f : FREEZE_SLOW_DURATION;

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
                params.slowAmount = slowAmount;
                params.slowDuration = slowDuration;
                params.color = projColor;
                ProjectileSpawnEx(pool, &params);
            }
            break;
        }

        case WEAPON_BLACK_HOLE:
        case WEAPON_SINGULARITY:
        {
            // Singularity has stronger pull and explosive effect
            float pullStrength = (weapon->type == WEAPON_SINGULARITY) ? BLACK_HOLE_PULL * 2.0f : BLACK_HOLE_PULL;
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
            params.pullStrength = pullStrength;
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

// Evolution system functions
bool WeaponIsEvolved(WeaponType type)
{
    return type >= WEAPON_BASE_COUNT && type < WEAPON_COUNT;
}

bool WeaponCanEvolve(Weapon *weapon, bool hasCatalyst)
{
    if (weapon->level < WEAPON_MAX_LEVEL) return false;
    if (weapon->type >= WEAPON_BASE_COUNT) return false;  // Already evolved
    return hasCatalyst;
}

WeaponType WeaponGetEvolvedType(WeaponType baseType)
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
        default: return baseType;  // Already evolved or invalid
    }
}

WeaponType WeaponGetBaseType(WeaponType evolvedType)
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
        default: return evolvedType;  // Already base or invalid
    }
}

void WeaponEvolve(Weapon *weapon)
{
    if (weapon->type >= WEAPON_BASE_COUNT) return;  // Already evolved

    WeaponType evolvedType = WeaponGetEvolvedType(weapon->type);
    int prevLevel = weapon->level;

    WeaponInit(weapon, evolvedType);
    weapon->level = prevLevel;  // Preserve level
}

void WeaponLevelUp(Weapon *weapon)
{
    if (weapon->level < WEAPON_MAX_LEVEL)
    {
        weapon->level++;
        // Boost stats slightly with each level
        weapon->damage *= 1.1f;
        weapon->fireRate *= 1.05f;
    }
}
