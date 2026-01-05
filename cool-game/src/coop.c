#include "coop.h"
#include "types.h"
#include "projectile.h"
#include "raymath.h"
#include <math.h>
#include <string.h>

// Forward declaration for GetXPForLevel (defined in game.c)
static int GetCoopXPForLevel(int level)
{
    return 10 * level * level;
}

void CoopStateInit(CoopState *coop, GameMode mode)
{
    memset(coop, 0, sizeof(CoopState));

    coop->playerCount = (mode == GAME_MODE_COOP) ? 2 : 1;
    coop->sharedXP = 0;
    coop->sharedLevel = 1;
    coop->sharedXPToNextLevel = GetCoopXPForLevel(1);
    coop->upgradeSelector = 0;
    coop->levelUpPending = false;
    coop->graceTimer = 0.0f;
    coop->bothDead = false;

    // Initialize player indices and input devices
    for (int i = 0; i < MAX_COOP_PLAYERS; i++)
    {
        coop->players[i].playerIndex = i;
        coop->players[i].revive.needsRevive = false;
        coop->players[i].revive.reviveProgress = 0.0f;
        coop->players[i].revive.deathPos = (Vector2){ 0.0f, 0.0f };
        coop->players[i].revive.reviveCount = 0;
    }

    // P1 defaults to keyboard+mouse or gamepad 0
    coop->players[0].inputDevice = IsGamepadAvailable(0) ? INPUT_GAMEPAD_0 : INPUT_KEYBOARD_P1;

    // P2 defaults to gamepad 1 or keyboard (arrows+IJKL)
    if (mode == GAME_MODE_COOP)
    {
        coop->players[1].inputDevice = IsGamepadAvailable(1) ? INPUT_GAMEPAD_1 : INPUT_KEYBOARD_P2;
    }
}

void CoopStateCleanup(CoopState *coop)
{
    CoopCleanupCameras(coop);
}

void CoopInitPlayers(CoopState *coop, CharacterType p1Char, CharacterType p2Char)
{
    // Initialize P1 with selected character
    PlayerInitWithCharacter(&coop->players[0].player, p1Char);
    coop->players[0].revive.needsRevive = false;
    coop->players[0].revive.reviveProgress = 0.0f;
    coop->players[0].revive.reviveCount = 0;

    // In solo mode, P1 starts at screen center
    // In co-op, both players start offset from center
    if (coop->playerCount == 2)
    {
        // P1 on left side
        coop->players[0].player.pos.x = SCREEN_WIDTH / 2.0f - 100.0f;
        coop->players[0].player.pos.y = SCREEN_HEIGHT / 2.0f;

        // Initialize P2
        PlayerInitWithCharacter(&coop->players[1].player, p2Char);
        coop->players[1].revive.needsRevive = false;
        coop->players[1].revive.reviveProgress = 0.0f;
        coop->players[1].revive.reviveCount = 0;

        // P2 on right side
        coop->players[1].player.pos.x = SCREEN_WIDTH / 2.0f + 100.0f;
        coop->players[1].player.pos.y = SCREEN_HEIGHT / 2.0f;

        // Different colors for P2 to distinguish from P1 (cyan) and enemies (red/orange)
        coop->players[1].player.primaryColor = NEON_GREEN;
        coop->players[1].player.secondaryColor = NEON_YELLOW;
    }

    // Reset shared progression
    coop->sharedXP = 0;
    coop->sharedLevel = 1;
    coop->sharedXPToNextLevel = GetCoopXPForLevel(1);
    coop->upgradeSelector = 0;
    coop->levelUpPending = false;
    coop->graceTimer = 0.0f;
    coop->bothDead = false;
}

void CoopResetPlayers(CoopState *coop)
{
    for (int i = 0; i < coop->playerCount; i++)
    {
        coop->players[i].player.alive = true;
        coop->players[i].player.health = coop->players[i].player.maxHealth;
        coop->players[i].revive.needsRevive = false;
        coop->players[i].revive.reviveProgress = 0.0f;
    }

    coop->sharedXP = 0;
    coop->sharedLevel = 1;
    coop->sharedXPToNextLevel = GetCoopXPForLevel(1);
    coop->graceTimer = 0.0f;
    coop->bothDead = false;
}

