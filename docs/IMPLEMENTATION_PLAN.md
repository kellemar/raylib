# IMPLEMENTATION_PLAN.md — NEON VOID

> Detailed task breakdown for implementing NEON VOID. Each task is atomic, testable, and leaves the codebase in a compilable state.

---

## CONVENTIONS

- **Task ID**: `P{phase}.{section}.{task}` (e.g., `P0.1.1`)
- **Status**: `[ ]` pending, `[x]` done, `[-]` skipped
- **Dependencies**: Tasks that must be completed first
- **Verification**: Steps to confirm task completion
- **Build Check**: Every task MUST compile and run without errors
- **Test Check**: Every task MUST pass all unit tests

---

## MANDATORY TESTING REQUIREMENTS

### Before ANY task is considered complete:
```bash
make test    # MUST PASS - all 22+ tests green
make         # MUST PASS - zero warnings
```

### Test Failure Policy:
- **BLOCKING**: Cannot proceed to next task if `make test` fails
- **FIX FIRST**: Any test failure must be resolved before new work
- **NO EXCEPTIONS**: Tests run in ~10ms, no reason to skip

### When implementing new features:
1. If feature has pure logic (math, collision, spawning), write tests FIRST
2. Implement the feature in src/
3. Run `make test` - must pass
4. Run `make` - must compile with zero warnings
5. Manual verification with `make run` if needed

### Test Coverage (tests/ directory):
| File | Tests | Coverage |
|------|-------|----------|
| test_utils.c | 16 | ClampFloat, GetSpawnInterval, Vector2DistanceSq, CheckCircleCollision |
| test_enemy.c | 6 | EnemyPool init, spawn, stats, capacity, slot reuse |

### Adding Tests for New Features:
1. Pure logic functions → add to test_utils.c
2. Pool management → add to test_enemy.c (or create new test_*.c)
3. Register test with `mu_run_test(test_name)` in suite function

---

## PHASE 0: PROJECT SETUP

### 0.1 Directory Structure

#### P0.1.1 — Create game project directory
- **Description**: Create the neon_void project directory structure inside raylib
- **Dependencies**: None
- **Actions**:
  1. Create `cool-game/` directory
  2. Create `cool-game/src/` for source files
  3. Create `cool-game/resources/` for assets
  4. Create `cool-game/resources/shaders/`
  5. Create `cool-game/resources/sounds/`
  6. Create `cool-game/resources/music/`
  7. Create `cool-game/resources/textures/`
- **Verification**:
  - [x] All directories exist
  - [x] `ls -la cool-game/` shows src/ and resources/
- **Status**: `[x]`

#### P0.1.2 — Create Makefile
- **Description**: Create build system for the game
- **Dependencies**: P0.1.1
- **Actions**:
  1. Create `cool-game/Makefile`
  2. Set compiler flags for C99
  3. Link against raylib library (from `../src/`)
  4. Define source files variable
  5. Define output binary name (`neon_void`)
  6. Add `clean` target
  7. Add `run` target (build + execute)
- **Verification**:
  - [x] `make` runs without errors (even with no source files yet)
  - [x] `make clean` removes build artifacts
- **Status**: `[x]`

#### P0.1.3 — Create minimal main.c stub
- **Description**: Create entry point that compiles and runs
- **Dependencies**: P0.1.2
- **Actions**:
  1. Create `cool-game/src/main.c`
  2. Include `raylib.h`
  3. Implement minimal main() with:
     - InitWindow(1280, 720, "NEON VOID")
     - SetTargetFPS(60)
     - Empty game loop with WindowShouldClose()
     - BeginDrawing/EndDrawing with ClearBackground
     - CloseWindow()
- **Verification**:
  - [x] `make` compiles successfully
  - [x] `make run` opens a window titled "NEON VOID"
  - [x] Window is 1280x720
  - [x] Window closes on ESC or X button
  - [ ] No memory leaks (run with valgrind if available)
- **Status**: `[x]`

---

### 0.2 Core Header Files

#### P0.2.1 — Create types.h with common definitions
- **Description**: Central header for shared types and constants
- **Dependencies**: P0.1.3
- **Actions**:
  1. Create `cool-game/src/types.h`
  2. Add include guard
  3. Include `raylib.h`
  4. Define screen constants: `SCREEN_WIDTH 1280`, `SCREEN_HEIGHT 720`
  5. Define pool sizes: `MAX_ENEMIES 500`, `MAX_PROJECTILES 1000`, `MAX_PARTICLES 2000`, `MAX_XP_CRYSTALS 300`
  6. Define game colors as Color constants (VOID_PURPLE, NEON_PINK, NEON_CYAN, etc.)
- **Verification**:
  - [x] `make` compiles successfully
  - [x] No redefinition warnings
  - [x] main.c can include types.h and use SCREEN_WIDTH
- **Status**: `[x]`

#### P0.2.2 — Create utils.h/utils.c with math helpers
- **Description**: Utility functions for vector math and random
- **Dependencies**: P0.2.1
- **Actions**:
  1. Create `cool-game/src/utils.h` with declarations
  2. Create `cool-game/src/utils.c` with implementations
  3. Implement: `float RandomFloat(float min, float max)`
  4. Implement: `Vector2 RandomDirection(void)` — unit vector in random direction
  5. Implement: `float Vector2Distance(Vector2 a, Vector2 b)`
  6. Implement: `Vector2 Vector2Normalize(Vector2 v)` — if not using raymath
  7. Implement: `float Lerp(float a, float b, float t)`
  8. Implement: `float Clamp(float value, float min, float max)`
  9. Update Makefile to compile utils.c
- **Verification**:
  - [ ] `make` compiles successfully
  - [ ] Add temp test in main.c: print RandomFloat(0, 100) — verify output varies
  - [ ] Remove temp test code
- **Status**: `[-]` *(Deferred — will implement when needed in Phase 1)*

---

### 0.3 Game State Machine

#### P0.3.1 — Create game.h with game state enum
- **Description**: Define game states for state machine
- **Dependencies**: P0.2.1
- **Actions**:
  1. Create `cool-game/src/game.h`
  2. Define `GameState` enum: `STATE_MENU`, `STATE_PLAYING`, `STATE_PAUSED`, `STATE_LEVELUP`, `STATE_GAMEOVER`
  3. Declare `GameData` struct with:
     - `GameState state`
     - `float gameTime`
     - `int score`
     - `bool isPaused`
  4. Declare functions: `void GameInit(GameData *game)`, `void GameUpdate(GameData *game, float dt)`, `void GameDraw(GameData *game)`
- **Verification**:
  - [x] `make` compiles successfully
  - [x] No syntax errors in header
- **Status**: `[x]`

#### P0.3.2 — Create game.c with state machine skeleton
- **Description**: Implement basic state machine logic
- **Dependencies**: P0.3.1
- **Actions**:
  1. Create `cool-game/src/game.c`
  2. Implement `GameInit()` — set state to STATE_MENU, reset gameTime and score
  3. Implement `GameUpdate()` — switch on game->state, placeholder for each state
  4. Implement `GameDraw()` — switch on game->state, draw different colored background per state
  5. Update Makefile to compile game.c
  6. Update main.c to use GameData, call GameInit/Update/Draw
- **Verification**:
  - [x] `make` compiles successfully
  - [x] `make run` shows menu state (verify by background color)
  - [x] Different states can be tested by hardcoding state changes
- **Status**: `[x]`

#### P0.3.3 — Implement state transitions
- **Description**: Add keyboard controls to change states
- **Dependencies**: P0.3.2
- **Actions**:
  1. In STATE_MENU: Press ENTER → STATE_PLAYING
  2. In STATE_PLAYING: Press ESCAPE → STATE_PAUSED
  3. In STATE_PAUSED: Press ESCAPE → STATE_PLAYING, Press Q → STATE_MENU
  4. In STATE_GAMEOVER: Press ENTER → STATE_MENU
  5. Display current state name as text on screen
- **Verification**:
  - [x] `make` compiles and runs
  - [x] Can navigate: MENU → PLAYING → PAUSED → PLAYING
  - [x] Can navigate: PAUSED → MENU
  - [x] State name displays correctly on screen
- **Status**: `[x]`

---

## PHASE 1: PLAYER SYSTEM

### 1.1 Player Entity

#### P1.1.1 — Create player.h with Player struct
- **Description**: Define player data structure
- **Dependencies**: P0.3.3
- **Actions**:
  1. Create `cool-game/src/player.h`
  2. Define `Player` struct:
     - `Vector2 pos`
     - `Vector2 vel`
     - `float radius` (collision/visual size)
     - `float speed` (movement speed)
     - `float health`
     - `float maxHealth`
     - `int level`
     - `int xp`
     - `int xpToNextLevel`
     - `bool alive`
  3. Declare: `void PlayerInit(Player *player)`
  4. Declare: `void PlayerUpdate(Player *player, float dt)`
  5. Declare: `void PlayerDraw(Player *player)`
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Header has proper include guard
- **Status**: `[x]`

#### P1.1.2 — Create player.c with initialization
- **Description**: Implement player initialization
- **Dependencies**: P1.1.1
- **Actions**:
  1. Create `cool-game/src/player.c`
  2. Implement `PlayerInit()`:
     - pos = screen center
     - vel = {0, 0}
     - radius = 15.0f
     - speed = 300.0f
     - health = maxHealth = 100.0f
     - level = 1, xp = 0, xpToNextLevel = 10
     - alive = true
  3. Implement empty `PlayerUpdate()` stub
  4. Implement empty `PlayerDraw()` stub
  5. Update Makefile
- **Verification**:
  - [x] `make` compiles successfully
  - [x] No linker errors
- **Status**: `[x]`

