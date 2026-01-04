#include "game.h"
#include "types.h"
#include "utils.h"
#include "ui.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define XP_COLLECT_RADIUS 15.0f
#define CAMERA_LERP_SPEED 5.0f

static int GetXPForLevel(int level)
{
    return 10 * level * level;
}

static Vector2 GetSpawnPosition(Vector2 playerPos)
{
    float angle = (float)(rand() % 360) * DEG2RAD;
    float distance = 400.0f + (float)(rand() % 200);
    Vector2 offset = { cosf(angle) * distance, sinf(angle) * distance };
    Vector2 spawnPos = Vector2Add(playerPos, offset);

    if (spawnPos.x < -50.0f) spawnPos.x = -50.0f;
    if (spawnPos.x > SCREEN_WIDTH + 50.0f) spawnPos.x = SCREEN_WIDTH + 50.0f;
    if (spawnPos.y < -50.0f) spawnPos.y = -50.0f;
    if (spawnPos.y > SCREEN_HEIGHT + 50.0f) spawnPos.y = SCREEN_HEIGHT + 50.0f;

    return spawnPos;
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

                SpawnHitParticles(particles, p->pos, NEON_YELLOW, 5);

                if (!p->pierce)
                {
                    p->active = false;
                    projectiles->count--;
                }

                if (e->health <= 0.0f)
                {
                    XPSpawn(xp, e->pos, e->xpValue);
                    SpawnExplosion(particles, e->pos, NEON_ORANGE, 15);
                    TriggerScreenShake(game, 3.0f, 0.15f);
                    e->active = false;
                    enemies->count--;
                    game->score += e->xpValue * 10;
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
            SpawnHitParticles(particles, player->pos, NEON_RED, 10);
            TriggerScreenShake(game, 8.0f, 0.25f);

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
    if (intensity > game->shakeIntensity)
    {
        game->shakeIntensity = intensity;
    }
    if (duration > game->shakeDuration)
    {
        game->shakeDuration = duration;
    }
}

void GameInit(GameData *game)
{
    game->state = STATE_MENU;
    game->gameTime = 0.0f;
    game->score = 0;
    game->isPaused = false;
    game->spawnTimer = 0.0f;
    PlayerInit(&game->player);
    ProjectilePoolInit(&game->projectiles);
    EnemyPoolInit(&game->enemies);
    XPPoolInit(&game->xp);
    ParticlePoolInit(&game->particles);
    InitCamera(game);
}

void GameUpdate(GameData *game, float dt)
{
    switch (game->state)
    {
        case STATE_MENU:
            if (IsKeyPressed(KEY_ENTER))
            {
                game->state = STATE_PLAYING;
                game->gameTime = 0.0f;
                game->score = 0;
                game->spawnTimer = 0.0f;
                PlayerInit(&game->player);
                ProjectilePoolInit(&game->projectiles);
                EnemyPoolInit(&game->enemies);
                XPPoolInit(&game->xp);
                ParticlePoolInit(&game->particles);
                InitCamera(game);
            }
            if (IsKeyPressed(KEY_ESCAPE)) CloseWindow();
            break;

        case STATE_PLAYING:
            game->gameTime += dt;
            PlayerUpdate(&game->player, dt, &game->projectiles, game->camera);
            ProjectilePoolUpdate(&game->projectiles, dt);
            EnemyPoolUpdate(&game->enemies, game->player.pos, dt);
            XPPoolUpdate(&game->xp, game->player.pos, game->player.magnetRadius, dt);
            ParticlePoolUpdate(&game->particles, dt);
            UpdateGameCamera(game, dt);

            game->spawnTimer += dt;
            float spawnInterval = GetSpawnInterval(game->gameTime);
            if (game->spawnTimer >= spawnInterval)
            {
                Vector2 spawnPos = GetSpawnPosition(game->player.pos);
                EnemySpawn(&game->enemies, ENEMY_CHASER, spawnPos);
                game->spawnTimer = 0.0f;
            }

            CheckProjectileEnemyCollisions(&game->projectiles, &game->enemies, &game->xp, &game->particles, game);
            CheckEnemyPlayerCollisions(&game->enemies, &game->player, &game->particles, game);

            int collectedXP = XPCollect(&game->xp, game->player.pos, XP_COLLECT_RADIUS);
            game->player.xp += collectedXP;

            if (CheckLevelUp(&game->player))
            {
                GenerateRandomUpgrades(game->upgradeOptions, 3);
                game->state = STATE_LEVELUP;
            }

            if (!game->player.alive)
            {
                game->state = STATE_GAMEOVER;
            }

            if (IsKeyPressed(KEY_ESCAPE)) game->state = STATE_PAUSED;
            break;

        case STATE_PAUSED:
            if (IsKeyPressed(KEY_ESCAPE)) game->state = STATE_PLAYING;
            if (IsKeyPressed(KEY_Q)) game->state = STATE_MENU;
            break;

        case STATE_LEVELUP:
            if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1))
            {
                ApplyUpgrade(game->upgradeOptions[0], &game->player);
                game->state = STATE_PLAYING;
            }
            else if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2))
            {
                ApplyUpgrade(game->upgradeOptions[1], &game->player);
                game->state = STATE_PLAYING;
            }
            else if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3))
            {
                ApplyUpgrade(game->upgradeOptions[2], &game->player);
                game->state = STATE_PLAYING;
            }
            break;

        case STATE_GAMEOVER:
            if (IsKeyPressed(KEY_ENTER)) game->state = STATE_MENU;
            break;
    }
}

