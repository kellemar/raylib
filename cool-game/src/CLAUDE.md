# cool-game/src

Game implementation lives here. Entry point is `main.c`, and the core loop/state machine is in `game.c` and `game.h`.

## Structure
- `main.c` - Startup and main loop wiring
- `game.c` and `game.h` - State machine, camera, update/draw orchestration
- `types.h` - Shared constants, pool sizes, colors
- `utils.c` and `utils.h` - Pure logic helpers, unit-tested
- Entity systems: `player`, `enemy`, `projectile`, `xp`, `upgrade`, `particle`, `weapon`, `audio`, `ui`, `achievement`, `leaderboard`, `unlocks`, `character`

## Workflow
- Build and tests are run from `cool-game` via `make test` and `make`

## Constraints
- Do not edit `*.o` files in this folder
- Keep gameplay state changes in the update phase, not in draw helpers
- Convert screen-space input to world space when interacting with world entities

## Additional Context
See `cool-game/agent_docs/` for deeper system notes.
