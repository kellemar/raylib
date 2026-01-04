#include "weapon.h"
#include "raymath.h"

void WeaponInit(Weapon *weapon, WeaponType type)
{
    weapon->type = type;
    weapon->cooldown = 0.0f;
    weapon->level = 1;

    switch (type)
    {
        case WEAPON_PULSE_CANNON:
        default:
            weapon->damage = 10.0f;
            weapon->fireRate = 5.0f;
            weapon->projectileSpeed = 500.0f;
            weapon->projectileRadius = 5.0f;
            weapon->projectileLifetime = 2.0f;
            weapon->projectileCount = 1;
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

void WeaponFire(Weapon *weapon, ProjectilePool *pool, Vector2 pos, Vector2 dir)
{
    if (!WeaponCanFire(weapon)) return;

    Vector2 vel = Vector2Scale(dir, weapon->projectileSpeed);
    ProjectileSpawn(pool, pos, vel, weapon->damage, weapon->projectileRadius, weapon->projectileLifetime);

    weapon->cooldown = 1.0f / weapon->fireRate;
}