static void DrawGameWorld(GameData *game)
{
    ParticlePoolDraw(&game->particles);
    XPPoolDraw(&game->xp);
    EnemyPoolDraw(&game->enemies);
    ProjectilePoolDraw(&game->projectiles);
    PlayerDraw(&game->player);
}

void GameDraw(GameData *game)
{
    switch (game->state)
    {
        case STATE_MENU:
            ClearBackground(VOID_BLACK);
            DrawText("NEON VOID", SCREEN_WIDTH/2 - MeasureText("NEON VOID", 60)/2, 200, 60, NEON_CYAN);
            DrawText("Press ENTER to Start", SCREEN_WIDTH/2 - MeasureText("Press ENTER to Start", 20)/2, 350, 20, NEON_PINK);
            DrawText("Press ESC to Quit", SCREEN_WIDTH/2 - MeasureText("Press ESC to Quit", 20)/2, 400, 20, GRAY);
            break;

        case STATE_PLAYING:
            ClearBackground(VOID_BLACK);
            BeginMode2D(game->camera);
                DrawGameWorld(game);
            EndMode2D();
            DrawHUD(game);
            break;

        case STATE_PAUSED:
            ClearBackground(VOID_BLACK);
            BeginMode2D(game->camera);
                DrawGameWorld(game);
            EndMode2D();
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 0, 0, 150 });
            DrawText("PAUSED", SCREEN_WIDTH/2 - MeasureText("PAUSED", 60)/2, 250, 60, NEON_YELLOW);
            DrawText("Press ESC to Resume", SCREEN_WIDTH/2 - MeasureText("Press ESC to Resume", 20)/2, 350, 20, NEON_CYAN);
            DrawText("Press Q to Quit to Menu", SCREEN_WIDTH/2 - MeasureText("Press Q to Quit to Menu", 20)/2, 400, 20, GRAY);
            break;

        case STATE_LEVELUP:
            ClearBackground(VOID_BLACK);
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
            ClearBackground(VOID_BLACK);
            DrawText("GAME OVER", SCREEN_WIDTH/2 - MeasureText("GAME OVER", 60)/2, 200, 60, NEON_RED);
            DrawText(TextFormat("Final Score: %d", game->score), SCREEN_WIDTH/2 - MeasureText(TextFormat("Final Score: %d", game->score), 30)/2, 300, 30, NEON_YELLOW);
            DrawText(TextFormat("Level Reached: %d", game->player.level), SCREEN_WIDTH/2 - MeasureText(TextFormat("Level Reached: %d", game->player.level), 20)/2, 350, 20, NEON_CYAN);
            DrawText(TextFormat("Time Survived: %.1fs", game->gameTime), SCREEN_WIDTH/2 - MeasureText(TextFormat("Time Survived: %.1fs", game->gameTime), 20)/2, 380, 20, NEON_WHITE);
            DrawText("Press ENTER to Return to Menu", SCREEN_WIDTH/2 - MeasureText("Press ENTER to Return to Menu", 20)/2, 450, 20, NEON_CYAN);
            break;
    }
}
