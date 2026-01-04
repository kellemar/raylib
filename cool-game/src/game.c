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
#define HIGHSCORE_FILE "highscore.dat"
#define SETTINGS_FILE "settings.dat"

static const Color GRID_COLOR = { 30, 25, 40, 100 };

// High score persistence
static int LoadHighScore(void)
{
    int score = 0;
    FILE *file = fopen(HIGHSCORE_FILE, "rb");
    if (file != NULL)
    {
        if (fread(&score, sizeof(int), 1, file) != 1)
        {
            score = 0;  // Handle read failure
        }
        fclose(file);
    }
    return score;
}

static void SaveHighScore(int score)
{
    FILE *file = fopen(HIGHSCORE_FILE, "wb");
    if (file != NULL)
    {
        fwrite(&score, sizeof(int), 1, file);
        fclose(file);
    }
}

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

        float pullRadius = p->radius * 5.0f;  // Pull radius is larger than collision radius

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

static void CheckProjectileEnemyCollisions(ProjectilePool *projectiles, EnemyPool *enemies, XPPool *xp, ParticlePool *particles, GameData *game)
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
                e->health -= p->damage;
                e->hitFlashTimer = 0.1f;

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
                    }

                    PlayGameSound(SOUND_EXPLOSION);
                    TriggerScreenShake(game, 3.0f, 0.15f);
                    e->active = false;
                    enemies->count--;
                    game->score += (int)(e->xpValue * 10 * game->scoreMultiplier);
                    game->killCount++;

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

static void CheckEnemyPlayerCollisions(EnemyPool *enemies, Player *player, ParticlePool *particles, GameData *game)
{
    if (!player->alive) return;
    if (player->invincibilityTimer > 0.0f) return;

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        Enemy *e = &enemies->enemies[i];
        if (!e->active) continue;

        if (CheckCircleCollision(player->pos, player->radius, e->pos, e->radius))
        {
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
                e->pos.x -= pushDir.x * 30.0f;
                e->pos.y -= pushDir.y * 30.0f;
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

static void DrawUpgradeOption(int index, Upgrade upgrade, float y)
{
    float boxWidth = 300.0f;
    float boxHeight = 80.0f;
    float boxX = SCREEN_WIDTH / 2.0f - boxWidth / 2.0f;

    Color boxColor = (Color){ 40, 20, 60, 230 };
    Color borderColor = NEON_PINK;

    DrawRectangle((int)boxX, (int)y, (int)boxWidth, (int)boxHeight, boxColor);
    DrawRectangleLinesEx((Rectangle){ boxX, y, boxWidth, boxHeight }, 2.0f, borderColor);

    char keyLabel[4];
    snprintf(keyLabel, sizeof(keyLabel), "[%d]", index + 1);
    DrawText(keyLabel, (int)(boxX + 15), (int)(y + 15), 24, NEON_CYAN);

    DrawText(upgrade.name, (int)(boxX + 60), (int)(y + 12), 22, NEON_WHITE);
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
    game->highScore = LoadHighScore();
    game->killCount = 0;
    game->scoreMultiplier = 1.0f;
    game->timeSinceLastHit = 0.0f;
    game->hitstopFrames = 0;
    game->timeScale = 1.0f;
    game->tutorialTimer = 0.0f;
    game->transitionTimer = 0.0f;
    game->fadeAlpha = 0.0f;
    game->settingsSelection = 0;

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
                // Start "Get Ready" transition
                game->state = STATE_STARTING;
                game->transitionTimer = 0.0f;
                game->fadeAlpha = 0.0f;
                // Music transition happens later when fading into game
            }
            if (IsKeyPressed(KEY_TAB))
            {
                game->settingsSelection = 0;
                game->state = STATE_SETTINGS;
            }
            if (IsKeyPressed(KEY_ESCAPE)) CloseWindow();
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
                PlayerInit(&game->player);
                ProjectilePoolInit(&game->projectiles);
                EnemyPoolInit(&game->enemies);
                XPPoolInit(&game->xp);
                ParticlePoolInit(&game->particles);
                InitCamera(game);
            }
            break;
        }

        case STATE_PLAYING:
            // Hitstop: freeze game for a few frames on big kills
            if (game->hitstopFrames > 0)
            {
                game->hitstopFrames--;
                break;  // Skip all updates during hitstop
            }

            // Apply time scale for slow-mo effects
            float scaledDt = dt * game->timeScale;

            // Near-death slow-mo: time slows when health < 25%
            float healthPercent = game->player.health / game->player.maxHealth;
            if (healthPercent < 0.25f && healthPercent > 0.0f)
            {
                game->timeScale = 0.5f;  // Half speed when near death
            }
            else
            {
                game->timeScale = 1.0f;  // Normal speed
            }

            // Chromatic aberration: intensity increases as health decreases below 50%
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

            // Update score multiplier (increases slowly while not hit)
            game->timeSinceLastHit += scaledDt;
            game->scoreMultiplier = 1.0f + (game->timeSinceLastHit / MULTIPLIER_GROWTH_RATE);
            if (game->scoreMultiplier > MULTIPLIER_MAX) game->scoreMultiplier = MULTIPLIER_MAX;

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
                EnemySpawn(&game->enemies, enemyType, spawnPos);
                game->spawnTimer = 0.0f;
            }

            // Apply black hole pull effect before collision checks
            ApplyBlackHolePull(&game->projectiles, &game->enemies, scaledDt);
            CheckProjectileEnemyCollisions(&game->projectiles, &game->enemies, &game->xp, &game->particles, game);
            CheckEnemyPlayerCollisions(&game->enemies, &game->player, &game->particles, game);

            int collectedXP = XPCollect(&game->xp, game->player.pos, XP_COLLECT_RADIUS);
            if (collectedXP > 0)
            {
                game->player.xp += collectedXP;
                PlayGameSound(SOUND_PICKUP);
            }

            if (CheckLevelUp(&game->player))
            {
                PlayGameSound(SOUND_LEVELUP);
                MusicPause();
                GenerateRandomUpgrades(game->upgradeOptions, 3);
                game->timeScale = 0.3f;  // Slow-mo during level up
                game->state = STATE_LEVELUP;
            }

            if (!game->player.alive)
            {
                MusicStop();
                // Update high score if beaten
                if (game->score > game->highScore)
                {
                    game->highScore = game->score;
                    SaveHighScore(game->highScore);
                }
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
                MusicResume();
                game->timeScale = 1.0f;  // Restore normal speed
                game->state = STATE_PLAYING;
            }
            else if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2))
            {
                ApplyUpgrade(game->upgradeOptions[1], &game->player);
                MusicResume();
                game->timeScale = 1.0f;  // Restore normal speed
                game->state = STATE_PLAYING;
            }
            else if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3))
            {
                ApplyUpgrade(game->upgradeOptions[2], &game->player);
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
            break;
    }
}

