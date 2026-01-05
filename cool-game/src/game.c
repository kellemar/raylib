#include "game.h"
#include "types.h"
#include "utils.h"
#include "ui.h"
#include "audio.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define XP_COLLECT_RADIUS 15.0f
#define CAMERA_LERP_SPEED 5.0f
#define GRID_SIZE 64
#define SETTINGS_FILE "settings.dat"
#define BLACK_HOLE_PULL_RADIUS_MULT 5.0f
#define WORLD_VIEW_MARGIN 200.0f

static const Color GRID_COLOR = { 30, 25, 40, 100 };


// Settings persistence
static void LoadSettings(GameSettings *settings)
{
    FILE *file = fopen(SETTINGS_FILE, "rb");
    if (file != NULL)
    {
        if (fread(settings, sizeof(GameSettings), 1, file) != 1)
        {
            // Set defaults on read failure
            settings->musicVolume = DEFAULT_MUSIC_VOLUME;
            settings->sfxVolume = DEFAULT_SFX_VOLUME;
            settings->screenShakeEnabled = DEFAULT_SCREEN_SHAKE;
            settings->crtEnabled = DEFAULT_CRT_ENABLED;
            settings->chromaticEnabled = DEFAULT_CHROMATIC_ENABLED;
        }
        fclose(file);
    }
    else
    {
        // Set defaults if file doesn't exist
        settings->musicVolume = DEFAULT_MUSIC_VOLUME;
        settings->sfxVolume = DEFAULT_SFX_VOLUME;
        settings->screenShakeEnabled = DEFAULT_SCREEN_SHAKE;
        settings->crtEnabled = DEFAULT_CRT_ENABLED;
        settings->chromaticEnabled = DEFAULT_CHROMATIC_ENABLED;
    }
}

static void SaveSettings(GameSettings *settings)
{
    FILE *file = fopen(SETTINGS_FILE, "wb");
    if (file != NULL)
    {
        fwrite(settings, sizeof(GameSettings), 1, file);
        fclose(file);
    }
}

static void ApplySettings(GameData *game)
{
    SetGameMusicVolume(game->settings.musicVolume);
    SetGameSFXVolume(game->settings.sfxVolume);
    game->crtEnabled = game->settings.crtEnabled;
}

// Menu starfield effect
#define MENU_STARS 100
#define STAR_MAX_DISTANCE 800.0f
#define STAR_BASE_SPEED 50.0f

typedef struct {
    float angle;      // Direction from center (radians)
    float distance;   // Distance from screen center
    float speed;      // Base speed multiplier
    float size;       // Star size (1-3)
} MenuStar;

static MenuStar menuStars[MENU_STARS];
static bool menuStarsInit = false;

static void InitMenuStars(void)
{
    for (int i = 0; i < MENU_STARS; i++)
    {
        menuStars[i].angle = ((float)(rand() % 360)) * DEG2RAD;
        menuStars[i].distance = (float)(rand() % (int)STAR_MAX_DISTANCE);
        menuStars[i].speed = 0.5f + (float)(rand() % 100) / 100.0f;  // 0.5 to 1.5
        menuStars[i].size = 1.0f + (float)(rand() % 3);  // 1 to 3
    }
    menuStarsInit = true;
}

static void UpdateMenuStars(float dt)
{
    for (int i = 0; i < MENU_STARS; i++)
    {
        // Move outward with acceleration based on distance
        float speedMultiplier = 1.0f + (menuStars[i].distance / STAR_MAX_DISTANCE) * 3.0f;
        menuStars[i].distance += STAR_BASE_SPEED * menuStars[i].speed * speedMultiplier * dt;

        // Respawn at center when out of bounds
        if (menuStars[i].distance > STAR_MAX_DISTANCE)
        {
            menuStars[i].angle = ((float)(rand() % 360)) * DEG2RAD;
            menuStars[i].distance = 5.0f + (float)(rand() % 20);  // Start near center
            menuStars[i].speed = 0.5f + (float)(rand() % 100) / 100.0f;
            menuStars[i].size = 1.0f + (float)(rand() % 3);
        }
    }
}

static void DrawMenuStars(void)
{
    Vector2 center = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };

    for (int i = 0; i < MENU_STARS; i++)
    {
        // Calculate position from polar coordinates
        float x = center.x + cosf(menuStars[i].angle) * menuStars[i].distance;
        float y = center.y + sinf(menuStars[i].angle) * menuStars[i].distance;

        // Size increases with distance (parallax effect)
        float size = menuStars[i].size * (0.5f + (menuStars[i].distance / STAR_MAX_DISTANCE) * 1.5f);

        // Alpha increases with distance (stars become more visible as they move out)
        float alpha = 0.3f + (menuStars[i].distance / STAR_MAX_DISTANCE) * 0.7f;
        unsigned char a = (unsigned char)(alpha * 255);

        // Color varies between white and cyan
        Color starColor = (i % 3 == 0) ? (Color){ 100, 200, 255, a } : (Color){ 255, 255, 255, a };

        DrawCircle((int)x, (int)y, size, starColor);
    }
}

static int GetXPForLevel(int level)
{
    return 10 * level * level;
}

// Achievement helper: earn achievement and queue popup
static void TryEarnAchievement(GameData *game, AchievementType type)
{
    if (AchievementEarn(&game->achievements, type))
    {
        // Newly earned - show popup
        game->pendingAchievement = type;
        game->achievementDisplayTimer = 3.0f;  // Show for 3 seconds
        AchievementSave(&game->achievements);
    }
}

static Vector2 GetSpawnPosition(Vector2 playerPos)
{
    // Spawn enemies in a ring around the player (400-600 units away)
    // No world bounds clamping - world is infinite with following camera
    float angle = (float)(rand() % 360) * DEG2RAD;
    float distance = 400.0f + (float)(rand() % 200);
    Vector2 offset = { cosf(angle) * distance, sinf(angle) * distance };
    return Vector2Add(playerPos, offset);
}

typedef struct BlackHolePullContext {
    Projectile *projectile;
    float pullRadius;
    float dt;
} BlackHolePullContext;

static bool ApplyBlackHolePullVisit(Enemy *enemy, int index, void *user)
{
    (void)index;
    BlackHolePullContext *ctx = (BlackHolePullContext *)user;
    Projectile *p = ctx->projectile;

    float dx = p->pos.x - enemy->pos.x;
    float dy = p->pos.y - enemy->pos.y;
    float distSq = dx * dx + dy * dy;

    if (distSq > 1.0f)
    {
        float dist = sqrtf(distSq);
        float pullFactor = 1.0f - (dist / ctx->pullRadius);
        float pullForce = p->pullStrength * pullFactor * ctx->dt;

        enemy->pos.x += (dx / dist) * pullForce;
        enemy->pos.y += (dy / dist) * pullForce;
    }

    return true;
}

static void ApplyBlackHolePull(ProjectilePool *projectiles, EnemyPool *enemies, EnemySpatialGrid *grid, float dt)
{
    // Black hole projectiles pull nearby enemies toward them
    for (int i = 0; i < projectiles->count; i++)
    {
        Projectile *p = &projectiles->projectiles[projectiles->activeIndices[i]];
        if (!p->active || p->behavior != PROJ_BEHAVIOR_PULL) continue;

        float pullRadius = p->radius * BLACK_HOLE_PULL_RADIUS_MULT;
        BlackHolePullContext ctx = { p, pullRadius, dt };
        EnemySpatialGridForEachInRadius(grid, enemies, p->pos, pullRadius, ApplyBlackHolePullVisit, &ctx);
    }
}

typedef struct ProjectileCollisionContext {
    ProjectilePool *projectiles;
    EnemyPool *enemies;
    XPPool *xp;
    ParticlePool *particles;
    DecalPool *decals;
    GameData *game;
    Player *player;
    Projectile *projectile;
    int projectileIndex;
} ProjectileCollisionContext;

static bool ProjectileEnemyCollisionVisit(Enemy *enemy, int index, void *user)
{
    ProjectileCollisionContext *ctx = (ProjectileCollisionContext *)user;
    Projectile *p = ctx->projectile;

    if (!CheckCircleCollision(p->pos, p->radius, enemy->pos, enemy->radius))
    {
        return true;
    }

    float damage = p->damage;
    if (ctx->player->weapon.critChance > 0.0f)
    {
        float roll = (float)(rand() % 100) / 100.0f;
        if (roll < ctx->player->weapon.critChance)
        {
            damage *= ctx->player->weapon.critMultiplier;
        }
    }

    enemy->health -= damage;
    enemy->hitFlashTimer = 0.1f;

    if (ctx->player->vampirism > 0.0f && ctx->player->health < ctx->player->maxHealth)
    {
        float healAmount = damage * ctx->player->vampirism;
        ctx->player->health += healAmount;
        if (ctx->player->health > ctx->player->maxHealth)
        {
            ctx->player->health = ctx->player->maxHealth;
        }
    }

    SpawnHitParticles(ctx->particles, p->pos, p->color, 5);

    if (p->effects & PROJ_EFFECT_SLOW)
    {
        EnemyApplySlow(enemy, p->slowAmount, p->slowDuration);
    }

    if (!p->pierce)
    {
        ProjectileDeactivate(ctx->projectiles, ctx->projectileIndex);
    }

    if (enemy->health <= 0.0f)
    {
        Vector2 deathPos = enemy->pos;
        EnemyType deathType = enemy->type;
        int deathSplitCount = enemy->splitCount;
        float deathRadius = enemy->radius;
        float deathMaxHealth = enemy->maxHealth;

        if (deathType == ENEMY_SPLITTER && deathSplitCount > 0)
        {
            float childRadius = deathRadius * 0.7f;
            float childHealth = deathMaxHealth * 0.5f;
            int childSplitCount = deathSplitCount - 1;

            Vector2 offset1 = { -deathRadius, 0.0f };
            Vector2 offset2 = { deathRadius, 0.0f };
            Vector2 childPos1 = Vector2Add(deathPos, offset1);
            Vector2 childPos2 = Vector2Add(deathPos, offset2);

            EnemySpawnSplitterChild(ctx->enemies, childPos1, childSplitCount, childRadius, childHealth);
            EnemySpawnSplitterChild(ctx->enemies, childPos2, childSplitCount, childRadius, childHealth);

            SpawnDeathExplosion(ctx->particles, deathPos, DEATH_EXPLOSION_SPLITTER, deathRadius);
        }
        else
        {
            XPSpawn(ctx->xp, deathPos, enemy->xpValue);

            // Choose explosion type based on enemy properties
            DeathExplosionType explosionType;
            if (enemy->isBoss)
            {
                explosionType = DEATH_EXPLOSION_BOSS;
            }
            else if (enemy->isElite)
            {
                explosionType = DEATH_EXPLOSION_ELITE;
            }
            else
            {
                switch (deathType)
                {
                    case ENEMY_CHASER:   explosionType = DEATH_EXPLOSION_CHASER;   break;
                    case ENEMY_ORBITER:  explosionType = DEATH_EXPLOSION_ORBITER;  break;
                    case ENEMY_SPLITTER: explosionType = DEATH_EXPLOSION_SPLITTER; break;
                    default:             explosionType = DEATH_EXPLOSION_CHASER;   break;
                }
            }

            SpawnDeathExplosion(ctx->particles, deathPos, explosionType, deathRadius);
            TriggerImpactFrame(ctx->game, deathPos, deathRadius * 2.0f);

            // Spawn floor decal based on weapon type
            switch (p->weaponType)
            {
                case WEAPON_FLAMETHROWER:
                case WEAPON_INFERNO:
                    DecalSpawnBurn(ctx->decals, deathPos, deathRadius * 1.5f);
                    break;
                case WEAPON_FREEZE_RAY:
                case WEAPON_BLIZZARD:
                    DecalSpawnIce(ctx->decals, deathPos, deathRadius * 1.8f);
                    break;
                case WEAPON_LIGHTNING:
                case WEAPON_TESLA_COIL:
                    DecalSpawnLightning(ctx->decals, deathPos, deathRadius * 1.3f);
                    break;
                case WEAPON_BLACK_HOLE:
                case WEAPON_SINGULARITY:
                    DecalSpawnPlasma(ctx->decals, deathPos, deathRadius * 2.0f);
                    break;
                default:
                    // Generic scorch for other weapons
                    DecalSpawnScorch(ctx->decals, deathPos, deathRadius * 1.2f);
                    break;
            }
        }

        PlayGameSound(SOUND_EXPLOSION);
        TriggerScreenShake(ctx->game, 3.0f, 0.15f);
        EnemyDeactivate(ctx->enemies, index);
        ctx->game->score += (int)(enemy->xpValue * 10 * ctx->game->scoreMultiplier);
        ctx->game->killCount++;

        TryEarnAchievement(ctx->game, ACH_FIRST_BLOOD);

        if (ctx->game->killCount >= 100)
        {
            TryEarnAchievement(ctx->game, ACH_CENTURION);
        }

        if (enemy->isBoss)
        {
            ctx->game->bossKillsThisRun++;
            TryEarnAchievement(ctx->game, ACH_BOSS_HUNTER);
        }

        int hitstopAmount = (enemy->xpValue >= 3) ? 4 : 2;
        if (hitstopAmount > ctx->game->hitstopFrames)
        {
            ctx->game->hitstopFrames = hitstopAmount;
        }
    }

    return false;
}

static void CheckProjectileEnemyCollisions(ProjectilePool *projectiles, EnemyPool *enemies, EnemySpatialGrid *grid, XPPool *xp, ParticlePool *particles, DecalPool *decals, GameData *game, Player *player)
{
    for (int i = 0; i < projectiles->count; )
    {
        int index = projectiles->activeIndices[i];
        Projectile *p = &projectiles->projectiles[index];
        if (!p->active)
        {
            ProjectileDeactivate(projectiles, index);
            continue;
        }

        ProjectileCollisionContext ctx = { projectiles, enemies, xp, particles, decals, game, player, p, index };
        float searchRadius = p->radius + BOSS_BASE_RADIUS;
        EnemySpatialGridForEachInRadius(grid, enemies, p->pos, searchRadius, ProjectileEnemyCollisionVisit, &ctx);

        if (!p->active)
        {
            continue;
        }

        i++;
    }
}

