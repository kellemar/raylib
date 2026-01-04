# PLAN.md — NEON VOID

> *A twin-stick roguelike shooter with procedural arenas, synth-wave aesthetics, and one rule: don't stop moving.*

---

## CONCEPT

You are a geometric shape trapped in an infinite neon void. Waves of hostile geometry spawn endlessly. Survive as long as possible, collect experience crystals, and evolve your weapons into screen-clearing chaos.

**Genre**: Twin-stick shooter / Bullet hell / Roguelike  
**Inspiration**: Vampire Survivors × Geometry Wars × Nuclear Throne  
**Vibe**: Synthwave, neon glow, CRT scanlines, bass drops

---

## CORE LOOP

```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│   SPAWN  ───►  SURVIVE  ───►  COLLECT XP  ───►  LEVEL UP   │
│     │                              │               │        │
│     │                              │               ▼        │
│     │                              │         CHOOSE UPGRADE │
│     │                              │               │        │
│     │◄─────────────────────────────┴───────────────┘        │
│     │                                                       │
│     ▼                                                       │
│   DEATH  ───►  UNLOCK META-PROGRESSION  ───►  RESTART      │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

**Session length**: 10-20 minutes  
**Difficulty curve**: Exponential enemy scaling, linear player power

---

## FEATURES

### Phase 1: Core (Week 1-2)
- [x] Player movement (WASD/left stick)
- [x] Auto-aim toward nearest enemy OR cursor aim (toggle)
- [x] Basic weapon: pulse cannon (fires toward aim direction)
- [x] Enemy spawner with wave system
- [x] 3 enemy types: Chaser, Orbiter, Splitter
- [x] XP crystals drop on enemy death
- [x] Level-up screen with 3 random upgrades
- [x] Health system + death state
- [x] Basic particle system (explosions, trails)

### Phase 2: Juice (Week 3)
- [x] Screen shake on hits/explosions
- [x] Hitstop (2-3 frame freeze on big kills)
- [x] Neon glow shader (bloom post-processing)
- [x] CRT scanline overlay shader
- [ ] Chromatic aberration on damage
- [x] Camera lerp (smooth follow with slight lag)
- [x] Slow-mo on level up / near-death
- [x] Sound effects (pew pew, explosions, pickups)
- [x] Synthwave background track (procedural or looped)

### Phase 3: Depth (Week 4)
- [ ] 8 weapon types (see Weapons section)
- [ ] 20+ upgrades (damage, speed, multishot, pierce, etc.)
- [ ] Weapon evolution system (combine 2 maxed weapons → ultimate)
- [ ] Elite enemies (larger, special attacks)
- [ ] Boss every 5 minutes
- [ ] Permanent unlocks (new starting weapons, characters)
- [ ] Leaderboard (local high scores)

### Phase 4: Polish (Week 5)
- [x] Main menu with neon animations
- [ ] Character select (3 characters with different stats)
- [ ] Settings (volume, screen shake toggle, CRT filter toggle)
- [x] Pause menu
- [x] Tutorial overlay (first 30 seconds)
- [ ] Achievement system
- [x] Stats screen post-death

---

## TECHNICAL DESIGN

### Architecture

```
src/
├── main.c                 # Entry point, game state machine
├── game.h / game.c        # Core game logic, update/draw loops
├── player.h / player.c    # Player entity, input handling
├── enemies.h / enemies.c  # Enemy types, AI behaviors, spawner
├── weapons.h / weapons.c  # Weapon systems, projectiles
├── particles.h / particles.c  # Particle emitters and pools
├── upgrades.h / upgrades.c    # Upgrade definitions and UI
├── audio.h / audio.c      # Sound/music manager
├── shaders.h / shaders.c  # Post-processing effects
├── ui.h / ui.c            # Menus, HUD, level-up screen
├── utils.h / utils.c      # Math helpers, random, easing
└── resources/
    ├── shaders/
    │   ├── bloom.fs
    │   ├── crt.fs
    │   └── chromatic.fs
    ├── sounds/
    │   ├── shoot.wav
    │   ├── explosion.wav
    │   ├── pickup.wav
    │   ├── levelup.wav
    │   └── hit.wav
    ├── music/
    │   └── synthwave_loop.ogg
    └── textures/
        ├── player.png
        ├── enemies.png
        └── particles.png
