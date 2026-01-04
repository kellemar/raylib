# CLAUDE.md - cool-game/

## OVERVIEW
NEON VOID: A survivor-like game built with raylib. Player fights waves of enemies, collects XP, and chooses upgrades on level-up.

## STRUCTURE
cool-game/
├── src/
│   ├── main.c          # Entry point, game loop
│   ├── game.h/c        # Game state machine, core loop
│   ├── types.h         # Constants, colors, pool sizes
│   ├── player.h/c      # Player entity (TODO)
│   ├── enemy.h/c       # Enemy pool and AI (TODO)
│   ├── projectile.h/c  # Projectile pool (TODO)
│   ├── weapon.h/c      # Weapon definitions (TODO)
│   ├── particle.h/c    # Particle effects (TODO)
│   ├── xp.h/c          # XP crystals (TODO)
│   ├── upgrade.h/c     # Upgrade system (TODO)
│   └── utils.h/c       # Math helpers (TODO)
├── resources/
│   ├── shaders/
│   ├── sounds/
│   ├── music/
│   └── textures/
├── Makefile
└── neon_void           # Output binary

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Game states | src/game.h | STATE_MENU, STATE_PLAYING, etc. |
| Screen/pool constants | src/types.h | SCREEN_WIDTH, MAX_ENEMIES, etc. |
| Color palette | src/types.h | NEON_CYAN, NEON_PINK, VOID_BLACK |
| State transitions | src/game.c | GameUpdate switch statement |
| Rendering | src/game.c | GameDraw switch statement |
| Implementation plan | ../IMPLEMENTATION_PLAN.md | Task breakdown by phase |

## CONVENTIONS
- Follow raylib code style: 4 spaces, Allman braces, TitleCase functions
- All pools use fixed-size arrays with active flags (no dynamic allocation)
- Each system: header declares struct + Init/Update/Draw, .c implements
- Game state changes only in GameUpdate(), never in Draw functions
- Use types.h colors (NEON_*) for consistent neon aesthetic


## DEVELOPMENT FLOW
Always run `make run` after every task is tested and completed to ensure that the game is able to build and compile.

## COMMANDS
```bash
make            # Build game
make run        # Build and run
make clean      # Remove build artifacts
```

## ANTI-PATTERNS
- Dynamic memory allocation (use fixed pools from types.h)
- Modifying game state in Draw functions
- Adding raylib.h include without going through types.h
- Hardcoding screen dimensions (use SCREEN_WIDTH/HEIGHT)
- Hardcoding colors (use NEON_* palette from types.h)

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
- [ ] Phase 1: Player System
- [ ] Phase 2: Weapons & Projectiles
- [ ] Phase 3: Enemies
- [ ] Phase 4: XP & Leveling
- [ ] Phase 5: Particles & Juice
- [ ] Phase 6: Additional Enemies
- [ ] Phase 7: Audio
- [ ] Phase 8: Visual Polish
- [ ] Phase 9: Menus & Polish
- [ ] Phase 10: Final Polish