typedef struct EnemyPlayerCollisionContext {
    EnemyPool *enemies;
    Player *player;
    ParticlePool *particles;
    GameData *game;
    XPPool *xp;
} EnemyPlayerCollisionContext;

static bool EnemyPlayerCollisionVisit(Enemy *enemy, int index, void *user)
{
    EnemyPlayerCollisionContext *ctx = (EnemyPlayerCollisionContext *)user;
    Player *player = ctx->player;

    if (!CheckCircleCollision(player->pos, player->radius, enemy->pos, enemy->radius))
    {
        return true;
    }

    if (player->isDashing && player->dashDamage > 0.0f)
    {
        enemy->health -= player->dashDamage;
        enemy->hitFlashTimer = 0.1f;
        SpawnHitParticles(ctx->particles, enemy->pos, NEON_PINK, 8);

        if (enemy->health <= 0.0f)
        {
            XPSpawn(ctx->xp, enemy->pos, enemy->xpValue);

            // Choose explosion type based on enemy properties
            DeathExplosionType explosionType;
            if (enemy->isBoss)
            {
                explosionType = DEATH_EXPLOSION_BOSS;
            }
            else if (enemy->isElite)
            {
                explosionType = DEATH_EXPLOSION_ELITE;
            }
            else
            {
                switch (enemy->type)
                {
                    case ENEMY_CHASER:   explosionType = DEATH_EXPLOSION_CHASER;   break;
                    case ENEMY_ORBITER:  explosionType = DEATH_EXPLOSION_ORBITER;  break;
                    case ENEMY_SPLITTER: explosionType = DEATH_EXPLOSION_SPLITTER; break;
                    default:             explosionType = DEATH_EXPLOSION_CHASER;   break;
                }
            }

            SpawnDeathExplosion(ctx->particles, enemy->pos, explosionType, enemy->radius);
            PlayGameSound(SOUND_EXPLOSION);
            TriggerScreenShake(ctx->game, 3.0f, 0.15f);
            EnemyDeactivate(ctx->enemies, index);
            ctx->game->score += (int)(enemy->xpValue * 10 * ctx->game->scoreMultiplier);
            ctx->game->killCount++;
        }

        return true;
    }

    if (player->invincibilityTimer > 0.0f) return true;

    PlayerTakeDamage(player, enemy->damage);
    PlayGameSound(SOUND_HIT);
    SpawnHitParticles(ctx->particles, player->pos, NEON_RED, 10);
    TriggerScreenShake(ctx->game, 8.0f, 0.25f);

    ctx->game->scoreMultiplier = 1.0f;
    ctx->game->timeSinceLastHit = 0.0f;

    float dx = player->pos.x - enemy->pos.x;
    float dy = player->pos.y - enemy->pos.y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist > 0.0f)
    {
        Vector2 pushDir = { dx / dist, dy / dist };
        float knockback = 30.0f * player->knockbackMultiplier;
        enemy->pos.x -= pushDir.x * knockback;
        enemy->pos.y -= pushDir.y * knockback;
    }

    return false;
}

static void CheckEnemyPlayerCollisions(EnemyPool *enemies, EnemySpatialGrid *grid, Player *player, ParticlePool *particles, GameData *game, XPPool *xp)
{
    if (!player->alive) return;

    EnemyPlayerCollisionContext ctx = { enemies, player, particles, game, xp };
    float searchRadius = player->radius + BOSS_BASE_RADIUS;
    EnemySpatialGridForEachInRadius(grid, enemies, player->pos, searchRadius, EnemyPlayerCollisionVisit, &ctx);
}

typedef struct SlowAuraContext {
    Player *player;
} SlowAuraContext;

static bool SlowAuraVisit(Enemy *enemy, int index, void *user)
{
    (void)index;
    SlowAuraContext *ctx = (SlowAuraContext *)user;
    EnemyApplySlow(enemy, ctx->player->slowAuraAmount, 0.5f);
    return true;
}

static bool CheckLevelUp(Player *player)
{
    if (player->xp >= player->xpToNextLevel)
    {
        player->level++;
        player->xpToNextLevel = GetXPForLevel(player->level);
        return true;
    }
    return false;
}

static Color GetRarityColor(UpgradeRarity rarity)
{
    switch (rarity)
    {
        case RARITY_COMMON:   return NEON_WHITE;
        case RARITY_UNCOMMON: return NEON_GREEN;
        case RARITY_RARE:     return NEON_YELLOW;
        default:              return NEON_WHITE;
    }
}

static Color GetRarityBorderColor(UpgradeRarity rarity)
{
    switch (rarity)
    {
        case RARITY_COMMON:   return NEON_PINK;
        case RARITY_UNCOMMON: return NEON_GREEN;
        case RARITY_RARE:     return NEON_YELLOW;
        default:              return NEON_PINK;
    }
}

static void DrawUpgradeOption(int index, Upgrade upgrade, float y)
{
    float boxWidth = 300.0f;
    float boxHeight = 80.0f;
    float boxX = SCREEN_WIDTH / 2.0f - boxWidth / 2.0f;

    Color boxColor = (Color){ 40, 20, 60, 230 };
    Color borderColor = GetRarityBorderColor(upgrade.rarity);
    Color nameColor = GetRarityColor(upgrade.rarity);

    DrawRectangle((int)boxX, (int)y, (int)boxWidth, (int)boxHeight, boxColor);
    DrawRectangleLinesEx((Rectangle){ boxX, y, boxWidth, boxHeight }, 2.0f, borderColor);

    char keyLabel[4];
    snprintf(keyLabel, sizeof(keyLabel), "[%d]", index + 1);
    DrawText(keyLabel, (int)(boxX + 15), (int)(y + 15), 24, NEON_CYAN);

    DrawText(upgrade.name, (int)(boxX + 60), (int)(y + 12), 22, nameColor);
    DrawText(upgrade.description, (int)(boxX + 60), (int)(y + 42), 16, NEON_GREEN);
}

static void UpdateSettingsMenu(GameData *game)
{
    // Navigation (up/down or W/S)
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
    {
        game->settingsSelection--;
        if (game->settingsSelection < 0) game->settingsSelection = 4;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
    {
        game->settingsSelection++;
        if (game->settingsSelection > 4) game->settingsSelection = 0;
    }

    // Adjust values (left/right or A/D)
    float volumeStep = 0.1f;
    bool leftPressed = IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A);
    bool rightPressed = IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D);

    switch (game->settingsSelection)
    {
        case 0: // Music Volume
            if (leftPressed && game->settings.musicVolume > 0.05f)
                game->settings.musicVolume -= volumeStep;
            else if (leftPressed)
                game->settings.musicVolume = 0.0f;
            if (rightPressed && game->settings.musicVolume < 0.95f)
                game->settings.musicVolume += volumeStep;
            else if (rightPressed)
                game->settings.musicVolume = 1.0f;
            SetGameMusicVolume(game->settings.musicVolume);
            break;

        case 1: // SFX Volume
            if (leftPressed && game->settings.sfxVolume > 0.05f)
                game->settings.sfxVolume -= volumeStep;
            else if (leftPressed)
                game->settings.sfxVolume = 0.0f;
            if (rightPressed && game->settings.sfxVolume < 0.95f)
                game->settings.sfxVolume += volumeStep;
            else if (rightPressed)
                game->settings.sfxVolume = 1.0f;
            SetGameSFXVolume(game->settings.sfxVolume);
            // Play test sound when adjusting
            if (leftPressed || rightPressed) PlayGameSound(SOUND_PICKUP);
            break;

        case 2: // Screen Shake
            if (leftPressed || rightPressed)
                game->settings.screenShakeEnabled = !game->settings.screenShakeEnabled;
            break;

        case 3: // CRT Filter
            if (leftPressed || rightPressed)
            {
                game->settings.crtEnabled = !game->settings.crtEnabled;
                game->crtEnabled = game->settings.crtEnabled;
            }
            break;

        case 4: // Chromatic Aberration
            if (leftPressed || rightPressed)
                game->settings.chromaticEnabled = !game->settings.chromaticEnabled;
            break;
    }

    // Exit settings
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER))
    {
        SaveSettings(&game->settings);
        game->state = STATE_MENU;
    }
}

static void DrawSettingsMenu(GameData *game)
{
    // Draw starfield background
    DrawMenuStars();

    // Title
    const char *title = "SETTINGS";
    DrawText(title, SCREEN_WIDTH/2 - MeasureText(title, 50)/2, 100, 50, NEON_CYAN);

    float startY = 180.0f;
    float itemHeight = 60.0f;
    float boxWidth = 500.0f;
    float boxX = SCREEN_WIDTH / 2.0f - boxWidth / 2.0f;

    const char *labels[] = { "Music Volume", "SFX Volume", "Screen Shake", "CRT Filter", "Chromatic FX" };

    for (int i = 0; i < 5; i++)
    {
        float y = startY + i * itemHeight;
        bool selected = (i == game->settingsSelection);

        // Box background
        Color boxColor = selected ? (Color){ 60, 30, 80, 230 } : (Color){ 40, 20, 60, 200 };
        Color borderColor = selected ? NEON_CYAN : NEON_PINK;
        DrawRectangle((int)boxX, (int)y, (int)boxWidth, 50, boxColor);
        DrawRectangleLinesEx((Rectangle){ boxX, y, boxWidth, 50 }, selected ? 3.0f : 2.0f, borderColor);

        // Label
        Color textColor = selected ? NEON_WHITE : GRAY;
        DrawText(labels[i], (int)(boxX + 20), (int)(y + 14), 22, textColor);

        // Value display - position from left side of box for consistency
        float sliderX = boxX + 220;
        if (i < 2) // Volume sliders
        {
            float volume = (i == 0) ? game->settings.musicVolume : game->settings.sfxVolume;
            int percent = (int)(volume * 100);

            // Slider background
            float sliderWidth = 180.0f;
            float sliderHeight = 12.0f;
            float sliderY = y + 19;
            DrawRectangle((int)sliderX, (int)sliderY, (int)sliderWidth, (int)sliderHeight, (Color){ 50, 50, 50, 255 });
            DrawRectangle((int)sliderX, (int)sliderY, (int)(sliderWidth * volume), (int)sliderHeight, selected ? NEON_GREEN : (Color){ 50, 200, 100, 200 });

            // Percentage text - right-aligned within box
            const char *percentText = TextFormat("%d%%", percent);
            int textWidth = MeasureText(percentText, 20);
            DrawText(percentText, (int)(boxX + boxWidth - textWidth - 20), (int)(y + 14), 20, selected ? NEON_GREEN : GRAY);
        }
        else // Toggle switches
        {
            bool enabled;
            if (i == 2) enabled = game->settings.screenShakeEnabled;
            else if (i == 3) enabled = game->settings.crtEnabled;
            else enabled = game->settings.chromaticEnabled;

            const char *state = enabled ? "ON" : "OFF";
            Color stateColor = enabled ? NEON_GREEN : NEON_RED;
            if (!selected) stateColor = enabled ? (Color){ 50, 200, 100, 200 } : (Color){ 200, 50, 50, 200 };
            // Right-aligned toggle text
            int textWidth = MeasureText(state, 22);
            DrawText(state, (int)(boxX + boxWidth - textWidth - 20), (int)(y + 14), 22, stateColor);
        }
    }

    // Instructions
    const char *navText = "W/S or Up/Down: Navigate";
    const char *adjustText = "A/D or Left/Right: Adjust";
    const char *exitText = "ESC or ENTER: Save and Exit";
    DrawText(navText, SCREEN_WIDTH/2 - MeasureText(navText, 16)/2, 510, 16, GRAY);
    DrawText(adjustText, SCREEN_WIDTH/2 - MeasureText(adjustText, 16)/2, 535, 16, GRAY);
    DrawText(exitText, SCREEN_WIDTH/2 - MeasureText(exitText, 16)/2, 560, 16, NEON_YELLOW);
}

static void InitCamera(GameData *game)
{
    game->camera.target = game->player.pos;
    game->camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    game->camera.rotation = 0.0f;
    game->camera.zoom = 1.0f;
    game->shakeIntensity = 0.0f;
    game->shakeDuration = 0.0f;
}

static void UpdateGameCamera(GameData *game, float dt)
{
    game->camera.target = Vector2Lerp(game->camera.target, game->player.pos, CAMERA_LERP_SPEED * dt);

    if (game->shakeDuration > 0.0f)
    {
        game->shakeDuration -= dt;
        float shakeFactor = game->shakeDuration > 0.0f ? game->shakeDuration / 0.25f : 0.0f;
        float offsetX = ((float)(rand() % 100) / 100.0f - 0.5f) * 2.0f * game->shakeIntensity * shakeFactor;
        float offsetY = ((float)(rand() % 100) / 100.0f - 0.5f) * 2.0f * game->shakeIntensity * shakeFactor;
        game->camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f + offsetX, SCREEN_HEIGHT / 2.0f + offsetY };
    }
    else
    {
        game->camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    }
}

void TriggerScreenShake(GameData *game, float intensity, float duration)
{
    // Check if screen shake is enabled in settings
    if (!game->settings.screenShakeEnabled) return;

    if (intensity > game->shakeIntensity)
    {
        game->shakeIntensity = intensity;
    }
    if (duration > game->shakeDuration)
    {
        game->shakeDuration = duration;
    }
}

