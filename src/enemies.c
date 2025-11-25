#include "enemies.h"
#include "resources.h"

void initEnemies()
{
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

    // Initialize powerup truck
    powerup_truck.active = FALSE;
    powerup_truck.sprite = NULL;
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
                                           TILE_ATTR(PAL1, 0, FALSE, from_left ? FALSE : TRUE));
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
                            bombs[j].vx = FIX16(0);  // No horizontal velocity initially
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

        // Add ammunition for completing the wave
        if (two_player_mode)
        {
            ammo_p1 += 5;  // Two-player mode: each player gets 5 per wave
            ammo_p2 += 5;
        }
        else
        {
            ammo_p1 += 10;  // Single-player mode: get 10 per wave
        }
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

            // Move bomb (both horizontal and vertical)
            bombs[i].x = bombs[i].x + bombs[i].vx;
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

// Apply blast wave effect from bomb explosion at position (bx, by)
// This can trigger chain reactions for bombs within BOMB_CHAIN_RADIUS
void applyBlastWave(s16 bx, s16 by)
{
    for (u8 k = 0; k < MAX_BOMBS; k++)
    {
        if (bombs[k].active)
        {
            // Get position of this bomb
            s16 other_bx = (s16)(bombs[k].x >> FIX16_FRAC_BITS);
            s16 other_by = (s16)(bombs[k].y >> FIX16_FRAC_BITS);

            // Calculate distance from impact point
            s16 dx = other_bx - bx;
            s16 dy = other_by - by;
            s16 dist = abs(dx) + abs(dy);  // Manhattan distance (faster than sqrt)

            // If within chain radius, destroy and trigger new blast wave
            if (dist < BOMB_CHAIN_RADIUS && dist > 0)
            {
                // Destroy this bomb
                bombs[k].active = FALSE;
                SPR_releaseSprite(bombs[k].sprite);
                bombs[k].sprite = NULL;

                // Recursively trigger blast wave from this bomb's position
                applyBlastWave(other_bx, other_by);
            }
            // Otherwise if within blast radius, apply knockback
            else if (dist < BOMB_BLAST_RADIUS && dist > 0)
            {
                // Calculate force that falls off with distance
                // Force is inversely proportional: closer = stronger
                // Formula: force = BOMB_BLAST_FORCE * (RADIUS - dist) / RADIUS
                s32 force_scale = ((s32)(BOMB_BLAST_RADIUS - dist) * (s32)BOMB_BLAST_FORCE) / BOMB_BLAST_RADIUS;

                // Normalize direction and apply scaled force
                s32 force_x = ((s32)dx * force_scale) / dist;
                s32 force_y = ((s32)dy * force_scale) / dist;

                // Add to bomb's velocity
                bombs[k].vx = bombs[k].vx + (fix16)force_x;
                bombs[k].vy = bombs[k].vy + (fix16)force_y;
            }
        }
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