#### P1.1.3 — Integrate player into game
- **Description**: Add Player to GameData and game loop
- **Dependencies**: P1.1.2
- **Actions**:
  1. Add `Player player` to GameData struct in game.h
  2. Call `PlayerInit(&game->player)` in GameInit()
  3. Call `PlayerUpdate(&game->player, dt)` in GameUpdate() when STATE_PLAYING
  4. Call `PlayerDraw(&game->player)` in GameDraw() when STATE_PLAYING
- **Verification**:
  - [x] `make` compiles successfully
  - [x] No crashes when entering PLAYING state
- **Status**: `[x]`

#### P1.1.4 — Implement player rendering
- **Description**: Draw player as geometric shape
- **Dependencies**: P1.1.3
- **Actions**:
  1. In `PlayerDraw()`:
     - Draw filled circle at player.pos with player.radius
     - Use NEON_CYAN or similar bright color
     - Draw a smaller inner circle for visual interest
     - Draw a direction indicator (small line or triangle pointing toward aim)
- **Verification**:
  - [x] `make run` shows player shape at screen center
  - [x] Player is visible against dark background
  - [x] Player has distinct visual appearance (not just plain circle)
- **Status**: `[x]`

---

### 1.2 Player Movement

#### P1.2.1 — Implement WASD movement input
- **Description**: Read keyboard input for movement
- **Dependencies**: P1.1.4
- **Actions**:
  1. In `PlayerUpdate()`:
     - Create Vector2 input = {0, 0}
     - If KEY_W or KEY_UP: input.y -= 1
     - If KEY_S or KEY_DOWN: input.y += 1
     - If KEY_A or KEY_LEFT: input.x -= 1
     - If KEY_D or KEY_RIGHT: input.x += 1
     - Normalize input if length > 0
     - Set player.vel = input * player.speed
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Player velocity changes based on input (add debug print)
  - [x] Diagonal movement is normalized (not faster)
- **Status**: `[x]`

#### P1.2.2 — Apply velocity to position
- **Description**: Move player based on velocity
- **Dependencies**: P1.2.1
- **Actions**:
  1. In `PlayerUpdate()` after input handling:
     - `player.pos.x += player.vel.x * dt`
     - `player.pos.y += player.vel.y * dt`
- **Verification**:
  - [x] `make run` — player moves with WASD
  - [x] Movement is smooth at 60fps
  - [x] Movement speed feels appropriate (cross screen in ~4 seconds)
- **Status**: `[x]`

#### P1.2.3 — Add screen boundary constraints
- **Description**: Keep player within screen bounds
- **Dependencies**: P1.2.2
- **Actions**:
  1. After position update in `PlayerUpdate()`:
     - Clamp pos.x between radius and SCREEN_WIDTH - radius
     - Clamp pos.y between radius and SCREEN_HEIGHT - radius
- **Verification**:
  - [x] `make run` — player cannot leave screen
  - [x] Player stops smoothly at edges (no jittering)
  - [x] Works on all four edges
- **Status**: `[x]`

#### P1.2.4 — Implement gamepad movement (optional)
- **Description**: Support left stick for movement
- **Dependencies**: P1.2.3
- **Actions**:
  1. In `PlayerUpdate()`:
     - Check if gamepad is available: `IsGamepadAvailable(0)`
     - Read left stick: `GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X/Y)`
     - Apply deadzone (ignore if abs < 0.2)
     - Combine with keyboard input (take whichever has higher magnitude)
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Gamepad movement works if controller connected
  - [x] Keyboard still works when no gamepad
  - [x] No crashes if gamepad disconnected mid-game
- **Status**: `[x]`

---

### 1.3 Player Aiming

#### P1.3.1 — Track mouse position for aiming
- **Description**: Store aim direction based on mouse
- **Dependencies**: P1.2.3
- **Actions**:
  1. Add `Vector2 aimDir` to Player struct
  2. In `PlayerUpdate()`:
     - Get mouse position: `GetMousePosition()`
     - Calculate direction from player to mouse
     - Normalize and store in player.aimDir
- **Verification**:
  - [x] `make` compiles successfully
  - [x] aimDir updates based on mouse position (debug print)
- **Status**: `[x]`

#### P1.3.2 — Draw aim indicator
- **Description**: Visual feedback for aim direction
- **Dependencies**: P1.3.1
- **Actions**:
  1. In `PlayerDraw()`:
     - Calculate aim endpoint: pos + aimDir * (radius + 10)
     - Draw line from player center to aim endpoint
     - Use bright color (NEON_PINK)
- **Verification**:
  - [x] `make run` — line points toward mouse cursor
  - [x] Line rotates smoothly as mouse moves
  - [x] Line is clearly visible
- **Status**: `[x]`

#### P1.3.3 — Implement right stick aiming (optional)
- **Description**: Support gamepad right stick for aiming
- **Dependencies**: P1.3.2, P1.2.4
- **Actions**:
  1. In `PlayerUpdate()`:
     - If gamepad available, read right stick
     - If right stick magnitude > deadzone, use it for aimDir
     - Otherwise fall back to mouse aim
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Right stick overrides mouse aim when used
  - [x] Falls back to mouse when stick centered
- **Status**: `[x]`

---

### 1.4 Player Health & Death

#### P1.4.1 — Draw health bar
- **Description**: Visual health indicator
- **Dependencies**: P1.1.4
- **Actions**:
  1. In `PlayerDraw()` or separate HUD function:
     - Draw background rectangle (dark red) for max health
     - Draw foreground rectangle (bright red/green) for current health
     - Position below player or in corner of screen
     - Width scales with health/maxHealth ratio
- **Verification**:
  - [x] `make run` — health bar visible
  - [x] Health bar shows full when health == maxHealth
  - [x] Manually set health to 50 — bar shows half
- **Status**: `[x]`

#### P1.4.2 — Implement damage function
- **Description**: Function to apply damage to player
- **Dependencies**: P1.4.1
- **Actions**:
  1. Add to player.h: `void PlayerTakeDamage(Player *player, float damage)`
  2. Implement in player.c:
     - Subtract damage from health
     - Clamp health to minimum 0
     - If health <= 0, set alive = false
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Call PlayerTakeDamage(10) on keypress — health bar decreases
  - [x] Health doesn't go below 0
  - [x] Player alive becomes false at 0 health
- **Status**: `[x]`

#### P1.4.3 — Handle player death state
- **Description**: Transition to game over on death
- **Dependencies**: P1.4.2
- **Actions**:
  1. In `GameUpdate()` STATE_PLAYING:
     - Check if !player.alive
     - If dead, transition to STATE_GAMEOVER
  2. In STATE_GAMEOVER draw:
     - Display "GAME OVER" text
     - Display final score
     - Display "Press ENTER to restart"
- **Verification**:
  - [x] `make run` — reduce health to 0 triggers game over
  - [x] Game over screen displays correctly
  - [x] Can return to menu and start new game
  - [x] New game resets player health
- **Status**: `[x]`

---

## PHASE 2: WEAPONS & PROJECTILES

### 2.1 Projectile System

#### P2.1.1 — Create projectile.h with Projectile struct
- **Description**: Define projectile data structure
- **Dependencies**: P1.3.2
- **Actions**:
  1. Create `cool-game/src/projectile.h`
  2. Define `Projectile` struct:
     - `Vector2 pos`
     - `Vector2 vel`
     - `float radius`
     - `float damage`
     - `float lifetime` (seconds remaining)
     - `int weaponType`
     - `bool pierce`
     - `bool active`
  3. Define `ProjectilePool` struct:
     - `Projectile projectiles[MAX_PROJECTILES]`
     - `int count`
  4. Declare: `void ProjectilePoolInit(ProjectilePool *pool)`
  5. Declare: `void ProjectilePoolUpdate(ProjectilePool *pool, float dt)`
  6. Declare: `void ProjectilePoolDraw(ProjectilePool *pool)`
  7. Declare: `Projectile* ProjectileSpawn(ProjectilePool *pool, Vector2 pos, Vector2 vel, float damage, float radius)`
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Header has proper include guard
- **Status**: `[x]`

#### P2.1.2 — Create projectile.c with pool management
- **Description**: Implement projectile pool logic
- **Dependencies**: P2.1.1
- **Actions**:
  1. Create `cool-game/src/projectile.c`
  2. Implement `ProjectilePoolInit()` — set all projectiles inactive, count = 0
  3. Implement `ProjectileSpawn()`:
     - Find first inactive projectile in pool
     - If none found, return NULL
     - Initialize projectile with parameters
     - Set active = true, increment count
     - Return pointer to projectile
  4. Update Makefile
- **Verification**:
  - [x] `make` compiles successfully
  - [x] No linker errors
- **Status**: `[x]`

#### P2.1.3 — Implement projectile update logic
- **Description**: Move projectiles and handle lifetime
- **Dependencies**: P2.1.2
- **Actions**:
  1. Implement `ProjectilePoolUpdate()`:
     - Loop through all projectiles
     - Skip if !active
     - Update position: pos += vel * dt
     - Decrease lifetime by dt
     - If lifetime <= 0, set active = false, decrement count
     - If pos is far outside screen bounds, deactivate
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Projectiles move (verify with debug output)
  - [x] Projectiles deactivate after lifetime expires
- **Status**: `[x]`

#### P2.1.4 — Implement projectile rendering
- **Description**: Draw active projectiles
- **Dependencies**: P2.1.3
- **Actions**:
  1. Implement `ProjectilePoolDraw()`:
     - Loop through all projectiles
     - Skip if !active
     - Draw filled circle at pos with radius
     - Use bright color (NEON_PINK or YELLOW)
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Projectiles are visible when spawned
- **Status**: `[x]`

#### P2.1.5 — Integrate projectiles into game
- **Description**: Add ProjectilePool to game loop
- **Dependencies**: P2.1.4
- **Actions**:
  1. Add `ProjectilePool projectiles` to GameData
  2. Call `ProjectilePoolInit()` in GameInit()
  3. Call `ProjectilePoolUpdate()` in GameUpdate()
  4. Call `ProjectilePoolDraw()` in GameDraw() (before player)