Player* CoopGetPlayer(CoopState *coop, int index)
{
    if (index < 0 || index >= coop->playerCount) return NULL;
    return &coop->players[index].player;
}

int CoopGetAlivePlayerCount(CoopState *coop)
{
    int count = 0;
    for (int i = 0; i < coop->playerCount; i++)
    {
        if (coop->players[i].player.alive && !coop->players[i].revive.needsRevive)
        {
            count++;
        }
    }
    return count;
}

bool CoopIsPlayerAlive(CoopState *coop, int index)
{
    if (index < 0 || index >= coop->playerCount) return false;
    return coop->players[index].player.alive && !coop->players[index].revive.needsRevive;
}

// P2 keyboard controls (arrows + IJKL for aim)
static void UpdateP2KeyboardInput(Player *player, float dt)
{
    Vector2 input = { 0.0f, 0.0f };

    // Arrow keys for movement
    if (IsKeyDown(KEY_UP)) input.y -= 1.0f;
    if (IsKeyDown(KEY_DOWN)) input.y += 1.0f;
    if (IsKeyDown(KEY_LEFT)) input.x -= 1.0f;
    if (IsKeyDown(KEY_RIGHT)) input.x += 1.0f;

    float inputLength = Vector2Length(input);
    if (inputLength > 0.0f)
    {
        input = Vector2Scale(input, 1.0f / inputLength);
    }

    player->vel = Vector2Scale(input, player->speed);
    player->pos = Vector2Add(player->pos, Vector2Scale(player->vel, dt));

    // IJKL for 8-directional aim
    Vector2 aim = { 0.0f, 0.0f };
    if (IsKeyDown(KEY_I)) aim.y -= 1.0f;  // Up
    if (IsKeyDown(KEY_K)) aim.y += 1.0f;  // Down
    if (IsKeyDown(KEY_J)) aim.x -= 1.0f;  // Left
    if (IsKeyDown(KEY_L)) aim.x += 1.0f;  // Right

    float aimLength = Vector2Length(aim);
    if (aimLength > 0.0f)
    {
        player->aimDir = Vector2Scale(aim, 1.0f / aimLength);
    }

    // Right Shift for dash
    if (IsKeyPressed(KEY_RIGHT_SHIFT) && player->dashCooldown <= 0.0f)
    {
        player->isDashing = true;
        player->dashTimer = 0.15f;
        player->dashCooldown = 1.5f;
        if (inputLength > 0.0f)
        {
            player->dashDir = input;
        }
        else
        {
            player->dashDir = player->aimDir;
        }
        player->invincibilityTimer = 0.2f;
    }
}

// Gamepad input for specific player
static void UpdateGamepadInput(Player *player, int gamepadId, float dt)
{
    if (!IsGamepadAvailable(gamepadId)) return;

    Vector2 input = { 0.0f, 0.0f };

    // Left stick for movement
    float axisX = GetGamepadAxisMovement(gamepadId, GAMEPAD_AXIS_LEFT_X);
    float axisY = GetGamepadAxisMovement(gamepadId, GAMEPAD_AXIS_LEFT_Y);
    if (fabsf(axisX) > 0.2f) input.x = axisX;
    if (fabsf(axisY) > 0.2f) input.y = axisY;

    float inputLength = Vector2Length(input);
    if (inputLength > 0.0f)
    {
        input = Vector2Scale(input, 1.0f / inputLength);
    }

    player->vel = Vector2Scale(input, player->speed);
    player->pos = Vector2Add(player->pos, Vector2Scale(player->vel, dt));

    // Right stick for aim
    float aimX = GetGamepadAxisMovement(gamepadId, GAMEPAD_AXIS_RIGHT_X);
    float aimY = GetGamepadAxisMovement(gamepadId, GAMEPAD_AXIS_RIGHT_Y);
    if (fabsf(aimX) > 0.2f || fabsf(aimY) > 0.2f)
    {
        Vector2 aim = { aimX, aimY };
        float aimLength = Vector2Length(aim);
        if (aimLength > 0.2f)
        {
            player->aimDir = Vector2Scale(aim, 1.0f / aimLength);
        }
    }

    // A button for dash
    if (IsGamepadButtonPressed(gamepadId, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) && player->dashCooldown <= 0.0f)
    {
        player->isDashing = true;
        player->dashTimer = 0.15f;
        player->dashCooldown = 1.5f;
        if (inputLength > 0.0f)
        {
            player->dashDir = input;
        }
        else
        {
            player->dashDir = player->aimDir;
        }
        player->invincibilityTimer = 0.2f;
    }

    // LB/RB for weapon switch
    if (IsGamepadButtonPressed(gamepadId, GAMEPAD_BUTTON_LEFT_TRIGGER_1))
    {
        PlayerCycleWeapon(player, -1);
    }
    if (IsGamepadButtonPressed(gamepadId, GAMEPAD_BUTTON_RIGHT_TRIGGER_1))
    {
        PlayerCycleWeapon(player, 1);
    }
}

