#include "raylib.h"
#include "types.h"
#include "game.h"

_Static_assert(sizeof(GameData) < 1048576, "GameData exceeds 1MB - ensure it is static or heap-allocated");

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "NEON VOID");
    SetTargetFPS(60);

    static GameData game = { 0 };
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
