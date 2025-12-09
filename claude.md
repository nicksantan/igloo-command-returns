# Snowball Defense - Sega Genesis Game

## Project Overview

This is an Arctic-themed defense game for the Sega Genesis/Mega Drive, built using SGDK (Sega Genesis Development Kit). Players defend igloos from aerial attacks by launching snowballs at enemy planes and bombs. The game features single-player and two-player modes, wave-based progression, and special enemies.

**Game Name:** MEGA Igloo Command
**Platform:** Sega Genesis/Mega Drive (16-bit)
**Development Kit:** SGDK v2.00+
**Language:** C99
**ROM Output:** `missile_command.bin`

## Architecture

### Modular Design

The codebase is organized into self-contained modules, each with a header file in `inc/` and implementation in `src/`:

- **main.c** - Game loop, initialization, title screen, state management
- **player.c/h** - Crosshair movement, input handling
- **weapons.c/h** - Missile firing, trajectory calculation, ammunition
- **enemies.c/h** - Enemy planes, bombs, powerup truck, polar bear
- **collision.c/h** - All collision detection (missiles vs enemies, bombs vs igloos)
- **explosions.c/h** - Visual explosion effects with timers
- **scoring.c/h** - Score tracking, bonus thresholds, bonus igloo queue
- **hud.c/h** - On-screen display (wave number, ammo, scores)
- **common.h** - Shared constants, structures, global state declarations

### Global State Pattern

The project uses global state variables defined in `main.c` and declared as `extern` in `common.h`:
- Game mode (`two_player_mode`)
- Wave progression (`current_wave`, `enemies_spawned`, `wave_complete`)
- Scores and ammo for both players
- Bonus igloo system tracking

### Object Pool Pattern

All game entities use fixed-size arrays (object pools) declared in `common.h`:
```c
Missile missiles[MAX_MISSILES];      // 30 max
Enemy enemies[MAX_ENEMIES];          // 7 max
Bomb bombs[MAX_BOMBS];               // 20 max
Igloo igloos[NUM_IGLOOS];            // 5 fixed
Explosion explosions[MAX_EXPLOSIONS]; // 10 max
PowerupTruck powerup_truck;          // Singleton
PolarBear polar_bear;                // Singleton
```

Each structure has an `active` flag to enable/disable objects without memory allocation.

## Sega Genesis Constraints

### Hardware Limits (ALWAYS RESPECT THESE)

1. **Sprites**: 80 hardware sprites maximum, 20 per scanline
2. **RAM**: 64KB total (be memory-conscious)
3. **Color Palettes**: 4 palettes × 16 colors = 64 on-screen colors max
4. **Screen Resolution**: 320×224 pixels (NTSC) or 320×240 (PAL)
5. **Frame Rate**: 60 FPS (NTSC) / 50 FPS (PAL)
6. **CPU**: Motorola 68000 @ 7.67 MHz (limited processing power)

### Current Palette Usage

- **PAL0**: Background/UI colors
- **PAL1**: Crosshair, igloo, cannon, snowball sprites
- **PAL2**: Explosion sprites
- **PAL3**: Available for future use

When adding new sprites, coordinate palette usage carefully. Sprites sharing a palette must use the same 16-color set.

## SGDK-Specific Conventions

### Fixed-Point Math

The Genesis has no floating-point unit. Use SGDK's `fix16` type for sub-pixel precision:
```c
fix16 x = FIX16(10.5);           // Convert float to fixed-point
fix16 result = fix16Add(x, y);   // Add two fixed-point values
s16 int_x = fix16ToInt(x);       // Convert to integer for rendering
```

All physics (velocity, position, acceleration) uses `fix16`.

### Sprite Management

```c
// Add sprite (done once during init)
sprite = SPR_addSprite(&sprite_def, x, y, TILE_ATTR(palette, priority, vflip, hflip));

// Update sprite position every frame
SPR_setPosition(sprite, new_x, new_y);

// Hide inactive sprites
SPR_setVisibility(sprite, HIDDEN);

// Update VDP at end of frame
SPR_update();
```