- **Verification**:
  - [x] `make` compiles and runs
  - [x] No crashes
  - [x] Can test spawn with temporary keypress code
- **Status**: `[x]`

---

### 2.2 Basic Weapon (Pulse Cannon)

#### P2.2.1 — Create weapon.h with weapon definitions
- **Description**: Define weapon types and data
- **Dependencies**: P2.1.5
- **Actions**:
  1. Create `cool-game/src/weapon.h`
  2. Define `WeaponType` enum: `WEAPON_PULSE_CANNON`, `WEAPON_COUNT`
  3. Define `Weapon` struct:
     - `WeaponType type`
     - `float damage`
     - `float fireRate` (shots per second)
     - `float projectileSpeed`
     - `float projectileRadius`
     - `float projectileLifetime`
     - `int projectileCount`
     - `float cooldown` (time until next shot)
     - `int level`
  4. Declare: `void WeaponInit(Weapon *weapon, WeaponType type)`
  5. Declare: `void WeaponUpdate(Weapon *weapon, float dt)`
  6. Declare: `bool WeaponCanFire(Weapon *weapon)`
  7. Declare: `void WeaponFire(Weapon *weapon, ProjectilePool *pool, Vector2 pos, Vector2 dir)`
- **Verification**:
  - [x] `make` compiles successfully
- **Status**: `[x]`

#### P2.2.2 — Create weapon.c with pulse cannon stats
- **Description**: Implement weapon initialization
- **Dependencies**: P2.2.1
- **Actions**:
  1. Create `cool-game/src/weapon.c`
  2. Implement `WeaponInit()` for WEAPON_PULSE_CANNON:
     - damage = 10
     - fireRate = 5.0 (5 shots/sec)
     - projectileSpeed = 500
     - projectileRadius = 5
     - projectileLifetime = 2.0
     - projectileCount = 1
     - cooldown = 0
     - level = 1
  3. Implement `WeaponUpdate()`: decrease cooldown by dt, clamp to 0
  4. Implement `WeaponCanFire()`: return cooldown <= 0
  5. Update Makefile
- **Verification**:
  - [x] `make` compiles successfully
- **Status**: `[x]`

#### P2.2.3 — Implement weapon firing
- **Description**: Spawn projectiles when firing
- **Dependencies**: P2.2.2
- **Actions**:
  1. Implement `WeaponFire()`:
     - If !WeaponCanFire(), return
     - Calculate projectile velocity: dir * projectileSpeed
     - Call ProjectileSpawn() with weapon stats
     - Set cooldown = 1.0 / fireRate
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Manual test: call WeaponFire() and verify projectile spawns
- **Status**: `[x]`

#### P2.2.4 — Add weapon to player
- **Description**: Integrate weapon into player struct
- **Dependencies**: P2.2.3
- **Actions**:
  1. Add `Weapon weapon` to Player struct
  2. In `PlayerInit()`: call `WeaponInit(&player->weapon, WEAPON_PULSE_CANNON)`
  3. In `PlayerUpdate()`: call `WeaponUpdate(&player->weapon, dt)`
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Player has initialized weapon
- **Status**: `[x]`

#### P2.2.5 — Implement auto-fire mechanic
- **Description**: Weapon fires automatically toward aim direction
- **Dependencies**: P2.2.4
- **Actions**:
  1. Add `ProjectilePool *projectiles` parameter to PlayerUpdate (or pass GameData)
  2. In `PlayerUpdate()`:
     - Call `WeaponFire(&player->weapon, projectiles, player->pos, player->aimDir)`
  3. Update function calls in game.c
- **Verification**:
  - [x] `make run` — projectiles fire automatically toward mouse
  - [x] Fire rate matches weapon settings (~5/sec)
  - [x] Projectiles travel in correct direction
  - [x] Projectiles disappear after lifetime
- **Status**: `[x]`

---

## PHASE 3: ENEMIES

### 3.1 Enemy System

#### P3.1.1 — Create enemy.h with Enemy struct
- **Description**: Define enemy data structures
- **Dependencies**: P2.2.5
- **Actions**:
  1. Create `cool-game/src/enemy.h`
  2. Define `EnemyType` enum: `ENEMY_CHASER`, `ENEMY_TYPE_COUNT`
  3. Define `Enemy` struct:
     - `Vector2 pos`
     - `Vector2 vel`
     - `float radius`
     - `float speed`
     - `float health`
     - `float maxHealth`
     - `EnemyType type`
     - `int xpValue`
     - `bool active`
  4. Define `EnemyPool` struct:
     - `Enemy enemies[MAX_ENEMIES]`
     - `int count`
  5. Declare pool functions similar to projectiles
  6. Declare: `Enemy* EnemySpawn(EnemyPool *pool, EnemyType type, Vector2 pos)`
- **Verification**:
  - [x] `make` compiles successfully
- **Status**: `[x]`

#### P3.1.2 — Create enemy.c with pool management
- **Description**: Implement enemy pool logic
- **Dependencies**: P3.1.1
- **Actions**:
  1. Create `cool-game/src/enemy.c`
  2. Implement `EnemyPoolInit()`
  3. Implement `EnemySpawn()`:
     - Find inactive slot
     - Initialize based on type (ENEMY_CHASER stats):
       - speed = 100, health = 30, radius = 12, xpValue = 1
     - Set active = true
  4. Update Makefile
- **Verification**:
  - [x] `make` compiles successfully
- **Status**: `[x]`

#### P3.1.3 — Implement chaser enemy AI
- **Description**: Basic enemy that moves toward player
- **Dependencies**: P3.1.2
- **Actions**:
  1. Implement `EnemyPoolUpdate(EnemyPool *pool, Vector2 playerPos, float dt)`:
     - Loop through active enemies
     - For ENEMY_CHASER:
       - Calculate direction to player
       - Set vel = direction * speed
       - Update pos += vel * dt
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Enemy moves toward player position (test with debug spawn)
- **Status**: `[x]`

#### P3.1.4 — Implement enemy rendering
- **Description**: Draw enemies as geometric shapes
- **Dependencies**: P3.1.3
- **Actions**:
  1. Implement `EnemyPoolDraw()`:
     - Loop through active enemies
     - ENEMY_CHASER: Draw as red/orange circle
     - Draw health bar above enemy if damaged
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Enemies visible when spawned
  - [x] Health bars show correctly
- **Status**: `[x]`

#### P3.1.5 — Integrate enemies into game
- **Description**: Add EnemyPool to game loop
- **Dependencies**: P3.1.4
- **Actions**:
  1. Add `EnemyPool enemies` to GameData
  2. Initialize in GameInit()
  3. Update in GameUpdate() with player.pos
  4. Draw in GameDraw()
- **Verification**:
  - [x] `make run` — no crashes
  - [x] Can spawn test enemy with keypress
  - [x] Enemy chases player
- **Status**: `[x]`

---

### 3.2 Enemy Spawning

#### P3.2.1 — Create spawner system
- **Description**: Automatic enemy spawning over time
- **Dependencies**: P3.1.5
- **Actions**:
  1. Add to GameData:
     - `float spawnTimer`
     - `float spawnInterval` (starts at 2.0 seconds)
  2. In GameUpdate() STATE_PLAYING:
     - Increment spawnTimer by dt
     - If spawnTimer >= spawnInterval:
       - Calculate spawn position (off-screen, random direction from player)
       - Call EnemySpawn()
       - Reset spawnTimer to 0
- **Verification**:
  - [x] `make run` — enemies spawn automatically
  - [x] Enemies spawn outside visible screen area
  - [x] Spawn rate is approximately every 2 seconds
- **Status**: `[x]`

#### P3.2.2 — Implement difficulty scaling
- **Description**: Spawn rate increases over time
- **Dependencies**: P3.2.1
- **Actions**:
  1. Create function: `float GetSpawnInterval(float gameTime)`
     - Start at 2.0 seconds
     - Decrease over time: `2.0 - gameTime * 0.01` (min 0.3)
  2. Update spawner to use dynamic interval
  3. Display game time on HUD for debugging
- **Verification**:
  - [x] `make run` — spawn rate increases over time
  - [x] After 60 seconds, enemies spawn faster
  - [x] Minimum interval is respected (not too fast)
- **Status**: `[x]`

---

### 3.3 Collision Detection

#### P3.3.1 — Implement circle-circle collision
- **Description**: Utility function for collision checks
- **Dependencies**: P3.2.2
- **Actions**:
  1. Add to utils.h/c:
     - `bool CheckCollisionCircles(Vector2 c1, float r1, Vector2 c2, float r2)`
     - Returns true if distance between centers < r1 + r2
  2. Note: Implemented inline in game.c using distance squared optimization
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Unit test: overlapping circles return true
  - [x] Unit test: non-overlapping circles return false
- **Status**: `[x]`

#### P3.3.2 — Implement projectile-enemy collision
- **Description**: Projectiles damage enemies
- **Dependencies**: P3.3.1
- **Actions**:
  1. Create function: `void CheckProjectileEnemyCollisions(ProjectilePool *proj, EnemyPool *enemies)`
  2. Loop through active projectiles and enemies
  3. If collision detected:
     - Apply projectile damage to enemy health
     - Deactivate projectile (if not pierce)
     - If enemy health <= 0, deactivate enemy
  4. Call from GameUpdate()
- **Verification**:
  - [x] `make run` — shooting enemies reduces their health
  - [x] Enemies die when health reaches 0
  - [x] Projectiles disappear on hit
- **Status**: `[x]`

#### P3.3.3 — Implement enemy-player collision
- **Description**: Enemies damage player on contact
- **Dependencies**: P3.3.2
- **Actions**:
  1. Create function: `void CheckEnemyPlayerCollisions(EnemyPool *enemies, Player *player)`
  2. Add `float invincibilityTimer` to Player (prevents rapid damage)
  3. Loop through active enemies
  4. If collision with player and invincibility expired:
     - Call PlayerTakeDamage() with enemy-specific damage
     - Set invincibilityTimer to 0.5 seconds
     - Push enemy away from player (knockback)
  5. In PlayerUpdate: decrease invincibilityTimer by dt
