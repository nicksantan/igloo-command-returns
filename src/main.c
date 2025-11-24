#include <genesis.h>
#include "resources.h"

// Game constants
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   224

#define CROSSHAIR_SPEED_NORMAL  2
#define CROSSHAIR_SPEED_BOOST   4

#define MAX_MISSILES 30
// Using fractional speed - 3 pixels per frame 
#define MISSILE_SPEED FIX16(3)

#define MAX_ENEMIES 5
#define MAX_BOMBS 20
#define NUM_IGLOOS 5

#define ENEMY_SPEED FIX16(0.3)  // Slow horizontal movement
#define BOMB_INITIAL_VY FIX16(0.05)  // Initial downward velocity (slower)
#define BOMB_GRAVITY FIX16(0.02)     // Gravity acceleration per frame (slower)
#define BOMB_MAX_VY FIX16(0.8)       // Terminal velocity (slower)

#define BOMB_DROP_CHANCE 2  // 0.2% chance per frame at wave 1 (uses % 1000)

// Powerup truck constants
#define TRUCK_SPEED FIX16(0.5)  // Truck horizontal speed
#define TRUCK_Y (SCREEN_HEIGHT - 32)  // Y position (below cannons)

// Cannon positions (left and right at bottom)
#define CANNON_LEFT_X  32
#define CANNON_RIGHT_X (SCREEN_WIDTH - 32)
#define CANNON_Y       (SCREEN_HEIGHT - 48)

// Missile structure
typedef struct {
    fix16 x, y;          // Position (fixed-point for smooth movement)
    fix16 target_x, target_y;  // Target position
    fix16 vx, vy;        // Velocity
    u8 active;           // Is this missile active?
    Sprite* sprite;      // Sprite pointer
} Missile;

// Enemy structure
typedef struct {
    fix16 x, y;          // Position
    fix16 vx;            // Horizontal velocity (negative = left, positive = right)
    u8 active;           // Is this enemy active?
    u8 from_left;        // Did it spawn from left side? (determines exit side)
    Sprite* sprite;      // Sprite pointer
} Enemy;

// Bomb structure
typedef struct {
    fix16 x, y;          // Position
    fix16 vy;            // Vertical velocity (accelerates with gravity)
    u8 active;           // Is this bomb active?
    Sprite* sprite;      // Sprite pointer
} Bomb;

// Igloo structure
typedef struct {
    s16 x, y;            // Position
    u8 alive;            // Is igloo still standing?
    Sprite* sprite;      // Sprite pointer
} Igloo;

// Powerup truck structure
typedef struct {
    fix16 x, y;          // Position
    fix16 vx;            // Horizontal velocity
    u8 active;           // Is truck active?
    u8 from_left;        // Direction (1 = left to right, 0 = right to left)
    Sprite* sprite;      // Sprite pointer
} PowerupTruck;

// Crosshair position
s16 crosshair_x = SCREEN_WIDTH / 2;
s16 crosshair_y = SCREEN_HEIGHT / 2;

// Sprites
Sprite* crosshair_sprite = NULL;
Sprite* cannon_left_sprite = NULL;
Sprite* cannon_right_sprite = NULL;

// Game object pools
Missile missiles[MAX_MISSILES];
Enemy enemies[MAX_ENEMIES];
Bomb bombs[MAX_BOMBS];
Igloo igloos[NUM_IGLOOS];
PowerupTruck powerup_truck;  // Single truck instance

// Game state
u16 current_wave = 1;
u8 enemies_spawned = 0;
u8 wave_complete = FALSE;
u8 game_over = FALSE;
u32 score = 0;  // Player score

// Previous A button state (for edge detection)
u16 prev_joy = 0;