void CoopUpdatePlayerInput(CoopPlayer *cp, float dt, ProjectilePool *projectiles, Camera2D camera)
{
    Player *player = &cp->player;

    if (!player->alive || cp->revive.needsRevive) return;

    // Update timers
    if (player->invincibilityTimer > 0.0f)
    {
        player->invincibilityTimer -= dt;
    }
    if (player->dashCooldown > 0.0f)
    {
        player->dashCooldown -= dt;
    }

    // Handle active dash
    if (player->isDashing)
    {
        player->dashTimer -= dt;
        if (player->dashTimer <= 0.0f)
        {
            player->isDashing = false;
            player->dashTimer = 0.0f;
        }
        else
        {
            float dashSpeed = 800.0f;
            player->pos = Vector2Add(player->pos, Vector2Scale(player->dashDir, dashSpeed * dt));
            if (player->invincibilityTimer < 0.1f)
            {
                player->invincibilityTimer = 0.1f;
            }
            WeaponUpdate(&player->weapon, dt);
            return;
        }
    }

    WeaponUpdate(&player->weapon, dt);

    // Input based on device
    switch (cp->inputDevice)
    {
        case INPUT_KEYBOARD_P1:
            // Use standard PlayerUpdate for P1 keyboard (handled elsewhere)
            PlayerUpdate(player, dt, projectiles, camera);
            return;

        case INPUT_KEYBOARD_P2:
            UpdateP2KeyboardInput(player, dt);
            break;

        case INPUT_GAMEPAD_0:
            UpdateGamepadInput(player, 0, dt);
            break;

        case INPUT_GAMEPAD_1:
            UpdateGamepadInput(player, 1, dt);
            break;
    }

    // Auto-fire for all players
    if (WeaponCanFire(&player->weapon))
    {
        WeaponFire(&player->weapon, projectiles, player->pos, player->aimDir, &player->pos);
    }

    // Handle regeneration
    if (player->regen > 0.0f && player->health < player->maxHealth)
    {
        player->regenTimer += dt;
        if (player->regenTimer >= 1.0f)
        {
            player->regenTimer -= 1.0f;
            player->health += player->regen;
            if (player->health > player->maxHealth)
            {
                player->health = player->maxHealth;
            }
        }
    }
}

void CoopUpdateInput(CoopState *coop, float dt, ProjectilePool *projectiles)
{
    for (int i = 0; i < coop->playerCount; i++)
    {
        CoopUpdatePlayerInput(&coop->players[i], dt, projectiles, coop->cameras[i].cam);
    }
}