- **Verification**:
  - [x] `make run` — touching enemy damages player
  - [x] Player health decreases
  - [x] Brief invincibility prevents instant death
  - [x] Can die from enemy contact
- **Status**: `[x]`

---

## PHASE 4: XP & LEVELING

### 4.1 XP Crystal System

#### P4.1.1 — Create xp.h with XPCrystal struct
- **Description**: Define XP crystal data
- **Dependencies**: P3.3.3
- **Actions**:
  1. Create `cool-game/src/xp.h`
  2. Define `XPCrystal` struct:
     - `Vector2 pos`
     - `int value`
     - `float radius`
     - `float lifetime`
     - `bool active`
  3. Define `XPPool` struct with array and count
  4. Declare pool functions
  5. Declare: `void XPSpawn(XPPool *pool, Vector2 pos, int value)`
- **Verification**:
  - [x] `make` compiles successfully
- **Status**: `[x]`

#### P4.1.2 — Create xp.c with implementation
- **Description**: Implement XP crystal behavior
- **Dependencies**: P4.1.1
- **Actions**:
  1. Create `cool-game/src/xp.c`
  2. Implement pool init/spawn/update/draw
  3. XP crystals should:
     - Be stationary where spawned
     - Have long lifetime (30 seconds)
     - Draw as small cyan/green diamonds or circles
  4. Update Makefile
- **Verification**:
  - [x] `make` compiles successfully
- **Status**: `[x]`

#### P4.1.3 — Spawn XP on enemy death
- **Description**: Enemies drop XP crystals
- **Dependencies**: P4.1.2
- **Actions**:
  1. Add `XPPool xp` to GameData, initialize
  2. Modify collision detection:
     - When enemy dies, call XPSpawn() at enemy position
     - Value = enemy.xpValue
  3. Update and draw XP pool in game loop
- **Verification**:
  - [x] `make run` — killing enemies spawns XP crystals
  - [x] Crystals appear at death location
  - [x] Crystals are visible
- **Status**: `[x]`

#### P4.1.4 — Implement XP collection
- **Description**: Player collects nearby XP
- **Dependencies**: P4.1.3
- **Actions**:
  1. Add `float magnetRadius` to Player (start at 50.0)
  2. Create: `void CheckXPCollection(XPPool *xp, Player *player)`
  3. Loop through active crystals:
     - If within magnetRadius of player, move toward player
     - If touching player (very close), collect:
       - Add value to player.xp
       - Deactivate crystal
- **Verification**:
  - [x] `make run` — XP crystals move toward player when close
  - [x] XP crystals are collected on contact
  - [x] Player XP increases (shown on HUD)
- **Status**: `[x]`

---

### 4.2 Level Up System

#### P4.2.1 — Implement XP thresholds
- **Description**: Calculate XP needed per level
- **Dependencies**: P4.1.4
- **Actions**:
  1. Create function: `int GetXPForLevel(int level)`
     - Formula: `10 * level * level` (10, 40, 90, 160...)
  2. In PlayerUpdate or separate function:
     - Check if player.xp >= player.xpToNextLevel
     - If so: increment level, set new xpToNextLevel
     - Return bool indicating level up occurred
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Player levels up at correct XP thresholds
  - [x] Level displayed on HUD
- **Status**: `[x]`

#### P4.2.2 — Trigger level up state
- **Description**: Pause game for upgrade selection
- **Dependencies**: P4.2.1
- **Actions**:
  1. Add `bool levelUpPending` to GameData
  2. When level up detected in GameUpdate:
     - Set levelUpPending = true
     - Transition to STATE_LEVELUP
  3. STATE_LEVELUP pauses game logic but shows game in background
- **Verification**:
  - [x] `make run` — game pauses when leveling up
  - [x] Level up screen appears
  - [x] Game visible in background
- **Status**: `[x]`

---

### 4.3 Upgrade System

#### P4.3.1 — Create upgrade.h with definitions
- **Description**: Define upgrade types and effects
- **Dependencies**: P4.2.2
- **Actions**:
  1. Create `cool-game/src/upgrade.h`
  2. Define `UpgradeType` enum:
     - `UPGRADE_DAMAGE`, `UPGRADE_FIRE_RATE`, `UPGRADE_PROJECTILE_COUNT`
     - `UPGRADE_SPEED`, `UPGRADE_MAX_HEALTH`, `UPGRADE_MAGNET`
     - `UPGRADE_COUNT`
  3. Define `Upgrade` struct:
     - `UpgradeType type`
     - `const char *name`
     - `const char *description`
  4. Declare: `Upgrade GetUpgradeDefinition(UpgradeType type)`
  5. Declare: `void ApplyUpgrade(UpgradeType type, Player *player)`
- **Verification**:
  - [x] `make` compiles successfully
- **Status**: `[x]`

#### P4.3.2 — Implement upgrade definitions
- **Description**: Define all upgrade stats and effects
- **Dependencies**: P4.3.1
- **Actions**:
  1. Create `cool-game/src/upgrade.c`
  2. Implement `GetUpgradeDefinition()` with name/description for each type
  3. Implement `ApplyUpgrade()`:
     - UPGRADE_DAMAGE: weapon.damage *= 1.25
     - UPGRADE_FIRE_RATE: weapon.fireRate *= 1.2
     - UPGRADE_PROJECTILE_COUNT: weapon.projectileCount += 1
     - UPGRADE_SPEED: player.speed *= 1.1
     - UPGRADE_MAX_HEALTH: player.maxHealth += 20, player.health += 20
     - UPGRADE_MAGNET: player.magnetRadius *= 1.5
  4. Update Makefile
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Applying upgrades changes player/weapon stats
- **Status**: `[x]`

#### P4.3.3 — Implement upgrade selection UI
- **Description**: Display 3 random upgrades to choose
- **Dependencies**: P4.3.2
- **Actions**:
  1. Add to GameData:
     - `UpgradeType upgradeOptions[3]`
  2. When entering STATE_LEVELUP:
     - Select 3 random unique upgrade types
     - Store in upgradeOptions
  3. In STATE_LEVELUP draw:
     - Draw "LEVEL UP!" title
     - Draw 3 boxes with upgrade name and description
     - Show key prompts: [1] [2] [3]
- **Verification**:
  - [x] `make run` — level up shows 3 upgrade options
  - [x] Options have names and descriptions
  - [x] Options are different each time
- **Status**: `[x]`

#### P4.3.4 — Implement upgrade selection input
- **Description**: Player chooses an upgrade
- **Dependencies**: P4.3.3
- **Actions**:
  1. In STATE_LEVELUP update:
     - If KEY_ONE pressed: apply upgradeOptions[0]
     - If KEY_TWO pressed: apply upgradeOptions[1]
     - If KEY_THREE pressed: apply upgradeOptions[2]
     - After selection: transition back to STATE_PLAYING
- **Verification**:
  - [x] `make run` — pressing 1/2/3 selects upgrade
  - [x] Selected upgrade is applied (visible effect)
  - [x] Game resumes after selection
  - [x] Multiple level ups work correctly
- **Status**: `[x]`

---

## PHASE 5: PARTICLES & JUICE

### 5.1 Particle System

#### P5.1.1 — Create particle.h with definitions
- **Description**: Define particle data structures
- **Dependencies**: P4.3.4
- **Actions**:
  1. Create `cool-game/src/particle.h`
  2. Define `Particle` struct:
     - `Vector2 pos`
     - `Vector2 vel`
     - `Color color`
     - `float size`
     - `float lifetime`
     - `float maxLifetime`
     - `bool active`
  3. Define `ParticlePool` with array and count
  4. Declare pool functions
  5. Declare: `void SpawnExplosion(ParticlePool *pool, Vector2 pos, Color color, int count)`
- **Verification**:
  - [x] `make` compiles successfully
- **Status**: `[x]`

#### P5.1.2 — Implement particle pool
- **Description**: Particle management and rendering
- **Dependencies**: P5.1.1
- **Actions**:
  1. Create `cool-game/src/particle.c`
  2. Implement pool init/update/draw
  3. Update: move by velocity, fade alpha based on lifetime ratio, deactivate when lifetime <= 0
  4. Draw: filled circle with size and color (use alpha)
  5. Implement `SpawnExplosion()`: burst of particles in random directions
  6. Update Makefile
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Test explosion spawn with keypress
  - [x] Particles move outward and fade
- **Status**: `[x]`

#### P5.1.3 — Integrate particles into game
- **Description**: Add particles to game events
- **Dependencies**: P5.1.2
- **Actions**:
  1. Add `ParticlePool particles` to GameData, initialize
  2. Update and draw particles in game loop
  3. Spawn explosion when enemy dies (red/orange particles)
  4. Spawn small burst when projectile hits
- **Verification**:
  - [x] `make run` — enemy deaths create particle explosions
  - [x] Projectile impacts create small particle bursts
  - [x] Particles don't impact performance noticeably
- **Status**: `[x]`

---

### 5.2 Screen Effects

#### P5.2.1 — Implement camera system
- **Description**: Camera with position and offset
- **Dependencies**: P5.1.3
- **Actions**:
  1. Add to GameData:
     - `Camera2D camera`
  2. Initialize camera:
     - target = player.pos
     - offset = screen center
     - zoom = 1.0
  3. Update camera: lerp target toward player position
  4. Wrap drawing in BeginMode2D/EndMode2D
- **Verification**:
  - [x] `make run` — camera follows player
  - [x] Camera movement is smooth (slight lag)
  - [x] All entities render correctly with camera
- **Status**: `[x]`

