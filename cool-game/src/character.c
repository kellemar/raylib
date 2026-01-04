#include "character.h"
#include "types.h"

// Character definitions
static const CharacterDef characterDefs[CHARACTER_COUNT] = {
    // CHARACTER_VANGUARD - Balanced
    {
        .type = CHARACTER_VANGUARD,
        .name = "VANGUARD",
        .description = "Balanced fighter - jack of all trades",
        .speed = 300.0f,
        .maxHealth = 100.0f,
        .radius = 15.0f,
        .magnetRadius = 80.0f,
        .armor = 0.0f,
        .startingWeapon = WEAPON_PULSE_CANNON,
        .damageMultiplier = 1.0f,
        .xpMultiplier = 1.0f,
        .dashCooldownMultiplier = 1.0f,
        .primaryColor = { 0, 255, 255, 255 },    // Cyan
        .secondaryColor = { 255, 0, 128, 255 }   // Pink
    },
    // CHARACTER_TITAN - Tank
    {
        .type = CHARACTER_TITAN,
        .name = "TITAN",
        .description = "Heavy tank - high HP, slow movement",
        .speed = 240.0f,          // 20% slower
        .maxHealth = 150.0f,      // 50% more HP
        .radius = 18.0f,          // Slightly larger
        .magnetRadius = 60.0f,    // Smaller magnet
        .armor = 5.0f,            // Starting armor
        .startingWeapon = WEAPON_PULSE_CANNON,
        .damageMultiplier = 1.2f, // 20% more damage
        .xpMultiplier = 0.9f,     // 10% less XP
        .dashCooldownMultiplier = 1.3f,  // 30% longer dash cooldown
        .primaryColor = { 255, 150, 0, 255 },   // Orange
        .secondaryColor = { 255, 50, 50, 255 }  // Red
    },
    // CHARACTER_PHANTOM - Speedster
    {
        .type = CHARACTER_PHANTOM,
        .name = "PHANTOM",
        .description = "Swift assassin - fast, fragile",
        .speed = 380.0f,          // 27% faster
        .maxHealth = 70.0f,       // 30% less HP
        .radius = 12.0f,          // Smaller hitbox
        .magnetRadius = 120.0f,   // 50% bigger magnet
        .armor = 0.0f,
        .startingWeapon = WEAPON_PULSE_CANNON,
        .damageMultiplier = 0.9f, // 10% less damage
        .xpMultiplier = 1.25f,    // 25% more XP
        .dashCooldownMultiplier = 0.7f,  // 30% faster dash
        .primaryColor = { 128, 255, 128, 255 }, // Light green
        .secondaryColor = { 255, 255, 0, 255 }  // Yellow
    }
};

CharacterDef GetCharacterDef(CharacterType type)
{
    if (type < 0 || type >= CHARACTER_COUNT)
    {
        return characterDefs[CHARACTER_VANGUARD];
    }
    return characterDefs[type];
}

const char* CharacterGetName(CharacterType type)
{
    if (type < 0 || type >= CHARACTER_COUNT)
    {
        return "UNKNOWN";
    }
    return characterDefs[type].name;
}

const char* CharacterGetDescription(CharacterType type)
{
    if (type < 0 || type >= CHARACTER_COUNT)
    {
        return "";
    }
    return characterDefs[type].description;
}
