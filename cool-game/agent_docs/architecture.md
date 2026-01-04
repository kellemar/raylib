# Architecture Notes

## Runtime Flow
- Entry point: `cool-game/src/main.c`
- Core state machine and frame orchestration: `cool-game/src/game.c`
- Game states and shared data types: `cool-game/src/game.h`

## Update Order (Gameplay State)
Order matters for collisions and effects. See `GameUpdate()` in `cool-game/src/game.c`.
- Player update
- Projectile pool update
- Enemy pool update
- XP pool update
- Particle pool update
- Camera update

## Draw Order (World)
World rendering is assembled in `DrawGameWorld()` in `cool-game/src/game.c`.
- Background grid
- Particles
- XP crystals
- Enemies
- Projectiles
- Player

HUD and menus are rendered in screen space after world rendering.

## Rendering Pipeline
Post-processing and menu rendering are handled in `DrawSceneToTexture()` and `GameDraw()` in `cool-game/src/game.c`.

## Coordinate Spaces
Camera2D is used for world rendering. Screen-space input must be converted to world space before gameplay comparisons. See `cool-game/src/game.c` and `cool-game/src/types.h` for constants.
