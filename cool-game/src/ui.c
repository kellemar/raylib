#include "ui.h"
#include "game.h"
#include "types.h"
#include "raylib.h"
#include <stdio.h>

void DrawHUD(GameData *game)
{
    int padding = 10;
    int lineHeight = 26;
    int startY = 10;

    DrawRectangle(5, 5, 180, 200, (Color){ 0, 0, 0, 150 });

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
}
