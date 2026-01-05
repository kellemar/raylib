---
name: game-feel-polish
description: Implement juicy game feel effects including screen shake, hitstop, squash/stretch, particles, impact frames, and chromatic aberration for impactful, responsive games. Use when adding polish, improving combat feel, or making actions feel satisfying. Triggers on shake, hitstop, juice, polish, particles, impact, or squash/stretch mentions.
---

# Game Feel & Polish for C99/raylib Games

Expert guidance for implementing juicy game effects that make actions feel impactful and satisfying.

## Core Principle

**Every action should have feedback**. Hits should feel heavy, movement should feel responsive, and deaths should be memorable.

---

## Screen Shake System

Shake the camera on impacts to convey force:

### Data Structure

```c
// In GameData struct
float shakeIntensity;      // Current shake strength (pixels)
float shakeDuration;       // Remaining shake time (seconds)
```

### Implementation

```c
void TriggerScreenShake(GameData *game, float intensity, float duration)
{
    // Respect settings toggle
    if (!game->settings.screenShakeEnabled) return;

    // Only increase, never decrease mid-shake
    if (intensity > game->shakeIntensity)
    {
        game->shakeIntensity = intensity;
    }
    if (duration > game->shakeDuration)
    {
        game->shakeDuration = duration;
    }
}

// In camera update (call every frame)
static void UpdateGameCamera(GameData *game, float dt)
{
    // Smooth camera follow
    game->camera.target = Vector2Lerp(game->camera.target, game->player.pos, 5.0f * dt);

    // Apply shake
    if (game->shakeDuration > 0.0f)
    {
        game->shakeDuration -= dt;
        float shakeFactor = game->shakeDuration > 0.0f ? game->shakeDuration / 0.25f : 0.0f;
        float offsetX = ((float)(rand() % 100) / 100.0f - 0.5f) * 2.0f * game->shakeIntensity * shakeFactor;
        float offsetY = ((float)(rand() % 100) / 100.0f - 0.5f) * 2.0f * game->shakeIntensity * shakeFactor;
        game->camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f + offsetX, SCREEN_HEIGHT / 2.0f + offsetY };
    }
    else
    {
        game->camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    }
}
```

### Intensity Guidelines

| Event | Intensity | Duration | Notes |
|-------|-----------|----------|-------|
| Small hit | 3.0f | 0.15s | Regular enemy death |
| Player damage | 8.0f | 0.25s | Significant feedback |
| Boss spawn | 15.0f | 0.8s | Major event |
| Boss death | 20.0f | 1.0s | Climactic moment |

---

## Hitstop (Frame Freeze)

Brief pause on impact to emphasize the hit:

### Data Structure

```c
// In GameData struct
int hitstopFrames;         // Frames to freeze (0 = none)
```

### Implementation

```c
// At the START of game update (before any logic)
if (game->hitstopFrames > 0)
{
    game->hitstopFrames--;
    return;  // Skip ALL updates during hitstop
}

// On enemy kill - vary by enemy importance
int hitstopAmount = (e->xpValue >= 3) ? 4 : 2;  // 4 frames for big enemies
if (hitstopAmount > game->hitstopFrames)
{
    game->hitstopFrames = hitstopAmount;
}
```

### Guidelines

- **2 frames** (~33ms at 60fps): Small kills
- **4 frames** (~66ms at 60fps): Big kills, elite enemies
- **6-8 frames**: Boss phase transitions
- **Do NOT stack**: Take max of current and new hitstop

---

## Impact Frames (Death Flash)

Bright flash at explosion position for visual punch:

### Data Structure

```c
// In GameData struct
Vector2 impactPos;         // Position of impact flash
int impactFrames;          // Frames remaining (0 = none)
float impactRadius;        // Radius of impact flash
```

### Implementation

```c
void TriggerImpactFrame(GameData *game, Vector2 pos, float radius)
{
    game->impactPos = pos;
    game->impactFrames = 2;  // 2 frames = ~33ms at 60fps
    game->impactRadius = radius;
}

// In update (decrement each frame)
if (game->impactFrames > 0)
{
    game->impactFrames--;
}

// In draw (before entities, after background)
if (game->impactFrames > 0)
{
    float intensity = (float)game->impactFrames / 2.0f;  // 1.0 first frame, 0.5 second
    float outerRadius = game->impactRadius * (2.0f - intensity);
    unsigned char alpha = (unsigned char)(255 * intensity);

    // Outer glow
    DrawCircleV(game->impactPos, outerRadius, (Color){ 255, 255, 255, (unsigned char)(alpha * 0.3f) });
    // Inner bright core
    DrawCircleV(game->impactPos, game->impactRadius * 0.5f, (Color){ 255, 255, 200, (unsigned char)(alpha * 0.6f) });
}
```

---

## Squash/Stretch Animation

Deform player based on velocity for organic movement:

### Data Structure

```c
// In Player struct
float squashScale;     // Horizontal scale (1.0 = normal)
float stretchScale;    // Vertical scale (1.0 = normal)
Vector2 prevVel;       // Previous velocity for direction change detection
```

### Implementation

