# CLAUDE.md - cool-game/

## OVERVIEW
NEON VOID: A survivor-like game built with raylib. Player fights waves of enemies, collects XP, and chooses upgrades on level-up.

## STRUCTURE
cool-game/
├── src/
│   ├── main.c          # Entry point, game loop
│   ├── game.h/c        # Game state machine, core loop
│   ├── types.h         # Constants, colors, pool sizes
│   ├── utils.h/c       # Pure testable functions (collision, math)
│   ├── player.h/c      # Player entity, movement, aiming, health, weapon
│   ├── projectile.h/c  # Projectile pool with update/draw
│   ├── weapon.h/c      # Weapon definitions (pulse cannon)
│   ├── enemy.h/c       # Enemy pool, AI, spawning
│   ├── particle.h/c    # Particle effects (TODO)
│   ├── xp.h/c          # XP crystals (TODO)
│   └── upgrade.h/c     # Upgrade system (TODO)
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
| Screen/pool constants | src/types.h | SCREEN_WIDTH, MAX_ENEMIES, etc. |
| Color palette | src/types.h | NEON_CYAN, NEON_PINK, VOID_BLACK |
| State transitions | src/game.c | GameUpdate switch statement |
| Rendering | src/game.c | GameDraw switch statement |
| Pure logic/math | src/utils.h/c | Collision, spawn interval |
| Enemy types/AI | src/enemy.h/c | ENEMY_CHASER, EnemyPool |
| Unit tests | tests/ | MinUnit framework |
| Implementation plan | ../IMPLEMENTATION_PLAN.md | Task breakdown by phase |

## CONVENTIONS
- Follow raylib code style: 4 spaces, Allman braces, TitleCase functions
- All pools use fixed-size arrays with active flags (no dynamic allocation)
- Each system: header declares struct + Init/Update/Draw, .c implements
- Game state changes only in GameUpdate(), never in Draw functions
- Use types.h colors (NEON_*) for consistent neon aesthetic
- Extract pure logic to utils.h/c for testability

## ANTI-PATTERNS
- Dynamic memory allocation (use fixed pools from types.h)
- Modifying game state in Draw functions
- Adding raylib.h include without going through types.h
- Hardcoding screen dimensions (use SCREEN_WIDTH/HEIGHT)
- Hardcoding colors (use NEON_* palette from types.h)
- Skipping `make test` before considering work complete
- Proceeding with failing tests

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
- [ ] Phase 4: XP & Leveling
- [ ] Phase 5: Particles & Juice
- [ ] Phase 6: Additional Enemies
- [ ] Phase 7: Audio
- [ ] Phase 8: Visual Polish
- [ ] Phase 9: Menus & Polish
- [ ] Phase 10: Final Polish

## TESTING WITH PEEKABOO

Use `peekaboo` to visually verify game changes without manual intervention.

### Launch Game and Capture Screen
```bash
cd cool-game
make test       # MUST PASS FIRST
make clean && make
./neon_void &
sleep 3
peekaboo image --mode screen --retina --path ~/Desktop/game_test.png
```

### Simulate Keyboard Input
```bash
osascript -e 'tell application "System Events" to key code 36'  # ENTER
sleep 2
peekaboo image --mode screen --path ~/Desktop/gameplay.png
```

### Key Codes for osascript
- ENTER: `key code 36`
- ESC: `key code 53`
- SPACE: `key code 49`
- W/A/S/D: `key code 13/0/1/2`
- 1/2/3: `key code 18/19/20`