void initGame()
{
    // Initialize VDP
    VDP_setScreenWidth320();

    // Clear background planes
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);

    // Set background color to dark blue
    PAL_setColor(0, RGB24_TO_VDPCOLOR(0x001040));

    // Load the crosshair sprite's palette into PAL1
    PAL_setPalette(PAL1, sprite_crosshair.palette->data, CPU);

    // Initialize sprite engine
    SPR_init();

    // Load crosshair sprite (16x16)
    crosshair_sprite = SPR_addSprite(&sprite_crosshair,
                                      crosshair_x - 8,
                                      crosshair_y - 8,
                                      TILE_ATTR(PAL1, 0, FALSE, FALSE));

    // Load cannon sprites (16x16 each)
    cannon_left_sprite = SPR_addSprite(&sprite_cannon,
                                        CANNON_LEFT_X - 8,
                                        CANNON_Y - 8,
                                        TILE_ATTR(PAL1, 0, FALSE, FALSE));

    cannon_right_sprite = SPR_addSprite(&sprite_cannon,
                                         CANNON_RIGHT_X - 8,
                                         CANNON_Y - 8,
                                         TILE_ATTR(PAL1, 0, FALSE, FALSE));

    // Initialize missile pool
    for (u8 i = 0; i < MAX_MISSILES; i++)
    {
        missiles[i].active = FALSE;
        missiles[i].sprite = NULL;
    }

    // Initialize enemy pool
    for (u8 i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].active = FALSE;
        enemies[i].sprite = NULL;
    }

    // Initialize bomb pool
    for (u8 i = 0; i < MAX_BOMBS; i++)
    {
        bombs[i].active = FALSE;
        bombs[i].sprite = NULL;
    }

    // Initialize igloos (5 evenly spaced along bottom, centered)
    s16 igloo_spacing = SCREEN_WIDTH / (NUM_IGLOOS + 1);
    for (u8 i = 0; i < NUM_IGLOOS; i++)
    {
        igloos[i].x = igloo_spacing * (i + 1);
        igloos[i].y = CANNON_Y;
        igloos[i].alive = TRUE;

        igloos[i].sprite = SPR_addSprite(&sprite_igloo,
                                          igloos[i].x - 8,
                                          igloos[i].y - 8,
                                          TILE_ATTR(PAL1, 0, FALSE, FALSE));
    }

    // Initialize powerup truck
    powerup_truck.active = FALSE;
    powerup_truck.sprite = NULL;

    // Draw game title and instructions
    // VDP_drawText("IGLOO COMMAND RETURNS", 9, 2);
    // VDP_drawText("D-Pad:Move  B:Fast  A:Fire", 5, 26);
}

void updateCrosshair()
{
    // Read joypad
    u16 joy = JOY_readJoypad(JOY_1);

    // Determine speed based on B button
    s16 speed = (joy & BUTTON_B) ? CROSSHAIR_SPEED_BOOST : CROSSHAIR_SPEED_NORMAL;

    // Handle D-pad input
    if (joy & BUTTON_UP)
        crosshair_y -= speed;
    if (joy & BUTTON_DOWN)
        crosshair_y += speed;
    if (joy & BUTTON_LEFT)
        crosshair_x -= speed;
    if (joy & BUTTON_RIGHT)
        crosshair_x += speed;

    // Keep crosshair within screen bounds (16x16 sprite)
    if (crosshair_x < 16)
        crosshair_x = 16;
    if (crosshair_x > SCREEN_WIDTH - 16)
        crosshair_x = SCREEN_WIDTH - 16;
    if (crosshair_y < 32)
        crosshair_y = 32;
    if (crosshair_y > SCREEN_HEIGHT - 16)
        crosshair_y = SCREEN_HEIGHT - 16;

    // Update sprite position
    SPR_setPosition(crosshair_sprite, crosshair_x - 8, crosshair_y - 8);
}

u8 active_missile_count = 0;

