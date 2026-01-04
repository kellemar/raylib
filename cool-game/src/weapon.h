#ifndef WEAPON_H
#define WEAPON_H

#include "raylib.h"
#include "projectile.h"
#include <stdbool.h>

typedef enum WeaponType {
    // Base weapons
    WEAPON_PULSE_CANNON,    // Single shot, medium speed
    WEAPON_SPREAD_SHOT,     // 3-way spread
    WEAPON_HOMING_MISSILE,  // Slow, seeks enemies
    WEAPON_LIGHTNING,       // Chain to nearby enemies
    WEAPON_ORBIT_SHIELD,    // Rotating projectiles around player
    WEAPON_FLAMETHROWER,    // Rapid fire short range cone
    WEAPON_FREEZE_RAY,      // Slows enemies on hit
    WEAPON_BLACK_HOLE,      // Pulls enemies in
    WEAPON_BASE_COUNT,      // Number of base weapons (for cycling)
    // Evolved weapons (unlocked via evolution)
    WEAPON_MEGA_CANNON = WEAPON_BASE_COUNT,  // Pulse Cannon + Pierce → huge piercing beam
    WEAPON_CIRCLE_BURST,    // Spread Shot + Multi Shot → 360° nova
    WEAPON_SWARM,           // Homing Missile + Double Shot → 20 mini missiles
    WEAPON_TESLA_COIL,      // Lightning + Crit Chance → permanent AoE damage
    WEAPON_BLADE_DANCER,    // Orbit Shield + Damage → 8 fast orbiting blades
    WEAPON_INFERNO,         // Flamethrower + Range → screen-wide fire
    WEAPON_BLIZZARD,        // Freeze Ray + Slow Aura → freeze + shatter
    WEAPON_SINGULARITY,     // Black Hole + Explosive → insta-kill vortex
    WEAPON_COUNT
} WeaponType;

#define WEAPON_MAX_LEVEL 5  // Level required for evolution

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
    bool pierce;            // Projectiles pass through enemies
    float spreadAngle;      // Angle between multi-shot projectiles
    int chainCount;         // For lightning - how many enemies to chain to
    float orbitSpawnAngle;  // For orbit shield - angle offset for next spawn
    // Upgrade-based stats
    float critChance;       // Chance for critical hit (0.0 - 1.0)
    float critMultiplier;   // Critical hit damage multiplier (default 2.0)
    bool doubleShot;        // Fire twice per shot
    int ricochetCount;      // Number of times projectiles can bounce
    bool explosive;         // Projectiles explode on hit
    float explosionRadius;  // Radius of explosion
    float homingStrength;   // Homing projectile tracking strength multiplier
} Weapon;

void WeaponInit(Weapon *weapon, WeaponType type);
void WeaponUpdate(Weapon *weapon, float dt);
bool WeaponCanFire(Weapon *weapon);
void WeaponFire(Weapon *weapon, ProjectilePool *pool, Vector2 pos, Vector2 dir, Vector2 *ownerPosPtr);
const char* WeaponGetName(WeaponType type);
Color WeaponGetColor(WeaponType type);

// Evolution system
bool WeaponIsEvolved(WeaponType type);
bool WeaponCanEvolve(Weapon *weapon, bool hasCatalyst);
WeaponType WeaponGetEvolvedType(WeaponType baseType);
WeaponType WeaponGetBaseType(WeaponType evolvedType);
void WeaponEvolve(Weapon *weapon);
void WeaponLevelUp(Weapon *weapon);

#endif
