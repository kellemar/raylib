# IMPLEMENTATION_PLAN.md — NEON VOID

> Detailed task breakdown for implementing NEON VOID. Each task is atomic, testable, and leaves the codebase in a compilable state.

---

## CONVENTIONS

- **Task ID**: `P{phase}.{section}.{task}` (e.g., `P0.1.1`)
- **Status**: `[ ]` pending, `[x]` done, `[-]` skipped
- **Dependencies**: Tasks that must be completed first
- **Verification**: Steps to confirm task completion
- **Build Check**: Every task MUST compile and run without errors

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
  - [ ] `make` compiles successfully
- **Status**: `[ ]`

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
  - [ ] `make` compiles successfully
- **Status**: `[ ]`

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
  - [ ] `make` compiles successfully
  - [ ] Enemy moves toward player position (test with debug spawn)
- **Status**: `[ ]`

#### P3.1.4 — Implement enemy rendering
- **Description**: Draw enemies as geometric shapes
- **Dependencies**: P3.1.3
- **Actions**:
  1. Implement `EnemyPoolDraw()`:
     - Loop through active enemies
     - ENEMY_CHASER: Draw as red/orange circle
     - Draw health bar above enemy if damaged
- **Verification**:
  - [ ] `make` compiles successfully
  - [ ] Enemies visible when spawned
  - [ ] Health bars show correctly
- **Status**: `[ ]`

#### P3.1.5 — Integrate enemies into game
- **Description**: Add EnemyPool to game loop
- **Dependencies**: P3.1.4
- **Actions**:
  1. Add `EnemyPool enemies` to GameData
  2. Initialize in GameInit()
  3. Update in GameUpdate() with player.pos
  4. Draw in GameDraw()
- **Verification**:
  - [ ] `make run` — no crashes
  - [ ] Can spawn test enemy with keypress
  - [ ] Enemy chases player
- **Status**: `[ ]`

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
  - [ ] `make run` — enemies spawn automatically
  - [ ] Enemies spawn outside visible screen area
  - [ ] Spawn rate is approximately every 2 seconds
- **Status**: `[ ]`

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
  - [ ] `make run` — spawn rate increases over time
  - [ ] After 60 seconds, enemies spawn faster
  - [ ] Minimum interval is respected (not too fast)
- **Status**: `[ ]`

---

### 3.3 Collision Detection

#### P3.3.1 — Implement circle-circle collision
- **Description**: Utility function for collision checks
- **Dependencies**: P3.2.2
- **Actions**:
  1. Add to utils.h/c:
     - `bool CheckCollisionCircles(Vector2 c1, float r1, Vector2 c2, float r2)`
     - Returns true if distance between centers < r1 + r2
- **Verification**:
  - [ ] `make` compiles successfully
  - [ ] Unit test: overlapping circles return true
  - [ ] Unit test: non-overlapping circles return false
- **Status**: `[ ]`

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
  - [ ] `make run` — shooting enemies reduces their health
  - [ ] Enemies die when health reaches 0
  - [ ] Projectiles disappear on hit
- **Status**: `[ ]`

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
  - [ ] `make run` — touching enemy damages player
  - [ ] Player health decreases
  - [ ] Brief invincibility prevents instant death
  - [ ] Can die from enemy contact
- **Status**: `[ ]`

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
  - [ ] `make` compiles successfully
- **Status**: `[ ]`

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
  - [ ] `make` compiles successfully
- **Status**: `[ ]`

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
  - [ ] `make run` — killing enemies spawns XP crystals
  - [ ] Crystals appear at death location
  - [ ] Crystals are visible
- **Status**: `[ ]`

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
  - [ ] `make run` — XP crystals move toward player when close
  - [ ] XP crystals are collected on contact
  - [ ] Player XP increases (debug print)
- **Status**: `[ ]`

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
  - [ ] `make` compiles successfully
  - [ ] Player levels up at correct XP thresholds
  - [ ] Level displayed on HUD
- **Status**: `[ ]`

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
  - [ ] `make run` — game pauses when leveling up
  - [ ] Level up screen appears
  - [ ] Game visible in background
- **Status**: `[ ]`

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
  - [ ] `make` compiles successfully
- **Status**: `[ ]`

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
  - [ ] `make` compiles successfully
  - [ ] Applying upgrades changes player/weapon stats
- **Status**: `[ ]`

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
  - [ ] `make run` — level up shows 3 upgrade options
  - [ ] Options have names and descriptions
  - [ ] Options are different each time
- **Status**: `[ ]`

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
  - [ ] `make run` — pressing 1/2/3 selects upgrade
  - [ ] Selected upgrade is applied (visible effect)
  - [ ] Game resumes after selection
  - [ ] Multiple level ups work correctly
- **Status**: `[ ]`

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
  - [ ] `make` compiles successfully