void fireMissile()
{
    // Don't fire if crosshair is below the cannon line
    // (allows clicking on powerups/trucks below cannons)
    if (crosshair_y >= CANNON_Y)
    {
        return;
    }

    // Find an inactive missile slot
    for (u8 i = 0; i < MAX_MISSILES; i++)
    {
        if (!missiles[i].active)
        {
            // Determine which cannon is closer to crosshair
            // If equidistant, prefer left cannon
            s16 dist_left = abs(crosshair_x - CANNON_LEFT_X);
            s16 dist_right = abs(crosshair_x - CANNON_RIGHT_X);

            s16 cannon_x = (dist_left <= dist_right) ? CANNON_LEFT_X : CANNON_RIGHT_X;
            s16 cannon_y = CANNON_Y;

            // Set missile start position (at cannon)
            missiles[i].x = FIX16(cannon_x);
            missiles[i].y = FIX16(cannon_y);

            // Set target (crosshair position)
            missiles[i].target_x = FIX16(crosshair_x);
            missiles[i].target_y = FIX16(crosshair_y);

            // Calculate velocity using INTEGER math to avoid buggy fix16 functions
            s32 dx = crosshair_x - cannon_x;
            s32 dy = crosshair_y - cannon_y;

            // Calculate distance squared (we'll use this to normalize)
            s32 dist_sq = dx * dx + dy * dy;

            // Safety check
            if (dist_sq == 0)
            {
                return;
            }

            // Calculate velocity scaled by 256 for precision
            // We want speed of 0.5 pixels/frame, so velocity = (dx,dy) * 0.5 / distance
            // To avoid floating point: velocity = (dx,dy) * 128 / distance / 256
            // We use 256 scale factor to maintain precision

            // Fast integer square root approximation
            s32 dist = 1;
            s32 test = dist_sq;
            while (test > 1)
            {
                test >>= 2;
                dist <<= 1;
            }

            // Calculate velocity: (direction * MISSILE_SPEED) / distance
            // MISSILE_SPEED is in fix16 format, so we scale by that value
            s32 speed_value = (s32)(MISSILE_SPEED >> 6); // Convert fix16 to usable scale
            s32 vx_scaled = (dx * speed_value * 64) / dist;
            s32 vy_scaled = (dy * speed_value * 64) / dist;

            // Convert to fix16 format (already scaled correctly)
            missiles[i].vx = (fix16)vx_scaled;
            missiles[i].vy = (fix16)vy_scaled;

            // Create sprite for this missile
            s16 sprite_x = (s16)(missiles[i].x >> FIX16_FRAC_BITS) - 4;
            s16 sprite_y = (s16)(missiles[i].y >> FIX16_FRAC_BITS) - 4;

            missiles[i].sprite = SPR_addSprite(&sprite_snowball,
                                                sprite_x,
                                                sprite_y,
                                                TILE_ATTR(PAL1, 0, FALSE, FALSE));

            // Check if sprite creation failed
            if (missiles[i].sprite == NULL)
            {
                // Sprite creation failed - don't fire
                return;
            }

            missiles[i].active = TRUE;

            // Only fire one missile per button press
            return;
        }
    }
}

void updateMissiles()
{
    active_missile_count = 0;

    for (u8 i = 0; i < MAX_MISSILES; i++)
    {
        if (missiles[i].active)
        {
            active_missile_count++;

            // Move missile
            missiles[i].x = missiles[i].x + missiles[i].vx;
            missiles[i].y = missiles[i].y + missiles[i].vy;

            s16 mx = (s16)(missiles[i].x >> FIX16_FRAC_BITS);
            s16 my = (s16)(missiles[i].y >> FIX16_FRAC_BITS);

            // Check if missile went off screen (left, right, or top)
            if (mx < 0 || mx > SCREEN_WIDTH || my < 0)
            {
                missiles[i].active = FALSE;
                SPR_releaseSprite(missiles[i].sprite);
                missiles[i].sprite = NULL;
            }
            else
            {
                // Update sprite position
                SPR_setPosition(missiles[i].sprite, mx - 4, my - 4);
            }
        }
    }
}