void TriggerImpactFrame(GameData *game, Vector2 pos, float radius)
{
    // Trigger a bright flash at the given position for 2 frames
    game->impactPos = pos;
    game->impactFrames = 2;  // 2 frames = ~33ms at 60fps
    game->impactRadius = radius;
}

void GameInitShaders(GameData *game)
{
    // Create render textures for post-processing (ping-pong for shader chaining)
    game->renderTarget = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    game->renderTarget2 = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Initialize shader locations to invalid
    game->bloomIntensityLoc = -1;
    game->crtTimeLoc = -1;

    // Load bloom shader with validation
    game->bloomShader = LoadShader(0, "resources/shaders/bloom.fs");
    if (game->bloomShader.id == 0)
    {
        TraceLog(LOG_WARNING, "SHADER: Failed to load bloom.fs - post-processing disabled");
        game->shadersEnabled = false;
    }
    else
    {
        game->shadersEnabled = true;
        game->bloomIntensityLoc = GetShaderLocation(game->bloomShader, "intensity");

        // Set default bloom intensity
        float bloomIntensity = 0.5f;
        SetShaderValue(game->bloomShader, game->bloomIntensityLoc, &bloomIntensity, SHADER_UNIFORM_FLOAT);
    }

    // Load CRT shader with validation
    game->crtShader = LoadShader(0, "resources/shaders/crt.fs");
    if (game->crtShader.id == 0)
    {
        TraceLog(LOG_WARNING, "SHADER: Failed to load crt.fs - CRT effect disabled");
        game->crtEnabled = false;
    }
    else
    {
        game->crtEnabled = true;
        game->crtTimeLoc = GetShaderLocation(game->crtShader, "time");
    }

    // Load chromatic aberration shader with validation
    game->chromaticShader = LoadShader(0, "resources/shaders/chromatic.fs");
    game->chromaticIntensityLoc = -1;
    game->chromaticTimeLoc = -1;
    game->chromaticIntensity = 0.0f;
    if (game->chromaticShader.id == 0)
    {
        TraceLog(LOG_WARNING, "SHADER: Failed to load chromatic.fs - chromatic aberration disabled");
    }
    else
    {
        game->chromaticIntensityLoc = GetShaderLocation(game->chromaticShader, "intensity");
        game->chromaticTimeLoc = GetShaderLocation(game->chromaticShader, "time");
    }
}

void GameCleanupShaders(GameData *game)
{
    UnloadShader(game->bloomShader);
    UnloadShader(game->crtShader);
    UnloadShader(game->chromaticShader);
    UnloadRenderTexture(game->renderTarget);
    UnloadRenderTexture(game->renderTarget2);
}

static void DrawBackgroundGrid(Camera2D camera)
{
    // Calculate visible area based on camera
    float halfWidth = SCREEN_WIDTH / (2.0f * camera.zoom);
    float halfHeight = SCREEN_HEIGHT / (2.0f * camera.zoom);

    float minX = camera.target.x - halfWidth - GRID_SIZE;
    float maxX = camera.target.x + halfWidth + GRID_SIZE;
    float minY = camera.target.y - halfHeight - GRID_SIZE;
    float maxY = camera.target.y + halfHeight + GRID_SIZE;

    // Align to grid
    int startX = ((int)minX / GRID_SIZE) * GRID_SIZE;
    int endX = ((int)maxX / GRID_SIZE + 1) * GRID_SIZE;
    int startY = ((int)minY / GRID_SIZE) * GRID_SIZE;
    int endY = ((int)maxY / GRID_SIZE + 1) * GRID_SIZE;

    // Draw vertical lines
    for (int x = startX; x <= endX; x += GRID_SIZE)
    {
        DrawLine(x, startY, x, endY, GRID_COLOR);
    }

    // Draw horizontal lines
    for (int y = startY; y <= endY; y += GRID_SIZE)
    {
        DrawLine(startX, y, endX, y, GRID_COLOR);
    }
}

void GameInit(GameData *game)
{
    game->state = STATE_MENU;
    game->gameTime = 0.0f;
    game->score = 0;
    game->isPaused = false;
    game->spawnTimer = 0.0f;
    game->killCount = 0;
    game->leaderboardPosition = -1;

    // Load leaderboard
    LeaderboardLoad(&game->leaderboard);
    game->highScore = LeaderboardGetHighScore(&game->leaderboard);
    game->scoreMultiplier = 1.0f;
    game->timeSinceLastHit = 0.0f;
    game->hitstopFrames = 0;
    game->timeScale = 1.0f;
    game->tutorialTimer = 0.0f;
    game->impactPos = (Vector2){ 0.0f, 0.0f };
    game->impactFrames = 0;
    game->impactRadius = 0.0f;
    game->transitionTimer = 0.0f;
    game->fadeAlpha = 0.0f;
    game->settingsSelection = 0;
    game->bossSpawnTimer = BOSS_SPAWN_INTERVAL;
    game->bossCount = 0;
    game->bossWarningTimer = 0.0f;
    game->bossWarningActive = false;
    game->bossKillsThisRun = 0;
    game->selectedCharacter = CHARACTER_VANGUARD;
    game->characterSelection = 0;

    // Load persistent unlocks
    UnlocksLoad(&game->unlocks);

    // Load persistent achievements
    AchievementLoad(&game->achievements);
    game->pendingAchievement = (AchievementType)-1;
    game->achievementDisplayTimer = 0.0f;
    game->achievementSelection = 0;

    // Initialize co-op fields
    game->gameMode = GAME_MODE_SOLO;
    game->modeSelection = 0;
    game->selectedCharacterP2 = CHARACTER_VANGUARD;
    CoopStateInit(&game->coop, GAME_MODE_SOLO);

    // Load and apply settings
    LoadSettings(&game->settings);
    ApplySettings(game);

    PlayerInit(&game->player);
    ProjectilePoolInit(&game->projectiles);
    EnemyPoolInit(&game->enemies);
    XPPoolInit(&game->xp);
    ParticlePoolInit(&game->particles);
    DecalPoolInit(&game->decals);
    InitCamera(game);

    // Start intro music on menu
    IntroMusicStart();
}

