#include "minunit.h"
#include <string.h>

// Character types and structures (matching character.h)
#define CHARACTER_COUNT 3

typedef enum CharacterType {
    CHARACTER_VANGUARD,
    CHARACTER_TITAN,
    CHARACTER_PHANTOM
} CharacterType;

typedef enum WeaponType {
    WEAPON_PULSE_CANNON,
    WEAPON_SPREAD_SHOT,
    WEAPON_HOMING_MISSILE,
    WEAPON_LIGHTNING,
    WEAPON_ORBIT_SHIELD,
    WEAPON_FLAMETHROWER,
    WEAPON_FREEZE_RAY,
    WEAPON_BLACK_HOLE,
    WEAPON_BASE_COUNT
} WeaponType;

typedef struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} Color;

typedef struct CharacterDef {
    CharacterType type;
    const char *name;
    const char *description;
    float speed;
    float maxHealth;
    float radius;
    float magnetRadius;
    float armor;
    WeaponType startingWeapon;
    float damageMultiplier;
    float xpMultiplier;
    float dashCooldownMultiplier;
    Color primaryColor;
    Color secondaryColor;
} CharacterDef;

// Test implementations (matching character.c)
static const CharacterDef characterDefs[CHARACTER_COUNT] = {
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
        .primaryColor = { 0, 255, 255, 255 },
        .secondaryColor = { 255, 0, 128, 255 }
    },
    {
        .type = CHARACTER_TITAN,
        .name = "TITAN",
        .description = "Heavy tank - high HP, slow movement",
        .speed = 240.0f,
        .maxHealth = 150.0f,
        .radius = 18.0f,
        .magnetRadius = 60.0f,
        .armor = 5.0f,
        .startingWeapon = WEAPON_PULSE_CANNON,
        .damageMultiplier = 1.2f,
        .xpMultiplier = 0.9f,
        .dashCooldownMultiplier = 1.3f,
        .primaryColor = { 255, 150, 0, 255 },
        .secondaryColor = { 255, 50, 50, 255 }
    },
    {
        .type = CHARACTER_PHANTOM,
        .name = "PHANTOM",
        .description = "Swift assassin - fast, fragile",
        .speed = 380.0f,
        .maxHealth = 70.0f,
        .radius = 12.0f,
        .magnetRadius = 120.0f,
        .armor = 0.0f,
        .startingWeapon = WEAPON_PULSE_CANNON,
        .damageMultiplier = 0.9f,
        .xpMultiplier = 1.25f,
        .dashCooldownMultiplier = 0.7f,
        .primaryColor = { 128, 255, 128, 255 },
        .secondaryColor = { 255, 255, 0, 255 }
    }
};

static CharacterDef GetCharacterDef(CharacterType type)
{
    if (type < 0 || type >= CHARACTER_COUNT)
    {
        return characterDefs[CHARACTER_VANGUARD];
    }
    return characterDefs[type];
}

static const char* CharacterGetName(CharacterType type)
{
    if (type < 0 || type >= CHARACTER_COUNT)
    {
        return "UNKNOWN";
    }
    return characterDefs[type].name;
}

static const char* CharacterGetDescription(CharacterType type)
{
    if (type < 0 || type >= CHARACTER_COUNT)
    {
        return "";
    }
    return characterDefs[type].description;
}

// Tests

static const char* test_character_count(void)
{
    mu_assert_int_eq(3, CHARACTER_COUNT);
    return 0;
}

static const char* test_character_vanguard_stats(void)
{
    CharacterDef def = GetCharacterDef(CHARACTER_VANGUARD);

    mu_assert_int_eq(CHARACTER_VANGUARD, def.type);
    mu_assert_float_eq(300.0f, def.speed);
    mu_assert_float_eq(100.0f, def.maxHealth);
    mu_assert_float_eq(15.0f, def.radius);
    mu_assert_float_eq(80.0f, def.magnetRadius);
    mu_assert_float_eq(0.0f, def.armor);
    mu_assert_float_eq(1.0f, def.damageMultiplier);
    mu_assert_float_eq(1.0f, def.xpMultiplier);
    return 0;
}

