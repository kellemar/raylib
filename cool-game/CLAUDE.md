# CLAUDE.md - cool-game/

## OVERVIEW
NEON VOID: A survivor-like game built with raylib. Player fights waves of enemies, collects XP, and chooses upgrades on level-up.

## STRUCTURE
cool-game/
├── src/
│   ├── main.c          # Entry point, game loop
│   ├── game.h/c        # Game state machine, core loop
│   ├── types.h         # Constants, colors, pool sizes
│   ├── player.h/c      # Player entity, movement, aiming, health, weapon
│   ├── projectile.h/c  # Projectile pool with update/draw
│   ├── weapon.h/c      # Weapon definitions (pulse cannon)
│   ├── enemy.h/c       # Enemy pool and AI (TODO)
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

## TESTING WITH PEEKABOO

Use `peekaboo` to visually verify game changes without manual intervention.

### Launch Game and Capture Screen
```bash
# Build and launch game in background
cd cool-game
make clean && make
./neon_void &
sleep 3

# Capture full screen (multi-monitor: captures all screens)
peekaboo image --mode screen --retina --path ~/Desktop/game_test.png

# The game window is typically on screen1 if you have multiple monitors
# Check ~/Desktop/game_test_1.png for the game window
```

### Simulate Keyboard Input
```bash
# Press ENTER to start game from menu
osascript -e 'tell application "System Events" to key code 36'
sleep 2

# Capture gameplay
peekaboo image --mode screen --path ~/Desktop/gameplay.png
```

### Verify Screenshots with look_at Tool
After capturing screenshots, use the `look_at` tool to analyze them:
```
look_at(file_path="~/Desktop/gameplay.png", goal="Check if projectiles are firing toward mouse, player visible, HUD showing")
```

### Testing Workflow
1. `make clean && make` - Rebuild game
2. `./neon_void &` - Launch in background
3. `sleep 3` - Wait for window to open
4. `osascript -e 'tell application "System Events" to key code 36'` - Press ENTER
5. `sleep 2` - Wait for game to start
6. `peekaboo image --mode screen --path ~/Desktop/test.png` - Capture
7. Use `look_at` tool to verify screenshot
8. `pkill -f neon_void` - Kill game process

### Key Codes for osascript
- ENTER: `key code 36`
- ESC: `key code 53`
- SPACE: `key code 49`
- W/A/S/D: `key code 13/0/1/2`
- 1/2/3: `key code 18/19/20`

### Notes
- Game window title is "NEON VOID" but peekaboo may not find it by name
- Use `--mode screen` to capture all displays, then check numbered files
- Multi-monitor setups create multiple files: `test.png`, `test_1.png`, etc.
- Game typically appears on `_1.png` suffix file on multi-monitor setups

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
- [x] Phase 1: Player System (complete)
- [x] Phase 2: Weapons & Projectiles (complete)
- [ ] Phase 3: Enemies
- [ ] Phase 4: XP & Leveling
- [ ] Phase 5: Particles & Juice
- [ ] Phase 6: Additional Enemies
- [ ] Phase 7: Audio
- [ ] Phase 8: Visual Polish
- [ ] Phase 9: Menus & Polish
- [ ] Phase 10: Final Polish
