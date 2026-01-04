# CLAUDE.md - cool-game/

## OVERVIEW
NEON VOID: A survivor-like game built with raylib. Player fights waves of enemies, collects XP, and chooses upgrades on level-up.

## STRUCTURE
cool-game/
├── src/
│   ├── main.c          # Entry point, game loop
│   ├── game.h/c        # Game state machine, core loop, camera, collisions
│   ├── types.h         # Constants, colors, pool sizes
│   ├── utils.h/c       # Pure testable functions (collision, math)
│   ├── player.h/c      # Player entity, movement, aiming, health, weapon
│   ├── projectile.h/c  # Projectile pool with update/draw
│   ├── weapon.h/c      # Weapon definitions (pulse cannon)
│   ├── enemy.h/c       # Enemy pool, AI, spawning
│   ├── particle.h/c    # Particle effects (explosions, hit sparks)
│   ├── xp.h/c          # XP crystals, magnet collection
│   ├── upgrade.h/c     # 22 upgrade types with rarity system
│   └── ui.h/c          # HUD rendering (health, XP, score, etc)
├── tests/
│   ├── minunit.h       # Lightweight test framework
│   ├── test_main.c     # Test runner
│   ├── test_utils.c    # Tests for utility functions
│   └── test_enemy.c    # Tests for enemy pool logic
├── resources/
│   ├── shaders/
│   ├── sounds/
│   ├── music/
│   └── textures/
├── Makefile
├── neon_void           # Output binary
└── test_runner         # Test binary

## COMMANDS
```bash
make test       # Run unit tests (MUST PASS before proceeding)
make            # Build game
make run        # Build and run game
make clean      # Remove build artifacts
```

## DEVELOPMENT WORKFLOW (MANDATORY)

### Before ANY code changes are considered complete:
1. **Run tests FIRST**: `make test`
2. **ALL tests must pass** - if any test fails, fix it before proceeding
3. **Build game**: `make`
4. **Verify no compiler warnings**
5. **Run game**: `make run` (manual verification if needed)

### When adding new features:
1. Write tests for pure logic functions FIRST (if applicable)
2. Implement the feature
3. Run `make test` - must pass
4. Run `make` - must compile with zero warnings
5. Test manually with `make run`

### Test failure policy:
- **BLOCKING**: Cannot proceed to next task if tests fail
- **FIX IMMEDIATELY**: Test failures must be fixed before any new work
- Tests run in ~10ms, no excuse to skip them

## TESTING

### What to test (in tests/):
- Pure math functions (collision, distance, clamp)
- Spawn interval calculations
- Pool management (init, spawn, count tracking)
- Enemy/projectile stats initialization
- Any function that doesn't require raylib runtime

### What NOT to test:
- Rendering functions (require window)
- Input handling (require raylib)
- Audio playback

### Adding new tests:
1. Add test function in appropriate test_*.c file
2. Register with `mu_run_test(test_function_name)` in the suite
3. Run `make test` to verify

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Game states | src/game.h | STATE_MENU, STATE_PLAYING, etc. |
| Screen/pool constants | src/types.h | SCREEN_WIDTH, MAX_ENEMIES, MAX_PARTICLES |
| Color palette | src/types.h | NEON_CYAN, NEON_PINK, VOID_BLACK |
| State transitions | src/game.c | GameUpdate switch statement |
| Rendering | src/game.c | GameDraw with Camera2D, DrawGameWorld |
| Camera & screen shake | src/game.c | InitCamera, UpdateGameCamera, TriggerScreenShake |
| Particle effects | src/particle.h/c | SpawnExplosion, SpawnHitParticles |
| HUD rendering | src/ui.h/c | DrawHUD (screen-space UI) |
| Pure logic/math | src/utils.h/c | Collision, spawn interval |
| Enemy types/AI | src/enemy.h/c | ENEMY_CHASER, EnemyPool |
| Unit tests | tests/ | MinUnit framework, 22 tests |
| Implementation plan | ../IMPLEMENTATION_PLAN.md | Task breakdown by phase |

## CONVENTIONS
- Follow raylib code style: 4 spaces, Allman braces, TitleCase functions
- All pools use fixed-size arrays with active flags (no dynamic allocation)
- Each system: header declares struct + Init/Update/Draw, .c implements
- Game state changes only in GameUpdate(), never in Draw functions
- Use types.h colors (NEON_*) for consistent neon aesthetic
- Extract pure logic to utils.h/c for testability