- **Status**: `[ ]`

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
  - [ ] `make` compiles successfully
  - [ ] Test explosion spawn with keypress
  - [ ] Particles move outward and fade
- **Status**: `[ ]`

#### P5.1.3 — Integrate particles into game
- **Description**: Add particles to game events
- **Dependencies**: P5.1.2
- **Actions**:
  1. Add `ParticlePool particles` to GameData, initialize
  2. Update and draw particles in game loop
  3. Spawn explosion when enemy dies (red/orange particles)
  4. Spawn small burst when projectile hits
- **Verification**:
  - [ ] `make run` — enemy deaths create particle explosions
  - [ ] Projectile impacts create small particle bursts
  - [ ] Particles don't impact performance noticeably
- **Status**: `[ ]`

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
  - [ ] `make run` — camera follows player
  - [ ] Camera movement is smooth (slight lag)
  - [ ] All entities render correctly with camera
- **Status**: `[ ]`

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
  - [ ] `make run` — screen shakes when enemy dies
  - [ ] Stronger shake on player damage
  - [ ] Shake diminishes over duration
  - [ ] No shake during normal gameplay
- **Status**: `[ ]`

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
  - [ ] `make run` — HUD displays all elements
  - [ ] Values update in real-time
  - [ ] HUD doesn't move with camera
  - [ ] HUD is readable (good contrast)
- **Status**: `[ ]`

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
  - [ ] `make` compiles successfully
  - [ ] Orbiter circles player when spawned
  - [ ] Orbiter slowly closes in
  - [ ] Orbiter can be killed
- **Status**: `[ ]`

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
  - [ ] `make run` — only Chasers before 30s
  - [ ] Orbiters start appearing after 30s
  - [ ] Mix of enemies mid-game
- **Status**: `[ ]`

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
  - [ ] `make` compiles successfully
  - [ ] Splitter moves toward player
  - [ ] Splitter is larger than Chaser
- **Status**: `[ ]`

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
  - [ ] `make run` — killing Splitter spawns 2 smaller ones
  - [ ] Small Splitters spawn even smaller ones
  - [ ] Eventually no more splits occur
  - [ ] XP only from final kills
- **Status**: `[ ]`

#### P6.2.3 — Add Splitter to spawn system
- **Description**: Splitters spawn after 60 seconds
- **Dependencies**: P6.2.2
- **Actions**:
  1. Update GetEnemyTypeForTime:
     - > 60s: include Splitter in rotation
     - > 90s: increase Splitter probability
- **Verification**:
  - [ ] `make run` — Splitters appear after 60s
  - [ ] Game remains playable with Splitters
- **Status**: `[ ]`

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
  - [ ] `make` compiles successfully
  - [ ] AudioInit/Cleanup don't crash
- **Status**: `[ ]`

#### P7.1.2 — Generate placeholder sounds
- **Description**: Create basic synth sounds programmatically
- **Dependencies**: P7.1.1
- **Actions**:
  1. In AudioInit, use raylib's audio generation or load WAV files
  2. For now, can use simple beeps at different frequencies
  3. Alternative: source free sound effects and place in resources/sounds/
- **Verification**:
  - [ ] `make` compiles successfully
  - [ ] Sounds load without errors
- **Status**: `[ ]`

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
  - [ ] `make run` — sounds play on events
  - [ ] No audio clipping or distortion
  - [ ] Sounds don't overlap excessively
- **Status**: `[ ]`

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
  - [ ] `make run` — music plays during gameplay
  - [ ] Music loops seamlessly
  - [ ] Music pauses/resumes correctly
- **Status**: `[ ]`

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
  - [ ] `make run` — game renders correctly through texture
  - [ ] No visual difference yet (pass-through)
- **Status**: `[ ]`

#### P8.1.2 — Implement bloom shader
- **Description**: Glow effect for bright objects
- **Dependencies**: P8.1.1
- **Actions**:
  1. Create `resources/shaders/bloom.fs` with bloom GLSL code from PLAN.md
  2. Load shader in game init
  3. Apply shader when drawing render texture to screen
  4. Add uniform for intensity control
- **Verification**:
  - [ ] `make run` — bloom effect visible
  - [ ] Bright objects (player, projectiles) glow
  - [ ] Effect is subtle, not overwhelming
- **Status**: `[ ]`

#### P8.1.3 — Implement CRT shader
- **Description**: Retro CRT monitor effect
- **Dependencies**: P8.1.2
- **Actions**:
  1. Create `resources/shaders/crt.fs` with CRT GLSL code from PLAN.md
  2. Chain after bloom shader (or combine)
  3. Add time uniform for animation
  4. Make toggleable in settings
- **Verification**:
  - [ ] `make run` — CRT scanlines visible
  - [ ] Slight barrel distortion on edges
  - [ ] RGB separation effect
- **Status**: `[ ]`

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
  - [ ] `make run` — player leaves trail when moving
  - [ ] Trail fades smoothly
  - [ ] Trail disappears when stationary
