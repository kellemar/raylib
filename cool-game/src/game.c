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

static void ApplyBlackHolePull(ProjectilePool *projectiles, EnemyPool *enemies, float dt)
{
    // Black hole projectiles pull nearby enemies toward them
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        Projectile *p = &projectiles->projectiles[i];
        if (!p->active || p->behavior != PROJ_BEHAVIOR_PULL) continue;

        float pullRadius = p->radius * BLACK_HOLE_PULL_RADIUS_MULT;

        for (int j = 0; j < MAX_ENEMIES; j++)
        {
            Enemy *e = &enemies->enemies[j];
            if (!e->active) continue;

            float dx = p->pos.x - e->pos.x;
            float dy = p->pos.y - e->pos.y;
            float distSq = dx * dx + dy * dy;
            float pullRadiusSq = pullRadius * pullRadius;

            if (distSq < pullRadiusSq && distSq > 1.0f)
            {
                float dist = sqrtf(distSq);
                // Pull force increases as enemies get closer
                float pullFactor = 1.0f - (dist / pullRadius);
                float pullForce = p->pullStrength * pullFactor * dt;

                // Apply pull toward black hole
                e->pos.x += (dx / dist) * pullForce;
                e->pos.y += (dy / dist) * pullForce;
            }
        }
    }
}

static void CheckProjectileEnemyCollisions(ProjectilePool *projectiles, EnemyPool *enemies, XPPool *xp, ParticlePool *particles, GameData *game, Player *player)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        Projectile *p = &projectiles->projectiles[i];
        if (!p->active) continue;

        for (int j = 0; j < MAX_ENEMIES; j++)
        {
            Enemy *e = &enemies->enemies[j];
            if (!e->active) continue;

            if (CheckCircleCollision(p->pos, p->radius, e->pos, e->radius))
            {
                // Apply critical hit
                float damage = p->damage;
                if (player->weapon.critChance > 0.0f)
                {
                    float roll = (float)(rand() % 100) / 100.0f;
                    if (roll < player->weapon.critChance)
                    {
                        damage *= player->weapon.critMultiplier;
                    }
                }

                e->health -= damage;
                e->hitFlashTimer = 0.1f;

                // Apply vampirism (lifesteal)
                if (player->vampirism > 0.0f && player->health < player->maxHealth)
                {
                    float healAmount = damage * player->vampirism;
                    player->health += healAmount;
                    if (player->health > player->maxHealth)
                    {
                        player->health = player->maxHealth;
                    }
                }

                // Use projectile color for hit particles
                SpawnHitParticles(particles, p->pos, p->color, 5);

                // Apply slow effect from freeze ray
                if (p->effects & PROJ_EFFECT_SLOW)
                {
                    EnemyApplySlow(e, p->slowAmount, p->slowDuration);
                }

                if (!p->pierce)
                {
                    p->active = false;
                    projectiles->count--;
                }

                if (e->health <= 0.0f)
                {
                    Vector2 deathPos = e->pos;
                    EnemyType deathType = e->type;
                    int deathSplitCount = e->splitCount;
                    float deathRadius = e->radius;
                    float deathMaxHealth = e->maxHealth;

                    if (deathType == ENEMY_SPLITTER && deathSplitCount > 0)
                    {
                        float childRadius = deathRadius * 0.7f;
                        float childHealth = deathMaxHealth * 0.5f;
                        int childSplitCount = deathSplitCount - 1;

                        Vector2 offset1 = { -deathRadius, 0.0f };
                        Vector2 offset2 = { deathRadius, 0.0f };
                        Vector2 childPos1 = Vector2Add(deathPos, offset1);
                        Vector2 childPos2 = Vector2Add(deathPos, offset2);

                        EnemySpawnSplitterChild(enemies, childPos1, childSplitCount, childRadius, childHealth);
                        EnemySpawnSplitterChild(enemies, childPos2, childSplitCount, childRadius, childHealth);

                        SpawnExplosion(particles, deathPos, p->color, 10);
                    }
                    else
                    {
                        XPSpawn(xp, deathPos, e->xpValue);
                        SpawnExplosion(particles, deathPos, p->color, 15);
                        // Trigger impact frame for larger enemies
                        TriggerImpactFrame(game, deathPos, deathRadius * 2.0f);
                    }

                    PlayGameSound(SOUND_EXPLOSION);
                    TriggerScreenShake(game, 3.0f, 0.15f);
                    e->active = false;
                    enemies->count--;
                    game->score += (int)(e->xpValue * 10 * game->scoreMultiplier);
                    game->killCount++;

                    // Achievement: First Blood (first kill ever)
                    TryEarnAchievement(game, ACH_FIRST_BLOOD);

                    // Achievement: Centurion (100 kills this run)
                    if (game->killCount >= 100)
                    {
                        TryEarnAchievement(game, ACH_CENTURION);
                    }

                    // Track boss kills for unlocks
                    if (e->isBoss)
                    {
                        game->bossKillsThisRun++;

                        // Achievement: Boss Hunter (first boss kill)
                        TryEarnAchievement(game, ACH_BOSS_HUNTER);
                    }

                    // Hitstop: brief freeze on kill (more frames for bigger enemies)
                    int hitstopAmount = (e->xpValue >= 3) ? 4 : 2;
                    if (hitstopAmount > game->hitstopFrames)
                    {
                        game->hitstopFrames = hitstopAmount;
                    }
                }

                break;
            }
        }
    }
}

