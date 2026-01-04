#ifndef CHARACTER_H
#define CHARACTER_H

#include "raylib.h"
#include "weapon.h"
#include <stdbool.h>

#define CHARACTER_COUNT 3

// Character types
typedef enum CharacterType {
    CHARACTER_VANGUARD,    // Balanced (default)
    CHARACTER_TITAN,       // Tank - more HP, slower, more armor
    CHARACTER_PHANTOM      // Speedster - faster, less HP, better magnet
} CharacterType;

// Character definition with stats
typedef struct CharacterDef {
    CharacterType type;
    const char *name;
    const char *description;
    // Base stats
    float speed;
    float maxHealth;
    float radius;
    float magnetRadius;
    float armor;
    WeaponType startingWeapon;
    // Passive bonuses
    float damageMultiplier;
    float xpMultiplier;
    float dashCooldownMultiplier;
    // Visual
    Color primaryColor;
    Color secondaryColor;
} CharacterDef;

// Get character definition by type
CharacterDef GetCharacterDef(CharacterType type);

// Get character name
const char* CharacterGetName(CharacterType type);

// Get character description
const char* CharacterGetDescription(CharacterType type);

#endif // CHARACTER_H
