# NEON VOID

> *A twin-stick roguelike shooter with procedural arenas, synthwave aesthetics, and one rule: don't stop moving.*

```
    ███╗   ██╗███████╗ ██████╗ ███╗   ██╗    ██╗   ██╗ ██████╗ ██╗██████╗
    ████╗  ██║██╔════╝██╔═══██╗████╗  ██║    ██║   ██║██╔═══██╗██║██╔══██╗
    ██╔██╗ ██║█████╗  ██║   ██║██╔██╗ ██║    ██║   ██║██║   ██║██║██║  ██║
    ██║╚██╗██║██╔══╝  ██║   ██║██║╚██╗██║    ╚██╗ ██╔╝██║   ██║██║██║  ██║
    ██║ ╚████║███████╗╚██████╔╝██║ ╚████║     ╚████╔╝ ╚██████╔╝██║██████╔╝
    ╚═╝  ╚═══╝╚══════╝ ╚═════╝ ╚═╝  ╚═══╝      ╚═══╝   ╚═════╝ ╚═╝╚═════╝
```

**Genre**: Twin-stick shooter / Bullet hell / Roguelike
**Inspiration**: Vampire Survivors meets Geometry Wars meets Nuclear Throne
**Built with**: [raylib](https://www.raylib.com/) in C99

---

## Building & Running

### Requirements
- C99 compiler (clang or gcc)
- raylib library (included in `../src/libraylib.a`)
- macOS, Linux, or Windows

### Build Commands

```bash
# Build and run
make run

# Build only
make

# Run tests
make test

# Clean build artifacts
make clean
```

---

## Controls

### Keyboard

| Key | Action |
|-----|--------|
| **WASD** / **Arrow Keys** | Move |
| **Mouse** | Aim |
| **SPACE** | Dash (invincible, 1.5s cooldown) |
| **Q** / **E** | Cycle weapons (during gameplay) |
| **1** / **2** / **3** | Select upgrade on level-up |
| **ESC** | Pause / Go back |
| **Q** | Quit (from main menu only) |
| **L** | Leaderboard (menu/game over) |
| **A** | Achievements (menu) |
| **TAB** | Settings (menu) |

### Gamepad

| Input | Action |
|-------|--------|
| Left Stick | Move |
| Right Stick | Aim |
| A Button | Dash / Select |
| Start | Pause |

---

## Game Overview

You are a geometric shape trapped in an infinite neon void. Waves of hostile geometry spawn endlessly. Survive as long as possible, collect experience crystals, and evolve your weapons into screen-clearing chaos.

### Core Loop

```
SPAWN → SURVIVE → COLLECT XP → LEVEL UP → CHOOSE UPGRADE → REPEAT
                                                ↓
                              DEATH → UNLOCK PROGRESS → RESTART
```

**Session length**: 10-20 minutes
**Difficulty**: Exponential enemy scaling, linear player power

---

## Features

### 1. Three Playable Characters

Select your character at the start of each run:

| Character | HP | Speed | Special |
|-----------|-----|-------|---------|
| **VANGUARD** | 100 | 300 | Balanced stats, good for learning |
| **TITAN** | 150 | 240 | 5 armor, 1.2x damage, tanky playstyle |
| **PHANTOM** | 70 | 380 | 1.25x XP gain, large magnet radius |

**How to unlock:**
- VANGUARD: Always available
- TITAN: Play 5 games
- PHANTOM: Survive 5 minutes in a single run

---

### 2. Eight Unique Weapons

Cycle through weapons with **Q/E** during gameplay:

| Weapon | Behavior |
|--------|----------|
| **Pulse Cannon** | Single shot, medium speed (default) |
| **Spread Shot** | 3-way spread pattern |
| **Homing Missiles** | Slow projectiles that seek enemies |
| **Lightning** | Fast piercing bolts with chain effect |
| **Orbit Shield** | Rotating projectiles orbit you |
| **Flamethrower** | Rapid short-range cone, DOT damage |
| **Freeze Ray** | Slows enemies on hit (ice effect) |
| **Black Hole** | Pulls nearby enemies toward projectile |

**Weapon unlocks** (persistent):
- Spread Shot → Kill 100 enemies total
- Homing Missile → Kill 500 enemies total
- Lightning → Reach level 10
- Orbit Shield → Survive 3 minutes
- Flamethrower → Kill 1 boss
- Freeze Ray → Kill 3 bosses
- Black Hole → Score 10,000 lifetime points

---

### 3. Weapon Evolution System

Max out a weapon (Level 5) + acquire its catalyst upgrade = **EVOLUTION**

| Base Weapon | Catalyst Upgrade | Evolved Form |
|-------------|------------------|--------------|
| Pulse Cannon | Pierce | **MEGA CANNON** - Huge piercing beam |
| Spread Shot | Multi Shot | **CIRCLE BURST** - 16-projectile 360° nova |
| Homing Missile | Double Shot | **SWARM** - 6 homing missiles |
| Lightning | Crit Chance | **TESLA COIL** - 3 chain bolts, 5 chains each |
| Orbit Shield | Damage | **BLADE DANCER** - 8 fast-orbiting blades |
| Flamethrower | Range | **INFERNO** - Double flames, wider cone |
| Freeze Ray | Slow Aura | **BLIZZARD** - 4 ice shards, 80% slow |
| Black Hole | Explosive | **SINGULARITY** - 2x pull, massive explosion |

---

### 4. 22 Upgrades with Rarity System

On level-up, choose from 3 random upgrades:

**Weapon Upgrades (8)**
- Damage +25%, Fire Rate +20%, Multi Shot +1, Pierce
- Long Range, Big Bullets, Quick Draw, Critical Eye

**Player Upgrades (8)**
- Speed +10%, Max HP +20, Magnet Range +50%, Armor +5
- Regen +1 HP/sec, Dash Damage, XP Boost +25%, Knockback +50%

**Special Upgrades (6)** - Rare!
- Double Tap, Vampirism (lifesteal), Explosive Shots
- Ricochet, Heat Seeker (homing boost), Time Warp (slow aura)

**Rarity indicators:**
- White border = Common
- Green border = Uncommon
- Yellow/Gold border = Rare

---

### 5. Enemy Types

| Enemy | Behavior | Threat |
|-------|----------|--------|
| **Chaser** | Moves directly at you | Fast, weak |
| **Orbiter** | Circles you, slowly closing in | Medium |
| **Splitter** | Splits into 2 smaller on death | Tricky |

**Elite Enemies** (gold glow):
- 1.5x size, 3x health, 1.5x damage
- 5x XP reward!
- Spawn chance: 10% base, +1% per minute (max 25%)

---

### 6. Boss Fights

A boss spawns every **5 minutes**:

**Warning signs:**
- "BOSS INCOMING" at 4:55
- 5-second countdown

**Boss attack pattern:**
1. Slowly approaches (purple glow, menacing eyes)
2. Stops and charges (shakes, red warning rings for 1 second)
3. Dashes at 8x speed
4. Repeats every 3 seconds

**Stats scale**: Each boss is 50% stronger than the last
**Rewards**: 100+ XP per boss

---

### 7. Permanent Progression

Progress persists between runs:

**Meta Upgrades** (5 levels each):
| Stat | Bonus per Level | Max |
|------|-----------------|-----|
| Speed | +2% | +10% |
| Health | +10 HP | +50 HP |
| Damage | +5% | +25% |
| XP Gain | +5% | +25% |
| Magnet | +10% | +50% |

**Tracked stats**: Total kills, boss kills, games played, highest level, longest survival

---

### 8. Leaderboard

**Access**: Press **L** from main menu or game over screen

Tracks your top 10 runs with:
- Score
- Level reached
- Enemies killed
- Survival time
- Date played

---

### 9. 12 Achievements

**Access**: Press **A** from main menu

| Achievement | Requirement |
|-------------|-------------|
| First Blood | Kill your first enemy |
| Centurion | Kill 100 enemies in one run |
| Slayer | Kill 1000 enemies total |
| Boss Hunter | Defeat your first boss |
| Boss Slayer | Defeat 5 bosses total |
| Survivor | Survive 3 minutes |
| Veteran | Survive 10 minutes |
| Immortal | Go 60 seconds without taking damage |
| Rising Star | Reach level 5 |
| Champion | Reach level 10 |
| Fully Evolved | Evolve a weapon |
| Completionist | Unlock all characters |

Achievements pop up during gameplay when earned!

---

### 10. Visual Effects & Game Feel

**Juice features:**
- Screen shake on hits and explosions
- Hitstop (brief freeze on big kills)
- Bloom shader (neon glow effect)
- CRT scanline overlay
- Chromatic aberration when low health
- Camera lerp (smooth follow)
- Slow-mo on level up
- Squash/stretch on player movement
- Impact frames (bright flash on kills)
- Particle explosions and trails

**Settings** (press TAB from menu):
- Music volume
- SFX volume
- Screen shake toggle
- CRT filter toggle

---

## Tips for New Players

1. **Keep moving** - Standing still is death
2. **Dash through danger** - You're invincible during dash (SPACE)
3. **Prioritize XP** - Level ups are your main power source
4. **Watch for gold enemies** - Elites give 5x XP
5. **Learn boss tells** - Red rings mean dodge NOW
6. **Build toward evolution** - Max weapon level + catalyst = power spike
7. **Check achievements** - They guide you toward content

---

## File Structure

```
cool-game/
├── src/
│   ├── main.c          # Entry point
│   ├── game.c/h        # Core game loop, state machine
│   ├── player.c/h      # Player movement, combat
│   ├── weapon.c/h      # 8 weapons + evolutions
│   ├── enemy.c/h       # Enemy AI, spawning, bosses
│   ├── projectile.c/h  # Projectile pool
│   ├── particle.c/h    # Visual effects
│   ├── upgrade.c/h     # 22 upgrade types
│   ├── xp.c/h          # XP crystals
│   ├── character.c/h   # 3 playable characters
│   ├── unlocks.c/h     # Permanent progression
│   ├── leaderboard.c/h # High scores
│   ├── achievement.c/h # Achievement system
│   ├── audio.c/h       # Sound management
│   └── ui.c/h          # HUD rendering
├── tests/              # Unit tests (154 tests)
├── resources/
│   ├── shaders/        # Bloom, CRT, chromatic
│   ├── sounds/         # SFX
│   └── music/          # Background track
├── Makefile
└── README.md
```

---

## Data Files

The game saves progress to these files (created automatically):

| File | Contents |
|------|----------|
| `settings.dat` | Volume, screen shake, CRT toggle |
| `unlocks.dat` | Weapon unlocks, meta upgrades, stats |
| `leaderboard.dat` | Top 10 high scores |
| `achievements.dat` | Earned achievements |

Delete these files to reset all progress.

---

## Credits

- **Engine**: [raylib](https://www.raylib.com/) by Ramon Santamaria
- **Design**: Inspired by Vampire Survivors, Geometry Wars, Nuclear Throne

---

*"In the neon void, geometry is war."*
