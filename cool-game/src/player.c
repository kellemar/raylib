#include "player.h"
#include "types.h"
#include "raymath.h"
#include <math.h>

void PlayerInit(Player *player)
{
    player->pos = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    player->vel = (Vector2){ 0.0f, 0.0f };
    player->aimDir = (Vector2){ 1.0f, 0.0f };
    player->radius = 15.0f;
    player->speed = 300.0f;
    player->health = 100.0f;
    player->maxHealth = 100.0f;
    player->invincibilityTimer = 0.0f;
    player->magnetRadius = 80.0f;
    player->level = 1;
    player->xp = 0;
    player->xpToNextLevel = 10;
    player->alive = true;
    WeaponInit(&player->weapon, WEAPON_PULSE_CANNON);
    // Initialize trail
    player->trailUpdateTimer = 0.0f;
    for (int i = 0; i < PLAYER_TRAIL_LENGTH; i++)
    {
        player->trailPositions[i] = player->pos;
    }
    // Initialize dash
    player->dashCooldown = 0.0f;
    player->dashTimer = 0.0f;
    player->isDashing = false;
    player->dashDir = (Vector2){ 0.0f, 0.0f };
    // Initialize upgrade stats
    player->armor = 0.0f;
    player->regen = 0.0f;
    player->regenTimer = 0.0f;
    player->xpMultiplier = 1.0f;
    player->knockbackMultiplier = 1.0f;
    player->dashDamage = 0.0f;
    player->vampirism = 0.0f;
    player->slowAuraRadius = 0.0f;
    player->slowAuraAmount = 0.0f;
}

void PlayerUpdate(Player *player, float dt, ProjectilePool *projectiles, Camera2D camera)
{
    if (!player->alive) return;

    // Handle health regeneration
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

    if (player->invincibilityTimer > 0.0f)
    {
        player->invincibilityTimer -= dt;
    }

    // Update dash cooldown
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
            // Dash movement: fast in dash direction
            float dashSpeed = 800.0f;
            player->pos = Vector2Add(player->pos, Vector2Scale(player->dashDir, dashSpeed * dt));
            // Keep invincible during dash
            if (player->invincibilityTimer < 0.1f)
            {
                player->invincibilityTimer = 0.1f;
            }
            // Skip normal movement during dash
            WeaponUpdate(&player->weapon, dt);
            return;
        }
    }

    // Trigger dash on SPACE or gamepad button
    bool dashPressed = IsKeyPressed(KEY_SPACE);
    if (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
    {
        dashPressed = true;
    }

    WeaponUpdate(&player->weapon, dt);

    Vector2 input = { 0.0f, 0.0f };

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) input.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) input.y += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) input.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) input.x += 1.0f;

    if (IsGamepadAvailable(0))
    {
        float axisX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        float axisY = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
        if (fabsf(axisX) > 0.2f) input.x = axisX;
        if (fabsf(axisY) > 0.2f) input.y = axisY;
    }

    float inputLength = Vector2Length(input);
    if (inputLength > 0.0f)
    {
        input = Vector2Scale(input, 1.0f / inputLength);
    }

    // Trigger dash if available
    if (dashPressed && player->dashCooldown <= 0.0f)
    {
        player->isDashing = true;
        player->dashTimer = 0.15f;       // Dash duration: 150ms
        player->dashCooldown = 1.5f;     // Cooldown: 1.5 seconds
        // Dash in movement direction, or aim direction if not moving
        if (inputLength > 0.0f)
        {
            player->dashDir = input;
        }
        else
        {
            player->dashDir = player->aimDir;
        }
        player->invincibilityTimer = 0.2f;  // Brief invincibility
    }

    player->vel = Vector2Scale(input, player->speed);
    player->pos = Vector2Add(player->pos, Vector2Scale(player->vel, dt));

    // Update trail positions (subtle effect)
    player->trailUpdateTimer += dt;
    float trailInterval = 0.04f;
    if (player->trailUpdateTimer >= trailInterval)
    {
        player->trailUpdateTimer = 0.0f;
        // Shift positions back
        for (int i = PLAYER_TRAIL_LENGTH - 1; i > 0; i--)
        {
            player->trailPositions[i] = player->trailPositions[i - 1];
        }
        player->trailPositions[0] = player->pos;
    }

    Vector2 mouseScreenPos = GetMousePosition();
    Vector2 mouseWorldPos = GetScreenToWorld2D(mouseScreenPos, camera);
    Vector2 toMouse = Vector2Subtract(mouseWorldPos, player->pos);
    float toMouseLength = Vector2Length(toMouse);

    if (IsGamepadAvailable(0))
    {
        float rightX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
        float rightY = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);
        float rightLen = sqrtf(rightX * rightX + rightY * rightY);
        if (rightLen > 0.3f)
        {
            player->aimDir = (Vector2){ rightX / rightLen, rightY / rightLen };
        }
        else if (toMouseLength > 1.0f)
        {
            player->aimDir = Vector2Scale(toMouse, 1.0f / toMouseLength);
        }
    }
    else if (toMouseLength > 1.0f)
    {
        player->aimDir = Vector2Scale(toMouse, 1.0f / toMouseLength);
    }

    WeaponFire(&player->weapon, projectiles, player->pos, player->aimDir, &player->pos);
}