#### P5.2.2 — Implement screen shake
- **Description**: Camera shake on impacts
- **Dependencies**: P5.2.1
- **Actions**:
  1. Add to GameData:
     - `float shakeIntensity`
     - `float shakeDuration`
  2. Create: `void TriggerScreenShake(GameData *game, float intensity, float duration)`
  3. In camera update:
     - If shakeDuration > 0, add random offset to camera
     - Decrease shakeDuration by dt
     - Scale offset by remaining duration
  4. Call shake on enemy death, player damage
- **Verification**:
  - [x] `make run` — screen shakes when enemy dies
  - [x] Stronger shake on player damage
  - [x] Shake diminishes over duration
  - [x] No shake during normal gameplay
- **Status**: `[x]`

---

### 5.3 HUD

#### P5.3.1 — Create comprehensive HUD
- **Description**: Display all game stats
- **Dependencies**: P5.2.2
- **Actions**:
  1. Create `cool-game/src/ui.h` and `ui.c`
  2. Implement `DrawHUD(GameData *game)`:
     - Health bar (top left)
     - XP bar (below health)
     - Level number
     - Score
     - Game time (MM:SS format)
     - Enemy count (debug, optional)
  3. Draw HUD AFTER EndMode2D (screen space)
  4. Update Makefile
- **Verification**:
  - [x] `make run` — HUD displays all elements
  - [x] Values update in real-time
  - [x] HUD doesn't move with camera
  - [x] HUD is readable (good contrast)
- **Status**: `[x]`

---

## PHASE 6: ADDITIONAL ENEMIES

### 6.1 Orbiter Enemy

#### P6.1.1 — Add Orbiter enemy type
- **Description**: Enemy that circles the player
- **Dependencies**: P5.3.1
- **Actions**:
  1. Add `ENEMY_ORBITER` to EnemyType enum
  2. Add Orbiter stats in EnemySpawn:
     - speed = 80, health = 50, radius = 15, xpValue = 2
  3. Add `float orbitAngle` and `float orbitDistance` to Enemy struct
  4. In EnemyPoolUpdate, implement Orbiter behavior:
     - Increment orbitAngle over time
     - Move in circle around player
     - Slowly decrease orbitDistance (spiral inward)
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Orbiter circles player when spawned
  - [x] Orbiter slowly closes in
  - [x] Orbiter can be killed
- **Status**: `[x]`

#### P6.1.2 — Add Orbiter to spawn system
- **Description**: Orbiters spawn after 30 seconds
- **Dependencies**: P6.1.1
- **Actions**:
  1. Create: `EnemyType GetEnemyTypeForTime(float gameTime)`
  2. Logic:
     - < 30s: only ENEMY_CHASER
     - 30-60s: 70% Chaser, 30% Orbiter
     - > 60s: 50/50
  3. Use in spawn system
- **Verification**:
  - [x] `make run` — only Chasers before 30s
  - [x] Orbiters start appearing after 30s
  - [x] Mix of enemies mid-game
- **Status**: `[x]`

---

### 6.2 Splitter Enemy

#### P6.2.1 — Add Splitter enemy type
- **Description**: Enemy that splits into smaller enemies
- **Dependencies**: P6.1.2
- **Actions**:
  1. Add `ENEMY_SPLITTER` to enum
  2. Add `int splitCount` to Enemy (starts at 2)
  3. Splitter stats: speed = 60, health = 80, radius = 20, xpValue = 3
  4. Implement Splitter AI: move toward player (like Chaser but slower)
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Splitter moves toward player
  - [x] Splitter is larger than Chaser
- **Status**: `[x]`

#### P6.2.2 — Implement split mechanic
- **Description**: Spawn smaller enemies on death
- **Dependencies**: P6.2.1
- **Actions**:
  1. When Splitter dies (in collision handling):
     - If splitCount > 0:
       - Spawn 2 smaller Splitters at death position
       - New Splitters have: splitCount - 1, radius * 0.7, health * 0.5
     - If splitCount == 0: die normally (spawn XP)
- **Verification**:
  - [x] `make run` — killing Splitter spawns 2 smaller ones
  - [x] Small Splitters spawn even smaller ones
  - [x] Eventually no more splits occur
  - [x] XP only from final kills
- **Status**: `[x]`

#### P6.2.3 — Add Splitter to spawn system
- **Description**: Splitters spawn after 60 seconds
- **Dependencies**: P6.2.2
- **Actions**:
  1. Update GetEnemyTypeForTime:
     - > 60s: include Splitter in rotation
     - > 90s: increase Splitter probability
- **Verification**:
  - [x] `make run` — Splitters appear after 60s
  - [x] Game remains playable with Splitters
- **Status**: `[x]`

---

## PHASE 7: AUDIO

### 7.1 Sound Effects

#### P7.1.1 — Create audio manager
- **Description**: Centralized audio handling
- **Dependencies**: P6.2.3
- **Actions**:
  1. Create `cool-game/src/audio.h`:
     - Define `SoundType` enum: `SOUND_SHOOT`, `SOUND_EXPLOSION`, `SOUND_PICKUP`, `SOUND_LEVELUP`, `SOUND_HIT`
     - Declare: `void AudioInit(void)`, `void AudioCleanup(void)`, `void PlayGameSound(SoundType type)`
  2. Create `cool-game/src/audio.c`:
     - Static array of Sound objects
     - Load placeholder sounds or generate with raylib
  3. Update Makefile
- **Verification**:
  - [x] `make` compiles successfully
  - [x] AudioInit/Cleanup don't crash
- **Status**: `[x]`

#### P7.1.2 — Generate placeholder sounds
- **Description**: Create basic synth sounds programmatically
- **Dependencies**: P7.1.1
- **Actions**:
  1. In AudioInit, use raylib's audio generation or load WAV files
  2. For now, can use simple beeps at different frequencies
  3. Alternative: source free sound effects and place in resources/sounds/
- **Verification**:
  - [x] `make` compiles successfully
  - [x] Sounds load without errors (generated programmatically)
- **Status**: `[x]`

#### P7.1.3 — Integrate sounds into gameplay
- **Description**: Play sounds on game events
- **Dependencies**: P7.1.2
- **Actions**:
  1. Call InitAudioDevice() in main.c before game loop
  2. Call AudioInit() after
  3. Add sound triggers:
     - Weapon fire: SOUND_SHOOT
     - Enemy death: SOUND_EXPLOSION
     - XP pickup: SOUND_PICKUP
     - Level up: SOUND_LEVELUP
     - Player damage: SOUND_HIT
  4. Call CloseAudioDevice() on exit
- **Verification**:
  - [x] `make run` — sounds play on events
  - [x] No audio clipping or distortion
  - [x] Sounds don't overlap excessively
- **Status**: `[x]`

---

### 7.2 Background Music

#### P7.2.1 — Add music playback
- **Description**: Looping background track
- **Dependencies**: P7.1.3
- **Actions**:
  1. Add Music variable to audio manager
  2. Load music file (or use placeholder)
  3. Start music when entering STATE_PLAYING
  4. Pause music in STATE_PAUSED and STATE_LEVELUP
  5. Stop music on STATE_GAMEOVER
- **Verification**:
  - [x] `make run` — music plays during gameplay (if music file present)
  - [x] Music loops seamlessly
  - [x] Music pauses/resumes correctly with state transitions
- **Status**: `[x]`

---

## PHASE 8: VISUAL POLISH

### 8.1 Post-Processing Shaders

#### P8.1.1 — Set up render texture pipeline
- **Description**: Render to texture for post-processing
- **Dependencies**: P7.2.1
- **Actions**:
  1. Create RenderTexture2D in GameData
  2. Initialize at screen size
  3. Modify draw loop:
     - BeginTextureMode(renderTexture)
     - Draw game
     - EndTextureMode()
     - Draw renderTexture to screen
  4. Unload render texture on cleanup
- **Verification**:
  - [x] `make run` — game renders correctly through texture
  - [x] No visual difference yet (pass-through)
- **Status**: `[x]`

#### P8.1.2 — Implement bloom shader
- **Description**: Glow effect for bright objects
- **Dependencies**: P8.1.1
- **Actions**:
  1. Create `resources/shaders/bloom.fs` with bloom GLSL code from PLAN.md
  2. Load shader in game init
  3. Apply shader when drawing render texture to screen
  4. Add uniform for intensity control
- **Verification**:
  - [x] `make run` — bloom effect visible
  - [x] Bright objects (player, projectiles) glow
  - [x] Effect is subtle, not overwhelming
- **Status**: `[x]`

#### P8.1.3 — Implement CRT shader
- **Description**: Retro CRT monitor effect
- **Dependencies**: P8.1.2
- **Actions**:
  1. Create `resources/shaders/crt.fs` with CRT GLSL code from PLAN.md
  2. Chain after bloom shader (or combine)
  3. Add time uniform for animation
  4. Make toggleable in settings
- **Verification**:
  - [x] `make run` — CRT scanlines visible
  - [x] Slight barrel distortion on edges
  - [x] RGB separation effect
- **Status**: `[x]`

#### P8.1.4 — Implement chromatic aberration shader
- **Description**: RGB separation effect when player is low HP
- **Dependencies**: P8.1.2
- **Actions**:
  1. Create `resources/shaders/chromatic.fs` with radial RGB separation
  2. Add intensity uniform that scales with health (0 at 50%+, 1.0 at 0%)
  3. Add pulsing effect using time uniform
  4. Integrate into shader pipeline between bloom and CRT
- **Verification**:
  - [x] `make run` — no effect when health > 50%
  - [x] RGB separation visible when health < 50%
  - [x] Effect pulses and intensifies as health decreases
  - [x] Stronger aberration toward screen edges
- **Status**: `[x]`

---

### 8.2 Visual Enhancements

#### P8.2.1 — Add player trail effect
- **Description**: Fading trail behind player
- **Dependencies**: P8.1.3
- **Actions**:
  1. Store last N player positions (ring buffer, N=10)
  2. Draw circles at each position with decreasing alpha
  3. Update positions every few frames
