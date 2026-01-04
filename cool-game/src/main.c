#include "raylib.h"
#include "types.h"
#include "game.h"

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "NEON VOID");
    SetTargetFPS(60);

    GameData game = { 0 };
    GameInit(&game);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        GameUpdate(&game, dt);

        BeginDrawing();
            GameDraw(&game);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
