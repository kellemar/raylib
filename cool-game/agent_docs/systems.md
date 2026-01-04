# Game Systems

## Entity Systems
Entity systems are organized by module in `cool-game/src`.
- Player: `player.c` and `player.h`
- Enemies: `enemy.c` and `enemy.h`
- Projectiles: `projectile.c` and `projectile.h`
- XP pickups: `xp.c` and `xp.h`
- Particles: `particle.c` and `particle.h`
- Weapons: `weapon.c` and `weapon.h`

Each system exposes init/update/draw entry points and uses fixed-size pools defined in `cool-game/src/types.h`.

## Progression
- Upgrades and selection flow: `upgrade.c` and `upgrade.h`
- Leveling and XP tuning: `xp.c` and `xp.h`
- Character selection and stats: `character.c` and `character.h`

## Meta Systems
- Achievements: `achievement.c` and `achievement.h`
- Leaderboard: `leaderboard.c` and `leaderboard.h`
- Permanent unlocks: `unlocks.c` and `unlocks.h`

## UI and Audio
- HUD, menus, overlays: `ui.c` and `ui.h`
- Music and SFX: `audio.c` and `audio.h`

## Persistence
Runtime data is stored in `cool-game/*.dat`. These are written by the systems above and should not be edited manually.
