---
name: entity-pool-patterns
description: Fixed-size entity pool patterns for C99 games with no runtime allocation. Use when implementing entity systems, spawning logic, collision handling, or managing game objects like enemies, projectiles, particles, and pickups. Triggers on MAX_ENTITIES defines, active flags, pool iteration patterns, or spawn/despawn functions.
---

# Entity Pool Patterns for C99 Games

Expert guidance for implementing fixed-size entity pools with no runtime allocation.

## Core Principle

**No malloc/free during gameplay**. Pre-allocate all entity storage at startup using fixed-size arrays with active flags.

## Pool Structure Pattern

```c
// In types.h - Central pool size definitions
#define MAX_ENEMIES       500
#define MAX_PROJECTILES   1000
#define MAX_PARTICLES     2000
#define MAX_XP_CRYSTALS   300

// Entity struct pattern
typedef struct Enemy {
    Vector2 pos;
    Vector2 vel;
    float radius;
    float health;
    float maxHealth;
    float damage;
    EnemyType type;
    int xpValue;
    bool active;           // CRITICAL: Active flag for pool management
    // Type-specific fields...
    float hitFlashTimer;
} Enemy;

// Pool container with count tracking
typedef struct EnemyPool {
    Enemy enemies[MAX_ENEMIES];
    int count;             // Track active count for quick reference
} EnemyPool;
```

## Pool Initialization

```c
void EnemyPoolInit(EnemyPool *pool)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        pool->enemies[i].active = false;
    }
    pool->count = 0;
}
```

## Entity Spawning Pattern

Linear search for free slot, initialize ALL fields to avoid stale data:

```c
Enemy* EnemySpawn(EnemyPool *pool, EnemyType type, Vector2 pos)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!pool->enemies[i].active)
        {
            Enemy *e = &pool->enemies[i];
            // Initialize ALL fields to avoid stale data
            e->pos = pos;
            e->vel = (Vector2){ 0.0f, 0.0f };
            e->type = type;
            e->health = 30.0f;      // Set based on type
            e->maxHealth = 30.0f;
            e->radius = 12.0f;
            e->speed = 100.0f;
            e->damage = 10.0f;
            e->xpValue = 1;
            e->hitFlashTimer = 0.0f;
            e->active = true;       // Mark active LAST
            pool->count++;
            return e;
        }
    }
    return NULL;  // Pool exhausted - handle gracefully
}
```

## Pool Update Pattern

Always check `active` flag first:

```c
void EnemyPoolUpdate(EnemyPool *pool, Vector2 playerPos, float dt)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        Enemy *e = &pool->enemies[i];
        if (!e->active) continue;  // Skip inactive - CRITICAL first check

        // Update logic...
        Vector2 dir = Vector2Subtract(playerPos, e->pos);
        dir = Vector2Normalize(dir);
        e->pos = Vector2Add(e->pos, Vector2Scale(dir, e->speed * dt));

        // Timers
        if (e->hitFlashTimer > 0.0f) e->hitFlashTimer -= dt;
    }
}
```

## Entity Despawning Pattern

Capture data BEFORE despawning, then mark inactive:

```c
// In collision handler
if (e->health <= 0.0f)
{
    // 1. Capture data BEFORE despawning
    Vector2 deathPos = e->pos;
    int xpValue = e->xpValue;
    float deathRadius = e->radius;

    // 2. Spawn effects/drops
    XPSpawn(xp, deathPos, xpValue);
    SpawnExplosion(particles, deathPos, color, 15);

    // 3. Trigger game feel effects
    PlayGameSound(SOUND_EXPLOSION);
    TriggerScreenShake(game, 3.0f, 0.15f);

    // 4. NOW despawn
    e->active = false;
    pool->count--;

    // 5. Update score/stats
    game->score += xpValue * 10;
    game->killCount++;
}
```

## Variant Spawning (Elite/Boss)

Reuse base spawn, then apply multipliers:

```c
// Multiplier defines
#define ELITE_SIZE_MULT     1.5f    // 50% larger
#define ELITE_HEALTH_MULT   3.0f    // 3x health
#define ELITE_DAMAGE_MULT   1.5f    // 50% more damage
#define ELITE_XP_MULT       5       // 5x XP reward
#define ELITE_SPEED_MULT    0.8f    // Slightly slower

Enemy* EnemySpawnElite(EnemyPool *pool, EnemyType type, Vector2 pos)
{
    Enemy *e = EnemySpawn(pool, type, pos);  // Reuse base spawn
    if (e)
    {
        e->isElite = true;
        e->radius *= ELITE_SIZE_MULT;
        e->health *= ELITE_HEALTH_MULT;
        e->maxHealth = e->health;
        e->damage *= ELITE_DAMAGE_MULT;
        e->xpValue *= ELITE_XP_MULT;
        e->speed *= ELITE_SPEED_MULT;
    }
    return e;
}
```

## Pool Query Patterns

### Find Nearest Entity

```c
Enemy* EnemyFindNearest(EnemyPool *pool, Vector2 pos, float maxDistance)
{
    Enemy *nearest = NULL;
    float nearestDistSq = maxDistance * maxDistance;

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        Enemy *e = &pool->enemies[i];
        if (!e->active) continue;

        float dx = pos.x - e->pos.x;
        float dy = pos.y - e->pos.y;
        float distSq = dx * dx + dy * dy;

        if (distSq < nearestDistSq)
        {
            nearestDistSq = distSq;
            nearest = e;
        }
    }
    return nearest;
}
```

### Check Pool for Condition

```c
bool EnemyPoolHasBoss(EnemyPool *pool)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (pool->enemies[i].active && pool->enemies[i].isBoss)
        {
            return true;
        }
    }
    return false;
}
```

## Draw Pattern

```c
void EnemyPoolDraw(EnemyPool *pool)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        Enemy *e = &pool->enemies[i];
        if (!e->active) continue;

        Color drawColor = GetEnemyColor(e->type);

        // Hit flash effect
        if (e->hitFlashTimer > 0.0f)
        {
            drawColor = WHITE;
        }

        DrawCircleV(e->pos, e->radius, drawColor);
    }
}
```

## Anti-Patterns (AVOID)

| Mistake | Why It's Bad | Correct Approach |
|---------|--------------|------------------|
| `malloc()` in spawn | Memory fragmentation, slower | Use pre-allocated pool |
| Forgetting `pool->count--` | Count mismatch causes bugs | Always decrement on despawn |
| Despawn before capturing data | Entity might be reused immediately | Capture data first, then despawn |
| Double despawn | Count goes negative | Check `active` before despawning |
| No `active` check in loop | Operates on invalid data | Always check `active` first |

## When to Use Pools

**Use when:**
- Managing runtime entities (enemies, projectiles, particles, pickups)
- Need deterministic memory usage
- Entities have short lifetimes with frequent create/destroy
- Building survivor-like, bullet-hell, or arcade games

**Do NOT use when:**
- Entity count varies dramatically beyond fixed max
- Entities need complex dynamic components
- Memory is severely constrained (pools pre-allocate)