void GameUpdate(GameData *game, float dt)
{
    switch (game->state)
    {
        case STATE_MENU:
            // Initialize and update starfield
            if (!menuStarsInit) InitMenuStars();
            UpdateMenuStars(dt);

            // Update intro music
            IntroMusicUpdate();

            // Restart intro music if it stopped (loop)
            if (!IsIntroMusicPlaying())
            {
                IntroMusicStart();
            }

            // Shader toggles
            if (IsKeyPressed(KEY_F1)) game->shadersEnabled = !game->shadersEnabled;
            if (IsKeyPressed(KEY_F2)) game->crtEnabled = !game->crtEnabled;

            if (IsKeyPressed(KEY_ENTER))
            {
                // Go to mode select (1 Player / 2 Players)
                game->modeSelection = 0;  // Default to 1 Player
                game->state = STATE_MODE_SELECT;
            }
            if (IsKeyPressed(KEY_TAB))
            {
                game->settingsSelection = 0;
                game->state = STATE_SETTINGS;
            }
            if (IsKeyPressed(KEY_L))
            {
                game->state = STATE_LEADERBOARD;
            }
            if (IsKeyPressed(KEY_A))
            {
                game->achievementSelection = 0;
                game->state = STATE_ACHIEVEMENTS;
            }
            if (IsKeyPressed(KEY_Q)) CloseWindow();
            break;

        case STATE_MODE_SELECT:
            // Initialize and update starfield
            if (!menuStarsInit) InitMenuStars();
            UpdateMenuStars(dt);

            // Keep intro music playing
            IntroMusicUpdate();
            if (!IsIntroMusicPlaying())
            {
                IntroMusicStart();
            }

            // Navigate with up/down or W/S
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
            {
                game->modeSelection = 0;  // 1 Player
            }
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
            {
                game->modeSelection = 1;  // 2 Players
            }

            if (IsKeyPressed(KEY_ENTER))
            {
                // Set game mode based on selection
                game->gameMode = (game->modeSelection == 0) ? GAME_MODE_SOLO : GAME_MODE_COOP;

                // Initialize co-op state
                CoopStateInit(&game->coop, game->gameMode);

                // Go to character select for P1
                game->characterSelection = game->selectedCharacter;
                game->state = STATE_CHARACTER_SELECT;
            }
            if (IsKeyPressed(KEY_ESCAPE))
            {
                game->state = STATE_MENU;
            }
            break;

        case STATE_LEADERBOARD:
            // Initialize and update starfield (shared with menu)
            if (!menuStarsInit) InitMenuStars();
            UpdateMenuStars(dt);

            // Keep intro music playing
            IntroMusicUpdate();
            if (!IsIntroMusicPlaying())
            {
                IntroMusicStart();
            }

            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_L))
            {
                game->state = STATE_MENU;
            }
            break;

        case STATE_ACHIEVEMENTS:
            // Initialize and update starfield (shared with menu)
            if (!menuStarsInit) InitMenuStars();
            UpdateMenuStars(dt);

            // Keep intro music playing
            IntroMusicUpdate();
            if (!IsIntroMusicPlaying())
            {
                IntroMusicStart();
            }

            // Scroll through achievements with up/down
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
            {
                game->achievementSelection--;
                if (game->achievementSelection < 0) game->achievementSelection = ACHIEVEMENT_COUNT - 1;
            }
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
            {
                game->achievementSelection++;
                if (game->achievementSelection >= ACHIEVEMENT_COUNT) game->achievementSelection = 0;
            }

            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_A))
            {
                game->state = STATE_MENU;
            }
            break;

        case STATE_CHARACTER_SELECT:
            // Initialize and update starfield (shared with menu)
            if (!menuStarsInit) InitMenuStars();
            UpdateMenuStars(dt);

            // Keep intro music playing
            IntroMusicUpdate();
            if (!IsIntroMusicPlaying())
            {
                IntroMusicStart();
            }

            // Navigate with left/right or A/D
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
            {
                game->characterSelection--;
                if (game->characterSelection < 0) game->characterSelection = CHARACTER_COUNT - 1;
            }
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
            {
                game->characterSelection++;
                if (game->characterSelection >= CHARACTER_COUNT) game->characterSelection = 0;
            }

            // Check if character is unlocked
            if (IsKeyPressed(KEY_ENTER))
            {
                CharacterType selected = (CharacterType)game->characterSelection;
                if (UnlocksHasCharacter(&game->unlocks, selected))
                {
                    game->selectedCharacter = selected;

                    if (game->gameMode == GAME_MODE_COOP)
                    {
                        // Go to P2 character select
                        game->characterSelection = CHARACTER_VANGUARD;  // Reset selection for P2
                        game->state = STATE_CHARACTER_SELECT_P2;
                    }
                    else
                    {
                        // Start "Get Ready" transition
                        game->state = STATE_STARTING;
                        game->transitionTimer = 0.0f;
                        game->fadeAlpha = 0.0f;
                    }
                }
            }
            if (IsKeyPressed(KEY_ESCAPE))
            {
                game->state = STATE_MODE_SELECT;
            }
            break;

        case STATE_CHARACTER_SELECT_P2:
            // Initialize and update starfield
            if (!menuStarsInit) InitMenuStars();
            UpdateMenuStars(dt);

            // Keep intro music playing
            IntroMusicUpdate();
            if (!IsIntroMusicPlaying())
            {
                IntroMusicStart();
            }

            // Navigate with left/right or A/D
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
            {
                game->characterSelection--;
                if (game->characterSelection < 0) game->characterSelection = CHARACTER_COUNT - 1;
            }
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
            {
                game->characterSelection++;
                if (game->characterSelection >= CHARACTER_COUNT) game->characterSelection = 0;
            }

            // P2 can also use arrow keys
            if (IsKeyPressed(KEY_J))
            {
                game->characterSelection--;
                if (game->characterSelection < 0) game->characterSelection = CHARACTER_COUNT - 1;
            }
            if (IsKeyPressed(KEY_L))
            {
                game->characterSelection++;
                if (game->characterSelection >= CHARACTER_COUNT) game->characterSelection = 0;
            }

            // Check if character is unlocked
            if (IsKeyPressed(KEY_ENTER))
            {
                CharacterType selected = (CharacterType)game->characterSelection;
                if (UnlocksHasCharacter(&game->unlocks, selected))
                {
                    game->selectedCharacterP2 = selected;
                    // Start "Get Ready" transition
                    game->state = STATE_STARTING;
                    game->transitionTimer = 0.0f;
                    game->fadeAlpha = 0.0f;
                }
            }
            if (IsKeyPressed(KEY_ESCAPE))
            {
                // Go back to P1 character select
                game->characterSelection = game->selectedCharacter;
                game->state = STATE_CHARACTER_SELECT;
            }
            break;

        case STATE_SETTINGS:
            // Initialize starfield if needed (shares with menu)
            if (!menuStarsInit) InitMenuStars();
            UpdateMenuStars(dt);

            // Keep intro music playing
            IntroMusicUpdate();
            if (!IsIntroMusicPlaying())
            {
                IntroMusicStart();
            }

            UpdateSettingsMenu(game);
            break;

        case STATE_STARTING:
        {
            // Transition timing:
            // 0.0 - 0.5s: Fade to black (intro music still playing)
            // 0.5 - 2.0s: Show "Get Ready" on black (intro music fades, game music starts at 2.0)
            // 2.0 - 2.5s: Fade from black to game (game music playing)
            // 2.5s+: Start playing
            float fadeInEnd = 0.5f;
            float holdEnd = 2.0f;
            float fadeOutEnd = 2.5f;

            float prevTimer = game->transitionTimer;
            game->transitionTimer += dt;

            if (game->transitionTimer < fadeInEnd)
            {
                // Fade to black
                game->fadeAlpha = game->transitionTimer / fadeInEnd;
            }
            else if (game->transitionTimer < holdEnd)
            {
                // Hold on black with "Get Ready"
                game->fadeAlpha = 1.0f;
            }
            else if (game->transitionTimer < fadeOutEnd)
            {
                // Start game music when entering fade-out phase
                if (prevTimer < holdEnd)
                {
                    IntroMusicStop();
                    MusicStart();
                }

                // Fade from black to game
                float progress = (game->transitionTimer - holdEnd) / (fadeOutEnd - holdEnd);
                game->fadeAlpha = 1.0f - progress;
            }
            else
            {
                // Transition complete - start playing
                game->fadeAlpha = 0.0f;
                game->state = STATE_PLAYING;
                game->gameTime = 0.0f;
                game->score = 0;
                game->killCount = 0;
                game->spawnTimer = 0.0f;
                game->scoreMultiplier = 1.0f;
                game->timeSinceLastHit = 0.0f;
                game->hitstopFrames = 0;
                game->timeScale = 1.0f;
                game->tutorialTimer = 0.0f;
                game->bossSpawnTimer = BOSS_SPAWN_INTERVAL;
                game->bossCount = 0;
                game->bossWarningTimer = 0.0f;
                game->bossWarningActive = false;
                game->bossKillsThisRun = 0;
                game->leaderboardPosition = -1;
                PlayerInitWithCharacter(&game->player, game->selectedCharacter);

                // Apply meta bonuses from permanent unlocks
                game->player.speed *= UnlocksGetSpeedBonus(&game->unlocks);
                game->player.maxHealth += UnlocksGetHealthBonus(&game->unlocks);
                game->player.health = game->player.maxHealth;
                game->player.weapon.damage *= UnlocksGetDamageBonus(&game->unlocks);
                game->player.xpMultiplier *= UnlocksGetXPBonus(&game->unlocks);
                game->player.magnetRadius *= UnlocksGetMagnetBonus(&game->unlocks);

                ProjectilePoolInit(&game->projectiles);
                EnemyPoolInit(&game->enemies);
                XPPoolInit(&game->xp);
                ParticlePoolInit(&game->particles);
                DecalPoolInit(&game->decals);
                InitCamera(game);

                // Initialize co-op players if in co-op mode
                if (game->gameMode == GAME_MODE_COOP)
                {
                    CoopInitPlayers(&game->coop, game->selectedCharacter, game->selectedCharacterP2);
                    CoopInitCameras(&game->coop);

                    // Apply meta bonuses to co-op players
                    for (int i = 0; i < game->coop.playerCount; i++)
                    {
                        Player *p = CoopGetPlayer(&game->coop, i);
                        if (p)
                        {
                            p->speed *= UnlocksGetSpeedBonus(&game->unlocks);
                            p->maxHealth += UnlocksGetHealthBonus(&game->unlocks);
                            p->health = p->maxHealth;
                            p->weapon.damage *= UnlocksGetDamageBonus(&game->unlocks);
                            p->xpMultiplier *= UnlocksGetXPBonus(&game->unlocks);
                            p->magnetRadius *= UnlocksGetMagnetBonus(&game->unlocks);
                        }
                    }
                }
            }
            break;
        }

        case STATE_PLAYING:
        {
            EnemySpatialGrid enemyGrid;
            // Update impact frames (decrement each frame)
            if (game->impactFrames > 0)
            {
                game->impactFrames--;
            }

            // Hitstop: freeze game for a few frames on big kills
            if (game->hitstopFrames > 0)
            {
                game->hitstopFrames--;
                break;  // Skip all updates during hitstop
            }

            // Apply time scale for slow-mo effects
            float scaledDt = dt * game->timeScale;
            game->timeScale = 1.0f;  // Reset to normal (level-up can override)

            // Chromatic aberration: intensity increases as health decreases below 50%
            float healthPercent;
            if (game->gameMode == GAME_MODE_COOP)
            {
                // Use lowest health player for chromatic effect
                float minHealth = 1.0f;
                for (int i = 0; i < game->coop.playerCount; i++)
                {
                    Player *p = CoopGetPlayer(&game->coop, i);
                    if (p && CoopIsPlayerAlive(&game->coop, i))
                    {
                        float hp = p->health / p->maxHealth;
                        if (hp < minHealth) minHealth = hp;
                    }
                }
                healthPercent = minHealth;
            }
            else
            {
                healthPercent = game->player.health / game->player.maxHealth;
            }

            if (game->settings.chromaticEnabled && healthPercent < 0.5f && healthPercent > 0.0f)
            {
                // Scale from 0 at 50% health to 1.0 at 0% health
                game->chromaticIntensity = 1.0f - (healthPercent / 0.5f);
            }
            else
            {
                game->chromaticIntensity = 0.0f;
            }

            game->gameTime += scaledDt;
            game->tutorialTimer += dt;  // Tutorial uses real time

            // Update achievement popup timer
            if (game->achievementDisplayTimer > 0.0f)
            {
                game->achievementDisplayTimer -= dt;
            }

            // Achievement: Survivor (3 minutes)
            if (game->gameTime >= 180.0f)
            {
                TryEarnAchievement(game, ACH_SURVIVOR);
            }

            // Achievement: Veteran (10 minutes)
            if (game->gameTime >= 600.0f)
            {
                TryEarnAchievement(game, ACH_VETERAN);
            }

            // Update score multiplier (increases slowly while not hit)
            game->timeSinceLastHit += scaledDt;
            game->scoreMultiplier = 1.0f + (game->timeSinceLastHit / MULTIPLIER_GROWTH_RATE);
            if (game->scoreMultiplier > MULTIPLIER_MAX) game->scoreMultiplier = MULTIPLIER_MAX;

            // Achievement: Immortal (no damage for 1 minute)
            if (game->timeSinceLastHit >= 60.0f)
            {
                TryEarnAchievement(game, ACH_IMMORTAL);
            }

            EnemySpatialGridBuild(&enemyGrid, &game->enemies);

            // Get target position for enemies (nearest alive player in co-op)
            Vector2 primaryPlayerPos;
            if (game->gameMode == GAME_MODE_COOP)
            {
                // Update all co-op players
                CoopUpdateInput(&game->coop, scaledDt, &game->projectiles);
                CoopUpdateCameras(&game->coop, scaledDt);
                CoopUpdateRevive(&game->coop, scaledDt);

                // Use first alive player as primary target for enemy pathfinding
                primaryPlayerPos = CoopGetNearestPlayerPos(&game->coop, (Vector2){ SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f });
            }
            else
            {
                PlayerUpdate(&game->player, scaledDt, &game->projectiles, game->camera);
                primaryPlayerPos = game->player.pos;
                UpdateGameCamera(game, scaledDt);
            }

            // Pass enemy pool so each homing projectile can find its own target
            ProjectilePoolUpdate(&game->projectiles, scaledDt, &game->enemies, &enemyGrid);
            ParticlePoolUpdate(&game->particles, scaledDt);
            DecalPoolUpdate(&game->decals, scaledDt);

            // Update enemies - they target primaryPlayerPos (nearest alive player in co-op)
            EnemyPoolUpdate(&game->enemies, primaryPlayerPos, scaledDt);

            // XP magnet behavior - in co-op, each player attracts XP
            if (game->gameMode == GAME_MODE_COOP)
            {
                for (int i = 0; i < game->coop.playerCount; i++)
                {
                    Player *p = CoopGetPlayer(&game->coop, i);
                    if (p && CoopIsPlayerAlive(&game->coop, i))
                    {
                        XPPoolUpdate(&game->xp, p->pos, p->magnetRadius, scaledDt);
                    }
                }
            }
            else
            {
                XPPoolUpdate(&game->xp, game->player.pos, game->player.magnetRadius, scaledDt);
            }

            game->spawnTimer += dt;
            float spawnInterval = GetSpawnInterval(game->gameTime);

            // In co-op, spawn more enemies
            if (game->gameMode == GAME_MODE_COOP)
            {
                spawnInterval /= CoopGetSpawnMultiplier(&game->coop);
            }

            if (game->spawnTimer >= spawnInterval)
            {
                Vector2 spawnPos = GetSpawnPosition(primaryPlayerPos);
                EnemyType enemyType = (EnemyType)GetEnemyTypeForTime(game->gameTime);

                // Elite spawn chance increases over time (10% base + 1% per minute, max 25%)
                float eliteChance = ELITE_SPAWN_CHANCE + (game->gameTime / 60.0f) * 0.01f;
                if (eliteChance > 0.25f) eliteChance = 0.25f;

                float roll = (float)(rand() % 1000) / 1000.0f;
                if (roll < eliteChance)
                {
                    EnemySpawnElite(&game->enemies, enemyType, spawnPos);
                }
                else
                {
                    EnemySpawn(&game->enemies, enemyType, spawnPos);
                }
                game->spawnTimer = 0.0f;
            }

            // Boss spawning logic (every 5 minutes)
            if (!EnemyPoolHasBoss(&game->enemies))
            {
                // Decrease boss spawn timer
                game->bossSpawnTimer -= dt;

                // Start warning 5 seconds before boss spawns
                if (game->bossSpawnTimer <= 5.0f && !game->bossWarningActive)
                {
                    game->bossWarningActive = true;
                    game->bossWarningTimer = 5.0f;
                }

                // Update warning timer
                if (game->bossWarningActive)
                {
                    game->bossWarningTimer -= dt;
                }

                // Spawn boss when timer reaches zero
                if (game->bossSpawnTimer <= 0.0f)
                {
                    game->bossCount++;
                    Vector2 spawnPos = GetSpawnPosition(primaryPlayerPos);

                    // Apply co-op boss health scaling
                    if (game->gameMode == GAME_MODE_COOP)
                    {
                        EnemySpawnBoss(&game->enemies, spawnPos, game->bossCount);
                        // Scale boss health for co-op
                        Enemy *boss = EnemyPoolGetBoss(&game->enemies);
                        if (boss)
                        {
                            boss->health *= CoopGetBossHealthMultiplier(&game->coop);
                            boss->maxHealth = boss->health;
                        }
                    }
                    else
                    {
                        EnemySpawnBoss(&game->enemies, spawnPos, game->bossCount);
                    }
                    game->bossSpawnTimer = BOSS_SPAWN_INTERVAL;  // Reset for next boss
                    game->bossWarningActive = false;
                    // Screen shake when boss spawns
                    TriggerScreenShake(game, 15.0f, 0.8f);
                }
            }

            EnemySpatialGridBuild(&enemyGrid, &game->enemies);

            // Apply slow aura around players if upgraded
            if (game->gameMode == GAME_MODE_COOP)
            {
                for (int i = 0; i < game->coop.playerCount; i++)
                {
                    Player *p = CoopGetPlayer(&game->coop, i);
                    if (p && CoopIsPlayerAlive(&game->coop, i) && p->slowAuraRadius > 0.0f)
                    {
                        SlowAuraContext ctx = { p };
                        EnemySpatialGridForEachInRadius(
                            &enemyGrid,
                            &game->enemies,
                            p->pos,
                            p->slowAuraRadius,
                            SlowAuraVisit,
                            &ctx
                        );
                    }
                }
            }
            else if (game->player.slowAuraRadius > 0.0f)
            {
                SlowAuraContext ctx = { &game->player };
                EnemySpatialGridForEachInRadius(
                    &enemyGrid,
                    &game->enemies,
                    game->player.pos,
                    game->player.slowAuraRadius,
                    SlowAuraVisit,
                    &ctx
                );
            }

            // Apply black hole pull effect before collision checks
            ApplyBlackHolePull(&game->projectiles, &game->enemies, &enemyGrid, scaledDt);

            // Check collisions for all players
            if (game->gameMode == GAME_MODE_COOP)
            {
                for (int i = 0; i < game->coop.playerCount; i++)
                {
                    Player *p = CoopGetPlayer(&game->coop, i);
                    if (p && CoopIsPlayerAlive(&game->coop, i))
                    {
                        CheckProjectileEnemyCollisions(&game->projectiles, &game->enemies, &enemyGrid, &game->xp, &game->particles, &game->decals, game, p);
                        CheckEnemyPlayerCollisions(&game->enemies, &enemyGrid, p, &game->particles, game, &game->xp);
                    }
                }
            }
            else
            {
                CheckProjectileEnemyCollisions(&game->projectiles, &game->enemies, &enemyGrid, &game->xp, &game->particles, &game->decals, game, &game->player);
                CheckEnemyPlayerCollisions(&game->enemies, &enemyGrid, &game->player, &game->particles, game, &game->xp);
            }

            // XP collection for all players
            int collectedXP = 0;
            if (game->gameMode == GAME_MODE_COOP)
            {
                for (int i = 0; i < game->coop.playerCount; i++)
                {
                    Player *p = CoopGetPlayer(&game->coop, i);
                    if (p && CoopIsPlayerAlive(&game->coop, i))
                    {
                        int xp = XPCollect(&game->xp, p->pos, XP_COLLECT_RADIUS);
                        if (xp > 0)
                        {
                            CoopAddXP(&game->coop, xp);
                            PlayGameSound(SOUND_PICKUP);
                        }
                    }
                }
            }
            else
            {
                collectedXP = XPCollect(&game->xp, game->player.pos, XP_COLLECT_RADIUS);
            }

            if (collectedXP > 0)
            {
                // Apply XP multiplier from upgrades (solo mode)
                int boostedXP = (int)(collectedXP * game->player.xpMultiplier);
                game->player.xp += boostedXP;
                PlayGameSound(SOUND_PICKUP);
            }

            // Level up check
            bool leveledUp = false;
            int currentLevel = 0;

            if (game->gameMode == GAME_MODE_COOP)
            {
                if (CoopCheckLevelUp(&game->coop))
                {
                    leveledUp = true;
                    currentLevel = game->coop.sharedLevel;
                }
            }
            else
            {
                if (CheckLevelUp(&game->player))
                {
                    leveledUp = true;
                    currentLevel = game->player.level;
                }
            }

            if (leveledUp)
            {
                PlayGameSound(SOUND_LEVELUP);
                MusicPause();
                GenerateRandomUpgrades(game->upgradeOptions, 3);
                game->timeScale = 0.3f;  // Slow-mo during level up

                // Achievement: Level 5
                if (currentLevel >= 5)
                {
                    TryEarnAchievement(game, ACH_LEVEL_5);
                }

                // Achievement: Level 10
                if (currentLevel >= 10)
                {
                    TryEarnAchievement(game, ACH_LEVEL_10);
                }

                game->state = STATE_LEVELUP;
            }

            // Game over check
            bool gameOver = false;
            int finalLevel = 0;

            if (game->gameMode == GAME_MODE_COOP)
            {
                // Check for total party kill in co-op
                if (CoopCheckTotalPartyKill(&game->coop, dt))
                {
                    gameOver = true;
                    finalLevel = game->coop.sharedLevel;
                }
            }
            else
            {
                if (!game->player.alive)
                {
                    gameOver = true;
                    finalLevel = game->player.level;
                }
            }

            if (gameOver)
            {
                MusicStop();

                // Add to leaderboard
                game->leaderboardPosition = LeaderboardAddEntry(
                    &game->leaderboard,
                    game->score,
                    finalLevel,
                    game->killCount,
                    game->gameTime
                );
                LeaderboardSave(&game->leaderboard);
                game->highScore = LeaderboardGetHighScore(&game->leaderboard);

                // Save run stats to unlocks
                UnlocksAddRunStats(&game->unlocks, game->killCount, game->bossKillsThisRun,
                                   game->score, finalLevel, game->gameTime);
                UnlocksCheckNewUnlocks(&game->unlocks);
                UnlocksSave(&game->unlocks);

                // Update achievement stats
                game->achievements.totalKills += game->killCount;
                game->achievements.totalBossKills += game->bossKillsThisRun;
                if (game->gameTime > game->achievements.longestSurvival)
                {
                    game->achievements.longestSurvival = game->gameTime;
                }
                if (finalLevel > game->achievements.highestLevel)
                {
                    game->achievements.highestLevel = finalLevel;
                }

                // Achievement: Slayer (1000 total kills)
                if (game->achievements.totalKills >= 1000)
                {
                    TryEarnAchievement(game, ACH_SLAYER);
                }

                // Achievement: Boss Slayer (5 total boss kills)
                if (game->achievements.totalBossKills >= 5)
                {
                    TryEarnAchievement(game, ACH_BOSS_SLAYER);
                }

                // Achievement: Completionist (all characters unlocked)
                bool allCharsUnlocked = true;
                for (int i = 0; i < CHARACTER_COUNT; i++)
                {
                    if (!UnlocksHasCharacter(&game->unlocks, (CharacterType)i))
                    {
                        allCharsUnlocked = false;
                        break;
                    }
                }
                if (allCharsUnlocked)
                {
                    TryEarnAchievement(game, ACH_COMPLETIONIST);
                }

                AchievementSave(&game->achievements);
                game->state = STATE_GAMEOVER;
            }

            // Weapon switching
            if (game->gameMode == GAME_MODE_COOP)
            {
                // P1: Q/E keys
                Player *p1 = CoopGetPlayer(&game->coop, 0);
                if (p1)
                {
                    if (IsKeyPressed(KEY_Q))
                    {
                        PlayerCycleWeapon(p1, -1);
                    }
                    if (IsKeyPressed(KEY_E))
                    {
                        PlayerCycleWeapon(p1, 1);
                    }
                }

                // P2: comma/period keys
                Player *p2 = CoopGetPlayer(&game->coop, 1);
                if (p2)
                {
                    if (IsKeyPressed(KEY_COMMA))
                    {
                        PlayerCycleWeapon(p2, -1);
                    }
                    if (IsKeyPressed(KEY_PERIOD))
                    {
                        PlayerCycleWeapon(p2, 1);
                    }
                }
            }
            else
            {
                // Solo: Q/E keys
                if (IsKeyPressed(KEY_Q))
                {
                    PlayerCycleWeapon(&game->player, -1);
                }
                if (IsKeyPressed(KEY_E))
                {
                    PlayerCycleWeapon(&game->player, 1);
                }
            }

            if (IsKeyPressed(KEY_ESCAPE))
            {
                MusicPause();
                game->state = STATE_PAUSED;
            }
            break;
        }

        case STATE_PAUSED:
            if (IsKeyPressed(KEY_ESCAPE))
            {
                MusicResume();
                game->state = STATE_PLAYING;
            }
            if (IsKeyPressed(KEY_Q))
            {
                MusicStop();
                IntroMusicStart();  // Restart intro music on menu
                game->state = STATE_MENU;
            }
            break;

        case STATE_LEVELUP:
            if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1))
            {
                ApplyUpgrade(game->upgradeOptions[0], &game->player);
                // Check for weapon evolution after upgrade
                if (PlayerCanEvolveWeapon(&game->player))
                {
                    PlayerEvolveWeapon(&game->player);
                    PlayGameSound(SOUND_LEVELUP);  // Extra fanfare for evolution
                    TryEarnAchievement(game, ACH_FULLY_EVOLVED);
                }
                MusicResume();
                game->timeScale = 1.0f;  // Restore normal speed
                game->state = STATE_PLAYING;
            }
            else if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2))
            {
                ApplyUpgrade(game->upgradeOptions[1], &game->player);
                if (PlayerCanEvolveWeapon(&game->player))
                {
                    PlayerEvolveWeapon(&game->player);
                    PlayGameSound(SOUND_LEVELUP);
                    TryEarnAchievement(game, ACH_FULLY_EVOLVED);
                }
                MusicResume();
                game->timeScale = 1.0f;  // Restore normal speed
                game->state = STATE_PLAYING;
            }
            else if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3))
            {
                ApplyUpgrade(game->upgradeOptions[2], &game->player);
                if (PlayerCanEvolveWeapon(&game->player))
                {
                    PlayerEvolveWeapon(&game->player);
                    PlayGameSound(SOUND_LEVELUP);
                    TryEarnAchievement(game, ACH_FULLY_EVOLVED);
                }
                MusicResume();
                game->timeScale = 1.0f;  // Restore normal speed
                game->state = STATE_PLAYING;
            }
            break;

        case STATE_GAMEOVER:
            if (IsKeyPressed(KEY_ENTER))
            {
                IntroMusicStart();  // Restart intro music on menu
                game->state = STATE_MENU;
            }
            if (IsKeyPressed(KEY_L))
            {
                IntroMusicStart();
                game->state = STATE_LEADERBOARD;
            }
            break;
    }
}