static const char* test_character_titan_stats(void)
{
    CharacterDef def = GetCharacterDef(CHARACTER_TITAN);

    mu_assert_int_eq(CHARACTER_TITAN, def.type);
    mu_assert_float_eq(240.0f, def.speed);
    mu_assert_float_eq(150.0f, def.maxHealth);
    mu_assert_float_eq(18.0f, def.radius);
    mu_assert_float_eq(5.0f, def.armor);
    mu_assert_float_eq(1.2f, def.damageMultiplier);
    mu_assert_float_eq(0.9f, def.xpMultiplier);
    return 0;
}

static const char* test_character_phantom_stats(void)
{
    CharacterDef def = GetCharacterDef(CHARACTER_PHANTOM);

    mu_assert_int_eq(CHARACTER_PHANTOM, def.type);
    mu_assert_float_eq(380.0f, def.speed);
    mu_assert_float_eq(70.0f, def.maxHealth);
    mu_assert_float_eq(12.0f, def.radius);
    mu_assert_float_eq(120.0f, def.magnetRadius);
    mu_assert_float_eq(1.25f, def.xpMultiplier);
    return 0;
}

static const char* test_character_names(void)
{
    mu_assert(strcmp(CharacterGetName(CHARACTER_VANGUARD), "VANGUARD") == 0,
              "Vanguard name should be VANGUARD");
    mu_assert(strcmp(CharacterGetName(CHARACTER_TITAN), "TITAN") == 0,
              "Titan name should be TITAN");
    mu_assert(strcmp(CharacterGetName(CHARACTER_PHANTOM), "PHANTOM") == 0,
              "Phantom name should be PHANTOM");
    return 0;
}

static const char* test_character_descriptions(void)
{
    mu_assert(strlen(CharacterGetDescription(CHARACTER_VANGUARD)) > 0,
              "Vanguard should have description");
    mu_assert(strlen(CharacterGetDescription(CHARACTER_TITAN)) > 0,
              "Titan should have description");
    mu_assert(strlen(CharacterGetDescription(CHARACTER_PHANTOM)) > 0,
              "Phantom should have description");
    return 0;
}

static const char* test_character_invalid_type(void)
{
    // Invalid type should return Vanguard (default)
    CharacterDef def = GetCharacterDef(-1);
    mu_assert_int_eq(CHARACTER_VANGUARD, def.type);

    def = GetCharacterDef(100);
    mu_assert_int_eq(CHARACTER_VANGUARD, def.type);

    mu_assert(strcmp(CharacterGetName(-1), "UNKNOWN") == 0,
              "Invalid type name should be UNKNOWN");
    return 0;
}

static const char* test_character_balance(void)
{
    // Vanguard is balanced baseline
    CharacterDef vanguard = GetCharacterDef(CHARACTER_VANGUARD);
    CharacterDef titan = GetCharacterDef(CHARACTER_TITAN);
    CharacterDef phantom = GetCharacterDef(CHARACTER_PHANTOM);

    // Titan should be slower than Vanguard
    mu_assert(titan.speed < vanguard.speed, "Titan should be slower than Vanguard");
    // Titan should have more HP
    mu_assert(titan.maxHealth > vanguard.maxHealth, "Titan should have more HP");

    // Phantom should be faster than Vanguard
    mu_assert(phantom.speed > vanguard.speed, "Phantom should be faster than Vanguard");
    // Phantom should have less HP
    mu_assert(phantom.maxHealth < vanguard.maxHealth, "Phantom should have less HP");

    return 0;
}

const char* run_character_tests(void)
{
    mu_run_test(test_character_count);
    mu_run_test(test_character_vanguard_stats);
    mu_run_test(test_character_titan_stats);
    mu_run_test(test_character_phantom_stats);
    mu_run_test(test_character_names);
    mu_run_test(test_character_descriptions);
    mu_run_test(test_character_invalid_type);
    mu_run_test(test_character_balance);
    return 0;
}