```c
// In PlayerUpdate
float velMag = Vector2Length(player->vel);
float prevVelMag = Vector2Length(player->prevVel);

// Detect direction change via dot product
float dirChange = 0.0f;
if (velMag > 10.0f && prevVelMag > 10.0f)
{
    Vector2 velNorm = Vector2Scale(player->vel, 1.0f / velMag);
    Vector2 prevNorm = Vector2Scale(player->prevVel, 1.0f / prevVelMag);
    dirChange = 1.0f - Vector2DotProduct(velNorm, prevNorm);  // 0 = same, 2 = opposite
}

// Calculate target scales based on speed
float targetSquash = 1.0f;
float targetStretch = 1.0f;

if (velMag > 50.0f)
{
    float speedFactor = fminf(velMag / player->speed, 1.0f);
    targetSquash = 1.0f + speedFactor * 0.15f;   // Wider when moving fast
    targetStretch = 1.0f - speedFactor * 0.10f;  // Shorter when moving fast
}

// Extra squash on direction change
if (dirChange > 0.3f)
{
    float changeFactor = fminf(dirChange, 1.0f);
    targetSquash += changeFactor * 0.2f;
    targetStretch -= changeFactor * 0.15f;
}

// Spring-like interpolation toward target
float lerpSpeed = 12.0f * dt;
player->squashScale += (targetSquash - player->squashScale) * lerpSpeed;
player->stretchScale += (targetStretch - player->stretchScale) * lerpSpeed;

// Store velocity for next frame
player->prevVel = player->vel;
```

### Drawing with Squash/Stretch

```c
// In PlayerDraw
float baseRadius = player->radius;
float radiusX, radiusY;

float absVelX = fabsf(player->vel.x);
float absVelY = fabsf(player->vel.y);

if (absVelX > absVelY)
{
    // Moving mostly horizontal: stretch X, squash Y
    radiusX = baseRadius * player->squashScale;
    radiusY = baseRadius * player->stretchScale;
}
else
{
    // Moving mostly vertical: stretch Y, squash X
    radiusX = baseRadius * player->stretchScale;
    radiusY = baseRadius * player->squashScale;
}

DrawEllipse((int)player->pos.x, (int)player->pos.y, radiusX, radiusY, player->color);
```

---

## Particle Explosions

Spawn particles on death/impact for visual feedback:

```c
void SpawnExplosion(ParticlePool *pool, Vector2 pos, Color color, int count)
{
    for (int i = 0; i < count; i++)
    {
        float angle = ((float)(rand() % 360)) * (PI / 180.0f);
        float speed = 100.0f + (float)(rand() % 200);

        Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };

        // Color variation for visual interest
        Color particleColor = color;
        int variation = (rand() % 60) - 30;
        particleColor.r = (unsigned char)Clamp(particleColor.r + variation, 0, 255);
        particleColor.g = (unsigned char)Clamp(particleColor.g + variation, 0, 255);

        float size = 3.0f + (float)(rand() % 5);
        float lifetime = 0.3f + (float)(rand() % 100) / 200.0f;

        ParticleSpawn(pool, pos, vel, particleColor, size, lifetime);
    }
}
```

### Particle Count Guidelines

| Event | Count | Notes |
|-------|-------|-------|
| Small hit | 5 | Subtle feedback |
| Enemy death | 15 | Standard explosion |
| Elite death | 25 | More dramatic |
| Boss death | 50+ | Screen-filling |

---

## Combining Effects

Layer multiple effects for maximum impact:

```c
// On enemy kill - FULL juice treatment
if (e->health <= 0.0f)
{
    // Capture data before despawn
    Vector2 deathPos = e->pos;
    float deathRadius = e->radius;

    // 1. Particles - visual explosion
    SpawnExplosion(particles, deathPos, p->color, 15);

    // 2. Impact frame - bright flash
    TriggerImpactFrame(game, deathPos, deathRadius * 2.0f);

    // 3. Screen shake - camera feedback
    TriggerScreenShake(game, 3.0f, 0.15f);

    // 4. Sound - audio feedback
    PlayGameSound(SOUND_EXPLOSION);

    // 5. Hitstop - freeze frame
    int hitstopAmount = (e->xpValue >= 3) ? 4 : 2;
    if (hitstopAmount > game->hitstopFrames)
    {
        game->hitstopFrames = hitstopAmount;
    }

    // Now despawn
    e->active = false;
    enemies->count--;
}
```

---

## Time Scale (Slow-Motion)

Optional slow-mo for dramatic moments:

```c
// In GameData
float timeScale;           // 1.0 = normal, 0.5 = half speed

// In update
float scaledDt = dt * game->timeScale;

// Use scaledDt for gameplay, raw dt for UI/timers
game->gameTime += scaledDt;      // Gameplay timer (affected)
game->tutorialTimer += dt;       // UI timer (not affected)
```

---

## Anti-Patterns (AVOID)

| Mistake | Why It's Bad | Correct Approach |
|---------|--------------|------------------|
| Shake on every bullet | Exhausting, loses meaning | Only on significant hits |
| Too much hitstop | Game feels sluggish | Max 4-6 frames for regular kills |
| No settings toggle | Some players get motion sick | Always provide disable option |
| Same intensity for all events | Lacks hierarchy | Vary by event importance |
| Stacking shakes | Excessive camera movement | Take maximum, don't add |
