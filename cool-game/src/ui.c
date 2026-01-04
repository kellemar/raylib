#include "ui.h"
#include "game.h"
#include "types.h"
#include "weapon.h"
#include "enemy.h"
#include "raylib.h"
#include <stdio.h>
#include <math.h>

void DrawHUD(GameData *game)
{
    int padding = 10;
    int lineHeight = 26;
    int startY = 10;

    DrawRectangle(5, 5, 180, 270, (Color){ 0, 0, 0, 150 });

    DrawText(TextFormat("TIME: %.1f", game->gameTime), padding, startY, 20, NEON_WHITE);
    DrawText(TextFormat("SCORE: %d", game->score), padding, startY + lineHeight, 20, NEON_YELLOW);

    // Score multiplier display (changes color based on value)
    Color multiplierColor = NEON_GREEN;
    if (game->scoreMultiplier >= MULTIPLIER_TIER_YELLOW) multiplierColor = NEON_YELLOW;
    if (game->scoreMultiplier >= MULTIPLIER_TIER_ORANGE) multiplierColor = NEON_ORANGE;
    if (game->scoreMultiplier >= MULTIPLIER_TIER_PINK) multiplierColor = NEON_PINK;
    DrawText(TextFormat("x%.1f", game->scoreMultiplier), padding + 120, startY + lineHeight, 16, multiplierColor);

    DrawText(TextFormat("LEVEL: %d", game->player.level), padding, startY + lineHeight * 2, 20, NEON_CYAN);

    float hpPercent = game->player.health / game->player.maxHealth;
    DrawRectangle(padding, startY + lineHeight * 3, 160, 16, (Color){ 50, 20, 20, 255 });
    DrawRectangle(padding, startY + lineHeight * 3, (int)(160 * hpPercent), 16, NEON_RED);
    DrawText(TextFormat("%.0f/%.0f", game->player.health, game->player.maxHealth), padding + 50, startY + lineHeight * 3, 16, NEON_WHITE);

    float xpPercent = (float)game->player.xp / (float)game->player.xpToNextLevel;
    DrawRectangle(padding, startY + lineHeight * 4, 160, 12, (Color){ 20, 20, 50, 255 });
    DrawRectangle(padding, startY + lineHeight * 4, (int)(160 * xpPercent), 12, NEON_PINK);
    DrawText(TextFormat("XP: %d/%d", game->player.xp, game->player.xpToNextLevel), padding, startY + lineHeight * 4 + 14, 16, NEON_PINK);

    DrawText(TextFormat("ENEMIES: %d", game->enemies.count), padding, startY + lineHeight * 5 + 10, 16, NEON_ORANGE);

    // Show dash cooldown indicator
    if (game->player.dashCooldown > 0.0f)
    {
        DrawText("DASH: ...", padding, startY + lineHeight * 6 + 10, 16, GRAY);
    }
    else
    {
        DrawText("DASH: READY", padding, startY + lineHeight * 6 + 10, 16, NEON_PINK);
    }

    // Show current weapon with color
    const char *weaponName = WeaponGetName(game->player.weapon.type);
    Color weaponColor = WeaponGetColor(game->player.weapon.type);
    DrawText(TextFormat("WEAPON: %s", weaponName), padding, startY + lineHeight * 7 + 10, 14, weaponColor);
    DrawText("[Q/E] Switch", padding, startY + lineHeight * 8 + 8, 12, GRAY);

    // Boss warning display
    if (game->bossWarningActive)
    {
        // Flashing "WARNING - BOSS INCOMING" text
        float flash = (sinf((float)GetTime() * 10.0f) + 1.0f) * 0.5f;
        unsigned char alpha = (unsigned char)(150 + 105 * flash);
        Color warningColor = (Color){ 255, 50, 50, alpha };

        const char *warningText = "!! BOSS INCOMING !!";
        int textWidth = MeasureText(warningText, 40);
        int centerX = SCREEN_WIDTH / 2 - textWidth / 2;
        int centerY = SCREEN_HEIGHT / 4;

        // Draw background
        DrawRectangle(centerX - 20, centerY - 10, textWidth + 40, 60, (Color){ 0, 0, 0, 180 });
        DrawText(warningText, centerX, centerY, 40, warningColor);

        // Countdown
        const char *countText = TextFormat("%.1f", game->bossWarningTimer);
        int countWidth = MeasureText(countText, 30);
        DrawText(countText, SCREEN_WIDTH / 2 - countWidth / 2, centerY + 45, 30, NEON_YELLOW);
    }

    // Boss health bar (top center of screen)
    Enemy *boss = EnemyPoolGetBoss(&game->enemies);
    if (boss)
    {
        int barWidth = 400;
        int barHeight = 20;
        int barX = SCREEN_WIDTH / 2 - barWidth / 2;
        int barY = 30;
        float healthPercent = boss->health / boss->maxHealth;

        // Background
        DrawRectangle(barX - 5, barY - 5, barWidth + 10, barHeight + 10, (Color){ 0, 0, 0, 200 });

        // Health bar
        DrawRectangle(barX, barY, barWidth, barHeight, (Color){ 80, 20, 80, 255 });
        DrawRectangle(barX, barY, (int)(barWidth * healthPercent), barHeight, (Color){ 200, 50, 200, 255 });

        // Border
        DrawRectangleLines(barX, barY, barWidth, barHeight, (Color){ 255, 100, 255, 255 });

        // Boss label
        const char *bossLabel = TextFormat("BOSS #%d", game->bossCount);
        int labelWidth = MeasureText(bossLabel, 24);
        DrawText(bossLabel, SCREEN_WIDTH / 2 - labelWidth / 2, barY - 25, 24, (Color){ 255, 100, 255, 255 });

        // Health text
        const char *healthText = TextFormat("%.0f / %.0f", boss->health, boss->maxHealth);
        int healthWidth = MeasureText(healthText, 16);
        DrawText(healthText, SCREEN_WIDTH / 2 - healthWidth / 2, barY + barHeight + 5, 16, NEON_WHITE);
    }
}

