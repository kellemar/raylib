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
    player->level = 1;
    player->xp = 0;
    player->xpToNextLevel = 10;
    player->alive = true;
}

void PlayerUpdate(Player *player, float dt)
{
    if (!player->alive) return;

    if (player->invincibilityTimer > 0.0f)
    {
        player->invincibilityTimer -= dt;
    }

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

    player->vel = Vector2Scale(input, player->speed);
    player->pos = Vector2Add(player->pos, Vector2Scale(player->vel, dt));

    if (player->pos.x < player->radius) player->pos.x = player->radius;
    if (player->pos.x > SCREEN_WIDTH - player->radius) player->pos.x = SCREEN_WIDTH - player->radius;
    if (player->pos.y < player->radius) player->pos.y = player->radius;
    if (player->pos.y > SCREEN_HEIGHT - player->radius) player->pos.y = SCREEN_HEIGHT - player->radius;

    Vector2 mousePos = GetMousePosition();
    Vector2 toMouse = Vector2Subtract(mousePos, player->pos);
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
}

void PlayerDraw(Player *player)
{
    if (!player->alive) return;

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

    player->health -= damage;
    player->invincibilityTimer = 0.5f;

    if (player->health <= 0.0f)
    {
        player->health = 0.0f;
        player->alive = false;
    }
}