// Draw the game world for a specific camera and player (for split-screen)
static void DrawGameWorldForCamera(GameData *game, Camera2D camera, int viewportWidth, int viewportHeight)
{
    float halfWidth = viewportWidth / (2.0f * camera.zoom);
    float halfHeight = viewportHeight / (2.0f * camera.zoom);
    Rectangle view = {
        camera.target.x - halfWidth - WORLD_VIEW_MARGIN,
        camera.target.y - halfHeight - WORLD_VIEW_MARGIN,
        halfWidth * 2.0f + WORLD_VIEW_MARGIN * 2.0f,
        halfHeight * 2.0f + WORLD_VIEW_MARGIN * 2.0f
    };

    DrawBackgroundGrid(camera);
    ParticlePoolDraw(&game->particles, view);

    // Draw impact frame flash
    if (game->impactFrames > 0)
    {
        float intensity = (float)game->impactFrames / 2.0f;
        float outerRadius = game->impactRadius * (2.0f - intensity);
        float innerRadius = game->impactRadius * 0.5f * (2.0f - intensity);
        unsigned char alpha = (unsigned char)(255 * intensity);
        DrawCircleV(game->impactPos, outerRadius, (Color){ 255, 255, 255, (unsigned char)(alpha * 0.3f) });
        DrawCircleV(game->impactPos, innerRadius, (Color){ 255, 255, 200, (unsigned char)(alpha * 0.6f) });
        DrawCircleLinesV(game->impactPos, outerRadius * 0.8f, (Color){ 255, 200, 100, alpha });
    }

    // Draw floor decals (behind all entities)
    DecalPoolDraw(&game->decals, view);

    XPPoolDraw(&game->xp, view);
    EnemyPoolDraw(&game->enemies, view);
    ProjectilePoolDraw(&game->projectiles, view);

    // Draw both players in co-op
    if (game->gameMode == GAME_MODE_COOP)
    {
        for (int i = 0; i < game->coop.playerCount; i++)
        {
            Player *p = CoopGetPlayer(&game->coop, i);
            if (p)
            {
                // Draw ghost if dead
                if (game->coop.players[i].revive.needsRevive)
                {
                    Vector2 ghostPos = game->coop.players[i].revive.deathPos;
                    float pulse = 0.5f + 0.3f * sinf((float)GetTime() * 5.0f);

                    // Ghost circle (semi-transparent, pulsing)
                    DrawCircleV(ghostPos, p->radius * 1.2f, (Color){ 100, 100, 150, (unsigned char)(100 * pulse) });
                    DrawCircleLinesV(ghostPos, p->radius * 1.5f, (Color){ 150, 150, 200, (unsigned char)(150 * pulse) });

                    // "REVIVE ME" text
                    const char *reviveText = "REVIVE ME";
                    int textWidth = MeasureText(reviveText, 14);
                    DrawText(reviveText, (int)(ghostPos.x - textWidth/2), (int)(ghostPos.y - p->radius - 30), 14, NEON_PINK);

                    // Revive progress bar
                    if (game->coop.players[i].revive.reviveProgress > 0.0f)
                    {
                        float progress = game->coop.players[i].revive.reviveProgress / REVIVE_TIME;
                        int barWidth = 50;
                        int barHeight = 6;
                        int barX = (int)(ghostPos.x - barWidth/2);
                        int barY = (int)(ghostPos.y - p->radius - 45);
                        DrawRectangle(barX, barY, barWidth, barHeight, (Color){ 50, 50, 50, 200 });
                        DrawRectangle(barX, barY, (int)(barWidth * progress), barHeight, NEON_GREEN);
                        DrawRectangleLinesEx((Rectangle){ (float)barX, (float)barY, (float)barWidth, (float)barHeight }, 1.0f, NEON_WHITE);
                    }
                }
                else
                {
                    PlayerDraw(p);
                }
            }
        }
    }
    else
    {
        PlayerDraw(&game->player);
    }
}

// Draw partner arrow indicator when partner is off-screen
static void DrawPartnerArrow(GameData *game, int viewerIndex, int partnerIndex, int viewportWidth, int viewportHeight)
{
    if (!game->coop.players[partnerIndex].player.alive && !game->coop.players[partnerIndex].revive.needsRevive) return;

    Camera2D cam = game->coop.cameras[viewerIndex].cam;
    Vector2 partnerPos;

    if (game->coop.players[partnerIndex].revive.needsRevive)
    {
        partnerPos = game->coop.players[partnerIndex].revive.deathPos;
    }
    else
    {
        partnerPos = game->coop.players[partnerIndex].player.pos;
    }

    // Transform partner position to screen space
    Vector2 screenPos = GetWorldToScreen2D(partnerPos, cam);

    // Check if off-screen (with margin)
    float margin = 50.0f;
    bool offScreen = screenPos.x < margin || screenPos.x > viewportWidth - margin ||
                     screenPos.y < margin || screenPos.y > viewportHeight - margin;

    if (!offScreen) return;

    // Calculate arrow position at screen edge
    Vector2 center = { viewportWidth / 2.0f, viewportHeight / 2.0f };
    Vector2 dir = Vector2Normalize(Vector2Subtract(screenPos, center));

    // Clamp to screen edges
    float arrowDist = fminf(viewportWidth / 2.0f - 40.0f, viewportHeight / 2.0f - 40.0f);
    Vector2 arrowPos = Vector2Add(center, Vector2Scale(dir, arrowDist));

    // Clamp to actual bounds
    arrowPos.x = fmaxf(30.0f, fminf(viewportWidth - 30.0f, arrowPos.x));
    arrowPos.y = fmaxf(30.0f, fminf(viewportHeight - 30.0f, arrowPos.y));

    // Draw arrow
    Color arrowColor = (partnerIndex == 0) ? NEON_CYAN : NEON_PINK;
    if (game->coop.players[partnerIndex].revive.needsRevive)
    {
        arrowColor = NEON_RED;  // Red for dead partner
    }

    // Draw arrow triangle pointing toward partner
    float angle = atan2f(dir.y, dir.x);
    float arrowSize = 15.0f;
    Vector2 tip = Vector2Add(arrowPos, Vector2Scale(dir, arrowSize));
    Vector2 left = Vector2Add(arrowPos, (Vector2){ cosf(angle + 2.5f) * arrowSize * 0.7f, sinf(angle + 2.5f) * arrowSize * 0.7f });
    Vector2 right = Vector2Add(arrowPos, (Vector2){ cosf(angle - 2.5f) * arrowSize * 0.7f, sinf(angle - 2.5f) * arrowSize * 0.7f });
    DrawTriangle(tip, left, right, arrowColor);

    // Draw distance
    float dist = Vector2Distance(game->coop.players[viewerIndex].player.pos, partnerPos);
    int meters = (int)(dist / 100.0f);
    const char *distText = TextFormat("%dm", meters);
    DrawText(distText, (int)(arrowPos.x - MeasureText(distText, 12)/2), (int)(arrowPos.y + 15), 12, arrowColor);
}