static void DrawGameWorld(GameData *game)
{
    DrawBackgroundGrid(game->camera);
    ParticlePoolDraw(&game->particles);
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
                DrawMenuStars();
                DrawText("NEON VOID", SCREEN_WIDTH/2 - MeasureText("NEON VOID", 60)/2, 200, 60, NEON_CYAN);
                DrawText(TextFormat("High Score: %d", game->highScore), SCREEN_WIDTH/2 - MeasureText(TextFormat("High Score: %d", game->highScore), 24)/2, 280, 24, NEON_YELLOW);
                DrawText("Press ENTER to Start", SCREEN_WIDTH/2 - MeasureText("Press ENTER to Start", 20)/2, 350, 20, NEON_PINK);
                DrawText("Press TAB for Settings", SCREEN_WIDTH/2 - MeasureText("Press TAB for Settings", 20)/2, 390, 20, NEON_CYAN);
                DrawText("Press ESC to Quit", SCREEN_WIDTH/2 - MeasureText("Press ESC to Quit", 20)/2, 430, 20, GRAY);
                DrawText("F1: Toggle Bloom | F2: Toggle CRT", SCREEN_WIDTH/2 - MeasureText("F1: Toggle Bloom | F2: Toggle CRT", 16)/2, 520, 16, (Color){ 100, 100, 100, 255 });
                break;

            case STATE_SETTINGS:
                DrawSettingsMenu(game);
                break;

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
                DrawText("GAME OVER", SCREEN_WIDTH/2 - MeasureText("GAME OVER", 60)/2, 200, 60, NEON_RED);
                DrawText(TextFormat("Final Score: %d", game->score), SCREEN_WIDTH/2 - MeasureText(TextFormat("Final Score: %d", game->score), 30)/2, 300, 30, NEON_YELLOW);
                DrawText(TextFormat("Enemies Killed: %d", game->killCount), SCREEN_WIDTH/2 - MeasureText(TextFormat("Enemies Killed: %d", game->killCount), 20)/2, 340, 20, NEON_ORANGE);
                DrawText(TextFormat("Level Reached: %d", game->player.level), SCREEN_WIDTH/2 - MeasureText(TextFormat("Level Reached: %d", game->player.level), 20)/2, 370, 20, NEON_CYAN);
                DrawText(TextFormat("Time Survived: %.1fs", game->gameTime), SCREEN_WIDTH/2 - MeasureText(TextFormat("Time Survived: %.1fs", game->gameTime), 20)/2, 400, 20, NEON_WHITE);
                DrawText("Press ENTER to Return to Menu", SCREEN_WIDTH/2 - MeasureText("Press ENTER to Return to Menu", 20)/2, 470, 20, NEON_CYAN);
                break;
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