## COORDINATE SYSTEMS (CRITICAL)

This game uses Camera2D. Understanding coordinate spaces prevents bugs.

| Space | Used For | Examples |
|-------|----------|----------|
| **Screen space** | Mouse input, HUD | `GetMousePosition()`, `DrawHUD()` (after EndMode2D) |
| **World space** | All game entities | Player, enemies, projectiles, particles, XP crystals |

### Converting Between Spaces
```c
Vector2 mouseScreen = GetMousePosition();
Vector2 mouseWorld = GetScreenToWorld2D(mouseScreen, camera);
```

### What Lives Where
| Screen Space | World Space |
|--------------|-------------|
| Mouse cursor position | Player position |
| HUD elements | Enemy positions |
| Menu/overlay text | Projectile positions |
| | Particle positions |
| | XP crystal positions |

### Camera Impact Checklist
When modifying camera or adding camera-dependent features:
- [ ] Mouse input uses `GetScreenToWorld2D()` before world-space comparisons
- [ ] No hardcoded `SCREEN_WIDTH/HEIGHT` bounds for world-space entities
- [ ] Projectile culling is lifetime-based, NOT position-based
- [ ] Enemy spawning is relative to player position, not screen origin
- [ ] HUD drawing happens AFTER `EndMode2D()` (screen space)

## ANTI-PATTERNS
- Dynamic memory allocation (use fixed pools from types.h)
- Modifying game state in Draw functions
- Adding raylib.h include without going through types.h
- Hardcoding colors (use NEON_* palette from types.h)
- Skipping `make test` before considering work complete
- Proceeding with failing tests
- **Using `GetMousePosition()` directly for world-space calculations** (must convert with `GetScreenToWorld2D`)
- **Clamping world-space positions to SCREEN_WIDTH/HEIGHT** (camera follows player, world is infinite)
- **Culling entities based on fixed screen bounds** (use lifetime or distance from player instead)

## GAME STATES
| State | Enter | Exit | Description |
|-------|-------|------|-------------|
| STATE_MENU | Game start, game over | ENTER | Title screen |
| STATE_PLAYING | ENTER from menu | ESC, death | Main gameplay |
| STATE_PAUSED | ESC from playing | ESC, Q | Pause overlay |
| STATE_LEVELUP | XP threshold | 1/2/3 | Upgrade selection |
| STATE_GAMEOVER | Player death | ENTER | Final score display |

## UPGRADE SYSTEM (22 Types)

### Weapon Upgrades (8)
| Type | Name | Effect | Rarity |
|------|------|--------|--------|
| UPGRADE_DAMAGE | Power Up | +25% Damage | Common |
| UPGRADE_FIRE_RATE | Rapid Fire | +20% Fire Rate | Common |
| UPGRADE_PROJECTILE_COUNT | Multi Shot | +1 Projectile | Uncommon |
| UPGRADE_PIERCE | Piercing | Shots pierce enemies | Uncommon |
| UPGRADE_RANGE | Long Range | +30% Projectile Range | Common |
| UPGRADE_PROJ_SIZE | Big Bullets | +25% Projectile Size | Common |
| UPGRADE_COOLDOWN | Quick Draw | -15% Weapon Cooldown | Common |
| UPGRADE_CRIT_CHANCE | Critical Eye | +10% Crit Chance | Uncommon |

### Player Upgrades (8)
| Type | Name | Effect | Rarity |
|------|------|--------|--------|
| UPGRADE_SPEED | Swift Feet | +10% Move Speed | Common |
| UPGRADE_MAX_HEALTH | Vitality | +20 Max HP | Common |
| UPGRADE_MAGNET | Magnetism | +50% Pickup Range | Common |
| UPGRADE_ARMOR | Tough Skin | +5 Armor | Common |
| UPGRADE_REGEN | Regeneration | +1 HP per second | Uncommon |
| UPGRADE_DASH_DAMAGE | Dash Strike | Deal damage while dashing | Uncommon |
| UPGRADE_XP_BOOST | Wisdom | +25% XP Gain | Common |
| UPGRADE_KNOCKBACK | Force Push | +50% Knockback | Common |

