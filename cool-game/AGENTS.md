# cool-game

NEON VOID is a survivor-like game built with raylib. The codebase is a single C99 app with fixed-size entity pools and lightweight tests for pure logic.

## Stack
- C99, raylib
- Makefile build
- Lightweight unit tests in `cool-game/tests` (minunit)

## Structure
- `cool-game/src` - Game code and headers
- `cool-game/resources` - Shaders, sounds, music, textures
- `cool-game/tests` - Unit tests for pure logic and pools
- `cool-game/Makefile` - Build and test targets
- `cool-game/neon_void` - Built game binary
- `cool-game/test_runner` - Built test binary

## Commands
- `make test` - Run unit tests
- `make` - Build the game
- `make run` - Build and run the game
- `make clean` - Remove build artifacts

## Key Patterns
- Fixed-size pools with active flags; no dynamic allocation for runtime entities
- Game state transitions happen in `cool-game/src/game.c` (update phase only)
- Camera2D means screen-space input must be converted to world space; avoid screen bounds for world entities

## Constraints
- Do not edit build artifacts or runtime data files (`*.o`, `neon_void`, `test_runner`, `*.dat`)
- Prefer adding pure logic to `cool-game/src/utils.c` with matching tests in `cool-game/tests`

## Additional Context
Read `cool-game/README.md` for gameplay details, `cool-game/agent_docs/` for deeper system notes, and `cool-game/src/types.h` for shared constants.
