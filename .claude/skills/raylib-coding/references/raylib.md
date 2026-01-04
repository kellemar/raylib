# raylib Patterns & Architecture Reference

Comprehensive guide to raylib game development patterns, resource management, and best practices.

## Table of Contents
1. [Game Architecture](#game-architecture)
2. [Resource Management](#resource-management)
3. [Input Handling](#input-handling)
4. [Camera Systems](#camera-systems)
5. [Audio Patterns](#audio-patterns)
6. [Shader Usage](#shader-usage)
7. [Collision Detection](#collision-detection)
8. [State Management](#state-management)
9. [Performance Patterns](#performance-patterns)
10. [Common Idioms](#common-idioms)

---

## Game Architecture

### Basic Game Structure
```c
#include "raylib.h"

// Game constants
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Game state
typedef enum {
    STATE_LOGO,
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAMEOVER
} GameState;

typedef struct {
    GameState state;
    int score;
    float timer;
    // Add game-specific data
} GameData;

// Function declarations
void GameInit(GameData *game);
void GameUpdate(GameData *game, float dt);
void GameDraw(GameData *game);
void GameUnload(GameData *game);

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Game");
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
    
    GameUnload(&game);
    CloseWindow();
    
    return 0;
}
```

### Entity-Component Pattern (Simple)
```c
#define MAX_ENTITIES 1000

typedef struct {
    Vector2 position;
    Vector2 velocity;
    bool active;
} Transform;

typedef struct {
    Texture2D texture;
    Rectangle sourceRec;
    Color tint;
} Sprite;

typedef struct {
    Rectangle bounds;
    bool solid;
} Collider;

// Parallel arrays for components
Transform transforms[MAX_ENTITIES];
Sprite sprites[MAX_ENTITIES];
Collider colliders[MAX_ENTITIES];
int entityCount = 0;

int CreateEntity(void)
{
    if (entityCount >= MAX_ENTITIES) return -1;
    int id = entityCount++;
    transforms[id] = (Transform){ 0 };
    return id;
}

void UpdateEntities(float dt)
{
    for (int i = 0; i < entityCount; i++)
    {
        if (!transforms[i].active) continue;
        transforms[i].position.x += transforms[i].velocity.x*dt;
        transforms[i].position.y += transforms[i].velocity.y*dt;
    }
}
```

### Object Pool Pattern
```c
#define MAX_BULLETS 500

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    bool active;
} Bullet;

typedef struct {
    Bullet items[MAX_BULLETS];
    int firstAvailable;
} BulletPool;

void InitBulletPool(BulletPool *pool)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        pool->items[i].active = false;
    }
    pool->firstAvailable = 0;
}

Bullet *SpawnBullet(BulletPool *pool, Vector2 pos, Vector2 vel)
{
    // Find first available slot
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        int idx = (pool->firstAvailable + i) % MAX_BULLETS;
        if (!pool->items[idx].active)
        {
            pool->items[idx] = (Bullet){
                .position = pos,
                .velocity = vel,
                .lifetime = 2.0f,
                .active = true
            };
            pool->firstAvailable = (idx + 1) % MAX_BULLETS;
            return &pool->items[idx];
        }
    }
    return NULL;  // Pool exhausted
}

void UpdateBulletPool(BulletPool *pool, float dt)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!pool->items[i].active) continue;
        
        pool->items[i].position.x += pool->items[i].velocity.x*dt;
        pool->items[i].position.y += pool->items[i].velocity.y*dt;
        pool->items[i].lifetime -= dt;
        
        if (pool->items[i].lifetime <= 0.0f)
        {
            pool->items[i].active = false;
        }
    }
}
```

---

## Resource Management

### Texture Loading & Unloading
```c
// ALWAYS check if texture loaded successfully
Texture2D LoadTextureChecked(const char *path)
{
    Texture2D tex = LoadTexture(path);
    if (tex.id == 0)
    {
        TraceLog(LOG_WARNING, "TEXTURE: Failed to load [%s]", path);
    }
    return tex;
}

// Texture atlas/spritesheet
typedef struct {
    Texture2D texture;
    int frameWidth;
    int frameHeight;
    int framesPerRow;
} SpriteSheet;

Rectangle GetSpriteFrame(SpriteSheet sheet, int frame)
{
    int row = frame / sheet.framesPerRow;
    int col = frame % sheet.framesPerRow;
    return (Rectangle){
        col*sheet.frameWidth,
        row*sheet.frameHeight,
        sheet.frameWidth,
        sheet.frameHeight
    };
}

void DrawSpriteFrame(SpriteSheet sheet, int frame, Vector2 pos, Color tint)
{
    Rectangle src = GetSpriteFrame(sheet, frame);
    Rectangle dst = { pos.x, pos.y, sheet.frameWidth, sheet.frameHeight };
    DrawTexturePro(sheet.texture, src, dst, (Vector2){ 0 }, 0.0f, tint);
}
```

### Font Management
```c
// Default font (always available after InitWindow)
DrawText("Hello", 10, 10, 20, BLACK);

// Custom font
Font customFont = LoadFont("resources/myfont.ttf");
DrawTextEx(customFont, "Hello", (Vector2){ 10, 10 }, 24, 2, BLACK);

// Font with specific size (for crisp rendering)
Font fontBig = LoadFontEx("resources/myfont.ttf", 48, NULL, 0);
Font fontSmall = LoadFontEx("resources/myfont.ttf", 16, NULL, 0);

// Cleanup
UnloadFont(customFont);
```

### Resource Manager Pattern
```c
#define MAX_TEXTURES 64
#define MAX_SOUNDS 32

typedef struct {
    Texture2D textures[MAX_TEXTURES];
    const char *textureNames[MAX_TEXTURES];
    int textureCount;
    
    Sound sounds[MAX_SOUNDS];
    const char *soundNames[MAX_SOUNDS];
    int soundCount;
} Resources;

Resources res = { 0 };

Texture2D *GetTexture(const char *name)
{
    // Check if already loaded
    for (int i = 0; i < res.textureCount; i++)
    {
        if (TextIsEqual(res.textureNames[i], name))
        {
            return &res.textures[i];
        }
    }
    
    // Load new texture
    if (res.textureCount < MAX_TEXTURES)
    {
        res.textures[res.textureCount] = LoadTexture(name);
        res.textureNames[res.textureCount] = name;
        return &res.textures[res.textureCount++];
    }
    
    return NULL;
}

void UnloadAllResources(void)
{
    for (int i = 0; i < res.textureCount; i++)
    {
        UnloadTexture(res.textures[i]);
    }
    for (int i = 0; i < res.soundCount; i++)
    {
        UnloadSound(res.sounds[i]);
    }
}
```

---

## Input Handling

### Keyboard Input
```c
// Single press (triggers once per key press)
if (IsKeyPressed(KEY_SPACE)) PlayerJump();
if (IsKeyPressed(KEY_ESCAPE)) TogglePause();

// Held down (triggers every frame while held)
if (IsKeyDown(KEY_RIGHT)) player.x += speed*dt;
if (IsKeyDown(KEY_LEFT)) player.x -= speed*dt;

// Released (triggers once when key is released)
if (IsKeyReleased(KEY_SPACE)) EndCharge();

// Get last pressed key (for rebinding)
int key = GetKeyPressed();
if (key != 0) lastPressedKey = key;
```

### Mouse Input
```c
// Position
Vector2 mousePos = GetMousePosition();
Vector2 worldPos = GetScreenToWorld2D(mousePos, camera);  // With camera

// Buttons
if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) Shoot();
if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) Aim();

// Wheel
float wheel = GetMouseWheelMove();
if (wheel != 0) camera.zoom += wheel*0.1f;

// Delta (for mouse look)
Vector2 mouseDelta = GetMouseDelta();
player.rotation += mouseDelta.x*sensitivity;
```

### Gamepad Input
```c
if (IsGamepadAvailable(0))  // Check gamepad 0
{
    // Buttons
    if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) Jump();  // A/Cross
    if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_2)) Accelerate();  // RT/R2
    
    // Analog sticks (-1.0 to 1.0)
    float leftX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
    float leftY = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
    
    // Apply deadzone
    const float deadzone = 0.2f;
    if (fabsf(leftX) < deadzone) leftX = 0.0f;
    if (fabsf(leftY) < deadzone) leftY = 0.0f;
    
    player.velocity.x = leftX*speed;
    player.velocity.y = leftY*speed;
}
```

### Input Abstraction
```c
typedef struct {
    bool jump;
    bool shoot;
    float moveX;
    float moveY;
} InputState;

InputState GetInput(void)
{
    InputState input = { 0 };
    
    // Keyboard
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W)) input.jump = true;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) input.shoot = true;
    if (IsKeyDown(KEY_D)) input.moveX += 1.0f;
    if (IsKeyDown(KEY_A)) input.moveX -= 1.0f;
    if (IsKeyDown(KEY_S)) input.moveY += 1.0f;
    if (IsKeyDown(KEY_W)) input.moveY -= 1.0f;
    
    // Gamepad (additive)
    if (IsGamepadAvailable(0))
    {
        if (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) input.jump = true;
        input.moveX += GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        input.moveY += GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
    }
    
    // Clamp
    input.moveX = Clamp(input.moveX, -1.0f, 1.0f);
    input.moveY = Clamp(input.moveY, -1.0f, 1.0f);
    
    return input;
}
```

---

## Camera Systems

### 2D Camera (Side-scroller/Top-down)
```c
Camera2D camera = {
    .offset = { screenWidth/2.0f, screenHeight/2.0f },  // Center of screen
    .target = { 0.0f, 0.0f },  // World position to follow
    .rotation = 0.0f,
    .zoom = 1.0f
};

// Follow player smoothly
void UpdateCamera(Camera2D *camera, Vector2 target, float dt)
{
    float smoothing = 5.0f;
    camera->target.x += (target.x - camera->target.x)*smoothing*dt;
    camera->target.y += (target.y - camera->target.y)*smoothing*dt;
}

// In draw loop
BeginMode2D(camera);
    // Draw world objects here (affected by camera)
    DrawRectangle(player.x, player.y, 32, 32, RED);
EndMode2D();

// Draw UI here (not affected by camera)
DrawText("Score: 100", 10, 10, 20, BLACK);
```

### 2D Camera with Bounds
```c
void UpdateCameraBounded(Camera2D *camera, Vector2 target, Rectangle bounds)
{
    camera->target = target;
    
    // Calculate visible area
    float halfWidth = (screenWidth/2.0f) / camera->zoom;
    float halfHeight = (screenHeight/2.0f) / camera->zoom;
    
    // Clamp to bounds
    if (camera->target.x - halfWidth < bounds.x)
        camera->target.x = bounds.x + halfWidth;
    if (camera->target.x + halfWidth > bounds.x + bounds.width)
        camera->target.x = bounds.x + bounds.width - halfWidth;
    if (camera->target.y - halfHeight < bounds.y)
        camera->target.y = bounds.y + halfHeight;
    if (camera->target.y + halfHeight > bounds.y + bounds.height)
        camera->target.y = bounds.y + bounds.height - halfHeight;
}
```

### 3D Camera (First Person)
```c
Camera3D camera = {
    .position = { 0.0f, 2.0f, 4.0f },
    .target = { 0.0f, 2.0f, 0.0f },
    .up = { 0.0f, 1.0f, 0.0f },
    .fovy = 60.0f,
    .projection = CAMERA_PERSPECTIVE
};

// Use built-in camera controls
DisableCursor();
UpdateCamera(&camera, CAMERA_FIRST_PERSON);

// Or manual control
void UpdateFirstPersonCamera(Camera3D *camera, float dt)
{
    Vector2 mouseDelta = GetMouseDelta();
    float sensitivity = 0.003f;
    
    // Yaw (left/right)
    Matrix rotation = MatrixRotateY(-mouseDelta.x*sensitivity);
    Vector3 forward = Vector3Subtract(camera->target, camera->position);
    forward = Vector3Transform(forward, rotation);
    camera->target = Vector3Add(camera->position, forward);
    
    // Pitch (up/down) - with clamping
    // ... (more complex, see raylib examples)
}
```

### 3D Camera (Third Person)
```c
typedef struct {
    Camera3D camera;
    float distance;
    float yaw;
    float pitch;
} ThirdPersonCamera;

void UpdateThirdPersonCamera(ThirdPersonCamera *tpc, Vector3 target, float dt)
{
    // Update angles from input
    Vector2 mouseDelta = GetMouseDelta();
    tpc->yaw -= mouseDelta.x*0.003f;
    tpc->pitch -= mouseDelta.y*0.003f;
    tpc->pitch = Clamp(tpc->pitch, -1.4f, 1.4f);  // Limit pitch
    
    // Zoom with wheel
    tpc->distance -= GetMouseWheelMove()*0.5f;
    tpc->distance = Clamp(tpc->distance, 2.0f, 20.0f);
    
    // Calculate camera position
    float cosP = cosf(tpc->pitch);
    Vector3 offset = {
        tpc->distance*cosP*sinf(tpc->yaw),
        tpc->distance*sinf(tpc->pitch),
        tpc->distance*cosP*cosf(tpc->yaw)
    };
    
    tpc->camera.position = Vector3Add(target, offset);
    tpc->camera.target = target;
}
```

---

## Audio Patterns

### Sound Effects
```c
// Initialize audio
InitAudioDevice();

// Load sounds (short effects)
Sound sfxJump = LoadSound("resources/jump.wav");
Sound sfxShoot = LoadSound("resources/shoot.wav");
Sound sfxExplosion = LoadSound("resources/explosion.wav");

// Play
PlaySound(sfxJump);

// With variations
void PlaySoundVaried(Sound sound)
{
    SetSoundPitch(sound, 0.9f + GetRandomValue(0, 20)/100.0f);  // 0.9-1.1
    SetSoundVolume(sound, 0.8f + GetRandomValue(0, 20)/100.0f);
    PlaySound(sound);
}

// Cleanup
UnloadSound(sfxJump);
CloseAudioDevice();
```

### Music Streaming
```c
Music music = LoadMusicStream("resources/background.ogg");
PlayMusicStream(music);
SetMusicVolume(music, 0.5f);

// MUST call every frame to keep streaming
while (!WindowShouldClose())
{
    UpdateMusicStream(music);  // Required!
    
    // Pause/resume
    if (IsKeyPressed(KEY_P))
    {
        if (IsMusicStreamPlaying(music)) PauseMusicStream(music);
        else ResumeMusicStream(music);
    }
    
    // ...
}

UnloadMusicStream(music);
```

### Audio Manager Pattern
```c
#define MAX_SOUND_CHANNELS 8

typedef struct {
    Sound sounds[MAX_SOUND_CHANNELS];
    int currentChannel;
} SoundPlayer;

// Play same sound multiple times (overlapping)
void PlaySoundMulti(SoundPlayer *player, Sound sound)
{
    // Find a free channel or use round-robin
    PlaySound(sound);  // raylib handles multi-channel internally
}

// Fade music
void FadeMusicVolume(Music music, float targetVolume, float duration, float dt)
{
    static float currentVolume = 1.0f;
    float speed = (targetVolume - currentVolume) / duration;
    currentVolume += speed*dt;
    currentVolume = Clamp(currentVolume, 0.0f, 1.0f);
    SetMusicVolume(music, currentVolume);
}
```

---

## Shader Usage

### Loading Shaders
```c
// Load from files
Shader shader = LoadShader("vertex.vs", "fragment.fs");

// Load with default vertex shader
Shader shader = LoadShader(NULL, "fragment.fs");

// From memory
Shader shader = LoadShaderFromMemory(vertexCode, fragmentCode);

// Get uniform locations
int timeLoc = GetShaderLocation(shader, "time");
int colorLoc = GetShaderLocation(shader, "tintColor");

// Set uniforms
float time = GetTime();
SetShaderValue(shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);

Vector4 color = { 1.0f, 0.5f, 0.0f, 1.0f };
SetShaderValue(shader, colorLoc, &color, SHADER_UNIFORM_VEC4);
```

### Applying Shaders to Sprites
```c
BeginShaderMode(shader);
    DrawTexture(texture, x, y, WHITE);
EndShaderMode();
```

### Post-Processing
```c
// Create render texture
RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);

// In game loop
BeginTextureMode(target);
    ClearBackground(RAYWHITE);
    // Draw your game here
EndTextureMode();

// Apply post-processing shader
BeginDrawing();
    BeginShaderMode(postProcessShader);
        // Note: Render textures are flipped vertically
        DrawTextureRec(target.texture,
            (Rectangle){ 0, 0, screenWidth, -screenHeight },
            (Vector2){ 0, 0 }, WHITE);
    EndShaderMode();
EndDrawing();

// Cleanup
UnloadRenderTexture(target);
UnloadShader(postProcessShader);
```

### Common Shader Uniforms
```c
// Time-based effects
float time = GetTime();
SetShaderValue(shader, GetShaderLocation(shader, "time"), &time, SHADER_UNIFORM_FLOAT);

// Resolution
Vector2 resolution = { screenWidth, screenHeight };
SetShaderValue(shader, GetShaderLocation(shader, "resolution"), &resolution, SHADER_UNIFORM_VEC2);

// Mouse position
Vector2 mousePos = GetMousePosition();
SetShaderValue(shader, GetShaderLocation(shader, "mouse"), &mousePos, SHADER_UNIFORM_VEC2);
```

---

## Collision Detection

### Rectangle Collisions
```c
// Basic overlap check
if (CheckCollisionRecs(recA, recB))
{
    HandleCollision();
}

// Get overlap rectangle
Rectangle overlap = GetCollisionRec(recA, recB);

// AABB resolution (push out of collision)
void ResolveCollision(Rectangle *moving, Rectangle obstacle)
{
    Rectangle overlap = GetCollisionRec(*moving, obstacle);
    
    if (overlap.width < overlap.height)
    {
        // Push horizontally
        if (moving->x < obstacle.x) moving->x -= overlap.width;
        else moving->x += overlap.width;
    }
    else
    {
        // Push vertically
        if (moving->y < obstacle.y) moving->y -= overlap.height;
        else moving->y += overlap.height;
    }
}
```

### Circle Collisions
```c
// Circle-circle
if (CheckCollisionCircles(pos1, radius1, pos2, radius2))
{
    HandleCollision();
}

// Circle-rectangle
if (CheckCollisionCircleRec(circlePos, radius, rect))
{
    HandleCollision();
}
```

### Point Checks
```c
// Point in rectangle (mouse hover)
if (CheckCollisionPointRec(mousePos, buttonRect))
{
    buttonHovered = true;
}

// Point in circle
if (CheckCollisionPointCircle(mousePos, circleCenter, radius))
{
    circleClicked = true;
}
```

### Line/Ray Collisions
```c
// Line-line intersection
Vector2 collision = { 0 };
if (CheckCollisionLines(start1, end1, start2, end2, &collision))
{
    // collision contains intersection point
}

// Ray-box (3D)
Ray ray = { cameraPos, rayDirection };
RayCollision hit = GetRayCollisionBox(ray, boundingBox);
if (hit.hit)
{
    DrawSphere(hit.point, 0.1f, RED);
}
```

---

## State Management

### Simple State Machine
```c
typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAMEOVER
} GameState;

GameState currentState = STATE_MENU;

void GameUpdate(float dt)
{
    switch (currentState)
    {
        case STATE_MENU:
            if (IsKeyPressed(KEY_ENTER)) currentState = STATE_PLAYING;
            break;
            
        case STATE_PLAYING:
            UpdateGameplay(dt);
            if (IsKeyPressed(KEY_ESCAPE)) currentState = STATE_PAUSED;
            if (playerHealth <= 0) currentState = STATE_GAMEOVER;
            break;
            
        case STATE_PAUSED:
            if (IsKeyPressed(KEY_ESCAPE)) currentState = STATE_PLAYING;
            break;
            
        case STATE_GAMEOVER:
            if (IsKeyPressed(KEY_ENTER))
            {
                ResetGame();
                currentState = STATE_MENU;
            }
            break;
    }
}

void GameDraw(void)
{
    switch (currentState)
    {
        case STATE_MENU: DrawMenu(); break;
        case STATE_PLAYING: DrawGameplay(); break;
        case STATE_PAUSED:
            DrawGameplay();  // Draw game behind
            DrawPauseOverlay();
            break;
        case STATE_GAMEOVER: DrawGameOver(); break;
    }
}
```

### State with Transitions
```c
typedef struct {
    GameState current;
    GameState next;
    float transitionTimer;
    float transitionDuration;
    bool transitioning;
} StateMachine;

void ChangeState(StateMachine *sm, GameState newState, float duration)
{
    sm->next = newState;
    sm->transitionTimer = 0.0f;
    sm->transitionDuration = duration;
    sm->transitioning = true;
}

void UpdateStateMachine(StateMachine *sm, float dt)
{
    if (sm->transitioning)
    {
        sm->transitionTimer += dt;
        if (sm->transitionTimer >= sm->transitionDuration)
        {
            sm->current = sm->next;
            sm->transitioning = false;
        }
    }
}

void DrawTransition(StateMachine *sm)
{
    if (sm->transitioning)
    {
        float progress = sm->transitionTimer / sm->transitionDuration;
        unsigned char alpha = (unsigned char)(progress*255);
        DrawRectangle(0, 0, screenWidth, screenHeight, (Color){ 0, 0, 0, alpha });
    }
}
```

---

## Performance Patterns

### Batch Drawing
```c
// BAD: Texture swaps every draw
for (int i = 0; i < 1000; i++)
{
    DrawTexture(textures[i % 10], positions[i].x, positions[i].y, WHITE);
}

// GOOD: Sort by texture, minimize swaps
void DrawBatched(Entity *entities, int count)
{
    // Sort entities by texture ID
    qsort(entities, count, sizeof(Entity), CompareByTexture);
    
    // Draw in batches
    for (int i = 0; i < count; i++)
    {
        DrawTexture(entities[i].texture, entities[i].x, entities[i].y, WHITE);
    }
}
```

### Spatial Partitioning (Grid)
```c
#define GRID_SIZE 64
#define GRID_WIDTH (WORLD_WIDTH / GRID_SIZE)
#define GRID_HEIGHT (WORLD_HEIGHT / GRID_SIZE)

typedef struct {
    int entities[MAX_ENTITIES_PER_CELL];
    int count;
} GridCell;

GridCell grid[GRID_WIDTH][GRID_HEIGHT];

void InsertIntoGrid(int entityId, Vector2 pos)
{
    int gx = (int)(pos.x / GRID_SIZE);
    int gy = (int)(pos.y / GRID_SIZE);
    
    if (gx >= 0 && gx < GRID_WIDTH && gy >= 0 && gy < GRID_HEIGHT)
    {
        GridCell *cell = &grid[gx][gy];
        if (cell->count < MAX_ENTITIES_PER_CELL)
        {
            cell->entities[cell->count++] = entityId;
        }
    }
}

// Only check collisions within same/adjacent cells
void CheckNearbyCollisions(int entityId, Vector2 pos)
{
    int gx = (int)(pos.x / GRID_SIZE);
    int gy = (int)(pos.y / GRID_SIZE);
    
    for (int dx = -1; dx <= 1; dx++)
    {
        for (int dy = -1; dy <= 1; dy++)
        {
            int nx = gx + dx;
            int ny = gy + dy;
            if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT)
            {
                GridCell *cell = &grid[nx][ny];
                for (int i = 0; i < cell->count; i++)
                {
                    if (cell->entities[i] != entityId)
                    {
                        CheckCollision(entityId, cell->entities[i]);
                    }
                }
            }
        }
    }
}
```

### Delta Time Independence
```c
// BAD: Frame-dependent
player.x += 5;  // Faster at higher FPS

// GOOD: Frame-independent
float dt = GetFrameTime();
player.x += 300.0f*dt;  // 300 pixels per second, regardless of FPS

// For physics
player.velocity.y += gravity*dt;
player.position.x += player.velocity.x*dt;
player.position.y += player.velocity.y*dt;
```

---

## Common Idioms

### Animation Frame Counter
```c
typedef struct {
    int currentFrame;
    int frameCount;
    float frameTime;
    float timer;
} Animation;

void UpdateAnimation(Animation *anim, float dt)
{
    anim->timer += dt;
    if (anim->timer >= anim->frameTime)
    {
        anim->timer = 0.0f;
        anim->currentFrame = (anim->currentFrame + 1) % anim->frameCount;
    }
}
```

### Screen Shake
```c
typedef struct {
    float duration;
    float intensity;
    Vector2 offset;
} ScreenShake;

void TriggerShake(ScreenShake *shake, float duration, float intensity)
{
    shake->duration = duration;
    shake->intensity = intensity;
}

void UpdateShake(ScreenShake *shake, float dt)
{
    if (shake->duration > 0.0f)
    {
        shake->duration -= dt;
        shake->offset.x = (float)GetRandomValue(-10, 10) * shake->intensity;
        shake->offset.y = (float)GetRandomValue(-10, 10) * shake->intensity;
    }
    else
    {
        shake->offset = (Vector2){ 0 };
    }
}

// Apply to camera
camera.offset.x = screenWidth/2.0f + shake.offset.x;
camera.offset.y = screenHeight/2.0f + shake.offset.y;
```

### Easing Functions
```c
// Linear
float EaseLinear(float t) { return t; }

// Quad
float EaseInQuad(float t) { return t*t; }
float EaseOutQuad(float t) { return t*(2.0f - t); }
float EaseInOutQuad(float t)
{
    return (t < 0.5f) ? 2.0f*t*t : -1.0f + (4.0f - 2.0f*t)*t;
}

// Usage
float Lerp(float start, float end, float t)
{
    return start + (end - start)*t;
}

float progress = EaseOutQuad(timer / duration);
currentValue = Lerp(startValue, endValue, progress);
```

### Timer Utility
```c
typedef struct {
    float duration;
    float elapsed;
    bool running;
    bool finished;
} Timer;

void TimerStart(Timer *timer, float duration)
{
    timer->duration = duration;
    timer->elapsed = 0.0f;
    timer->running = true;
    timer->finished = false;
}

void TimerUpdate(Timer *timer, float dt)
{
    if (!timer->running) return;
    
    timer->elapsed += dt;
    if (timer->elapsed >= timer->duration)
    {
        timer->running = false;
        timer->finished = true;
    }
}

float TimerProgress(Timer *timer)
{
    return Clamp(timer->elapsed / timer->duration, 0.0f, 1.0f);
}

// Usage
Timer invincibilityTimer = { 0 };
TimerStart(&invincibilityTimer, 2.0f);

// In update
TimerUpdate(&invincibilityTimer, dt);
if (invincibilityTimer.finished)
{
    playerInvincible = false;
}
```

### Particle System (Simple)
```c
#define MAX_PARTICLES 1000

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float size;
    float life;
    float maxLife;
    bool active;
} Particle;

Particle particles[MAX_PARTICLES];

void SpawnParticle(Vector2 pos, Vector2 vel, Color color, float size, float life)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (!particles[i].active)
        {
            particles[i] = (Particle){
                .position = pos,
                .velocity = vel,
                .color = color,
                .size = size,
                .life = life,
                .maxLife = life,
                .active = true
            };
            break;
        }
    }
}

void UpdateParticles(float dt)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (!particles[i].active) continue;
        
        particles[i].position.x += particles[i].velocity.x*dt;
        particles[i].position.y += particles[i].velocity.y*dt;
        particles[i].life -= dt;
        
        if (particles[i].life <= 0.0f)
        {
            particles[i].active = false;
        }
    }
}

void DrawParticles(void)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (!particles[i].active) continue;
        
        float alpha = particles[i].life / particles[i].maxLife;
        Color c = particles[i].color;
        c.a = (unsigned char)(alpha*255);
        
        DrawCircleV(particles[i].position, particles[i].size*alpha, c);
    }
}
```