```

### Entity System (Simple ECS-lite)

```c
#define MAX_ENEMIES 500
#define MAX_PROJECTILES 1000
#define MAX_PARTICLES 2000
#define MAX_XP_CRYSTALS 300

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float radius;
    float health;
    float maxHealth;
    int type;
    bool active;
} Enemy;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float radius;
    float damage;
    float lifetime;
    int weaponType;
    bool pierce;
    bool active;
} Projectile;

typedef struct {
    Enemy enemies[MAX_ENEMIES];
    Projectile projectiles[MAX_PROJECTILES];
    Particle particles[MAX_PARTICLES];
    XPCrystal crystals[MAX_XP_CRYSTALS];
    int enemyCount;
    int projectileCount;
    // ... counts for others
} GameWorld;
```

### Collision Detection

Use spatial hashing for O(n) collision checks:

```c
#define CELL_SIZE 64
#define GRID_WIDTH 32
#define GRID_HEIGHT 32

typedef struct {
    int entities[64];  // indices into enemy array
    int count;
} GridCell;

GridCell grid[GRID_WIDTH][GRID_HEIGHT];

// Only check collisions within same + adjacent cells
```

### Render Pipeline

```
1. Clear with dark purple (#0d0221)
2. Draw arena background (subtle grid)
3. Draw XP crystals
4. Draw enemies (sorted by Y for fake depth)
5. Draw projectiles
6. Draw player
7. Draw particles (additive blending)
8. Apply bloom shader to render texture
9. Apply CRT shader
10. Draw UI on top (no post-processing)
```

---

## WEAPONS

| # | Name | Behavior | Evolution |
|---|------|----------|-----------|
| 1 | **Pulse Cannon** | Single shot, medium speed | → **Mega Cannon** (huge piercing beam) |
| 2 | **Spread Shot** | 3-way spread | → **Circle Burst** (360° nova) |
| 3 | **Homing Missiles** | Slow, seeks enemies | → **Swarm** (20 mini missiles) |
| 4 | **Lightning** | Chain to 3 enemies | → **Tesla Coil** (permanent AoE) |
| 5 | **Orbit Shield** | Rotating projectiles | → **Blade Dancer** (8 orbiting swords) |
| 6 | **Flamethrower** | Cone, DOT damage | → **Inferno** (screen-wide fire) |
| 7 | **Freeze Ray** | Slows enemies | → **Blizzard** (freeze + shatter) |
| 8 | **Black Hole** | Pulls enemies in | → **Singularity** (insta-kill vortex) |

**Evolution requirement**: Max level weapon + specific upgrade item

---

## ENEMIES

| Type | Behavior | Speed | Health | XP |
|------|----------|-------|--------|-----|
| **Chaser** | Moves directly toward player | Fast | Low | 1 |
| **Orbiter** | Circles player, slowly closing in | Medium | Medium | 2 |
| **Splitter** | Splits into 2 smaller on death | Slow | High | 3 |
| **Shooter** | Stops and fires at player | Slow | Medium | 3 |
| **Tank** | Slow, absorbs damage, big knockback | Very Slow | Very High | 5 |
| **Swarm** | Spawns in groups of 10, weak alone | Very Fast | Very Low | 0.5 |
| **Teleporter** | Blinks toward player | N/A | Low | 4 |
| **Elite (any)** | 3x size, special attack, glowing | Varies | 10x | 20 |

### Spawn Algorithm

```c
float GetSpawnRate(float gameTime) {
    // Starts slow, ramps up
    return 1.0f + gameTime * 0.1f;  // +1 enemy/sec every 10 seconds
}

int GetEnemyType(float gameTime) {
    // Unlock harder enemies over time
    if (gameTime < 30) return ENEMY_CHASER;
    if (gameTime < 60) return GetRandomValue(0, 1);  // Chaser or Orbiter
    if (gameTime < 120) return GetRandomValue(0, 3);
    return GetRandomValue(0, ENEMY_TYPE_COUNT - 1);
}

Vector2 GetSpawnPosition(Vector2 playerPos) {
    // Spawn off-screen, random direction
    float angle = GetRandomValue(0, 360) * DEG2RAD;
    float distance = 600.0f;  // Just outside view
    return (Vector2){
        playerPos.x + cosf(angle) * distance,
        playerPos.y + sinf(angle) * distance
    };
}
```

---

## UPGRADES

### Weapon Upgrades (Specific)
- **+1 Projectile**: Add another bullet
- **+Pierce**: Bullets go through enemies
- **+Fire Rate**: 20% faster shooting
- **+Damage**: 25% more damage
- **+Range**: 30% longer lifetime
- **+Size**: Bigger projectiles, bigger hitbox

### Player Upgrades (Global)
- **Max Health +20**
- **Movement Speed +10%**
- **XP Magnet Range +50%**
- **Armor +5** (flat damage reduction)
- **Regen +1 HP/sec**
- **Luck +10%** (better upgrade options)

### Special Upgrades (Rare)
- **Double Shot**: All weapons fire twice
- **Ricochet**: Projectiles bounce once
- **Vampirism**: 1% lifesteal
- **Time Warp**: 10% slow-mo aura around player
- **Nuclear Option**: Explosions on kill

### Level-Up Screen

```
┌─────────────────────────────────────────────────────────┐
│                     LEVEL UP!                           │
│                                                         │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐     │
│  │   WEAPON    │  │   PLAYER    │  │   SPECIAL   │     │
│  │             │  │             │  │             │     │
│  │ +1 Projectile│ │ +20% Speed  │ │  Vampirism  │     │
│  │             │  │             │  │             │     │
│  │   [1]       │  │   [2]       │  │   [3]       │     │
│  └─────────────┘  └─────────────┘  └─────────────┘     │
│                                                         │
│              Press 1, 2, or 3 to choose                 │
└─────────────────────────────────────────────────────────┘
```

---

## SHADERS

### Bloom (bloom.fs)

```glsl
#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float intensity;

void main() {
    vec4 color = texture(texture0, fragTexCoord);
    
    // Extract bright parts
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    vec4 bright = (brightness > 0.7) ? color : vec4(0.0);
    
    // Blur (simplified - use multi-pass for real bloom)
    vec4 blur = vec4(0.0);
    float blurSize = 0.003;
    for (int x = -2; x <= 2; x++) {
        for (int y = -2; y <= 2; y++) {
            blur += texture(texture0, fragTexCoord + vec2(x, y) * blurSize);
        }
    }
    blur /= 25.0;
    
    finalColor = color + blur * intensity;
}
```

### CRT Effect (crt.fs)

```glsl
#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
uniform float time;

void main() {
    vec2 uv = fragTexCoord;
    
    // Barrel distortion
    vec2 center = uv - 0.5;
    float dist = dot(center, center);
    uv = uv + center * dist * 0.1;
    
    // Scanlines
    float scanline = sin(uv.y * 800.0) * 0.04;
    
    // RGB offset
    float r = texture(texture0, uv + vec2(0.001, 0.0)).r;
    float g = texture(texture0, uv).g;
    float b = texture(texture0, uv - vec2(0.001, 0.0)).b;
    
    // Vignette
    float vignette = 1.0 - dist * 1.5;
    
    finalColor = vec4(r, g, b, 1.0) * vignette - scanline;
}
```

---

## AUDIO DESIGN

### Sound Effects
| Event | Sound | Priority |
|-------|-------|----------|
| Player shoot | Quick synth "pew" | Low |
| Enemy death | Bass hit + shatter | Medium |
| XP pickup | Rising arpeggio | Low |
| Level up | Fanfare chord | High |
| Player hit | Distorted thump | High |
| Boss spawn | Horn + bass drop | Critical |

### Music
- Single synthwave track, ~3 minutes, loopable
- Dynamic layers: bass always plays, drums kick in at wave 3, lead at wave 5
- BPM: 120-140 (matches gameplay intensity)

---

## PARTICLE SYSTEM

```c
typedef struct {
    Vector2 pos;
    Vector2 vel;
    Color color;
    float size;
    float lifetime;
    float maxLifetime;
    bool active;
} Particle;

typedef enum {
    EMITTER_EXPLOSION,    // Burst, outward velocity
    EMITTER_TRAIL,        // Continuous, follows entity
    EMITTER_FOUNTAIN,     // Upward with gravity
    EMITTER_RING,         // Expands outward in circle
} EmitterType;

void SpawnExplosion(Vector2 pos, Color color, int count) {
    for (int i = 0; i < count; i++) {
        Particle *p = GetFreeParticle();
        if (!p) return;
        
        float angle = GetRandomValue(0, 360) * DEG2RAD;
        float speed = GetRandomValue(100, 300);
        
        p->pos = pos;
        p->vel = (Vector2){ cosf(angle) * speed, sinf(angle) * speed };
        p->color = color;
        p->size = GetRandomValue(2, 6);
        p->lifetime = p->maxLifetime = GetRandomValue(20, 40) / 60.0f;
        p->active = true;
    }
}
```

---

## GAME FEEL CHECKLIST

- [x] **Screen shake**: Intensity scales with damage dealt
- [x] **Hitstop**: 2 frames on kill, 4 frames on elite kill
- [x] **Knockback**: Enemies pushed by projectiles
- [x] **Trails**: Player and projectiles leave fading trails
- [x] **Flash**: Enemies flash white when hit
- [ ] **Squash/stretch**: Player squishes on direction change
- [ ] **Anticipation**: 0.1s windup before boss attacks
- [ ] **Impact frames**: Single bright frame on explosions
- [x] **Sound layering**: Never more than 8 concurrent sounds
- [x] **Music ducking**: Music quiets during level-up screen
- [ ] **Chromatic aberration**: Pulses when player is low HP
- [x] **Dash ability**: SPACE to dash with invincibility
- [x] **Slow-mo**: Time slows near death and during level-up
- [x] **Tutorial**: Control hints shown for first 20 seconds

---

## PERFORMANCE TARGETS

| Metric | Target | How |
|--------|--------|-----|
| **FPS** | 60 locked | Fixed timestep, object pooling |
| **Enemies** | 500 simultaneous | Spatial hashing, culling off-screen |
| **Particles** | 2000 simultaneous | Pool, batch rendering |
| **Draw calls** | <100 | Texture atlasing, instancing |
| **Memory** | <50MB | Pre-allocated pools, no runtime allocs |

---

## MILESTONES

### Week 1: Prototype
- Player moves and shoots
- Enemies chase and die
- XP drops and levels up
- Single upgrade works
- **Deliverable**: Playable loop, placeholder art

### Week 2: Core Complete
- All 8 weapons implemented
- All enemy types working
- Spawn scaling functional
- 20 upgrades available
- **Deliverable**: Full gameplay, still ugly

### Week 3: Juice Pass
- All shaders working
- Particles everywhere
- Screen shake / hitstop
- Sound effects in
- **Deliverable**: Feels amazing to play

### Week 4: Content & Balance
- Evolution system
- Boss fights
- Meta-progression
- Balance pass (spreadsheet time)
- **Deliverable**: Has depth, replayable

### Week 5: Polish
- Menus and UI
- Settings
- Achievements
- Bug fixes
- **Deliverable**: Shippable game

---

## STRETCH GOALS

- [ ] **Co-op**: Split screen local multiplayer
- [ ] **Daily Challenge**: Seeded run with leaderboard
- [ ] **Endless Mode**: No boss, pure survival
- [ ] **Arena Hazards**: Laser walls, pits, turrets
- [ ] **Unlockable Characters**: Each with unique ability
- [ ] **Mod Support**: Lua scripting for custom weapons

---

## CONTROLS

### Keyboard
| Input | Action |
|-------|--------|
| WASD | Move |
| Mouse | Aim |
| Left Click | (Auto-fire, no input needed) |
| Space | Dash (1.5s cooldown, invincible) ✓ |
| 1/2/3 | Select upgrade |
| Escape | Pause |
| Tab | Show stats |

### Gamepad
| Input | Action |
|-------|--------|
| Left Stick | Move |
| Right Stick | Aim |
| A | Select upgrade |
| Start | Pause |

---

## FILE SIZE BUDGET

| Category | Target |
|----------|--------|
| Executable | <2 MB |
| Textures | <5 MB (compressed PNGs) |
| Shaders | <50 KB |
| Sounds | <10 MB (WAV for short, OGG for music) |
| **Total** | <20 MB |

---

## INSPIRATION / REFERENCES

- **Vampire Survivors** — Upgrade loop, exponential scaling
- **Geometry Wars** — Neon aesthetic, twin-stick feel
- **Nuclear Throne** — Screen shake, weapon variety
- **Enter the Gungeon** — Bullet patterns, boss design
- **Hades** — Polish, game feel, upgrade choices
- **Synthwave playlists** — Audio vibe

---

## FINAL NOTES

This game is designed to be:

1. **Achievable** — Uses only raylib built-in features (shapes, textures, shaders, audio)
2. **Polished** — Heavy focus on game feel and juice
3. **Addictive** — Simple loop with depth through upgrades
4. **Beautiful** — Neon aesthetic is striking with minimal art skill required
5. **Performant** — Designed for 500+ entities at 60fps

The key insight: **geometric shapes + glow shaders = instant style**. You don't need an artist. A white circle with bloom IS the aesthetic.

---

*"In the neon void, geometry is war."*