void DrawTutorial(GameData *game)
{
    // Only show tutorial for first 20 seconds
    if (game->tutorialTimer > 20.0f) return;

    // Fade out in last 5 seconds
    float alpha = 1.0f;
    if (game->tutorialTimer > 15.0f)
    {
        alpha = 1.0f - (game->tutorialTimer - 15.0f) / 5.0f;
    }

    int centerX = SCREEN_WIDTH / 2;
    int baseY = SCREEN_HEIGHT - 120;
    unsigned char a = (unsigned char)(alpha * 200);
    Color bgColor = (Color){ 0, 0, 0, (unsigned char)(alpha * 150) };
    Color textColor = (Color){ 255, 255, 255, a };
    Color highlightColor = (Color){ 100, 255, 255, a };
    Color dashColor = (Color){ 255, 100, 255, a };

    // Background box
    DrawRectangle(centerX - 220, baseY - 10, 440, 120, bgColor);

    // Control hints
    const char *moveText = "WASD / Arrow Keys - Move";
    const char *aimText = "Mouse - Aim";
    const char *dashText = "SPACE - Dash (invincible!)";
    const char *weaponText = "Q / E - Switch Weapon";
    const char *collectText = "Collect green crystals for XP";

    DrawText(moveText, centerX - MeasureText(moveText, 18)/2, baseY, 18, textColor);
    DrawText(aimText, centerX - MeasureText(aimText, 18)/2, baseY + 22, 18, textColor);
    DrawText(dashText, centerX - MeasureText(dashText, 18)/2, baseY + 44, 18, dashColor);
    DrawText(weaponText, centerX - MeasureText(weaponText, 18)/2, baseY + 66, 18, highlightColor);
    DrawText(collectText, centerX - MeasureText(collectText, 16)/2, baseY + 90, 16, highlightColor);
}