- **Status**: `[ ]`

#### P8.2.2 — Add enemy hit flash
- **Description**: Enemies flash white when damaged
- **Dependencies**: P8.2.1
- **Actions**:
  1. Add `float hitFlashTimer` to Enemy struct
  2. Set hitFlashTimer = 0.1 when damaged
  3. In draw: if hitFlashTimer > 0, draw white instead of normal color
  4. Decrease hitFlashTimer in update
- **Verification**:
  - [ ] `make run` — enemies flash when hit
  - [ ] Flash is brief and noticeable
  - [ ] Doesn't interfere with normal rendering
- **Status**: `[ ]`

#### P8.2.3 — Add background grid
- **Description**: Subtle grid showing movement
- **Dependencies**: P8.2.2
- **Actions**:
  1. Draw grid lines in world space (before entities)
  2. Grid moves with camera (world-aligned)
  3. Use very dark color, subtle alpha
  4. Grid spacing: 64 pixels
- **Verification**:
  - [ ] `make run` — grid visible in background
  - [ ] Grid moves as camera moves
  - [ ] Grid doesn't distract from gameplay
- **Status**: `[ ]`

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
  - [ ] `make run` — starts in menu
  - [ ] Title displays correctly
  - [ ] Can start game and quit
- **Status**: `[ ]`

#### P9.1.2 — Add high score display
- **Description**: Show best score on menu
- **Dependencies**: P9.1.1
- **Actions**:
  1. Add `int highScore` to game (static/global or saved)
  2. Update high score when game ends if score > highScore
  3. Display on menu: "High Score: XXXXX"
  4. (Optional) Save/load from file for persistence
- **Verification**:
  - [ ] `make run` — high score displays on menu
  - [ ] High score updates after good run
  - [ ] Persists between game sessions (if implemented)
- **Status**: `[ ]`

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
  - [ ] `make run` — ESC pauses game
  - [ ] Game visible behind pause overlay
  - [ ] Can resume or quit to menu
- **Status**: `[ ]`

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
  - [ ] `make run` — game over shows all stats
  - [ ] Stats are accurate
  - [ ] Can return to menu
- **Status**: `[ ]`

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
  - [ ] No crashes during extended play
  - [ ] All features work as expected
  - [ ] No visual artifacts
- **Status**: `[ ]`

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
  - [ ] Game feels fair
  - [ ] Progression feels rewarding
  - [ ] Death doesn't feel cheap
- **Status**: `[ ]`

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
  - [ ] `make run` — player flashes after taking damage
  - [ ] Clear feedback that player is temporarily invincible
- **Status**: `[ ]`

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
  - [ ] `make run` — multiplier increases over time
  - [ ] Multiplier resets on damage
  - [ ] Higher scores possible with skill
- **Status**: `[ ]`

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
  - [ ] `make` compiles with no warnings
  - [ ] Code is readable and documented
  - [ ] No debug output in console
- **Status**: `[ ]`

---

## SUMMARY

### Task Counts by Phase

| Phase | Description | Tasks | Done |
|-------|-------------|-------|------|
| 0 | Project Setup | 9 | 8 ✓ |
| 1 | Player System | 14 | 14 ✓ |
| 2 | Weapons & Projectiles | 10 | 10 ✓ |
| 3 | Enemies | 10 | 0 |
| 4 | XP & Leveling | 10 | 0 |
| 5 | Particles & Juice | 7 | 0 |
| 6 | Additional Enemies | 5 | 0 |
| 7 | Audio | 4 | 0 |
| 8 | Visual Polish | 6 | 0 |
| 9 | Menus & Polish | 4 | 0 |
| 10 | Final Polish | 5 | 0 |
| **Total** | | **84** | **32** |

### Estimated Time

- **Minimum**: 2-3 weeks (experienced developer, full-time)
- **Comfortable**: 4-5 weeks (part-time or learning)
- **With polish**: 6+ weeks (all stretch goals)

---

## VERIFICATION CHECKLIST (Run After Each Phase)

After completing each phase, verify:

- [ ] `make` compiles with zero warnings
- [ ] `make run` launches without crashes
- [ ] Play for 2 minutes without issues
- [ ] All features from phase work correctly
- [ ] No obvious visual glitches
- [ ] Memory usage stable (no leaks)
- [ ] Git commit: `git add -A && git commit -m "Complete Phase X"`

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

---

## NOTES FOR ENGINEERS

1. **Always test after each task** — never batch multiple tasks without testing
2. **Keep functions small** — if a function exceeds 50 lines, split it
3. **Use const where possible** — prevents accidental modifications
4. **Initialize all variables** — raylib style guide requirement
5. **Comment unclear code** — but prefer clear code over comments
6. **Profile if slow** — use raylib's `GetFPS()` to monitor performance

---

*Last updated: 2026-01-04 — Phase 2 Complete (32/84 tasks, 38.1%)*