static void DrawGameWorld(GameData *game)
{
    float halfWidth = SCREEN_WIDTH / (2.0f * game->camera.zoom);
    float halfHeight = SCREEN_HEIGHT / (2.0f * game->camera.zoom);
    Rectangle view = {
        game->camera.target.x - halfWidth - WORLD_VIEW_MARGIN,
        game->camera.target.y - halfHeight - WORLD_VIEW_MARGIN,
        halfWidth * 2.0f + WORLD_VIEW_MARGIN * 2.0f,
        halfHeight * 2.0f + WORLD_VIEW_MARGIN * 2.0f
    };

    DrawBackgroundGrid(game->camera);
    ParticlePoolDraw(&game->particles, view);

    // Draw impact frame flash (bright burst on explosions)
    if (game->impactFrames > 0)
    {
        // Bright expanding ring effect
        float intensity = (float)game->impactFrames / 2.0f;  // 1.0 on first frame, 0.5 on second
        float outerRadius = game->impactRadius * (2.0f - intensity);
        float innerRadius = game->impactRadius * 0.5f * (2.0f - intensity);

        // Draw bright flash rings (additive feel)
        unsigned char alpha = (unsigned char)(255 * intensity);
        DrawCircleV(game->impactPos, outerRadius, (Color){ 255, 255, 255, (unsigned char)(alpha * 0.3f) });
        DrawCircleV(game->impactPos, innerRadius, (Color){ 255, 255, 200, (unsigned char)(alpha * 0.6f) });
        DrawCircleLinesV(game->impactPos, outerRadius * 0.8f, (Color){ 255, 200, 100, alpha });
    }

    // Draw floor decals (behind all entities)
    DecalPoolDraw(&game->decals, view);

    XPPoolDraw(&game->xp, view);
    EnemyPoolDraw(&game->enemies, view);
    ProjectilePoolDraw(&game->projectiles, view);
    PlayerDraw(&game->player);
}