### Special Upgrades (6)
| Type | Name | Effect | Rarity |
|------|------|--------|--------|
| UPGRADE_DOUBLE_SHOT | Double Tap | Fire twice per shot | Rare |
| UPGRADE_VAMPIRISM | Vampirism | 1% Lifesteal on hit | Rare |
| UPGRADE_EXPLOSIVE | Explosive Shots | Shots explode on hit | Rare |
| UPGRADE_RICOCHET | Ricochet | Shots bounce once | Rare |
| UPGRADE_HOMING_BOOST | Heat Seeker | +100% Homing Strength | Uncommon |
| UPGRADE_SLOW_AURA | Time Warp | Slow nearby enemies | Rare |

### Rarity Colors
- **Common**: White border
- **Uncommon**: Green border
- **Rare**: Yellow/Gold border

## WEAPON EVOLUTION SYSTEM

Weapons evolve when reaching max level (5) + acquiring catalyst upgrade.

| Base Weapon | Catalyst Upgrade | Evolved Weapon | Key Enhancement |
|-------------|------------------|----------------|-----------------|
| Pulse Cannon | Pierce | MEGA CANNON | 50 damage, huge piercing beam |
| Spread Shot | Multi Shot | CIRCLE BURST | 16-projectile 360° nova |
| Homing Missile | Double Shot | SWARM | 6 homing missiles |
| Lightning | Crit Chance | TESLA COIL | 3 chain bolts, 5 chains each |
| Orbit Shield | Damage | BLADE DANCER | 2x orbit speed, 8 blades |
| Flamethrower | Range | INFERNO | Double flames, wider cone |
| Freeze Ray | Slow Aura | BLIZZARD | 4 ice shards, 80% slow |
| Black Hole | Explosive | SINGULARITY | 2x pull, 100 explosion radius |

### Evolution Requirements
- **Weapon Level 5**: Each weapon upgrade increases weapon level
- **Catalyst Upgrade**: Player must have acquired the specific catalyst
- **Auto-Check**: Evolution triggers automatically after upgrade selection

## ELITE ENEMIES

Elite variants of any enemy type spawn with increased stats and visual distinction.

### Elite Multipliers
| Stat | Multiplier | Effect |
|------|------------|--------|
| Size | 1.5x | 50% larger hitbox |
| Health | 3.0x | Much tankier |
| Damage | 1.5x | 50% more contact damage |
| XP | 5x | 5x XP reward on kill |
| Speed | 0.8x | Slightly slower (compensates for tankiness) |

### Spawn Chance
- Base: 10% chance for any enemy to be elite
- Scales: +1% per minute of gameplay
- Max: 25% at 15+ minutes

### Visual Identification
- **Gold Glow**: Pulsing gold glow effect behind elite enemies
- **Gold Border**: Double gold ring around elite enemies
- All other enemy visuals remain (color by type, ice effect if slowed)

### Key Functions
```c
Enemy* EnemySpawnElite(EnemyPool *pool, EnemyType type, Vector2 pos);
// Spawns enemy with elite multipliers applied

// Elite multiplier constants in enemy.h
#define ELITE_SPAWN_CHANCE  0.1f    // 10% base
#define ELITE_SIZE_MULT     1.5f
#define ELITE_HEALTH_MULT   3.0f
#define ELITE_DAMAGE_MULT   1.5f
#define ELITE_XP_MULT       5
#define ELITE_SPEED_MULT    0.8f
```

## BOSS ENEMIES

Boss enemies spawn every 5 minutes as major challenge events.