static void CheckEnemyPlayerCollisions(EnemyPool *enemies, Player *player, ParticlePool *particles, GameData *game, XPPool *xp)
{
    if (!player->alive) return;

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        Enemy *e = &enemies->enemies[i];
        if (!e->active) continue;

        if (CheckCircleCollision(player->pos, player->radius, e->pos, e->radius))
        {
            // If player is dashing and has dash damage, damage the enemy instead
            if (player->isDashing && player->dashDamage > 0.0f)
            {
                e->health -= player->dashDamage;
                e->hitFlashTimer = 0.1f;
                SpawnHitParticles(particles, e->pos, NEON_PINK, 8);

                if (e->health <= 0.0f)
                {
                    XPSpawn(xp, e->pos, e->xpValue);
                    SpawnExplosion(particles, e->pos, NEON_PINK, 15);
                    PlayGameSound(SOUND_EXPLOSION);
                    TriggerScreenShake(game, 3.0f, 0.15f);
                    e->active = false;
                    enemies->count--;
                    game->score += (int)(e->xpValue * 10 * game->scoreMultiplier);
                    game->killCount++;
                }
                continue;  // Check next enemy, don't take damage
            }

            // Normal collision - player takes damage if not invincible
            if (player->invincibilityTimer > 0.0f) continue;

            PlayerTakeDamage(player, e->damage);
            PlayGameSound(SOUND_HIT);
            SpawnHitParticles(particles, player->pos, NEON_RED, 10);
            TriggerScreenShake(game, 8.0f, 0.25f);

            // Reset score multiplier on damage
            game->scoreMultiplier = 1.0f;
            game->timeSinceLastHit = 0.0f;

            float dx = player->pos.x - e->pos.x;
            float dy = player->pos.y - e->pos.y;
            float dist = sqrtf(dx * dx + dy * dy);
            if (dist > 0.0f)
            {
                Vector2 pushDir = { dx / dist, dy / dist };
                float knockback = 30.0f * player->knockbackMultiplier;
                e->pos.x -= pushDir.x * knockback;
                e->pos.y -= pushDir.y * knockback;
            }

            break;
        }
    }
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

    // Load and apply settings
    LoadSettings(&game->settings);
    ApplySettings(game);

    PlayerInit(&game->player);
    ProjectilePoolInit(&game->projectiles);
    EnemyPoolInit(&game->enemies);
    XPPoolInit(&game->xp);
    ParticlePoolInit(&game->particles);
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
                // Go to character select
                game->characterSelection = game->selectedCharacter;
                game->state = STATE_CHARACTER_SELECT;
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
                    // Start "Get Ready" transition
                    game->state = STATE_STARTING;
                    game->transitionTimer = 0.0f;
                    game->fadeAlpha = 0.0f;
                }
            }
            if (IsKeyPressed(KEY_ESCAPE))
            {
                game->state = STATE_MENU;
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
                InitCamera(game);
            }
            break;
        }

        case STATE_PLAYING:
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
            float healthPercent = game->player.health / game->player.maxHealth;
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

            PlayerUpdate(&game->player, scaledDt, &game->projectiles, game->camera);
            // Pass enemy pool so each homing projectile can find its own target
            ProjectilePoolUpdate(&game->projectiles, scaledDt, &game->enemies);
            EnemyPoolUpdate(&game->enemies, game->player.pos, scaledDt);
            XPPoolUpdate(&game->xp, game->player.pos, game->player.magnetRadius, scaledDt);
            ParticlePoolUpdate(&game->particles, scaledDt);
            UpdateGameCamera(game, scaledDt);

            game->spawnTimer += dt;
            float spawnInterval = GetSpawnInterval(game->gameTime);
            if (game->spawnTimer >= spawnInterval)
            {
                Vector2 spawnPos = GetSpawnPosition(game->player.pos);
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
                    Vector2 spawnPos = GetSpawnPosition(game->player.pos);
                    EnemySpawnBoss(&game->enemies, spawnPos, game->bossCount);
                    game->bossSpawnTimer = BOSS_SPAWN_INTERVAL;  // Reset for next boss
                    game->bossWarningActive = false;
                    // Screen shake when boss spawns
                    TriggerScreenShake(game, 15.0f, 0.8f);
                }
            }

            // Apply slow aura around player if upgraded
            if (game->player.slowAuraRadius > 0.0f)
            {
                for (int i = 0; i < MAX_ENEMIES; i++)
                {
                    Enemy *e = &game->enemies.enemies[i];
                    if (!e->active) continue;

                    float dx = game->player.pos.x - e->pos.x;
                    float dy = game->player.pos.y - e->pos.y;
                    float distSq = dx * dx + dy * dy;
                    float radiusSq = game->player.slowAuraRadius * game->player.slowAuraRadius;

                    if (distSq < radiusSq)
                    {
                        EnemyApplySlow(e, game->player.slowAuraAmount, 0.5f);
                    }
                }
            }

            // Apply black hole pull effect before collision checks
            ApplyBlackHolePull(&game->projectiles, &game->enemies, scaledDt);
            CheckProjectileEnemyCollisions(&game->projectiles, &game->enemies, &game->xp, &game->particles, game, &game->player);
            CheckEnemyPlayerCollisions(&game->enemies, &game->player, &game->particles, game, &game->xp);

            int collectedXP = XPCollect(&game->xp, game->player.pos, XP_COLLECT_RADIUS);
            if (collectedXP > 0)
            {
                // Apply XP multiplier from upgrades
                int boostedXP = (int)(collectedXP * game->player.xpMultiplier);
                game->player.xp += boostedXP;
                PlayGameSound(SOUND_PICKUP);
            }

            if (CheckLevelUp(&game->player))
            {
                PlayGameSound(SOUND_LEVELUP);
                MusicPause();
                GenerateRandomUpgrades(game->upgradeOptions, 3);
                game->timeScale = 0.3f;  // Slow-mo during level up

                // Achievement: Level 5
                if (game->player.level >= 5)
                {
                    TryEarnAchievement(game, ACH_LEVEL_5);
                }

                // Achievement: Level 10
                if (game->player.level >= 10)
                {
                    TryEarnAchievement(game, ACH_LEVEL_10);
                }

                game->state = STATE_LEVELUP;
            }

            if (!game->player.alive)
            {
                MusicStop();

                // Add to leaderboard
                game->leaderboardPosition = LeaderboardAddEntry(
                    &game->leaderboard,
                    game->score,
                    game->player.level,
                    game->killCount,
                    game->gameTime
                );
                LeaderboardSave(&game->leaderboard);
                game->highScore = LeaderboardGetHighScore(&game->leaderboard);

                // Save run stats to unlocks
                UnlocksAddRunStats(&game->unlocks, game->killCount, game->bossKillsThisRun,
                                   game->score, game->player.level, game->gameTime);
                UnlocksCheckNewUnlocks(&game->unlocks);
                UnlocksSave(&game->unlocks);

                // Update achievement stats
                game->achievements.totalKills += game->killCount;
                game->achievements.totalBossKills += game->bossKillsThisRun;
                if (game->gameTime > game->achievements.longestSurvival)
                {
                    game->achievements.longestSurvival = game->gameTime;
                }
                if (game->player.level > game->achievements.highestLevel)
                {
                    game->achievements.highestLevel = game->player.level;
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

            // Weapon switching with Q/E keys
            if (IsKeyPressed(KEY_Q))
            {
                PlayerCycleWeapon(&game->player, -1);  // Previous weapon
            }
            if (IsKeyPressed(KEY_E))
            {
                PlayerCycleWeapon(&game->player, 1);   // Next weapon
            }

            if (IsKeyPressed(KEY_ESCAPE))
            {
                MusicPause();
                game->state = STATE_PAUSED;
            }
            break;

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

static void DrawGameWorld(GameData *game)
{
    DrawBackgroundGrid(game->camera);
    ParticlePoolDraw(&game->particles);

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

    XPPoolDraw(&game->xp);
    EnemyPoolDraw(&game->enemies);
    ProjectilePoolDraw(&game->projectiles);
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
                DrawText("A/D or Left/Right to select - ENTER to confirm - ESC to go back", SCREEN_WIDTH/2 - MeasureText("A/D or Left/Right to select - ENTER to confirm - ESC to go back", 16)/2, SCREEN_HEIGHT - 40, 16, GRAY);
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
                BeginMode2D(game->camera);
                    DrawGameWorld(game);
                EndMode2D();
                DrawHUD(game);
                DrawTutorial(game);

                // Achievement notification popup
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