### VBlank Synchronization

Always end the game loop with:
```c
SYS_doVBlankProcess();  // Wait for vertical blank (60 FPS sync)
```

This prevents tearing and maintains consistent frame timing.

## Resource Pipeline

### Sprite Creation Workflow

1. **Create or Edit PNG Files**: Place in `res/sprites/`
   - Use **magenta (#FF00FF)** as transparent color
   - Minimum size: 128×32 pixels (SGDK requirement for palette storage)
   - Actual sprite can be in top-left corner of larger canvas

2. **Python Helper Scripts**:
   - `create_sprites.py` - Generates simple sprites programmatically
   - `convert_sprites.py` - Converts existing images to SGDK format
   - `create_shared_palette.py` - Manages palette optimization

3. **Define in resources.res**:
   ```
   SPRITE sprite_name "sprites/filename.png" width height FAST 0
   ```
   - Width/height in tiles (8×8 pixels each)
   - Example: 16×16 sprite = 2×2 tiles

4. **Build Process**: SGDK's `rescomp` tool processes `resources.res` and generates:
   - `resources.h` (sprite definitions)
   - `resources.res` (compiled resource data)

5. **Include in Code**:
   ```c
   #include "resources.h"
   SPR_addSprite(&sprite_name, x, y, TILE_ATTR(PAL1, 0, FALSE, FALSE));
   ```

### Current Resource Definitions

See `res/resources.res` for all sprites and assets:
- Crosshair, cannon, snowball, plane, bomb, igloo, polar bear, explosion
- Background music (XGM format)
- Some sprites currently reuse `cannon.png` as placeholder

## Gameplay Systems

### Two-Player Mode

- **P1 Controls**: Left side of screen, left cannon
- **P2 Controls**: Right side of screen, right cannon
- **Ammo Distribution**:
  - Single-player: 50 snowballs
  - Two-player: 25 snowballs each
- Separate score tracking for each player

### Wave System

Enemies spawn in waves with increasing difficulty:
- Wave number tracked in `current_wave`
- Enemy count increases with waves
- Special enemies (truck, polar bear) spawn on specific waves
- Wave completes when all enemies are destroyed

### Bonus Igloo System

- Score thresholds (5000, 10000, etc.) award bonus igloos
- Bonus igloos queued in `bonus_igloos_queued`
- One bonus igloo restored per wave (if needed)
- Maximum igloos on screen: 5

### Special Enemies

**Powerup Truck**:
- Appears on specific waves
- Moves horizontally across bottom of screen
- Shooting it awards ammo powerup
- Spawns from alternating sides

**Polar Bear**:
- Rare enemy with multi-hit system
- Each hit increases its speed (up to 3 hits)
- Score increases with each successive hit (200, 400, 800)
- Must be clicked multiple times to maximize score

### Bomb Blast Mechanics

When a bomb explodes near the ground:
- Creates blast wave effect (`applyBlastWave()`)
- Nearby bombs get launched away (chain reaction possible)
- Blast radius: 32 pixels
- Blast force applied as horizontal velocity

## Build System

### Building

```bash
# Windows (ensure SGDK environment variable is set)
make

# The build will:
# 1. Compile all .c files in src/
# 2. Process resources.res with rescomp
# 3. Link everything into out/missile_command.bin
```

### Environment Setup

Must set `SGDK` environment variable:
```bash
# Windows
set SGDK=C:\sgdk

# Or set in system environment variables permanently
```

### Clean Build

```bash
make clean  # Removes all build artifacts
```

## Code Style & Conventions

### Naming Conventions

- **Functions**: camelCase (`updateEnemies`, `spawnWave`)
- **Structs**: PascalCase (`PowerupTruck`, `Missile`)
- **Constants**: UPPER_SNAKE_CASE (`MAX_MISSILES`, `SCREEN_WIDTH`)
- **Variables**: snake_case (`two_player_mode`, `current_wave`)

### Structure Organization

Each `.c` file typically contains:
1. Static helper functions (file-local)
2. `init*()` function for initialization
3. `update*()` function called every frame
4. `spawn*()` functions for entity creation

### Memory Management

- **No dynamic allocation** (malloc/free) - Genesis has limited heap
- Use object pools with `active` flags
- Reuse inactive objects instead of creating new ones
- Be mindful of stack usage (64KB RAM total)

## Common Patterns

### Entity Update Pattern

```c
void updateEntityType() {
    for (u8 i = 0; i < MAX_ENTITIES; i++) {
        if (!entities[i].active) continue;

        // Update position
        entities[i].x = fix16Add(entities[i].x, entities[i].vx);

        // Update sprite
        SPR_setPosition(entities[i].sprite,
                       fix16ToInt(entities[i].x),
                       fix16ToInt(entities[i].y));

        // Check bounds, deactivate if needed
        if (out_of_bounds) {
            entities[i].active = FALSE;
            SPR_setVisibility(entities[i].sprite, HIDDEN);
        }
    }
}
```

### Collision Detection Pattern

See `collision.c` for examples. Most use distance checks:
```c
s16 dx = obj1.x - obj2.x;
s16 dy = obj1.y - obj2.y;
u16 dist_sq = (dx * dx) + (dy * dy);
if (dist_sq < COLLISION_RADIUS_SQ) {
    // Handle collision
}
```

## Testing & Debugging

### Emulators

- **BlastEm**: Most accurate, best for testing
- **Gens/GS**: Good compatibility, built-in debugger
- **Fusion**: Fast, good for quick testing

### Debug Output

Use SGDK's KDebug functions:
```c
KDebug_Alert("Debug message");
KDebug_AlertNumber(value);
```

Output appears in emulator's debug console (if supported).

### Common Issues

1. **Sprite Flickering**: Too many sprites on one scanline (>20)
2. **Slowdown**: Too much computation per frame, optimize loops
3. **Magenta Pixels**: Transparency color not set correctly in PNG
4. **Resource Not Found**: Check `resources.res` path and syntax

## Current Project Status

### Implemented Features
- Title screen with 1P/2P selection
- Two-player simultaneous gameplay
- Wave-based enemy spawning
- Multiple enemy types (planes, polar bear)
- Bomb dropping and physics
- Collision detection (missiles, bombs, igloos)
- Explosion effects
- Powerup truck system
- Bonus igloo restoration
- Score tracking and HUD
- Ammo management
- Game over detection

### Known Placeholders
- Some sprites reuse `cannon.png` (need unique graphics)
- Background music disabled (line 128 in main.c)
- Background image removed from resources

### Potential Improvements
- Add more enemy variety
- Implement sound effects (XGM_startPlayPCM)
- Create unique sprite graphics for plane/igloo
- Add particle effects
- Implement difficulty scaling
- Add high score persistence (SRAM)

## Important Notes for AI Assistants

1. **Always use fixed-point math** for positions and velocities
2. **Respect sprite limits** - check total sprite count when adding features
3. **Update resources.res** when adding new sprites
4. **Test in emulator** - Genesis hardware is unforgiving
5. **Keep frame processing minimal** - 60 FPS means ~16ms per frame
6. **Use object pools** - never dynamically allocate during gameplay
7. **Coordinate palette usage** - limited to 4 palettes
8. **Magenta = transparent** in all sprite PNGs
9. **Call `SPR_update()` once per frame** at end of game loop
10. **Don't forget `SYS_doVBlankProcess()`** for proper frame timing

## References

- SGDK Documentation: https://github.com/Stephane-D/SGDK/wiki
- Genesis Technical Specs: https://segaretro.org/Sega_Mega_Drive/Technical_specifications
- SGDK Examples: Check `SGDK/samples/` directory