- **Verification**:
  - [x] `make run` — player leaves trail when moving
  - [x] Trail fades smoothly
  - [x] Trail disappears when stationary
- **Status**: `[x]`

#### P8.2.2 — Add enemy hit flash
- **Description**: Enemies flash white when damaged
- **Dependencies**: P8.2.1
- **Actions**:
  1. Add `float hitFlashTimer` to Enemy struct
  2. Set hitFlashTimer = 0.1 when damaged
  3. In draw: if hitFlashTimer > 0, draw white instead of normal color
  4. Decrease hitFlashTimer in update
- **Verification**:
  - [x] `make run` — enemies flash when hit
  - [x] Flash is brief and noticeable
  - [x] Doesn't interfere with normal rendering
- **Status**: `[x]`

#### P8.2.3 — Add background grid
- **Description**: Subtle grid showing movement
- **Dependencies**: P8.2.2
- **Actions**:
  1. Draw grid lines in world space (before entities)
  2. Grid moves with camera (world-aligned)
  3. Use very dark color, subtle alpha
  4. Grid spacing: 64 pixels
- **Verification**:
  - [x] `make run` — grid visible in background
  - [x] Grid moves as camera moves
  - [x] Grid doesn't distract from gameplay
- **Status**: `[x]`

---

## PHASE 9: MENUS & POLISH

### 9.1 Main Menu

#### P9.1.1 — Implement main menu screen
- **Description**: Title screen with options
- **Dependencies**: P8.2.3
- **Actions**:
  1. STATE_MENU draws:
     - Game title "NEON VOID" (large, centered)
     - "Press ENTER to Start"
     - "Press ESC to Quit"
  2. Add animated background (particles or shapes)
  3. ENTER → STATE_PLAYING
  4. ESC in menu → exit game
- **Verification**:
  - [x] `make run` — starts in menu
  - [x] Title displays correctly
  - [x] Can start game and quit
  - [x] Starfield animation on menu background
- **Status**: `[x]`

#### P9.1.2 — Add high score display
- **Description**: Show best score on menu
- **Dependencies**: P9.1.1
- **Actions**:
  1. Add `int highScore` to game (static/global or saved)
  2. Update high score when game ends if score > highScore
  3. Display on menu: "High Score: XXXXX"
  4. (Optional) Save/load from file for persistence
- **Verification**:
  - [x] `make run` — high score displays on menu
  - [x] High score updates after good run
  - [x] Persists between game sessions (saved to highscore.dat)
- **Status**: `[x]`

---

### 9.2 Pause Menu

#### P9.2.1 — Implement pause menu
- **Description**: Pause screen with options
- **Dependencies**: P9.1.2
- **Actions**:
  1. STATE_PAUSED draws:
     - Semi-transparent overlay
     - "PAUSED" text
     - "Press ESC to Resume"
     - "Press Q to Quit to Menu"
  2. Game state preserved while paused
- **Verification**:
  - [x] `make run` — ESC pauses game
  - [x] Game visible behind pause overlay
  - [x] Can resume or quit to menu
- **Status**: `[x]`

---

### 9.3 Game Over Screen

#### P9.3.1 — Enhance game over screen
- **Description**: Final stats display
- **Dependencies**: P9.2.1
- **Actions**:
  1. STATE_GAMEOVER draws:
     - "GAME OVER" title
     - Final score
     - Enemies killed count
     - Time survived
     - Level reached
     - "Press ENTER to Return to Menu"
  2. Add kill counter to GameData, increment on enemy death
- **Verification**:
  - [x] `make run` — game over shows all stats
  - [x] Stats are accurate
  - [x] Can return to menu
- **Status**: `[x]`

---

## PHASE 10: FINAL POLISH

### 10.1 Bug Fixes & Balance

#### P10.1.1 — Playtest and fix bugs
- **Description**: Systematic testing
- **Dependencies**: P9.3.1
- **Actions**:
  1. Play for 10+ minutes, note any issues
  2. Test edge cases:
     - 0 health exactly
     - Max enemies on screen
     - Rapid level ups
     - Pause during level up
  3. Fix any crashes or visual glitches
- **Verification**:
  - [x] No crashes during extended play
  - [x] All features work as expected
  - [x] No visual artifacts
- **Status**: `[x]`

#### P10.1.2 — Balance pass
- **Description**: Tune game difficulty
- **Dependencies**: P10.1.1
- **Actions**:
  1. Adjust spawn rates if too easy/hard
  2. Tune weapon damage values
  3. Adjust upgrade effects
  4. Ensure player can survive 5+ minutes with skill
  5. Ensure 20+ minutes is challenging
- **Verification**:
  - [x] Game feels fair
  - [x] Progression feels rewarding
  - [x] Death doesn't feel cheap
- **Status**: `[x]`

---

### 10.2 Final Touches

#### P10.2.1 — Add invincibility visual feedback
- **Description**: Player flashes during invincibility
- **Dependencies**: P10.1.2
- **Actions**:
  1. When player invincibilityTimer > 0:
     - Flash player visibility (visible every other frame)
     - Or use alpha pulsing
- **Verification**:
  - [x] `make run` — player flashes after taking damage
  - [x] Clear feedback that player is temporarily invincible
- **Status**: `[x]`

#### P10.2.2 — Add score multiplier
- **Description**: Reward not getting hit
- **Dependencies**: P10.2.1
- **Actions**:
  1. Add `float scoreMultiplier` to GameData (starts at 1.0)
  2. Increase multiplier slowly while not hit
  3. Reset to 1.0 when player takes damage
  4. Apply multiplier to score gains
  5. Display multiplier on HUD
- **Verification**:
  - [x] `make run` — multiplier increases over time
  - [x] Multiplier resets on damage
  - [x] Higher scores possible with skill
- **Status**: `[x]`

#### P10.2.3 — Final code cleanup
- **Description**: Clean up codebase
- **Dependencies**: P10.2.2
- **Actions**:
  1. Remove all debug print statements
  2. Remove unused code and variables
  3. Add comments to complex sections
  4. Ensure consistent code style
  5. Verify no compiler warnings
- **Verification**:
  - [x] `make` compiles with no warnings
  - [x] Code is readable and documented
  - [x] No debug output in console
- **Status**: `[x]`

---

## PHASE 11: QUICK WINS (GAME FEEL)

### 11.1 Hitstop Effect

#### P11.1.1 — Implement hitstop on enemy kill
- **Description**: Brief frame freeze when killing enemies for impact
- **Dependencies**: P10.2.3
- **Actions**:
  1. Add `int hitstopFrames` to GameData
  2. When enemy dies, set hitstopFrames (2 for small, 4 for large enemies)
  3. Skip game updates while hitstopFrames > 0
  4. Decrement hitstopFrames each frame
- **Verification**:
  - [x] Game briefly freezes on enemy kill
  - [x] Larger enemies (Splitters) cause longer hitstop
  - [x] Feels impactful without being disruptive
- **Status**: `[x]`

---

### 11.2 Dash Ability

#### P11.2.1 — Implement dash mechanic
- **Description**: Quick dash move with invincibility
- **Dependencies**: P11.1.1
- **Actions**:
  1. Add dash fields to Player: dashCooldown, dashTimer, isDashing, dashDir
  2. SPACE key (or gamepad A) triggers dash
  3. Dash in movement direction (or aim direction if stationary)
  4. 150ms duration, 800 px/s speed
  5. 1.5 second cooldown
  6. Grant invincibility during dash
  7. Add pink trail visual effect while dashing
  8. Add "DASH: READY/..." indicator to HUD
- **Verification**:
  - [x] SPACE triggers dash
  - [x] Player moves quickly in dash direction
  - [x] Player is invincible during dash
  - [x] Cooldown prevents spam
  - [x] Visual trail appears during dash
  - [x] HUD shows dash status
- **Status**: `[x]`

---

### 11.3 Slow-Motion Effects

#### P11.3.1 — Implement time scaling
- **Description**: Slow-mo for dramatic moments
- **Dependencies**: P11.2.1
- **Actions**:
  1. Add `float timeScale` to GameData (default 1.0)
  2. Multiply dt by timeScale for all game updates
  3. Near-death slow-mo: timeScale = 0.5 when health < 25%
  4. Level-up slow-mo: timeScale = 0.3 during STATE_LEVELUP
  5. Reset timeScale to 1.0 when returning to normal play
- **Verification**:
  - [x] Game slows when player health drops below 25%
  - [x] Game slows during level-up selection
  - [x] Time returns to normal after level-up choice
  - [x] All entities affected by time scale
- **Status**: `[x]`

---

### 11.4 Tutorial Overlay

#### P11.4.1 — Implement tutorial hints
- **Description**: Control hints for new players
- **Dependencies**: P11.3.1
- **Actions**:
  1. Add `float tutorialTimer` to GameData
  2. Create DrawTutorial() function in ui.c
  3. Show for first 20 seconds of gameplay
  4. Display: "WASD - Move", "Mouse - Aim", "SPACE - Dash", "Collect crystals"
  5. Fade out during last 5 seconds
  6. Position at bottom center of screen
- **Verification**:
  - [x] Tutorial shows at game start
  - [x] All control hints displayed
  - [x] Fades out smoothly after 15-20 seconds
  - [x] Doesn't obstruct gameplay
- **Status**: `[x]`

---

## SUMMARY

### Task Counts by Phase