void CoopInitCameras(CoopState *coop)
{
    for (int i = 0; i < coop->playerCount; i++)
    {
        // Create viewport render texture (half screen width in co-op, full in solo)
        int vpWidth = (coop->playerCount == 2) ? VIEWPORT_WIDTH : SCREEN_WIDTH;
        int vpHeight = VIEWPORT_HEIGHT;

        coop->cameras[i].viewport = LoadRenderTexture(vpWidth, vpHeight);

        // Source rect (flipped Y for OpenGL)
        coop->cameras[i].sourceRect = (Rectangle){
            0, 0,
            (float)vpWidth,
            -(float)vpHeight  // Flip Y
        };

        // Destination rect (where to draw on screen)
        if (coop->playerCount == 2)
        {
            coop->cameras[i].destRect = (Rectangle){
                (float)(i * VIEWPORT_WIDTH), 0,
                (float)VIEWPORT_WIDTH, (float)VIEWPORT_HEIGHT
            };
        }
        else
        {
            coop->cameras[i].destRect = (Rectangle){
                0, 0,
                (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT
            };
        }

        // Initialize camera
        coop->cameras[i].cam.target = coop->players[i].player.pos;
        coop->cameras[i].cam.offset = (Vector2){
            (float)vpWidth / 2.0f,
            (float)vpHeight / 2.0f
        };
        coop->cameras[i].cam.rotation = 0.0f;
        coop->cameras[i].cam.zoom = 1.0f;
    }
}

void CoopUpdateCameras(CoopState *coop, float dt)
{
    float lerpSpeed = 5.0f;

    for (int i = 0; i < coop->playerCount; i++)
    {
        Vector2 targetPos;

        if (coop->players[i].revive.needsRevive)
        {
            // Camera stays on death position when dead
            targetPos = coop->players[i].revive.deathPos;
        }
        else
        {
            targetPos = coop->players[i].player.pos;
        }

        // Smooth lerp toward target
        coop->cameras[i].cam.target = Vector2Lerp(
            coop->cameras[i].cam.target,
            targetPos,
            lerpSpeed * dt
        );
    }
}

void CoopCleanupCameras(CoopState *coop)
{
    for (int i = 0; i < MAX_COOP_PLAYERS; i++)
    {
        if (coop->cameras[i].viewport.id != 0)
        {
            UnloadRenderTexture(coop->cameras[i].viewport);
            coop->cameras[i].viewport.id = 0;
        }
    }
}

void CoopUpdateRevive(CoopState *coop, float dt)
{
    if (coop->playerCount < 2) return;

    for (int dead = 0; dead < coop->playerCount; dead++)
    {
        if (!coop->players[dead].revive.needsRevive) continue;

        // Find alive player
        int alive = (dead == 0) ? 1 : 0;
        if (coop->players[alive].revive.needsRevive) continue;

        // Check proximity
        float dist = Vector2Distance(
            coop->players[alive].player.pos,
            coop->players[dead].revive.deathPos
        );

        if (dist <= REVIVE_RANGE)
        {
            // Progress revive
            coop->players[dead].revive.reviveProgress += dt;

            if (coop->players[dead].revive.reviveProgress >= REVIVE_TIME)
            {
                CoopRespawnPlayer(coop, dead);
            }
        }
        else
        {
            // Reset progress if out of range
            coop->players[dead].revive.reviveProgress = 0.0f;
        }
    }
}

bool CoopCheckReviveProximity(CoopState *coop, int aliveIndex, int deadIndex)
{
    if (aliveIndex < 0 || aliveIndex >= coop->playerCount) return false;
    if (deadIndex < 0 || deadIndex >= coop->playerCount) return false;

    float dist = Vector2Distance(
        coop->players[aliveIndex].player.pos,
        coop->players[deadIndex].revive.deathPos
    );

    return dist <= REVIVE_RANGE;
}

void CoopRespawnPlayer(CoopState *coop, int playerIndex)
{
    if (playerIndex < 0 || playerIndex >= coop->playerCount) return;

    CoopPlayer *cp = &coop->players[playerIndex];
    Player *player = &cp->player;

    // Calculate respawn HP (decreases with each revive, min 25%)
    float baseHpPercent = 0.5f - (float)cp->revive.reviveCount * 0.1f;
    if (baseHpPercent < 0.25f) baseHpPercent = 0.25f;

    player->health = player->maxHealth * baseHpPercent;
    player->alive = true;
    player->pos = cp->revive.deathPos;
    player->invincibilityTimer = 2.0f;  // 2 seconds of invincibility

    // Reset revive state
    cp->revive.needsRevive = false;
    cp->revive.reviveProgress = 0.0f;
    cp->revive.reviveCount++;
}

bool CoopCheckTotalPartyKill(CoopState *coop, float dt)
{
    if (coop->playerCount < 2) return false;

    // Check if both players need revive
    bool allDead = true;
    for (int i = 0; i < coop->playerCount; i++)
    {
        if (!coop->players[i].revive.needsRevive)
        {
            allDead = false;
            break;
        }
    }

    if (allDead)
    {
        if (!coop->bothDead)
        {
            // Just became all dead - start grace timer
            coop->bothDead = true;
            coop->graceTimer = REVIVE_GRACE_PERIOD;
        }
        else
        {
            // Count down grace timer
            coop->graceTimer -= dt;
            if (coop->graceTimer <= 0.0f)
            {
                return true;  // Total party kill!
            }
        }
    }
    else
    {
        coop->bothDead = false;
        coop->graceTimer = 0.0f;
    }

    return false;
}

void CoopAddXP(CoopState *coop, int amount)
{
    // Apply XP multiplier from first alive player (or P1)
    float mult = 1.0f;
    for (int i = 0; i < coop->playerCount; i++)
    {
        if (!coop->players[i].revive.needsRevive)
        {
            mult = coop->players[i].player.xpMultiplier;
            break;
        }
    }

    coop->sharedXP += (int)(amount * mult);

    // Update individual player XP for display purposes
    for (int i = 0; i < coop->playerCount; i++)
    {
        coop->players[i].player.xp = coop->sharedXP;
        coop->players[i].player.xpToNextLevel = coop->sharedXPToNextLevel;
        coop->players[i].player.level = coop->sharedLevel;
    }
}

bool CoopCheckLevelUp(CoopState *coop)
{
    if (coop->sharedXP >= coop->sharedXPToNextLevel)
    {
        coop->sharedLevel++;
        coop->sharedXPToNextLevel = GetCoopXPForLevel(coop->sharedLevel);
        coop->levelUpPending = true;

        // Sync to players
        for (int i = 0; i < coop->playerCount; i++)
        {
            coop->players[i].player.level = coop->sharedLevel;
            coop->players[i].player.xpToNextLevel = coop->sharedXPToNextLevel;
        }

        return true;
    }
    return false;
}

void CoopApplyUpgrade(CoopState *coop, int upgradeType)
{
    // Apply upgrade to ALL players (shared progression)
    for (int i = 0; i < coop->playerCount; i++)
    {
        ApplyUpgrade((UpgradeType)upgradeType, &coop->players[i].player);
    }

    // Alternate selector for next upgrade
    coop->upgradeSelector = (coop->upgradeSelector + 1) % coop->playerCount;
    coop->levelUpPending = false;
}

float CoopGetSpawnMultiplier(CoopState *coop)
{
    return (coop->playerCount == 2) ? COOP_SPAWN_MULTIPLIER : 1.0f;
}

float CoopGetHealthMultiplier(CoopState *coop)
{
    return (coop->playerCount == 2) ? COOP_HEALTH_MULTIPLIER : 1.0f;
}

float CoopGetBossHealthMultiplier(CoopState *coop)
{
    return (coop->playerCount == 2) ? COOP_BOSS_HEALTH_MULTIPLIER : 1.0f;
}

Vector2 CoopGetNearestPlayerPos(CoopState *coop, Vector2 fromPos)
{
    float minDist = 999999.0f;
    Vector2 nearestPos = coop->players[0].player.pos;

    for (int i = 0; i < coop->playerCount; i++)
    {
        if (coop->players[i].revive.needsRevive) continue;

        float dist = Vector2DistanceSqr(fromPos, coop->players[i].player.pos);
        if (dist < minDist)
        {
            minDist = dist;
            nearestPos = coop->players[i].player.pos;
        }
    }

    return nearestPos;
}

int CoopGetNearestPlayerIndex(CoopState *coop, Vector2 fromPos)
{
    float minDist = 999999.0f;
    int nearestIndex = 0;

    for (int i = 0; i < coop->playerCount; i++)
    {
        if (coop->players[i].revive.needsRevive) continue;

        float dist = Vector2DistanceSqr(fromPos, coop->players[i].player.pos);
        if (dist < minDist)
        {
            minDist = dist;
            nearestIndex = i;
        }
    }

    return nearestIndex;
}
