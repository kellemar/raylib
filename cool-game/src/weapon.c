#include "weapon.h"
#include "audio.h"
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

    // Spread angle for multi-shot (radians between projectiles)
    float spreadAngle = 0.15f;
    float startAngle = -spreadAngle * (weapon->projectileCount - 1) / 2.0f;

    for (int i = 0; i < weapon->projectileCount; i++)
    {
        float angle = startAngle + i * spreadAngle;
        Vector2 rotDir = Vector2Rotate(dir, angle);
        Vector2 vel = Vector2Scale(rotDir, weapon->projectileSpeed);
        ProjectileSpawn(pool, pos, vel, weapon->damage, weapon->projectileRadius, weapon->projectileLifetime);
    }

    PlayGameSound(SOUND_SHOOT);
    weapon->cooldown = 1.0f / weapon->fireRate;
}
