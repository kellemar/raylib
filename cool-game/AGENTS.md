# AGENTS.md - cool-game/

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
│   ├── enemy.h/c       # Enemy pool, AI, spawning, collisions
│   ├── xp.h/c          # XP crystals, magnet collection
│   ├── upgrade.h/c     # Upgrade definitions and application
│   ├── particle.h/c    # Particle effects (explosions, hit sparks)
│   └── ui.h/c          # HUD rendering (health, XP, score, etc)
├── tests/
│   ├── minunit.h       # Lightweight test framework
│   ├── test_main.c     # Test runner
│   ├── test_utils.c    # Tests for utility functions (16 tests)
│   └── test_enemy.c    # Tests for enemy pool logic (6 tests)
├── resources/
│   ├── shaders/
│   ├── sounds/
│   ├── music/
│   └── textures/
├── Makefile
├── neon_void           # Game binary
└── test_runner         # Test binary

## COMMANDS (in priority order)
```bash
make test       # Run unit tests - MUST PASS FIRST
make            # Build game - MUST have zero warnings
make run        # Build and run game
make clean      # Remove build artifacts
```

## MANDATORY WORKFLOW
**Before ANY task is considered complete:**
```bash
make test && make && echo "Ready to proceed"
```
- If `make test` fails: FIX TESTS FIRST, cannot proceed
- If `make` has warnings: FIX WARNINGS, cannot proceed

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
- **Skipping `make test` before completing a task**
- **Proceeding when tests fail**
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

## IMPLEMENTATION STATUS
- [x] Phase 0: Project Setup (complete)
- [x] Phase 1: Player System (complete)
- [x] Phase 2: Weapons & Projectiles (complete)
- [x] Phase 3: Enemies (complete)
- [x] Phase 4: XP & Leveling (complete)
- [x] Phase 5: Particles & Juice (complete)
- [x] Phase 6: Additional Enemies (complete)
- [ ] Phase 7: Audio
- [ ] Phase 8: Visual Polish
- [ ] Phase 9: Menus & Polish
- [ ] Phase 10: Final Polish