void PlayerDraw(Player *player)
{
    if (!player->alive) return;

    // Draw dash trail (more prominent when dashing)
    if (player->isDashing)
    {
        for (int i = 0; i < 5; i++)
        {
            float t = (float)i * 0.2f;
            Vector2 trailPos = Vector2Subtract(player->pos, Vector2Scale(player->dashDir, t * 60.0f));
            float alpha = (1.0f - t) * 200.0f;
            float size = player->radius * (1.0f - t * 0.5f);
            DrawCircleV(trailPos, size, (Color){ 255, 100, 255, (unsigned char)alpha });
        }
    }

    // Draw trail (behind player) - subtle effect
    float velMagnitude = Vector2Length(player->vel);
    if (velMagnitude > 50.0f)  // Only show trail when moving faster
    {
        for (int i = PLAYER_TRAIL_LENGTH - 1; i >= 0; i--)
        {
            float t = (float)i / (float)PLAYER_TRAIL_LENGTH;
            float alpha = (1.0f - t) * 60.0f;  // Lower max alpha (was 150)
            float radius = player->radius * (0.6f - t * 0.4f);  // Smaller circles
            Color trailColor = (Color){ 50, 255, 255, (unsigned char)alpha };
            DrawCircleV(player->trailPositions[i], radius, trailColor);
        }
    }

    bool visible = true;
    if (player->invincibilityTimer > 0.0f)
    {
        visible = ((int)(player->invincibilityTimer * 10.0f) % 2) == 0;
    }

    if (visible)
    {
        DrawCircleV(player->pos, player->radius, NEON_CYAN);
        DrawCircleV(player->pos, player->radius * 0.6f, (Color){ 100, 255, 255, 200 });
        DrawCircleV(player->pos, player->radius * 0.3f, NEON_WHITE);

        Vector2 aimEnd = Vector2Add(player->pos, Vector2Scale(player->aimDir, player->radius + 12.0f));
        DrawLineEx(player->pos, aimEnd, 3.0f, NEON_PINK);
    }

    float healthBarWidth = 50.0f;
    float healthBarHeight = 6.0f;
    float healthBarX = player->pos.x - healthBarWidth / 2.0f;
    float healthBarY = player->pos.y + player->radius + 10.0f;
    float healthRatio = player->health / player->maxHealth;

    DrawRectangle((int)healthBarX, (int)healthBarY, (int)healthBarWidth, (int)healthBarHeight, (Color){ 80, 20, 20, 255 });
    DrawRectangle((int)healthBarX, (int)healthBarY, (int)(healthBarWidth * healthRatio), (int)healthBarHeight, NEON_GREEN);
    DrawRectangleLinesEx((Rectangle){ healthBarX, healthBarY, healthBarWidth, healthBarHeight }, 1.0f, NEON_WHITE);
}

void PlayerTakeDamage(Player *player, float damage)
{
    if (!player->alive) return;
    if (player->invincibilityTimer > 0.0f) return;

    // Apply armor damage reduction
    float actualDamage = damage - player->armor;
    if (actualDamage < 1.0f) actualDamage = 1.0f;  // Minimum 1 damage

    player->health -= actualDamage;
    player->invincibilityTimer = 0.5f;

    if (player->health <= 0.0f)
    {
        player->health = 0.0f;
        player->alive = false;
    }
}

void PlayerSwitchWeapon(Player *player, WeaponType type)
{
    if (type >= 0 && type < WEAPON_COUNT)
    {
        WeaponInit(&player->weapon, type);
    }
}

void PlayerCycleWeapon(Player *player, int direction)
{
    int newType = (int)player->weapon.type + direction;

    // Wrap around
    if (newType < 0) newType = WEAPON_COUNT - 1;
    if (newType >= WEAPON_COUNT) newType = 0;

    PlayerSwitchWeapon(player, (WeaponType)newType);
}
