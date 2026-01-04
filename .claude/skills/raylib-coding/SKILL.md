---
name: raylib-coding
description: Create good raylib-supported games with expert  game development guidance covering C99 best practices, raylib conventions, game architecture, and performance optimization. Use when writing raylib games, creating game systems, working with raylib types (Vector2, Texture2D, Camera2D), or optimizing game performance. Triggers on raylib.h includes, game loop patterns, or raylib function calls (InitWindow, LoadTexture, DrawText, etc.).
---

# raylib Coding Expert

Expert guidance for writing high-quality raylib games in C99.

## Quick Reference

### Game Loop Pattern
```c
#include "raylib.h"

int main(void)
{
    // Initialization
    const int screenWidth = 800;
    const int screenHeight = 450;
    
    InitWindow(screenWidth, screenHeight, "Game Title");
    SetTargetFPS(60);
    
    // Load resources here
    Texture2D texture = LoadTexture("resources/sprite.png");
    
    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        float deltaTime = GetFrameTime();
        // Game logic here
        
        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);
            // Draw calls here
        EndDrawing();
    }
    
    // Cleanup
    UnloadTexture(texture);
    CloseWindow();
    
    return 0;
}
```

### Naming Conventions
```c
// Functions: TitleCase
InitWindow();
LoadTexture();
DrawCircle();

// Variables: lowerCase
int screenWidth = 800;
Vector2 playerPosition = { 0 };
float targetFrameTime = 0.016f;

// Macros/Defines: ALL_CAPS
#define MAX_ENEMIES 100
#define PLAYER_SPEED 200.0f

// Enums: TitleCase type, ALL_CAPS members
enum GameState { STATE_MENU, STATE_PLAYING, STATE_GAMEOVER };

// Structs: TitleCase
struct Player {
    Vector2 position;
    float speed;
    int health;
};
```

### Key Types
```c
// 2D position/direction
Vector2 position = { 100.0f, 200.0f };
Vector2 velocity = { 1.0f, 0.0f };

// Colors (RGBA 0-255)
Color customColor = { 255, 128, 0, 255 };
Color semiTransparent = Fade(RED, 0.5f);

// Rectangle (x, y, width, height)
Rectangle bounds = { 0.0f, 0.0f, 64.0f, 64.0f };

// Texture (GPU-side image)
Texture2D sprite = LoadTexture("sprite.png");

// 2D Camera
Camera2D camera = {
    .offset = { screenWidth/2.0f, screenHeight/2.0f },
    .target = { playerX, playerY },
    .rotation = 0.0f,
    .zoom = 1.0f
};
```

## Detailed References

| Topic | File | When to Read |
|-------|------|--------------|
| raylib Patterns & Architecture | [references/raylib.md](references/raylib.md) | Game architecture, resource management, cameras, audio |
| Performance & Optimization | [references/optimization.md](references/optimization.md) | Hot paths, profiling, cache efficiency |
| Memory Management | [references/memory.md](references/memory.md) | Pool allocators, leak prevention |
| Code Style & Conventions | [references/style.md](references/style.md) | Formatting, documentation |
| Safety & Security | [references/safety.md](references/safety.md) | Buffer safety, secure coding |
| C Patterns & Idioms | [references/patterns.md](references/patterns.md) | Error handling, data structures |

## raylib Module Overview

| Module | Header | Purpose | Key Functions |
|--------|--------|---------|---------------|
| Core | rcore.c | Window, input, timing | `InitWindow`, `WindowShouldClose`, `GetFrameTime`, `IsKeyPressed` |
| Shapes | rshapes.c | 2D primitives | `DrawRectangle`, `DrawCircle`, `CheckCollisionRecs` |
| Textures | rtextures.c | Images & textures | `LoadTexture`, `DrawTexture`, `UnloadTexture` |
| Text | rtext.c | Fonts & text | `DrawText`, `LoadFont`, `MeasureText` |
| Models | rmodels.c | 3D meshes & models | `LoadModel`, `DrawModel`, `LoadModelAnimations` |
| Audio | raudio.c | Sound & music | `InitAudioDevice`, `LoadSound`, `PlaySound`, `LoadMusicStream` |
| Math | raymath.h | Vector/matrix ops | `Vector2Add`, `Vector2Normalize`, `MatrixRotateZ` |

## Core Principles

1. **Load once, use many** - Load resources at init, not in game loop
2. **Match Load with Unload** - Every `LoadX()` needs corresponding `UnloadX()`
3. **Delta time everything** - Multiply movement by `GetFrameTime()` for frame-independence
4. **Initialize all variables** - raylib style requires explicit initialization
5. **Keep draw calls between Begin/End** - All rendering inside `BeginDrawing()`/`EndDrawing()`

## Anti-Patterns (BLOCKING)

| Mistake | Why It's Bad | Correct Approach |
|---------|--------------|------------------|
| `LoadTexture()` in game loop | Loads from disk every frame, destroys performance | Load once at init |
| Missing `UnloadTexture()` | GPU memory leak | Unload in cleanup section |
| `10.f` float literals | raylib style violation | Use `10.0f` |
| Tabs in code | raylib uses 4 spaces | Configure editor for spaces |
| `++i` increment | raylib uses postfix | Use `i++` |
| Movement without deltaTime | Frame-rate dependent physics | `position.x += speed*GetFrameTime()` |
| Comments ending with `.` | raylib style violation | Omit trailing period |
| Multi-line function declarations | Breaks rlparser tool | Keep on single line |

## Build Commands

```bash
# Compile single file (Linux/macOS)
gcc game.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o game

# Compile single file (macOS with framework)
gcc game.c -framework IOKit -framework Cocoa -lraylib -o game

# Using Makefile (recommended)
make

# CMake build
cmake -B build && cmake --build build

# Web build (requires Emscripten)
emcc game.c -s USE_GLFW=3 -s ASYNCIFY -o game.html
```

## Common Patterns

### Input Handling
```c
// Keyboard
if (IsKeyPressed(KEY_SPACE)) Jump();
if (IsKeyDown(KEY_RIGHT)) player.x += speed*GetFrameTime();

// Mouse
Vector2 mousePos = GetMousePosition();
if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) Shoot();

// Gamepad
if (IsGamepadAvailable(0))
{
    if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) Jump();
    float axisX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
}
```

### Collision Detection
```c
// Rectangle collision
Rectangle playerRec = { player.x, player.y, 32, 32 };
Rectangle enemyRec = { enemy.x, enemy.y, 32, 32 };
if (CheckCollisionRecs(playerRec, enemyRec)) HandleCollision();

// Circle collision
if (CheckCollisionCircles(pos1, radius1, pos2, radius2)) HandleCollision();

// Point in rectangle
if (CheckCollisionPointRec(mousePos, buttonRec)) buttonHovered = true;
```

### Resource Management
```c
// Textures
Texture2D tex = LoadTexture("sprite.png");
if (tex.id == 0) TraceLog(LOG_ERROR, "Failed to load texture");
// ... use texture ...
UnloadTexture(tex);

// Sounds
Sound sfx = LoadSound("jump.wav");
PlaySound(sfx);
UnloadSound(sfx);

// Music (streaming)
Music music = LoadMusicStream("background.ogg");
PlayMusicStream(music);
// In game loop:
UpdateMusicStream(music);
// Cleanup:
UnloadMusicStream(music);
```