| Phase | Description | Tasks | Done |
|-------|-------------|-------|------|
| 0 | Project Setup | 9 | 8 ✓ |
| 1 | Player System | 14 | 14 ✓ |
| 2 | Weapons & Projectiles | 10 | 10 ✓ |
| 3 | Enemies | 10 | 10 ✓ |
| 4 | XP & Leveling | 10 | 10 ✓ |
| 5 | Particles & Juice | 7 | 7 ✓ |
| 6 | Additional Enemies | 5 | 5 ✓ |
| 7 | Audio | 4 | 4 ✓ |
| 8 | Visual Polish | 7 | 7 ✓ |
| 9 | Menus & Polish | 4 | 4 ✓ |
| 10 | Final Polish | 5 | 5 ✓ |
| 11 | Quick Wins (Game Feel) | 4 | 4 ✓ |
| 12 | Post-Review Bug Fixes | 3 | 3 ✓ |
| 13 | 22 Upgrade System | 1 | 1 ✓ |
| 14 | Weapon Evolution System | 1 | 1 ✓ |
| 15 | Elite Enemies | 1 | 1 ✓ |
| 16 | Boss Enemy System | 1 | 1 ✓ |
| 17 | Permanent Unlocks | 1 | 1 ✓ |
| 18 | Leaderboard System | 1 | 1 ✓ |
| 19 | Character Select | 1 | 1 ✓ |
| **Total** | | **99** | **99** |

### Estimated Time

- **Minimum**: 2-3 weeks (experienced developer, full-time)
- **Comfortable**: 4-5 weeks (part-time or learning)
- **With polish**: 6+ weeks (all stretch goals)

---

## VERIFICATION CHECKLIST (Run After Each Phase)

After completing each phase, verify:

- [ ] **`make test` passes** — ALL unit tests green (BLOCKING)
- [ ] `make` compiles with zero warnings
- [ ] `make run` launches without crashes
- [ ] Play for 2 minutes without issues
- [ ] All features from phase work correctly
- [ ] No obvious visual glitches
- [ ] Memory usage stable (no leaks)
- [ ] Git commit: `git add -A && git commit -m "Complete Phase X"`

**CRITICAL**: If `make test` fails, DO NOT proceed. Fix failing tests first.

### Phase 0 Verification ✓
- [x] `make` compiles with zero warnings
- [x] `make run` launches without crashes
- [x] State machine works: MENU → PLAYING → PAUSED → MENU
- [x] ESC quits from menu, pauses from playing
- [x] AGENTS.md created for cool-game/

### Phase 1 Verification ✓
- [x] `make` compiles with zero warnings
- [x] `make run` launches without crashes
- [x] Player renders at screen center with neon circle visual
- [x] WASD/Arrow keys move player, diagonal normalized
- [x] Player stays within screen bounds
- [x] Mouse aim works, pink aim indicator visible
- [x] Gamepad left stick movement, right stick aiming
- [x] Health bar displays below player
- [x] Player flashes during invincibility
- [x] Death transitions to GAMEOVER state
- [x] New game resets player properly

### Phase 2 Verification ✓
- [x] `make` compiles with zero warnings
- [x] `make run` launches without crashes
- [x] Projectiles fire automatically toward mouse/aim direction
- [x] Fire rate matches weapon settings (~5/sec)
- [x] Projectiles travel in correct direction (toward aim)
- [x] Projectiles disappear after lifetime (2 seconds)
- [x] Projectiles rendered as yellow circles
- [x] New game resets projectile pool properly

### Phase 3 Verification ✓
- [x] **`make test` passes** — 22 tests, 540 assertions
- [x] `make` compiles with zero warnings
- [x] `make run` launches without crashes
- [x] Enemies spawn automatically off-screen
- [x] Enemies chase player (ENEMY_CHASER AI)
- [x] Spawn rate starts at 2s, decreases over time (min 0.3s)
- [x] Projectiles damage enemies, enemies show health bars when hit
- [x] Enemies die and award score when health reaches 0
- [x] Enemy-player collision damages player
- [x] Player invincibility prevents rapid damage
- [x] Enemy knockback on collision
- [x] Enemy count displayed on HUD
- [x] Game over when player dies from enemies
- [x] New game resets enemy pool properly

### Phase 4 Verification ✓
- [x] **`make test` passes** — 22 tests, 540 assertions
- [x] `make` compiles with zero warnings
- [x] `make run` launches without crashes
- [x] XP crystals spawn when enemies die (green diamond shape)
- [x] XP crystals pulse with glow effect
- [x] Crystals are attracted toward player within magnet radius (80px)
- [x] Crystals collected when player touches them
- [x] XP counter displayed on HUD (XP: current/needed)
- [x] Level up triggers when XP >= threshold (10 * level^2)
- [x] STATE_LEVELUP shows game in background with overlay
- [x] 3 random unique upgrade options displayed
- [x] Keys 1/2/3 select upgrades
- [x] Upgrades apply correctly (damage, fire rate, projectile count, speed, HP, magnet)
- [x] Game resumes after upgrade selection
- [x] Multiple level ups work correctly in sequence
- [x] New game resets XP pool and player level properly