static void DrawSceneToTexture(GameData *game)
{
    BeginTextureMode(game->renderTarget);
        ClearBackground(VOID_BLACK);

        switch (game->state)
        {
            case STATE_MENU:
            {
                DrawMenuStars();
                DrawText("NEON VOID", SCREEN_WIDTH/2 - MeasureText("NEON VOID", 60)/2, 180, 60, NEON_CYAN);
                DrawText(TextFormat("High Score: %d", game->highScore), SCREEN_WIDTH/2 - MeasureText(TextFormat("High Score: %d", game->highScore), 24)/2, 260, 24, NEON_YELLOW);
                DrawText("Press ENTER to Start", SCREEN_WIDTH/2 - MeasureText("Press ENTER to Start", 20)/2, 320, 20, NEON_PINK);
                DrawText("Press L for Leaderboard", SCREEN_WIDTH/2 - MeasureText("Press L for Leaderboard", 20)/2, 355, 20, NEON_YELLOW);
                DrawText("Press A for Achievements", SCREEN_WIDTH/2 - MeasureText("Press A for Achievements", 20)/2, 390, 20, NEON_GREEN);
                DrawText("Press TAB for Settings", SCREEN_WIDTH/2 - MeasureText("Press TAB for Settings", 20)/2, 425, 20, NEON_CYAN);
                DrawText("Press Q to Quit", SCREEN_WIDTH/2 - MeasureText("Press Q to Quit", 20)/2, 460, 20, GRAY);
                // Achievement progress
                int earnedCount = AchievementGetEarnedCount(&game->achievements);
                const char *achText = TextFormat("Achievements: %d/%d", earnedCount, ACHIEVEMENT_COUNT);
                DrawText(achText, SCREEN_WIDTH/2 - MeasureText(achText, 16)/2, 505, 16, (Color){ 150, 150, 150, 255 });
                DrawText("F1: Toggle Bloom | F2: Toggle CRT", SCREEN_WIDTH/2 - MeasureText("F1: Toggle Bloom | F2: Toggle CRT", 16)/2, 540, 16, (Color){ 100, 100, 100, 255 });
                break;
            }

            case STATE_MODE_SELECT:
            {
                DrawMenuStars();
                DrawText("SELECT MODE", SCREEN_WIDTH/2 - MeasureText("SELECT MODE", 50)/2, 150, 50, NEON_CYAN);

                // Draw mode options
                int boxWidth = 400;
                int boxHeight = 80;
                int startY = 280;
                int spacing = 100;

                const char *modes[] = { "1 PLAYER", "2 PLAYERS" };
                const char *descs[] = { "Solo survival", "Local co-op split screen" };

                for (int i = 0; i < 2; i++)
                {
                    int boxX = SCREEN_WIDTH / 2 - boxWidth / 2;
                    int boxY = startY + i * spacing;
                    bool selected = (i == game->modeSelection);

                    Color bgColor = selected ? (Color){ 60, 30, 80, 230 } : (Color){ 40, 20, 60, 200 };
                    Color borderColor = selected ? NEON_CYAN : NEON_PINK;

                    DrawRectangle(boxX, boxY, boxWidth, boxHeight, bgColor);
                    DrawRectangleLinesEx((Rectangle){ (float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight },
                                         selected ? 3.0f : 2.0f, borderColor);

                    Color textColor = selected ? NEON_WHITE : GRAY;
                    DrawText(modes[i], SCREEN_WIDTH/2 - MeasureText(modes[i], 28)/2, boxY + 15, 28, textColor);
                    DrawText(descs[i], SCREEN_WIDTH/2 - MeasureText(descs[i], 16)/2, boxY + 50, 16,
                             selected ? NEON_GREEN : (Color){ 100, 100, 100, 255 });
                }

                DrawText("W/S or Up/Down to select - ENTER to confirm - ESC to go back",
                         SCREEN_WIDTH/2 - MeasureText("W/S or Up/Down to select - ENTER to confirm - ESC to go back", 16)/2,
                         SCREEN_HEIGHT - 60, 16, GRAY);
                break;
            }

            case STATE_LEADERBOARD:
            {
                DrawMenuStars();
                DrawText("LEADERBOARD", SCREEN_WIDTH/2 - MeasureText("LEADERBOARD", 50)/2, 50, 50, NEON_CYAN);

                // Column headers
                int startY = 120;
                int rankX = 150;
                int scoreX = 280;
                int levelX = 450;
                int killsX = 550;
                int timeX = 680;
                int dateX = 820;

                DrawText("RANK", rankX, startY, 20, NEON_PINK);
                DrawText("SCORE", scoreX, startY, 20, NEON_PINK);
                DrawText("LVL", levelX, startY, 20, NEON_PINK);
                DrawText("KILLS", killsX, startY, 20, NEON_PINK);
                DrawText("TIME", timeX, startY, 20, NEON_PINK);
                DrawText("DATE", dateX, startY, 20, NEON_PINK);

                // Draw entries
                for (int i = 0; i < LEADERBOARD_MAX_ENTRIES; i++)
                {
                    int y = startY + 40 + i * 35;
                    Color rowColor = (i % 2 == 0) ? WHITE : (Color){ 180, 180, 180, 255 };

                    LeaderboardEntry *entry = LeaderboardGetEntry(&game->leaderboard, i);
                    if (entry != NULL)
                    {
                        DrawText(TextFormat("#%d", i + 1), rankX, y, 20, rowColor);
                        DrawText(TextFormat("%d", entry->score), scoreX, y, 20, rowColor);
                        DrawText(TextFormat("%d", entry->level), levelX, y, 20, rowColor);
                        DrawText(TextFormat("%d", entry->kills), killsX, y, 20, rowColor);
                        int minutes = (int)entry->survivalTime / 60;
                        int seconds = (int)entry->survivalTime % 60;
                        DrawText(TextFormat("%d:%02d", minutes, seconds), timeX, y, 20, rowColor);
                        DrawText(TextFormat("%d/%d/%d", entry->month, entry->day, entry->year), dateX, y, 20, rowColor);
                    }
                    else
                    {
                        DrawText(TextFormat("#%d", i + 1), rankX, y, 20, (Color){ 80, 80, 80, 255 });
                        DrawText("---", scoreX, y, 20, (Color){ 80, 80, 80, 255 });
                    }
                }

                DrawText("Press ESC or ENTER to return", SCREEN_WIDTH/2 - MeasureText("Press ESC or ENTER to return", 18)/2, SCREEN_HEIGHT - 60, 18, GRAY);
                break;
            }

            case STATE_SETTINGS:
                DrawSettingsMenu(game);
                break;

            case STATE_ACHIEVEMENTS:
            {
                DrawMenuStars();
                DrawText("ACHIEVEMENTS", SCREEN_WIDTH/2 - MeasureText("ACHIEVEMENTS", 50)/2, 40, 50, NEON_CYAN);

                // Show earned count
                int earnedCount = AchievementGetEarnedCount(&game->achievements);
                const char *progressText = TextFormat("%d / %d Unlocked", earnedCount, ACHIEVEMENT_COUNT);
                DrawText(progressText, SCREEN_WIDTH/2 - MeasureText(progressText, 20)/2, 100, 20, NEON_GREEN);

                // Draw achievement list
                int startY = 140;
                int itemHeight = 45;
                int boxWidth = 700;
                int boxX = (SCREEN_WIDTH - boxWidth) / 2;

                for (int i = 0; i < ACHIEVEMENT_COUNT; i++)
                {
                    int y = startY + i * itemHeight;
                    AchievementDef def = GetAchievementDef((AchievementType)i);
                    bool isEarned = AchievementIsEarned(&game->achievements, (AchievementType)i);
                    bool isSelected = (i == game->achievementSelection);

                    // Box background
                    Color bgColor = isSelected ? (Color){ 50, 40, 70, 230 } : (Color){ 30, 25, 45, 200 };
                    if (isEarned) bgColor = isSelected ? (Color){ 40, 60, 40, 230 } : (Color){ 25, 45, 25, 200 };
                    DrawRectangle(boxX, y, boxWidth, itemHeight - 5, bgColor);

                    // Selection indicator
                    if (isSelected)
                    {
                        DrawRectangleLinesEx((Rectangle){ (float)boxX, (float)y, (float)boxWidth, (float)(itemHeight - 5) }, 2.0f, NEON_CYAN);
                    }

                    // Trophy icon (earned = gold, unearned = gray)
                    Color trophyColor = isEarned ? NEON_YELLOW : (Color){ 60, 60, 60, 255 };
                    DrawCircle(boxX + 25, y + 20, 12, trophyColor);
                    if (isEarned)
                    {
                        DrawText("*", boxX + 19, y + 8, 24, VOID_BLACK);
                    }
                    else
                    {
                        DrawText("?", boxX + 19, y + 10, 20, (Color){ 40, 40, 40, 255 });
                    }

                    // Achievement name and description
                    Color nameColor = isEarned ? NEON_GREEN : (Color){ 120, 120, 120, 255 };
                    Color descColor = isEarned ? (Color){ 180, 180, 180, 255 } : (Color){ 80, 80, 80, 255 };
                    DrawText(def.name, boxX + 50, y + 6, 20, nameColor);
                    DrawText(def.description, boxX + 50, y + 26, 14, descColor);

                    // Checkmark for earned
                    if (isEarned)
                    {
                        DrawText("EARNED", boxX + boxWidth - 80, y + 12, 18, NEON_GREEN);
                    }
                }

                DrawText("W/S or Up/Down: Navigate - ESC or ENTER: Back", SCREEN_WIDTH/2 - MeasureText("W/S or Up/Down: Navigate - ESC or ENTER: Back", 16)/2, SCREEN_HEIGHT - 40, 16, GRAY);
                break;
            }

            case STATE_CHARACTER_SELECT:
            {
                DrawMenuStars();
                DrawText("SELECT CHARACTER", SCREEN_WIDTH/2 - MeasureText("SELECT CHARACTER", 50)/2, 60, 50, NEON_CYAN);

                // Draw character cards
                int cardWidth = 280;
                int cardHeight = 380;
                int cardSpacing = 30;
                int totalWidth = CHARACTER_COUNT * cardWidth + (CHARACTER_COUNT - 1) * cardSpacing;
                int startX = (SCREEN_WIDTH - totalWidth) / 2;
                int cardY = 140;

                for (int i = 0; i < CHARACTER_COUNT; i++)
                {
                    int cardX = startX + i * (cardWidth + cardSpacing);
                    CharacterDef def = GetCharacterDef((CharacterType)i);
                    bool isSelected = (i == game->characterSelection);
                    bool isUnlocked = UnlocksHasCharacter(&game->unlocks, i);

                    // Card background
                    Color bgColor = isUnlocked ? (Color){ 30, 30, 50, 220 } : (Color){ 20, 20, 20, 220 };
                    DrawRectangle(cardX, cardY, cardWidth, cardHeight, bgColor);

                    // Selection highlight
                    if (isSelected)
                    {
                        DrawRectangleLinesEx((Rectangle){ (float)cardX - 3, (float)cardY - 3, (float)cardWidth + 6, (float)cardHeight + 6 }, 3.0f, NEON_YELLOW);
                    }

                    // Character preview circle
                    int previewY = cardY + 80;
                    int previewRadius = 50;
                    if (isUnlocked)
                    {
                        DrawCircle(cardX + cardWidth/2, previewY, previewRadius + 5, (Color){ def.primaryColor.r/3, def.primaryColor.g/3, def.primaryColor.b/3, 255 });
                        DrawCircle(cardX + cardWidth/2, previewY, previewRadius, def.primaryColor);
                        DrawCircle(cardX + cardWidth/2, previewY, previewRadius * 0.6f, (Color){ 200, 200, 200, 200 });
                        DrawCircle(cardX + cardWidth/2, previewY, previewRadius * 0.3f, WHITE);
                    }
                    else
                    {
                        DrawCircle(cardX + cardWidth/2, previewY, previewRadius, (Color){ 50, 50, 50, 255 });
                        DrawText("?", cardX + cardWidth/2 - 15, previewY - 20, 50, (Color){ 80, 80, 80, 255 });
                    }

                    // Character name
                    Color nameColor = isUnlocked ? def.primaryColor : (Color){ 100, 100, 100, 255 };
                    DrawText(def.name, cardX + cardWidth/2 - MeasureText(def.name, 28)/2, cardY + 150, 28, nameColor);

                    // Stats (only if unlocked)
                    if (isUnlocked)
                    {
                        int statsY = cardY + 190;
                        int statsX = cardX + 20;
                        Color statColor = WHITE;

                        DrawText(TextFormat("HP: %.0f", def.maxHealth), statsX, statsY, 18, statColor);
                        DrawText(TextFormat("Speed: %.0f", def.speed), statsX, statsY + 25, 18, statColor);
                        DrawText(TextFormat("Magnet: %.0f", def.magnetRadius), statsX, statsY + 50, 18, statColor);
                        DrawText(TextFormat("Damage: x%.1f", def.damageMultiplier), statsX, statsY + 75, 18, statColor);
                        DrawText(TextFormat("XP: x%.2f", def.xpMultiplier), statsX, statsY + 100, 18, statColor);

                        // Description
                        DrawText(def.description, cardX + 10, cardY + cardHeight - 50, 14, (Color){ 150, 150, 150, 255 });
                    }
                    else
                    {
                        // Show unlock requirement
                        const char *lockMsg = "";
                        if (i == 1) lockMsg = "Play 5 games";
                        else if (i == 2) lockMsg = "Survive 5 minutes";
                        DrawText("LOCKED", cardX + cardWidth/2 - MeasureText("LOCKED", 24)/2, cardY + 200, 24, (Color){ 150, 50, 50, 255 });
                        DrawText(lockMsg, cardX + cardWidth/2 - MeasureText(lockMsg, 16)/2, cardY + 235, 16, (Color){ 100, 100, 100, 255 });
                    }
                }

                // Controls hint
                const char *p1Hint = (game->gameMode == GAME_MODE_COOP) ?
                    "P1: A/D or Left/Right - ENTER to confirm - ESC to go back" :
                    "A/D or Left/Right to select - ENTER to confirm - ESC to go back";
                DrawText(p1Hint, SCREEN_WIDTH/2 - MeasureText(p1Hint, 16)/2, SCREEN_HEIGHT - 40, 16, GRAY);
                break;
            }

            case STATE_CHARACTER_SELECT_P2:
            {
                DrawMenuStars();
                DrawText("PLAYER 2 - SELECT CHARACTER", SCREEN_WIDTH/2 - MeasureText("PLAYER 2 - SELECT CHARACTER", 40)/2, 40, 40, NEON_PINK);

                // Show P1's selection
                CharacterDef p1Def = GetCharacterDef(game->selectedCharacter);
                const char *p1Text = TextFormat("P1: %s", p1Def.name);
                DrawText(p1Text, SCREEN_WIDTH/2 - MeasureText(p1Text, 20)/2, 90, 20, NEON_CYAN);

                // Draw character cards (same layout as P1 select)
                int cardWidth = 280;
                int cardHeight = 380;
                int cardSpacing = 30;
                int totalWidth = CHARACTER_COUNT * cardWidth + (CHARACTER_COUNT - 1) * cardSpacing;
                int startX = (SCREEN_WIDTH - totalWidth) / 2;
                int cardY = 140;

                for (int i = 0; i < CHARACTER_COUNT; i++)
                {
                    int cardX = startX + i * (cardWidth + cardSpacing);
                    CharacterDef def = GetCharacterDef((CharacterType)i);
                    bool isSelected = (i == game->characterSelection);
                    bool isUnlocked = UnlocksHasCharacter(&game->unlocks, i);

                    // Card background
                    Color bgColor = isUnlocked ? (Color){ 30, 30, 50, 220 } : (Color){ 20, 20, 20, 220 };
                    DrawRectangle(cardX, cardY, cardWidth, cardHeight, bgColor);

                    // Selection highlight (pink for P2)
                    if (isSelected)
                    {
                        DrawRectangleLinesEx((Rectangle){ (float)cardX - 3, (float)cardY - 3, (float)cardWidth + 6, (float)cardHeight + 6 }, 3.0f, NEON_PINK);
                    }

                    // Character preview circle
                    int previewY = cardY + 80;
                    int previewRadius = 50;
                    if (isUnlocked)
                    {
                        DrawCircle(cardX + cardWidth/2, previewY, previewRadius + 5, (Color){ def.primaryColor.r/3, def.primaryColor.g/3, def.primaryColor.b/3, 255 });
                        DrawCircle(cardX + cardWidth/2, previewY, previewRadius, def.primaryColor);
                        DrawCircle(cardX + cardWidth/2, previewY, previewRadius * 0.6f, (Color){ 200, 200, 200, 200 });
                        DrawCircle(cardX + cardWidth/2, previewY, previewRadius * 0.3f, WHITE);
                    }
                    else
                    {
                        DrawCircle(cardX + cardWidth/2, previewY, previewRadius, (Color){ 50, 50, 50, 255 });
                        DrawText("?", cardX + cardWidth/2 - 15, previewY - 20, 50, (Color){ 80, 80, 80, 255 });
                    }

                    // Character name
                    Color nameColor = isUnlocked ? def.primaryColor : (Color){ 100, 100, 100, 255 };
                    DrawText(def.name, cardX + cardWidth/2 - MeasureText(def.name, 28)/2, cardY + 150, 28, nameColor);

                    // Stats (only if unlocked)
                    if (isUnlocked)
                    {
                        int statsY = cardY + 190;
                        int statsX = cardX + 20;
                        Color statColor = WHITE;

                        DrawText(TextFormat("HP: %.0f", def.maxHealth), statsX, statsY, 18, statColor);
                        DrawText(TextFormat("Speed: %.0f", def.speed), statsX, statsY + 25, 18, statColor);
                        DrawText(TextFormat("Magnet: %.0f", def.magnetRadius), statsX, statsY + 50, 18, statColor);
                        DrawText(TextFormat("Damage: x%.1f", def.damageMultiplier), statsX, statsY + 75, 18, statColor);
                        DrawText(TextFormat("XP: x%.2f", def.xpMultiplier), statsX, statsY + 100, 18, statColor);

                        // Description
                        DrawText(def.description, cardX + 10, cardY + cardHeight - 50, 14, (Color){ 150, 150, 150, 255 });
                    }
                    else
                    {
                        // Show unlock requirement
                        const char *lockMsg = "";
                        if (i == 1) lockMsg = "Play 5 games";
                        else if (i == 2) lockMsg = "Survive 5 minutes";
                        DrawText("LOCKED", cardX + cardWidth/2 - MeasureText("LOCKED", 24)/2, cardY + 200, 24, (Color){ 150, 50, 50, 255 });
                        DrawText(lockMsg, cardX + cardWidth/2 - MeasureText(lockMsg, 16)/2, cardY + 235, 16, (Color){ 100, 100, 100, 255 });
                    }
                }

                // Controls hint for P2
                DrawText("P2: J/L or Arrows - ENTER to confirm - ESC to go back", SCREEN_WIDTH/2 - MeasureText("P2: J/L or Arrows - ENTER to confirm - ESC to go back", 16)/2, SCREEN_HEIGHT - 40, 16, GRAY);
                break;
            }

            case STATE_STARTING:
            {
                // Draw appropriate background based on transition phase
                float fadeInEnd = 0.5f;
                float holdEnd = 2.0f;

                if (game->transitionTimer < fadeInEnd)
                {
                    // Still fading from menu - show starfield
                    DrawMenuStars();
                }
                else if (game->transitionTimer >= holdEnd)
                {
                    // Fading into game - show game world
                    BeginMode2D(game->camera);
                        DrawGameWorld(game);
                    EndMode2D();
                }

                // Draw fade overlay
                unsigned char alpha = (unsigned char)(game->fadeAlpha * 255.0f);
                DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 0, 0, alpha });

                // Draw "Get Ready" text when fully faded to black
                if (game->fadeAlpha > 0.9f)
                {
                    const char *readyText = "GET READY...";
                    int textWidth = MeasureText(readyText, 50);

                    // Pulsing effect
                    float pulse = 0.8f + 0.2f * sinf(game->transitionTimer * 4.0f);
                    Color textColor = (Color){
                        (unsigned char)(NEON_CYAN.r * pulse),
                        (unsigned char)(NEON_CYAN.g * pulse),
                        (unsigned char)(NEON_CYAN.b * pulse),
                        255
                    };

                    DrawText(readyText, SCREEN_WIDTH/2 - textWidth/2, SCREEN_HEIGHT/2 - 25, 50, textColor);
                }
                break;
            }

            case STATE_PLAYING:
                if (game->gameMode == GAME_MODE_COOP)
                {
                    // Split-screen rendering for co-op
                    int vpWidth = VIEWPORT_WIDTH;
                    int vpHeight = VIEWPORT_HEIGHT;

                    // First pass: render each player's view to their viewport texture
                    // Must end main render target first since we can't nest BeginTextureMode
                    EndTextureMode();

                    for (int i = 0; i < game->coop.playerCount; i++)
                    {
                        BeginTextureMode(game->coop.cameras[i].viewport);
                            ClearBackground(VOID_BLACK);
                            BeginMode2D(game->coop.cameras[i].cam);
                                DrawGameWorldForCamera(game, game->coop.cameras[i].cam, vpWidth, vpHeight);
                            EndMode2D();

                            // Draw partner arrow indicator
                            int partnerIndex = (i == 0) ? 1 : 0;
                            DrawPartnerArrow(game, i, partnerIndex, vpWidth, vpHeight);

                            // Draw player-specific HUD elements (health bar, etc.)
                            Player *p = CoopGetPlayer(&game->coop, i);
                            if (p)
                            {
                                // P1 HUD on left of viewport, P2 HUD on right (to avoid shared HUD overlap)
                                int barWidth = 150;
                                int barHeight = 16;
                                int barY = 10;
                                int barX = (i == 0) ? 10 : (vpWidth - barWidth - 40);

                                float healthRatio = p->health / p->maxHealth;
                                DrawRectangle(barX, barY, barWidth, barHeight, (Color){ 50, 30, 30, 200 });
                                DrawRectangle(barX, barY, (int)(barWidth * healthRatio), barHeight,
                                    healthRatio > 0.5f ? NEON_GREEN : (healthRatio > 0.25f ? NEON_YELLOW : NEON_RED));
                                DrawRectangleLinesEx((Rectangle){ (float)barX, (float)barY, (float)barWidth, (float)barHeight }, 1.0f, NEON_WHITE);

                                // Player label (P1=cyan, P2=green to match player colors)
                                const char *playerLabel = (i == 0) ? "P1" : "P2";
                                int labelX = (i == 0) ? (barX + barWidth + 10) : (barX - 30);
                                DrawText(playerLabel, labelX, barY, 16, (i == 0) ? NEON_CYAN : NEON_GREEN);

                                // Dash indicator
                                const char *dashText = (p->dashCooldown <= 0.0f) ? "DASH: READY" : "DASH: ...";
                                DrawText(dashText, barX, barY + 22, 12, (p->dashCooldown <= 0.0f) ? NEON_GREEN : GRAY);

                                // Weapon info
                                const char *weaponName = WeaponGetName(p->weapon.type);
                                Color weaponColor = WeaponGetColor(p->weapon.type);
                                DrawText(weaponName, barX, barY + 38, 12, weaponColor);

                                // Weapon switch hint
                                const char *switchHint = (i == 0) ? "[Q/E]" : "[,/.]";
                                DrawText(switchHint, barX + 100, barY + 38, 10, GRAY);
                            }
                        EndTextureMode();
                    }

                    // Second pass: composite viewports onto main render target
                    BeginTextureMode(game->renderTarget);
                    ClearBackground(VOID_BLACK);

                    for (int i = 0; i < game->coop.playerCount; i++)
                    {
                        Rectangle srcRec = game->coop.cameras[i].sourceRect;
                        Rectangle dstRec = game->coop.cameras[i].destRect;
                        DrawTexturePro(game->coop.cameras[i].viewport.texture, srcRec, dstRec, (Vector2){ 0, 0 }, 0.0f, WHITE);
                    }

                    // Draw split line with glow
                    int splitX = SCREEN_WIDTH / 2;
                    DrawRectangle(splitX - 2, 0, 4, SCREEN_HEIGHT, (Color){ 50, 30, 80, 200 });
                    DrawLine(splitX, 0, splitX, SCREEN_HEIGHT, NEON_PINK);

                    // Shared HUD elements (centered at top)
                    // XP bar (shared level)
                    int xpBarWidth = 300;
                    int xpBarX = SCREEN_WIDTH / 2 - xpBarWidth / 2;
                    int xpBarY = SCREEN_HEIGHT - 40;
                    float xpRatio = (float)game->coop.sharedXP / (float)game->coop.sharedXPToNextLevel;
                    DrawRectangle(xpBarX, xpBarY, xpBarWidth, 12, (Color){ 30, 30, 50, 200 });
                    DrawRectangle(xpBarX, xpBarY, (int)(xpBarWidth * xpRatio), 12, NEON_CYAN);
                    DrawRectangleLinesEx((Rectangle){ (float)xpBarX, (float)xpBarY, (float)xpBarWidth, 12.0f }, 1.0f, NEON_WHITE);

                    // Shared level display
                    const char *levelText = TextFormat("LV %d", game->coop.sharedLevel);
                    DrawText(levelText, SCREEN_WIDTH / 2 - MeasureText(levelText, 20) / 2, SCREEN_HEIGHT - 65, 20, NEON_YELLOW);

                    // Shared HUD panel background (centered at top, below split line)
                    int hudPanelWidth = 200;
                    int hudPanelHeight = 70;
                    int hudPanelX = SCREEN_WIDTH / 2 - hudPanelWidth / 2;
                    int hudPanelY = 8;
                    DrawRectangle(hudPanelX, hudPanelY, hudPanelWidth, hudPanelHeight, (Color){ 0, 0, 0, 180 });
                    DrawRectangleLinesEx((Rectangle){ (float)hudPanelX, (float)hudPanelY, (float)hudPanelWidth, (float)hudPanelHeight }, 1.0f, (Color){ 100, 50, 150, 200 });

                    // Score with multiplier (larger font, inside panel)
                    const char *scoreText = TextFormat("SCORE: %d", game->score);
                    DrawText(scoreText, SCREEN_WIDTH / 2 - MeasureText(scoreText, 20) / 2, hudPanelY + 8, 20, NEON_YELLOW);

                    // Score multiplier
                    Color multiplierColor = NEON_GREEN;
                    if (game->scoreMultiplier >= MULTIPLIER_TIER_YELLOW) multiplierColor = NEON_YELLOW;
                    if (game->scoreMultiplier >= MULTIPLIER_TIER_ORANGE) multiplierColor = NEON_ORANGE;
                    if (game->scoreMultiplier >= MULTIPLIER_TIER_PINK) multiplierColor = NEON_PINK;
                    const char *multText = TextFormat("x%.1f", game->scoreMultiplier);
                    int scoreWidth = MeasureText(scoreText, 20);
                    DrawText(multText, SCREEN_WIDTH / 2 + scoreWidth / 2 + 8, hudPanelY + 10, 16, multiplierColor);

                    // Game time (centered, below score)
                    int minutes = (int)game->gameTime / 60;
                    int seconds = (int)game->gameTime % 60;
                    const char *timeText = TextFormat("%d:%02d", minutes, seconds);
                    DrawText(timeText, SCREEN_WIDTH / 2 - MeasureText(timeText, 18) / 2, hudPanelY + 32, 18, NEON_CYAN);

                    // Kill count (below time)
                    const char *killText = TextFormat("KILLS: %d", game->killCount);
                    DrawText(killText, SCREEN_WIDTH / 2 - MeasureText(killText, 16) / 2, hudPanelY + 52, 16, NEON_ORANGE);

                    // Boss warning (centered)
                    if (game->bossWarningActive)
                    {
                        float flash = (sinf((float)GetTime() * 10.0f) + 1.0f) * 0.5f;
                        unsigned char alpha = (unsigned char)(150 + 105 * flash);
                        Color warningColor = (Color){ 255, 50, 50, alpha };

                        const char *warningText = "!! BOSS INCOMING !!";
                        int textWidth = MeasureText(warningText, 30);
                        int centerX = SCREEN_WIDTH / 2 - textWidth / 2;
                        int centerY = SCREEN_HEIGHT / 3;

                        DrawRectangle(centerX - 15, centerY - 8, textWidth + 30, 50, (Color){ 0, 0, 0, 180 });
                        DrawText(warningText, centerX, centerY, 30, warningColor);

                        const char *countText = TextFormat("%.1f", game->bossWarningTimer);
                        int countWidth = MeasureText(countText, 24);
                        DrawText(countText, SCREEN_WIDTH / 2 - countWidth / 2, centerY + 32, 24, NEON_YELLOW);
                    }

                    // Boss health bar (centered below shared HUD panel)
                    Enemy *boss = EnemyPoolGetBoss(&game->enemies);
                    if (boss)
                    {
                        int bossBarWidth = 300;
                        int bossBarHeight = 16;
                        int bossBarX = SCREEN_WIDTH / 2 - bossBarWidth / 2;
                        int bossBarY = hudPanelY + hudPanelHeight + 8;
                        float bossHealthPercent = boss->health / boss->maxHealth;

                        DrawRectangle(bossBarX - 3, bossBarY - 3, bossBarWidth + 6, bossBarHeight + 6, (Color){ 0, 0, 0, 200 });
                        DrawRectangle(bossBarX, bossBarY, bossBarWidth, bossBarHeight, (Color){ 80, 20, 80, 255 });
                        DrawRectangle(bossBarX, bossBarY, (int)(bossBarWidth * bossHealthPercent), bossBarHeight, (Color){ 200, 50, 200, 255 });
                        DrawRectangleLines(bossBarX, bossBarY, bossBarWidth, bossBarHeight, (Color){ 255, 100, 255, 255 });

                        const char *bossLabel = TextFormat("BOSS #%d", game->bossCount);
                        DrawText(bossLabel, SCREEN_WIDTH / 2 - MeasureText(bossLabel, 14) / 2, bossBarY + bossBarHeight + 3, 14, (Color){ 255, 100, 255, 255 });
                    }
                }
                else
                {
                    // Solo mode - existing rendering
                    BeginMode2D(game->camera);
                        DrawGameWorld(game);
                    EndMode2D();
                    DrawHUD(game);
                    DrawTutorial(game);
                }

                // Achievement notification popup (both modes)
                if (game->achievementDisplayTimer > 0.0f && game->pendingAchievement >= 0)
                {
                    AchievementDef def = GetAchievementDef(game->pendingAchievement);

                    // Slide in from top
                    float slideProgress = 1.0f;
                    if (game->achievementDisplayTimer > 2.5f)
                    {
                        slideProgress = (3.0f - game->achievementDisplayTimer) / 0.5f;  // Slide in
                    }
                    else if (game->achievementDisplayTimer < 0.5f)
                    {
                        slideProgress = game->achievementDisplayTimer / 0.5f;  // Slide out
                    }

                    int popupWidth = 350;
                    int popupHeight = 70;
                    int popupX = SCREEN_WIDTH / 2 - popupWidth / 2;
                    int popupY = (int)(-popupHeight + (popupHeight + 20) * slideProgress);

                    // Draw popup background
                    DrawRectangle(popupX, popupY, popupWidth, popupHeight, (Color){ 30, 50, 30, 230 });
                    DrawRectangleLinesEx((Rectangle){ (float)popupX, (float)popupY, (float)popupWidth, (float)popupHeight }, 3.0f, NEON_GREEN);

                    // Trophy icon
                    DrawCircle(popupX + 35, popupY + 35, 20, NEON_YELLOW);
                    DrawText("*", popupX + 27, popupY + 18, 30, VOID_BLACK);

                    // Achievement text
                    DrawText("ACHIEVEMENT UNLOCKED!", popupX + 65, popupY + 10, 16, NEON_GREEN);
                    DrawText(def.name, popupX + 65, popupY + 32, 22, NEON_WHITE);
                }
                break;

            case STATE_PAUSED:
                BeginMode2D(game->camera);
                    DrawGameWorld(game);
                EndMode2D();
                DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 0, 0, 150 });
                DrawText("PAUSED", SCREEN_WIDTH/2 - MeasureText("PAUSED", 60)/2, 250, 60, NEON_YELLOW);
                DrawText("Press ESC to Resume", SCREEN_WIDTH/2 - MeasureText("Press ESC to Resume", 20)/2, 350, 20, NEON_CYAN);
                DrawText("Press Q to Quit to Menu", SCREEN_WIDTH/2 - MeasureText("Press Q to Quit to Menu", 20)/2, 400, 20, GRAY);
                break;

            case STATE_LEVELUP:
                BeginMode2D(game->camera);
                    DrawGameWorld(game);
                EndMode2D();
                DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 0, 0, 180 });

                DrawText("LEVEL UP!", SCREEN_WIDTH/2 - MeasureText("LEVEL UP!", 50)/2, 120, 50, NEON_GREEN);
                DrawText(TextFormat("Level %d", game->player.level), SCREEN_WIDTH/2 - MeasureText(TextFormat("Level %d", game->player.level), 24)/2, 180, 24, NEON_CYAN);
                DrawText("Choose an upgrade:", SCREEN_WIDTH/2 - MeasureText("Choose an upgrade:", 20)/2, 230, 20, NEON_WHITE);

                for (int i = 0; i < 3; i++)
                {
                    Upgrade upgrade = GetUpgradeDefinition(game->upgradeOptions[i]);
                    DrawUpgradeOption(i, upgrade, 280.0f + i * 100.0f);
                }
                break;

            case STATE_GAMEOVER:
            {
                DrawText("GAME OVER", SCREEN_WIDTH/2 - MeasureText("GAME OVER", 60)/2, 160, 60, NEON_RED);

                // Show leaderboard position if made it
                if (game->leaderboardPosition >= 0)
                {
                    const char *rankText = TextFormat("NEW HIGH SCORE! Rank #%d", game->leaderboardPosition + 1);
                    DrawText(rankText, SCREEN_WIDTH/2 - MeasureText(rankText, 28)/2, 240, 28, NEON_GREEN);
                }

                int statsY = (game->leaderboardPosition >= 0) ? 290 : 260;
                DrawText(TextFormat("Final Score: %d", game->score), SCREEN_WIDTH/2 - MeasureText(TextFormat("Final Score: %d", game->score), 30)/2, statsY, 30, NEON_YELLOW);
                DrawText(TextFormat("Enemies Killed: %d", game->killCount), SCREEN_WIDTH/2 - MeasureText(TextFormat("Enemies Killed: %d", game->killCount), 20)/2, statsY + 45, 20, NEON_ORANGE);
                DrawText(TextFormat("Level Reached: %d", game->player.level), SCREEN_WIDTH/2 - MeasureText(TextFormat("Level Reached: %d", game->player.level), 20)/2, statsY + 75, 20, NEON_CYAN);
                int minutes = (int)game->gameTime / 60;
                int seconds = (int)game->gameTime % 60;
                DrawText(TextFormat("Time Survived: %d:%02d", minutes, seconds), SCREEN_WIDTH/2 - MeasureText(TextFormat("Time Survived: %d:%02d", minutes, seconds), 20)/2, statsY + 105, 20, NEON_WHITE);

                DrawText("Press ENTER to Return to Menu", SCREEN_WIDTH/2 - MeasureText("Press ENTER to Return to Menu", 20)/2, statsY + 170, 20, NEON_CYAN);
                DrawText("Press L to View Leaderboard", SCREEN_WIDTH/2 - MeasureText("Press L to View Leaderboard", 18)/2, statsY + 200, 18, GRAY);
                break;
            }
        }
    EndTextureMode();
}

