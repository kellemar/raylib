#include "raylib.h"
#include "types.h"
#include "game.h"
#include "audio.h"

// Note: GameData is large due to object pools - must be static or heap-allocated

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "NEON VOID");
    InitAudioDevice();
    SetTargetFPS(60);

    AudioInit();

    static GameData game = { 0 };
    GameInit(&game);
    GameInitShaders(&game);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        GameUpdate(&game, dt);
        MusicUpdate();

        BeginDrawing();
            GameDraw(&game);
        EndDrawing();
    }

    GameCleanupShaders(&game);
    AudioCleanup();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