void spawnWave()
{
    // Spawn 5 enemies at random positions
    for (u8 i = 0; i < MAX_ENEMIES; i++)
    {
        // Random side (0 = left, 1 = right)
        u8 from_left = random() % 2;

        // Random Y position between 16 and 132-16
        s16 spawn_y = 16 + (random() % 101);

        // Set position and velocity based on spawn side
        // Spawn 20-60 pixels off-screen
        s16 spawn_offset = 20 + (random() % 40);  // Range: 20 to 59 pixels off-screen

        // Add random velocity offset: +/- 20% variation
        // ENEMY_SPEED is 0.3, so variation is +/- 0.06 (20% of 0.3)
        s16 velocity_percent = (random() % 40) - 20;  // Range: -20 to +19 percent
        fix16 speed_variation = (ENEMY_SPEED * velocity_percent) / 100;

        if (from_left)
        {
            enemies[i].x = FIX16(-spawn_offset);  // Start off left edge
            enemies[i].vx = ENEMY_SPEED + speed_variation;  // Move right with variation
        }
        else
        {
            enemies[i].x = FIX16(SCREEN_WIDTH + spawn_offset);  // Start off right edge
            enemies[i].vx = -ENEMY_SPEED - speed_variation;  // Move left with variation
        }

        enemies[i].y = FIX16(spawn_y);
        enemies[i].from_left = from_left;
        enemies[i].active = TRUE;

        // Create sprite
        s16 sprite_x = (s16)(enemies[i].x >> FIX16_FRAC_BITS) - 8;
        s16 sprite_y = spawn_y - 8;

        enemies[i].sprite = SPR_addSprite(&sprite_plane,
                                           sprite_x,
                                           sprite_y,
                                           TILE_ATTR(PAL1, 0, from_left ? FALSE : TRUE, FALSE));
    }

    enemies_spawned = MAX_ENEMIES;
    wave_complete = FALSE;
}

void updateEnemies()
{
    u8 active_count = 0;

    for (u8 i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            active_count++;

            // Move enemy horizontally
            enemies[i].x = enemies[i].x + enemies[i].vx;

            s16 ex = (s16)(enemies[i].x >> FIX16_FRAC_BITS);
            s16 ey = (s16)(enemies[i].y >> FIX16_FRAC_BITS);

            // Check if enemy went off screen
            if ((enemies[i].from_left && ex > SCREEN_WIDTH) ||
                (!enemies[i].from_left && ex < 0))
            {
                // Enemy escaped
                enemies[i].active = FALSE;
                SPR_releaseSprite(enemies[i].sprite);
                enemies[i].sprite = NULL;
            }
            else
            {
                // Update sprite position
                SPR_setPosition(enemies[i].sprite, ex - 8, ey - 8);

                // Randomly drop bombs (0.2% base chance, scales with wave)
                u16 drop_chance = BOMB_DROP_CHANCE * current_wave;
                if (drop_chance > 300) drop_chance = 300;  // Cap at 30%

                if ((random() % 1000) < drop_chance)
                {
                    // Find an inactive bomb slot
                    for (u8 j = 0; j < MAX_BOMBS; j++)
                    {
                        if (!bombs[j].active)
                        {
                            bombs[j].x = enemies[i].x;
                            bombs[j].y = enemies[i].y;
                            bombs[j].vy = BOMB_INITIAL_VY;
                            bombs[j].active = TRUE;

                            bombs[j].sprite = SPR_addSprite(&sprite_bomb,
                                                             ex - 4,
                                                             ey - 4,
                                                             TILE_ATTR(PAL1, 0, FALSE, FALSE));
                            break;  // Only drop one bomb
                        }
                    }
                }
            }
        }
    }

    // Check if wave is complete (no active enemies)
    if (active_count == 0 && enemies_spawned > 0 && !wave_complete)
    {
        wave_complete = TRUE;
        current_wave++;
        enemies_spawned = 0;
    }
}

void updateBombs()
{
    for (u8 i = 0; i < MAX_BOMBS; i++)
    {
        if (bombs[i].active)
        {
            // Apply gravity
            bombs[i].vy = bombs[i].vy + BOMB_GRAVITY;
            if (bombs[i].vy > BOMB_MAX_VY)
                bombs[i].vy = BOMB_MAX_VY;

            // Move bomb
            bombs[i].y = bombs[i].y + bombs[i].vy;

            s16 bx = (s16)(bombs[i].x >> FIX16_FRAC_BITS);
            s16 by = (s16)(bombs[i].y >> FIX16_FRAC_BITS);

            // Check if bomb went off screen (bottom)
            if (by > SCREEN_HEIGHT)
            {
                bombs[i].active = FALSE;
                SPR_releaseSprite(bombs[i].sprite);
                bombs[i].sprite = NULL;
            }
            else
            {
                // Update sprite position
                SPR_setPosition(bombs[i].sprite, bx - 4, by - 4);
            }
        }
    }
}