void GameDraw(GameData *game)
{
    // Render scene to texture for post-processing
    DrawSceneToTexture(game);

    // Source rectangle (flipped vertically for OpenGL)
    Rectangle sourceRec = { 0, 0, (float)game->renderTarget.texture.width, -(float)game->renderTarget.texture.height };
    Rectangle destRec = { 0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
    Vector2 origin = { 0, 0 };

    if (game->shadersEnabled)
    {
        // Update shader time uniforms
        float time = (float)GetTime();
        SetShaderValue(game->crtShader, game->crtTimeLoc, &time, SHADER_UNIFORM_FLOAT);
        SetShaderValue(game->chromaticShader, game->chromaticTimeLoc, &time, SHADER_UNIFORM_FLOAT);
        SetShaderValue(game->chromaticShader, game->chromaticIntensityLoc, &game->chromaticIntensity, SHADER_UNIFORM_FLOAT);

        // Determine if chromatic aberration should be applied
        bool applyChromatic = (game->chromaticShader.id != 0) && (game->chromaticIntensity > 0.01f);

        if (game->crtEnabled)
        {
            // Multi-pass shader chain: Scene -> Bloom -> [Chromatic] -> CRT -> Screen
            // Pass 1: Apply bloom to renderTarget, output to renderTarget2
            BeginTextureMode(game->renderTarget2);
                ClearBackground(BLACK);
                BeginShaderMode(game->bloomShader);
                    DrawTexturePro(game->renderTarget.texture, sourceRec, destRec, origin, 0.0f, WHITE);
                EndShaderMode();
            EndTextureMode();

            if (applyChromatic)
            {
                // Pass 2: Apply chromatic to renderTarget2, output to renderTarget
                BeginTextureMode(game->renderTarget);
                    ClearBackground(BLACK);
                    BeginShaderMode(game->chromaticShader);
                        DrawTexturePro(game->renderTarget2.texture, sourceRec, destRec, origin, 0.0f, WHITE);
                    EndShaderMode();
                EndTextureMode();

                // Pass 3: Apply CRT to renderTarget, output to screen
                BeginShaderMode(game->crtShader);
                    DrawTexturePro(game->renderTarget.texture, sourceRec, destRec, origin, 0.0f, WHITE);
                EndShaderMode();
            }
            else
            {
                // Pass 2: Apply CRT to renderTarget2, output to screen (no chromatic)
                BeginShaderMode(game->crtShader);
                    DrawTexturePro(game->renderTarget2.texture, sourceRec, destRec, origin, 0.0f, WHITE);
                EndShaderMode();
            }
        }
        else
        {
            // No CRT - simpler chain
            if (applyChromatic)
            {
                // Bloom -> Chromatic -> Screen
                BeginTextureMode(game->renderTarget2);
                    ClearBackground(BLACK);
                    BeginShaderMode(game->bloomShader);
                        DrawTexturePro(game->renderTarget.texture, sourceRec, destRec, origin, 0.0f, WHITE);
                    EndShaderMode();
                EndTextureMode();

                BeginShaderMode(game->chromaticShader);
                    DrawTexturePro(game->renderTarget2.texture, sourceRec, destRec, origin, 0.0f, WHITE);
                EndShaderMode();
            }
            else
            {
                // Bloom only: Apply directly to screen
                BeginShaderMode(game->bloomShader);
                    DrawTexturePro(game->renderTarget.texture, sourceRec, destRec, origin, 0.0f, WHITE);
                EndShaderMode();
            }
        }
    }
    else
    {
        // No shaders, just draw the texture
        DrawTexturePro(game->renderTarget.texture, sourceRec, destRec, origin, 0.0f, WHITE);
    }
}