### Boss Stats
| Stat | Value | Notes |
|------|-------|-------|
| Health | 2000 base | Scales 50% per boss (boss #2 = 3000, #3 = 4000...) |
| Radius | 60 | Large, imposing visual |
| Damage | 30 | High contact damage |
| Speed | 50 | Slow approach, fast dash attack |
| XP | 100 × boss number | Major reward for defeating |

### Attack Pattern
1. **Approach Phase**: Slowly moves toward player (speed 50)
2. **Charge Phase**: Stops, shakes, red warning rings expand (1 second)
3. **Dash Phase**: Lunges at player at 8x normal speed
4. Cycle repeats every 3 seconds

### Spawn Timing
- First boss: 5:00 (300 seconds)
- Subsequent bosses: 5 minutes after previous defeated
- Warning: "BOSS INCOMING" display 5 seconds before spawn
- Boss timer pauses while boss is alive

### Visual Identification
- **Purple Glow**: Pulsing purple aura behind boss
- **Red Warning Rings**: Expanding red circles when charging
- **Menacing Eyes**: Red glowing eyes
- **Crown Spikes**: Magenta spikes radiating from top
- **Health Bar**: Large purple health bar at top of screen

### Key Functions
```c
Enemy* EnemySpawnBoss(EnemyPool *pool, Vector2 pos, int bossNumber);
bool EnemyPoolHasBoss(EnemyPool *pool);
Enemy* EnemyPoolGetBoss(EnemyPool *pool);

// Boss constants in enemy.h
#define BOSS_SPAWN_INTERVAL 300.0f  // 5 minutes
#define BOSS_BASE_HEALTH    2000.0f
#define BOSS_BASE_RADIUS    60.0f
#define BOSS_BASE_DAMAGE    30.0f
#define BOSS_BASE_SPEED     50.0f
#define BOSS_XP_VALUE       100
#define BOSS_ATTACK_INTERVAL 3.0f   // Seconds between attacks
#define BOSS_CHARGE_TIME    1.0f    // Anticipation before dash
```

## PERMANENT UNLOCKS SYSTEM

Persistent meta-progression saved to `unlocks.dat`.

### Weapon Unlock Conditions
| Weapon | Unlock Requirement |
|--------|-------------------|
| Pulse Cannon | Default (always unlocked) |
| Spread Shot | Kill 100 enemies |
| Homing Missile | Kill 500 enemies |
| Lightning | Reach level 10 |
| Orbit Shield | Survive 3 minutes |
| Flamethrower | Kill 1 boss |
| Freeze Ray | Kill 3 bosses |
| Black Hole | Score 10,000 lifetime points |

### Meta Upgrades (5 Levels Each)
| Stat | Bonus per Level | Max Bonus |
|------|-----------------|-----------|
| Speed | +2% | +10% |
| Health | +10 HP | +50 HP |
| Damage | +5% | +25% |
| XP Gain | +5% | +25% |
| Magnet | +10% | +50% |

### Character Unlock Conditions
| Character | Unlock Requirement |
|-----------|-------------------|
| Default | Always unlocked |
| Character 2 (Tank) | Play 5 games |
| Character 3 (Speedster) | Survive 5 minutes |

### Key Functions
```c
void UnlocksInit(UnlockData *unlocks);
void UnlocksSave(UnlockData *unlocks);
void UnlocksLoad(UnlockData *unlocks);
bool UnlocksHasWeapon(UnlockData *unlocks, WeaponType weapon);
void UnlocksUnlockWeapon(UnlockData *unlocks, WeaponType weapon);
void UnlocksAddRunStats(UnlockData *unlocks, int kills, int bossKills,
                        int score, int level, float survivalTime);
bool UnlocksCheckNewUnlocks(UnlockData *unlocks);
float UnlocksGetSpeedBonus(UnlockData *unlocks);
float UnlocksGetHealthBonus(UnlockData *unlocks);
float UnlocksGetDamageBonus(UnlockData *unlocks);
float UnlocksGetXPBonus(UnlockData *unlocks);
float UnlocksGetMagnetBonus(UnlockData *unlocks);

// Unlock constants in unlocks.h
#define META_UPGRADE_MAX_LEVEL 5
#define META_UPGRADE_COST_BASE 1000
#define META_UPGRADE_COST_MULT 2
```

## IMPLEMENTATION STATUS
- [x] Phase 0: Project Setup (complete)
- [x] Phase 1: Player System (complete)
- [x] Phase 2: Weapons & Projectiles (complete)
- [x] Phase 3: Enemies (complete)
- [x] Phase 4: XP & Leveling (complete)
- [x] Phase 5: Particles & Juice (complete)
- [x] Phase 6: Additional Enemies (complete)
- [x] Phase 7: Audio (complete)
- [x] Phase 8: Visual Polish (complete)
- [x] Phase 9: Menus & Polish (complete)
- [x] Phase 10: Final Polish (complete)
- [x] Phase 13: 22 Upgrade System (complete)
- [x] Phase 14: Weapon Evolution System (complete)
- [x] Phase 15: Elite Enemies (complete)
- [x] Phase 16: Boss Enemy System (complete)
- [x] Phase 17: Permanent Unlocks (complete)
- [x] Phase 18: Leaderboard System (complete)
- [x] Phase 19: Character Select (complete)
- [x] Phase 20: Achievement System (complete)

## CHARACTER SELECT SYSTEM

3 playable characters with distinct stats and colors.

### Character Stats
| Character | HP | Speed | Magnet | Armor | Damage | XP |
|-----------|-----|-------|--------|-------|--------|-----|
| VANGUARD | 100 | 300 | 80 | 0 | 1.0x | 1.0x |
| TITAN | 150 | 240 | 60 | 5 | 1.2x | 0.9x |
| PHANTOM | 70 | 380 | 120 | 0 | 0.9x | 1.25x |

### Unlock Conditions
- VANGUARD: Always unlocked
- TITAN: Play 5 games
- PHANTOM: Survive 5 minutes

### Key Functions
```c
CharacterDef GetCharacterDef(CharacterType type);
const char* CharacterGetName(CharacterType type);
const char* CharacterGetDescription(CharacterType type);
void PlayerInitWithCharacter(Player *player, CharacterType type);

#define CHARACTER_COUNT 3
```

## LEADERBOARD SYSTEM

Top 10 high scores saved to `leaderboard.dat`.

### Leaderboard Entry Data
| Field | Description |
|-------|-------------|
| score | Final score achieved |
| level | Level reached |
| kills | Enemies killed |
| survivalTime | Time survived (seconds) |
| day/month/year | Date played |

### Key Functions
```c
void LeaderboardInit(Leaderboard *lb);
void LeaderboardLoad(Leaderboard *lb);
void LeaderboardSave(Leaderboard *lb);
int LeaderboardAddEntry(Leaderboard *lb, int score, int level, int kills, float survivalTime);
bool LeaderboardIsHighScore(Leaderboard *lb, int score);
int LeaderboardGetMinScore(Leaderboard *lb);
LeaderboardEntry* LeaderboardGetEntry(Leaderboard *lb, int position);
int LeaderboardGetHighScore(Leaderboard *lb);

#define LEADERBOARD_MAX_ENTRIES 10
```

### Accessing the Leaderboard
- **From Menu**: Press L
- **From Game Over**: Press L
- **Returns to**: Menu (press ESC or ENTER)

## ACHIEVEMENT SYSTEM

12 achievements with persistent tracking saved to `achievements.dat`.

### Achievements
| Achievement | Type | Requirement |
|-------------|------|-------------|
| First Blood | Combat | Kill your first enemy |
| Centurion | Combat | Kill 100 enemies in one run |
| Slayer | Combat | Kill 1000 enemies total |
| Boss Hunter | Combat | Defeat your first boss |
| Boss Slayer | Combat | Defeat 5 bosses total |
| Survivor | Survival | Survive for 3 minutes |
| Veteran | Survival | Survive for 10 minutes |
| Immortal | Survival | No damage for 1 minute |
| Rising Star | Progression | Reach level 5 |
| Champion | Progression | Reach level 10 |
| Fully Evolved | Progression | Evolve a weapon |
| Completionist | Progression | Unlock all characters |

### Key Functions
```c
AchievementDef GetAchievementDef(AchievementType type);
bool AchievementIsEarned(AchievementData *data, AchievementType type);
bool AchievementEarn(AchievementData *data, AchievementType type);
void AchievementInit(AchievementData *data);
void AchievementSave(AchievementData *data);
void AchievementLoad(AchievementData *data);
int AchievementGetEarnedCount(AchievementData *data);

#define ACHIEVEMENT_COUNT 12
```

### Accessing Achievements
- **From Menu**: Press A
- **Returns to**: Menu (press ESC or ENTER)
- **Notification**: Popup appears when achievement unlocked during gameplay

## TESTING WITH PEEKABOO

Use `peekaboo` to visually verify game changes without manual intervention.

### Launch Game and Capture Screen
```bash
cd cool-game
make test       # MUST PASS FIRST
make clean && make
./neon_void &
sleep 3
peekaboo see --window-title "Neon Void" --app neon_void --json-output --path ~/Desktop/game_test.png
```

### Simulate Keyboard Input
```bash
osascript -e 'tell application "System Events" to key code 36'  # ENTER
sleep 2
peekaboo see --window-title "Neon Void" --app neon_void --json-output --path ~/Desktop/game_test.png
```

### Key Codes for osascript
- ENTER: `key code 36`
- ESC: `key code 53`
- SPACE: `key code 49`
- W/A/S/D: `key code 13/0/1/2`
- 1/2/3: `key code 18/19/20`