void checkCollisions()
{
    // Check snowball vs enemy collisions
    for (u8 i = 0; i < MAX_MISSILES; i++)
    {
        if (missiles[i].active)
        {
            s16 mx = (s16)(missiles[i].x >> FIX16_FRAC_BITS);
            s16 my = (s16)(missiles[i].y >> FIX16_FRAC_BITS);

            for (u8 j = 0; j < MAX_ENEMIES; j++)
            {
                if (enemies[j].active)
                {
                    s16 ex = (s16)(enemies[j].x >> FIX16_FRAC_BITS);
                    s16 ey = (s16)(enemies[j].y >> FIX16_FRAC_BITS);

                    // Simple AABB collision (8px snowball vs 16px enemy)
                    if (abs(mx - ex) < 12 && abs(my - ey) < 12)
                    {
                        // Hit! Destroy both
                        missiles[i].active = FALSE;
                        SPR_releaseSprite(missiles[i].sprite);
                        missiles[i].sprite = NULL;

                        enemies[j].active = FALSE;
                        SPR_releaseSprite(enemies[j].sprite);
                        enemies[j].sprite = NULL;

                        // Award points for destroying enemy
                        score += 100;

                        break;
                    }
                }
            }
        }
    }

    // Check snowball vs bomb collisions
    for (u8 i = 0; i < MAX_MISSILES; i++)
    {
        if (missiles[i].active)
        {
            s16 mx = (s16)(missiles[i].x >> FIX16_FRAC_BITS);
            s16 my = (s16)(missiles[i].y >> FIX16_FRAC_BITS);

            for (u8 j = 0; j < MAX_BOMBS; j++)
            {
                if (bombs[j].active)
                {
                    s16 bx = (s16)(bombs[j].x >> FIX16_FRAC_BITS);
                    s16 by = (s16)(bombs[j].y >> FIX16_FRAC_BITS);

                    // Simple AABB collision (8px snowball vs 8px bomb)
                    if (abs(mx - bx) < 8 && abs(my - by) < 8)
                    {
                        // Hit! Destroy both
                        missiles[i].active = FALSE;
                        SPR_releaseSprite(missiles[i].sprite);
                        missiles[i].sprite = NULL;

                        bombs[j].active = FALSE;
                        SPR_releaseSprite(bombs[j].sprite);
                        bombs[j].sprite = NULL;

                        // Award points for destroying bomb
                        score += 10;

                        break;
                    }
                }
            }
        }
    }

    // Check bomb vs igloo collisions
    for (u8 i = 0; i < MAX_BOMBS; i++)
    {
        if (bombs[i].active)
        {
            s16 bx = (s16)(bombs[i].x >> FIX16_FRAC_BITS);
            s16 by = (s16)(bombs[i].y >> FIX16_FRAC_BITS);

            for (u8 j = 0; j < NUM_IGLOOS; j++)
            {
                if (igloos[j].alive)
                {
                    // Simple AABB collision (8px bomb vs 16px igloo)
                    if (abs(bx - igloos[j].x) < 12 && abs(by - igloos[j].y) < 12)
                    {
                        // Hit! Destroy both
                        bombs[i].active = FALSE;
                        SPR_releaseSprite(bombs[i].sprite);
                        bombs[i].sprite = NULL;

                        igloos[j].alive = FALSE;
                        SPR_releaseSprite(igloos[j].sprite);
                        igloos[j].sprite = NULL;
                        break;
                    }
                }
            }
        }
    }
}

void checkGameOver()
{
    // Count living igloos
    u8 alive_count = 0;
    for (u8 i = 0; i < NUM_IGLOOS; i++)
    {
        if (igloos[i].alive)
            alive_count++;
    }

    if (alive_count == 0)
    {
        game_over = TRUE;
    }
}

u8 shouldSpawnTruck(u16 wave)
{
    // First truck on wave 3
    if (wave < 3) return FALSE;

    // Waves 3-11: every 3 waves (3, 6, 9)
    if (wave <= 11)
    {
        return (wave % 3) == 0;
    }

    // Waves 12-20: every 4 waves (12, 16, 20)
    if (wave <= 20)
    {
        return (wave % 4) == 0;
    }

    // After wave 20: every 4 waves (24, 28, 32...)
    return (wave % 4) == 0;
}

