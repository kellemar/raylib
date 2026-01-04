#include "game.h"
#include "types.h"

void GameInit(GameData *game)
{
    game->state = STATE_MENU;
    game->gameTime = 0.0f;
    game->score = 0;
    game->isPaused = false;
    PlayerInit(&game->player);
    ProjectilePoolInit(&game->projectiles);
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
                PlayerInit(&game->player);
                ProjectilePoolInit(&game->projectiles);
            }
            if (IsKeyPressed(KEY_ESCAPE)) CloseWindow();
            break;

        case STATE_PLAYING:
            game->gameTime += dt;
            PlayerUpdate(&game->player, dt, &game->projectiles);
            ProjectilePoolUpdate(&game->projectiles, dt);

            if (!game->player.alive)
            {
                game->state = STATE_GAMEOVER;
            }

            if (IsKeyPressed(KEY_ESCAPE)) game->state = STATE_PAUSED;

#ifdef DEBUG
            if (IsKeyPressed(KEY_P)) PlayerTakeDamage(&game->player, 10.0f);
#endif
            break;

        case STATE_PAUSED:
            if (IsKeyPressed(KEY_ESCAPE)) game->state = STATE_PLAYING;
            if (IsKeyPressed(KEY_Q)) game->state = STATE_MENU;
            break;

        case STATE_LEVELUP:
            break;

        case STATE_GAMEOVER:
            if (IsKeyPressed(KEY_ENTER)) game->state = STATE_MENU;
            break;
    }
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
            ProjectilePoolDraw(&game->projectiles);
            PlayerDraw(&game->player);
            DrawText(TextFormat("TIME: %.1f", game->gameTime), 10, 10, 20, NEON_WHITE);
            DrawText(TextFormat("SCORE: %d", game->score), 10, 40, 20, NEON_YELLOW);
            DrawText(TextFormat("LEVEL: %d", game->player.level), 10, 70, 20, NEON_CYAN);
            DrawText(TextFormat("HP: %.0f/%.0f", game->player.health, game->player.maxHealth), 10, 100, 20, NEON_GREEN);
            break;

        case STATE_PAUSED:
            ClearBackground(VOID_BLACK);
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 0, 0, 150 });
            DrawText("PAUSED", SCREEN_WIDTH/2 - MeasureText("PAUSED", 60)/2, 250, 60, NEON_YELLOW);
            DrawText("Press ESC to Resume", SCREEN_WIDTH/2 - MeasureText("Press ESC to Resume", 20)/2, 350, 20, NEON_CYAN);
            DrawText("Press Q to Quit to Menu", SCREEN_WIDTH/2 - MeasureText("Press Q to Quit to Menu", 20)/2, 400, 20, GRAY);
            break;

        case STATE_LEVELUP:
            ClearBackground(VOID_PURPLE);
            DrawText("LEVEL UP!", SCREEN_WIDTH/2 - MeasureText("LEVEL UP!", 60)/2, 250, 60, NEON_GREEN);
            break;

        case STATE_GAMEOVER:
            ClearBackground(VOID_BLACK);
            DrawText("GAME OVER", SCREEN_WIDTH/2 - MeasureText("GAME OVER", 60)/2, 200, 60, NEON_RED);
            DrawText(TextFormat("Final Score: %d", game->score), SCREEN_WIDTH/2 - MeasureText(TextFormat("Final Score: %d", game->score), 30)/2, 300, 30, NEON_YELLOW);
            DrawText("Press ENTER to Return to Menu", SCREEN_WIDTH/2 - MeasureText("Press ENTER to Return to Menu", 20)/2, 400, 20, NEON_CYAN);
            break;
    }
}