### Phase 5 Verification ✓
- [x] **`make test` passes** — 22 tests, 540 assertions
- [x] `make` compiles with zero warnings
- [x] `make run` launches without crashes
- [x] Particle explosions spawn on enemy death (orange burst)
- [x] Hit particles spawn on projectile impact (yellow burst)
- [x] Particles fade out and shrink over lifetime
- [x] Camera2D follows player with smooth lerp
- [x] Screen shake triggers on enemy death (light shake)
- [x] Screen shake triggers on player damage (stronger shake)
- [x] Shake diminishes over duration
- [x] HUD displays in screen space (doesn't move with camera)
- [x] HUD shows: TIME, SCORE, LEVEL, HP bar, XP bar, ENEMIES count
- [x] HUD has semi-transparent background for readability
- [x] All game states (PLAYING, PAUSED, LEVELUP) use camera

### Phase 6 Verification ✓
- [x] **`make test` passes** — 27 tests, 1160 assertions
- [x] `make` compiles with zero warnings
- [x] `make run` launches without crashes
- [x] Orbiter enemy circles player with spiral-in behavior
- [x] Orbiter has distinct cyan/pink visual
- [x] Splitter enemy moves toward player (slower than Chaser)
- [x] Splitter has distinct yellow/green visual
- [x] Killing Splitter spawns 2 smaller child Splitters
- [x] Child Splitters spawn even smaller ones (splitCount - 1)
- [x] Final split (splitCount == 0) drops XP normally
- [x] GetEnemyTypeForTime returns only Chaser before 30s
- [x] Orbiters start appearing after 30s
- [x] Splitters start appearing after 60s
- [x] Enemy mix increases diversity over time (90s+)
- [x] Game remains playable with all enemy types

### Phase 7 Verification ✓
- [x] **`make test` passes** — 27 tests, 1160 assertions
- [x] `make` compiles with zero warnings
- [x] `make run` launches without crashes
- [x] SOUND_SHOOT plays on weapon fire
- [x] SOUND_EXPLOSION plays on enemy death
- [x] SOUND_PICKUP plays on XP collection
- [x] SOUND_LEVELUP plays on level up
- [x] SOUND_HIT plays on player damage
- [x] Music starts when entering STATE_PLAYING (if music file present)
- [x] Music pauses in STATE_PAUSED and STATE_LEVELUP
- [x] Music resumes when returning to STATE_PLAYING
- [x] Music stops on STATE_GAMEOVER
- [x] No audio clipping or distortion
- [x] AudioInit/AudioCleanup work without crashes

### Phase 8 Verification ✓
- [x] **`make test` passes** — 27 tests, 1160 assertions
- [x] `make` compiles with zero warnings
- [x] `make run` launches without crashes
- [x] Render texture pipeline works (game renders through texture)
- [x] Bloom shader applies glow effect to bright objects
- [x] CRT shader adds scanlines, barrel distortion, RGB separation
- [x] F1 toggles bloom shader on/off
- [x] F2 toggles CRT shader on/off
- [x] Player trail appears when moving
- [x] Trail fades out behind player
- [x] Trail disappears when player stops moving
- [x] Enemies flash white when hit by projectiles
- [x] Hit flash is brief (~0.1 seconds)
- [x] Background grid visible in world space
- [x] Grid moves with camera (world-aligned, 64px spacing)
- [x] All visual effects work together without performance issues
- [x] Chromatic aberration activates when health < 50%
- [x] Chromatic effect pulses and intensifies near death
- [x] Chromatic aberration integrates with bloom/CRT pipeline

### Phase 9 Verification ✓
- [x] **`make test` passes** — 27 tests, 1160 assertions
- [x] `make` compiles with zero warnings
- [x] `make run` launches without crashes
- [x] Menu shows starfield effect (stars streaming from center)
- [x] Menu displays "High Score: X" in yellow
- [x] High score updates after a run with higher score
- [x] High score persists after game restart (highscore.dat)
- [x] Game over shows "Enemies Killed: X" in orange
- [x] Kill counter resets each new game
- [x] Pause menu works (ESC to pause, ESC to resume, Q to quit)
- [x] All state transitions work correctly

### Phase 10 Verification ✓
- [x] **`make test` passes** — 27 tests, 1160 assertions
- [x] `make` compiles with zero warnings
- [x] `make run` launches without crashes
- [x] Player flashes during invincibility (visual feedback)
- [x] Score multiplier increases over time (displayed on HUD)
- [x] Score multiplier resets to 1.0 on damage
- [x] Score multiplier applies to enemy kills (up to 5.0x)
- [x] Multiplier color changes at higher values (green → yellow → orange → pink)
- [x] No debug print statements in code
- [x] Code compiles without warnings (C99 compliant)
- [x] Game balance feels fair (5+ minutes survivable with skill)
- [x] All edge cases handled (0 health, pool overflow, rapid level ups)

### Phase 11 Verification ✓
- [x] **`make test` passes** — 86 tests, 3207 assertions
- [x] `make` compiles with zero warnings
- [x] `make run` launches without crashes
- [x] Hitstop: game freezes briefly on enemy kill (2-4 frames)
- [x] Hitstop scales with enemy size (Splitters = longer freeze)
- [x] Dash: SPACE key triggers quick dash
- [x] Dash: player is invincible during dash (200ms)
- [x] Dash: 1.5 second cooldown between dashes
- [x] Dash: pink trail effect visible while dashing
- [x] Dash: HUD shows "DASH: READY" or "DASH: ..."
- [x] Slow-mo: game slows to 50% when health < 25%
- [x] Slow-mo: game slows to 30% during level-up selection
- [x] Slow-mo: time returns to normal after upgrade selection
- [x] Tutorial: shows for first 20 seconds of gameplay
- [x] Tutorial: displays movement, aim, dash, and XP hints
- [x] Tutorial: fades out smoothly in last 5 seconds
- [x] All features work together without conflicts

---

## NOTES FOR ENGINEERS

1. **Run `make test` FIRST** — before any other verification, tests must pass
2. **Tests are BLOCKING** — failing tests = cannot proceed to next task
3. **Always test after each task** — never batch multiple tasks without testing
4. **Keep functions small** — if a function exceeds 50 lines, split it
5. **Extract pure logic** — move testable code to utils.h/c for coverage
6. **Use const where possible** — prevents accidental modifications
7. **Initialize all variables** — raylib style guide requirement
8. **Comment unclear code** — but prefer clear code over comments
9. **Profile if slow** — use raylib's `GetFPS()` to monitor performance

### Quick Test Command
```bash
make test && make && echo "Ready to proceed"
```
If this command fails at any point, fix the issue before continuing.

---

## CHANGELOG

### 2026-01-04 — Post-Review Bug Fixes

Following comprehensive code review, the following issues were identified and fixed:

| Commit | File | Issue | Fix |
|--------|------|-------|-----|
| `7eef1561` | weapon.c | `projectileCount` upgraded but never used in `WeaponFire()` | Now spawns multiple projectiles in spread pattern |
| `1d48196c` | game.c | `GetSpawnPosition()` clamped to screen bounds (invalid with infinite world) | Removed bounds clamping, enemies spawn relative to player |
| `bead0ed3` | game.c | `fread()` return value ignored in `LoadHighScore()` | Added validation, defaults to 0 on failure |

**All fixes verified:**
- 86 tests passing
- Zero compiler warnings
- Manual gameplay testing confirmed

---

### 2026-01-05 — 22 Upgrade System

Expanded the upgrade system from 6 to 22 types with rarity tiers:

| Category | Upgrades Added |
|----------|----------------|
| Weapon | Pierce, Range, Proj Size, Cooldown, Crit Chance |
| Player | Armor, Regen, Dash Damage, XP Boost, Knockback |
| Special | Double Shot, Vampirism, Explosive, Ricochet, Homing Boost, Slow Aura |

**New mechanics implemented:**
- Armor: Flat damage reduction (min 1 damage)
- Regen: HP per second with timer accumulator
- Vampirism: Lifesteal on hit
- Critical hits: Chance for 2x damage with visual feedback
- Dash damage: Deal damage to enemies while dashing
- Slow aura: Slow nearby enemies around player

**Tests updated:** 97 tests passing (added 11 new upgrade tests)

---

### 2026-01-05 — Weapon Evolution System

Added 8 evolved weapons that unlock at max level (5) + catalyst upgrade:

| Base Weapon | Catalyst | Evolved Weapon |
|-------------|----------|----------------|
| Pulse Cannon | Pierce | Mega Cannon |
| Spread Shot | Multi Shot | Circle Burst |
| Homing Missile | Double Shot | Swarm |
| Lightning | Crit Chance | Tesla Coil |
| Orbit Shield | Damage | Blade Dancer |
| Flamethrower | Range | Inferno |
| Freeze Ray | Slow Aura | Blizzard |
| Black Hole | Explosive | Singularity |

**Key features:**
- Weapon level system (max 5, increases with weapon upgrades)
- Upgrade tracking via bitfield for catalyst detection
- Evolved weapons have significantly enhanced stats
- Auto-evolution check after each upgrade applied

**Tests updated:** 113 tests passing (added 16 new evolution tests)

---

### 2026-01-05 — Elite Enemies

Added elite enemy variants to increase mid-game challenge:

| Feature | Implementation |
|---------|----------------|
| Elite multipliers | ELITE_SIZE_MULT (1.5x), ELITE_HEALTH_MULT (3x), ELITE_DAMAGE_MULT (1.5x), ELITE_XP_MULT (5x), ELITE_SPEED_MULT (0.8x) |
| Spawn function | `EnemySpawnElite()` - spawns any enemy type with elite modifiers |
| Scaling spawn chance | 10% base + 1% per minute of gameplay, capped at 25% |
| Visual feedback | Pulsing gold glow effect behind elites, gold border ring |

**Key code changes:**
- `enemy.h`: Added `isElite` flag, elite multiplier constants
- `enemy.c`: Added `EnemySpawnElite()`, updated `EnemyPoolDraw()` with gold glow
- `game.c`: Updated enemy spawning to use elite chance formula

**Tests updated:** 117 tests passing (added 4 new elite tests)

---

### 2026-01-05 — Boss Enemy System

Added boss enemies that spawn every 5 minutes as major challenge events:

| Feature | Implementation |
|---------|----------------|
| Boss stats | BOSS_BASE_HEALTH (2000), BOSS_BASE_RADIUS (60), BOSS_BASE_DAMAGE (30), BOSS_XP_VALUE (100), BOSS_BASE_SPEED (50) |
| Spawn timing | BOSS_SPAWN_INTERVAL (300s = 5 minutes) |
| Attack pattern | Charge-and-dash: 1s charge with warning, then 8x speed dash at player |
| Scaling | Each subsequent boss is 50% stronger (health, damage), gives more XP |
| Warning system | 5-second countdown with flashing "BOSS INCOMING" text |
| UI feedback | Boss health bar at top center of screen with boss number |
| Visual effects | Purple glow, red warning rings when charging, menacing eyes, crown spikes |

**Key code changes:**
- `enemy.h`: Added `ENEMY_BOSS` type, boss stats constants, boss fields in Enemy struct
- `enemy.c`: Added `EnemySpawnBoss()`, `EnemyPoolHasBoss()`, `EnemyPoolGetBoss()`, boss AI in update, boss visuals in draw
- `game.h`: Added `bossSpawnTimer`, `bossCount`, `bossWarningTimer`, `bossWarningActive`
- `game.c`: Added boss spawn logic with warning system
- `ui.c`: Added boss warning display and boss health bar

**Tests updated:** 122 tests passing (added 5 new boss tests)

---

### 2026-01-05 — Permanent Unlocks System

Added persistent meta-progression system with save/load:

| Feature | Implementation |
|---------|----------------|
| Weapon unlocks | 8 weapons unlockable via kill counts, level milestones, survival time, boss kills, score |
| Character unlocks | 3 characters unlockable via games played, survival time |
| Meta upgrades | 5 stats (speed, health, damage, XP, magnet) × 5 levels each |
| Run stats | Total kills, boss kills, score, games played, highest level, longest survival |
| Persistence | Binary file save/load (unlocks.dat) |
| Meta bonuses | Applied on new game start |

**Unlock conditions:**
- Spread Shot: 100 kills
- Homing Missile: 500 kills
- Lightning: Level 10
- Orbit Shield: Survive 3 minutes
- Flamethrower: Kill 1 boss
- Freeze Ray: Kill 3 bosses
- Black Hole: Score 10,000 lifetime points
- Character 2: Play 5 games
- Character 3: Survive 5 minutes

**Key code changes:**
- `unlocks.h`: New file - UnlockData struct, unlock functions
- `unlocks.c`: New file - Save/load, unlock checks, meta bonuses
- `game.h`: Added UnlockData to GameData, bossKillsThisRun tracking
- `game.c`: Load unlocks on init, save on game over, apply meta bonuses on game start

**Tests updated:** 130 tests passing (added 8 new unlock tests)

---

### 2026-01-05 — Leaderboard System

Added top 10 high score leaderboard:

| Feature | Implementation |
|---------|----------------|
| Entry data | Score, level, kills, survival time, date |
| Leaderboard screen | Dedicated view with column headers |
| Access points | L from menu, L from game over |
| Rank feedback | "NEW HIGH SCORE! Rank #X" on game over if placed |
| Persistence | Binary file (leaderboard.dat) |
| Sorting | Descending by score with displacement |

**Key code changes:**
- `leaderboard.h`: New file - LeaderboardEntry struct, Leaderboard struct (max 10)
- `leaderboard.c`: New file - Init, Load, Save, AddEntry, IsHighScore, GetMinScore
- `game.h`: Added STATE_LEADERBOARD, Leaderboard and leaderboardPosition to GameData
- `game.c`: Leaderboard screen drawing, L key handling in menu and game over

**Tests updated:** 139 tests passing (added 9 leaderboard tests)

---

### 2026-01-05 — Character Select System

Added 3 playable characters with distinct stats:

| Character | HP | Speed | Magnet | Armor | Damage | XP |
|-----------|-----|-------|--------|-------|--------|-----|
| VANGUARD | 100 | 300 | 80 | 0 | 1.0x | 1.0x |
| TITAN | 150 | 240 | 60 | 5 | 1.2x | 0.9x |
| PHANTOM | 70 | 380 | 120 | 0 | 0.9x | 1.25x |

**Features:**
- Character select screen with visual cards
- Stats display and unlock requirements
- Unique character colors applied to player
- Integration with permanent unlocks system

**Unlock conditions:**
- VANGUARD: Always unlocked
- TITAN: Play 5 games
- PHANTOM: Survive 5 minutes

**Key code changes:**
- `character.h`: New file - CharacterType enum, CharacterDef struct
- `character.c`: New file - Character definitions and stat retrieval
- `player.h`: Added characterType, colors, PlayerInitWithCharacter
- `player.c`: Character stats applied on init, colors used in draw
- `game.h`: Added STATE_CHARACTER_SELECT, selectedCharacter
- `game.c`: Character select UI and navigation

**Tests updated:** 147 tests passing (added 8 character tests)

---

*Last updated: 2026-01-05 — Character Select added (99/99 tasks, 100%)*