void spawnPowerupTruck()
{
    // Don't spawn if already active
    if (powerup_truck.active) return;

    // Random direction
    u8 from_left = random() % 2;

    powerup_truck.y = FIX16(TRUCK_Y);
    powerup_truck.from_left = from_left;

    if (from_left)
    {
        powerup_truck.x = FIX16(-20);  // Start off left edge
        powerup_truck.vx = TRUCK_SPEED;  // Move right
    }
    else
    {
        powerup_truck.x = FIX16(SCREEN_WIDTH + 20);  // Start off right edge
        powerup_truck.vx = -TRUCK_SPEED;  // Move left
    }

    powerup_truck.active = TRUE;

    // Create sprite (flip horizontally if coming from right)
    s16 sprite_x = (s16)(powerup_truck.x >> FIX16_FRAC_BITS) - 8;
    s16 sprite_y = TRUCK_Y - 8;

    powerup_truck.sprite = SPR_addSprite(&sprite_cannon,
                                          sprite_x,
                                          sprite_y,
                                          TILE_ATTR(PAL1, 0, FALSE, from_left ? FALSE : TRUE));
}

void updatePowerupTruck()
{
    if (!powerup_truck.active) return;

    // Move truck
    powerup_truck.x = powerup_truck.x + powerup_truck.vx;

    s16 tx = (s16)(powerup_truck.x >> FIX16_FRAC_BITS);

    // Check if truck went off screen
    if ((powerup_truck.from_left && tx > SCREEN_WIDTH + 20) ||
        (!powerup_truck.from_left && tx < -20))
    {
        // Truck left screen
        powerup_truck.active = FALSE;
        SPR_releaseSprite(powerup_truck.sprite);
        powerup_truck.sprite = NULL;
    }
    else
    {
        // Update sprite position
        SPR_setPosition(powerup_truck.sprite, tx - 8, TRUCK_Y - 8);
    }
}

void handleInput()
{
    u16 joy = JOY_readJoypad(JOY_1);

    // Check for A button press (edge detection - only fire once per press)
    if ((joy & BUTTON_A) && !(prev_joy & BUTTON_A))
    {
        fireMissile();
    }

    prev_joy = joy;
}

int main()
{
    // Initialize game
    initGame();

    // Spawn first wave
    spawnWave();

    // Main game loop
    while(1)
    {
        if (!game_over)
        {
            // Handle input
            handleInput();

            // Update crosshair position
            updateCrosshair();

            // Update missiles
            updateMissiles();

            // Update enemies
            updateEnemies();

            // Update bombs
            updateBombs();

            // Update powerup truck
            updatePowerupTruck();

            // Check collisions
            checkCollisions();

            // Check for game over
            checkGameOver();

            // If wave is complete, spawn next wave after a brief delay
            if (wave_complete && enemies_spawned == 0)
            {
                spawnWave();

                // Check if we should spawn a truck on this wave
                if (shouldSpawnTruck(current_wave))
                {
                    spawnPowerupTruck();
                }
            }

            // Display wave number, igloos remaining, and score
            char status[40];
            u8 igloos_alive = 0;
            for (u8 i = 0; i < NUM_IGLOOS; i++)
            {
                if (igloos[i].alive) igloos_alive++;
            }
            sprintf(status, "WAVE:%02d IGLOOS:%d SCORE:%lu", current_wave, igloos_alive, score);
            VDP_drawText(status, 1, 1);
        }
        else
        {
            // Game over - display message and final score
            char final_score[40];
            VDP_drawText("    GAME OVER!    ", 11, 14);
            VDP_drawText("  All Igloos Lost ", 11, 15);
            sprintf(final_score, " Final Score: %lu ", score);
            VDP_drawText(final_score, 11, 16);
        }

        // Update all sprites (sends sprite data to VDP)
        SPR_update();

        // Wait for VBlank (60 FPS sync)
        SYS_doVBlankProcess();
    }

    return 0;
}
