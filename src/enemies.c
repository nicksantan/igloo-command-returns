#include "enemies.h"
#include "explosions.h"
#include "resources.h"

void initEnemies()
{
    // Initialize enemy pool
    for (u8 i = 0; i < MAX_ENEMIES; i++)
    {
        enemies[i].active = FALSE;
        enemies[i].sprite = NULL;
    }

    // Initialize large enemy pool
    for (u8 i = 0; i < MAX_LARGE_ENEMIES; i++)
    {
        large_enemies[i].active = FALSE;
        large_enemies[i].sprite = NULL;
    }

    // Initialize bomb pool
    for (u8 i = 0; i < MAX_BOMBS; i++)
    {
        bombs[i].active = FALSE;
        bombs[i].sprite = NULL;
    }

    // Initialize powerup truck
    powerup_truck.active = FALSE;
    powerup_truck.spawn_pending = FALSE;
    powerup_truck.spawn_timer = 0;
    powerup_truck.sprite = NULL;

    // Initialize polar bear
    polar_bear.active = FALSE;
    polar_bear.spawn_pending = FALSE;
    polar_bear.spawn_timer = 0;
    polar_bear.click_count = 0;
    polar_bear.sprite = NULL;
}

// Determine how many enemies to spawn based on wave number
u8 getEnemyCountForWave(u16 wave)
{
    if (wave <= 3) return 3;
    if (wave <= 7) return 4;
    if (wave <= 15) return 5;
    if (wave <= 25) return 6;
    return 7;
}

// Determine how many large enemies to spawn based on wave number
u8 getLargeEnemyCountForWave(u16 wave)
{
    if (wave < 5) return 0;          // No large enemies before wave 5
    if (wave < 15) return 1;         // 1 large enemy waves 5-14
    if (wave < 25) return 2;         // 2 large enemies waves 15-24
    if (wave < 45) return 3;         // 3 large enemies waves 25-44
    if (wave < 65) return 4;         // 4 large enemies waves 45-64
    return 5;                         // 5 large enemies wave 65+
}

// Determine base enemy speed based on wave number
fix16 getEnemySpeedForWave(u16 wave)
{
    if (wave < 10) return ENEMY_SPEED;        // Waves 1-9: 0.3
    if (wave < 20) return FIX16(0.35);        // Waves 10-19: 0.35
    if (wave < 30) return FIX16(0.37);        // Waves 20-29: 0.37
    if (wave < 40) return FIX16(0.40);        // Waves 30-39: 0.40
    if (wave < 50) return FIX16(0.45);        // Waves 40-49: 0.45
    return FIX16(0.50);                       // Wave 50+: 0.50
}

void spawnWave()
{
    // Determine enemy count for this wave
    u8 enemy_count = getEnemyCountForWave(current_wave);

    // Track spawn positions to enforce spacing
    typedef struct {
        s16 x, y;
    } SpawnPos;
    SpawnPos spawn_positions[MAX_ENEMIES + MAX_LARGE_ENEMIES];
    u8 spawn_count = 0;

    // Spawn enemies at random positions
    for (u8 i = 0; i < enemy_count; i++)
    {
        // Random side (0 = left, 1 = right)
        u8 from_left = random() % 2;

        // Try to find a valid spawn position (max 10 attempts)
        s16 spawn_y = 0;
        s16 spawn_x = 0;
        u8 valid_position = FALSE;

        for (u8 attempt = 0; attempt < 10; attempt++)
        {
            // Random Y position between 16 and 132-16
            spawn_y = 16 + (random() % 101);

            // Set position based on spawn side
            s16 spawn_offset = 20 + (random() % 40);  // Range: 20 to 59 pixels off-screen

            if (from_left)
                spawn_x = -spawn_offset;
            else
                spawn_x = SCREEN_WIDTH + spawn_offset;

            // Check distance to all previously spawned enemies
            valid_position = TRUE;
            for (u8 j = 0; j < spawn_count; j++)
            {
                s16 dx = abs(spawn_x - spawn_positions[j].x);
                s16 dy = abs(spawn_y - spawn_positions[j].y);
                s16 dist = dx + dy;  // Manhattan distance

                if (dist < ENEMY_MIN_SPACING)
                {
                    valid_position = FALSE;
                    break;
                }
            }

            if (valid_position)
                break;
        }

        // Record this spawn position
        spawn_positions[spawn_count].x = spawn_x;
        spawn_positions[spawn_count].y = spawn_y;
        spawn_count++;

        // Get base speed for this wave and add random velocity offset: +/- 33% variation
        fix16 base_speed = getEnemySpeedForWave(current_wave);
        s16 velocity_percent = (random() % 67) - 33;  // Range: -33 to +33 percent
        fix16 speed_variation = (base_speed * velocity_percent) / 100;

        if (from_left)
        {
            enemies[i].x = FIX16(spawn_x);  // Start off left edge
            enemies[i].vx = base_speed + speed_variation;  // Move right with variation
        }
        else
        {
            enemies[i].x = FIX16(spawn_x);  // Start off right edge
            enemies[i].vx = -base_speed - speed_variation;  // Move left with variation
        }

        enemies[i].y = FIX16(spawn_y);
        enemies[i].from_left = from_left;
        enemies[i].hp = 2;
        enemies[i].active = TRUE;

        // Create sprite (24x16, so offset by 12 horizontally and 8 vertically)
        s16 sprite_x = (s16)(enemies[i].x >> FIX16_FRAC_BITS) - 12;
        s16 sprite_y = spawn_y - 8;

        enemies[i].sprite = SPR_addSprite(&sprite_plane,
                                           sprite_x,
                                           sprite_y,
                                           TILE_ATTR(PAL2, 0, FALSE, from_left ? FALSE : TRUE));
    }

    enemies_spawned = enemy_count;

    // Spawn large enemies
    u8 large_enemy_count = getLargeEnemyCountForWave(current_wave);

    for (u8 i = 0; i < large_enemy_count; i++)
    {
        // Random side (0 = left, 1 = right)
        u8 from_left = random() % 2;

        // Try to find a valid spawn position (max 10 attempts)
        s16 spawn_y = 0;
        s16 spawn_x = 0;
        u8 valid_position = FALSE;

        for (u8 attempt = 0; attempt < 10; attempt++)
        {
            // Random Y position between 16 and 132-16
            spawn_y = 16 + (random() % 101);

            // Set position based on spawn side
            s16 spawn_offset = 20 + (random() % 40);  // Range: 20 to 59 pixels off-screen

            if (from_left)
                spawn_x = -spawn_offset;
            else
                spawn_x = SCREEN_WIDTH + spawn_offset;

            // Check distance to all previously spawned enemies (regular + large)
            valid_position = TRUE;
            for (u8 j = 0; j < spawn_count; j++)
            {
                s16 dx = abs(spawn_x - spawn_positions[j].x);
                s16 dy = abs(spawn_y - spawn_positions[j].y);
                s16 dist = dx + dy;  // Manhattan distance

                if (dist < ENEMY_MIN_SPACING)
                {
                    valid_position = FALSE;
                    break;
                }
            }

            if (valid_position)
                break;
        }

        // Record this spawn position
        spawn_positions[spawn_count].x = spawn_x;
        spawn_positions[spawn_count].y = spawn_y;
        spawn_count++;

        // Get base speed for this wave and add random velocity offset: +/- 33% variation
        fix16 base_speed = getEnemySpeedForWave(current_wave);
        s16 velocity_percent = (random() % 67) - 33;  // Range: -33 to +33 percent
        fix16 speed_variation = (base_speed * velocity_percent) / 100;

        if (from_left)
        {
            large_enemies[i].x = FIX16(spawn_x);  // Start off left edge
            large_enemies[i].vx = base_speed + speed_variation;  // Move right with variation
        }
        else
        {
            large_enemies[i].x = FIX16(spawn_x);  // Start off right edge
            large_enemies[i].vx = -base_speed - speed_variation;  // Move left with variation
        }

        large_enemies[i].y = FIX16(spawn_y);
        large_enemies[i].from_left = from_left;
        large_enemies[i].hp = 4;  // Large enemies have 4 HP
        large_enemies[i].hurt_timer = 0;  // Not hurt initially
        large_enemies[i].active = TRUE;

        // Create sprite (40x24, so offset by 20 horizontally and 12 vertically)
        s16 sprite_x = (s16)(large_enemies[i].x >> FIX16_FRAC_BITS) - 20;
        s16 sprite_y = spawn_y - 12;

        large_enemies[i].sprite = SPR_addSprite(&sprite_plane_large,
                                                 sprite_x,
                                                 sprite_y,
                                                 TILE_ATTR(PAL2, 0, FALSE, from_left ? FALSE : TRUE));
    }

    large_enemies_spawned = large_enemy_count;
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
                // Update sprite position (24x16 sprite)
                SPR_setPosition(enemies[i].sprite, ex - 12, ey - 8);

                // Only drop bombs when fully on screen (at least 12 pixels from edge)
                u8 on_screen = (ex >= 12 && ex <= SCREEN_WIDTH - 12);

                // Randomly drop bombs (0.2% base chance, scales with wave)
                u16 drop_chance = BOMB_DROP_CHANCE * current_wave;
                if (drop_chance > 300) drop_chance = 300;  // Cap at 30%

                if (on_screen && (random() % 1000) < drop_chance)
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
                                                             TILE_ATTR(PAL2, 0, FALSE, FALSE));
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
        // Base: 15 (single-player) or 7 (two-player)
        // Bonus: +2 or +1 per 10 waves completed
        u16 wave_bonus_multiplier = (current_wave - 1) / 10;  // 0 for waves 1-10, 1 for 11-20, etc.

        if (two_player_mode)
        {
            u16 ammo_reward = 7 + (wave_bonus_multiplier * 1);
            ammo_p1 += ammo_reward;
            ammo_p2 += ammo_reward;
        }
        else
        {
            u16 ammo_reward = 15 + (wave_bonus_multiplier * 2);
            ammo_p1 += ammo_reward;
        }
    }
}

void updateLargeEnemies()
{
    u8 active_count = 0;

    for (u8 i = 0; i < MAX_LARGE_ENEMIES; i++)
    {
        if (large_enemies[i].active)
        {
            active_count++;

            // Move large enemy horizontally
            large_enemies[i].x = large_enemies[i].x + large_enemies[i].vx;

            s16 ex = (s16)(large_enemies[i].x >> FIX16_FRAC_BITS);
            s16 ey = (s16)(large_enemies[i].y >> FIX16_FRAC_BITS);

            // Check if large enemy went off screen
            if ((large_enemies[i].from_left && ex > SCREEN_WIDTH) ||
                (!large_enemies[i].from_left && ex < 0))
            {
                // Large enemy escaped
                large_enemies[i].active = FALSE;
                SPR_releaseSprite(large_enemies[i].sprite);
                large_enemies[i].sprite = NULL;
            }
            else
            {
                // Handle hurt timer and sprite swapping
                if (large_enemies[i].hurt_timer > 0)
                {
                    large_enemies[i].hurt_timer--;

                    // If timer just started, swap to hurt sprite
                    if (large_enemies[i].hurt_timer == LARGE_ENEMY_HURT_DURATION - 1)
                    {
                        // Release old sprite and create hurt sprite
                        SPR_releaseSprite(large_enemies[i].sprite);
                        large_enemies[i].sprite = SPR_addSprite(&sprite_plane_large_hurt,
                                                                 ex - 20,
                                                                 ey - 12,
                                                                 TILE_ATTR(PAL2, 0, FALSE, large_enemies[i].from_left ? FALSE : TRUE));
                    }
                    // If timer expired, swap back to normal sprite
                    else if (large_enemies[i].hurt_timer == 0)
                    {
                        // Release hurt sprite and create normal sprite
                        SPR_releaseSprite(large_enemies[i].sprite);
                        large_enemies[i].sprite = SPR_addSprite(&sprite_plane_large,
                                                                 ex - 20,
                                                                 ey - 12,
                                                                 TILE_ATTR(PAL2, 0, FALSE, large_enemies[i].from_left ? FALSE : TRUE));
                    }
                }

                // Update sprite position (40x24 sprite)
                SPR_setPosition(large_enemies[i].sprite, ex - 20, ey - 12);

                // Only drop bombs when fully on screen (at least 20 pixels from edge for 40px wide sprite)
                u8 on_screen = (ex >= 20 && ex <= SCREEN_WIDTH - 20);

                // Randomly drop bombs (same chance as regular enemies)
                u16 drop_chance = BOMB_DROP_CHANCE * current_wave;
                if (drop_chance > 300) drop_chance = 300;  // Cap at 30%

                if (on_screen && (random() % 1000) < drop_chance)
                {
                    // Find an inactive bomb slot
                    for (u8 j = 0; j < MAX_BOMBS; j++)
                    {
                        if (!bombs[j].active)
                        {
                            bombs[j].x = large_enemies[i].x;
                            bombs[j].y = large_enemies[i].y;
                            bombs[j].vx = FIX16(0);  // No horizontal velocity initially
                            bombs[j].vy = BOMB_INITIAL_VY;
                            bombs[j].active = TRUE;

                            bombs[j].sprite = SPR_addSprite(&sprite_bomb,
                                                             ex - 4,
                                                             ey - 4,
                                                             TILE_ATTR(PAL2, 0, FALSE, FALSE));
                            break;  // Only drop one bomb
                        }
                    }
                }
            }
        }
    }

    // Check if all large enemies are gone
    if (active_count == 0 && large_enemies_spawned > 0)
    {
        large_enemies_spawned = 0;
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

            // Check if bomb reached ground level (CANNON_Y + 5 pixels)
            if (by >= CANNON_Y + 5)
            {
                // Explode at ground level
                spawnExplosion(bx, by);

                // Destroy bomb
                bombs[i].active = FALSE;
                SPR_releaseSprite(bombs[i].sprite);
                bombs[i].sprite = NULL;

                // Apply blast wave (no player attribution since it hit ground)
                applyBlastWave(bx, by, 0);
            }
            // Check if bomb went off screen (left or right)
            else if (bx < -20 || bx > SCREEN_WIDTH + 20)
            {
                bombs[i].active = FALSE;
                SPR_releaseSprite(bombs[i].sprite);
                bombs[i].sprite = NULL;
            }
            // Check if bomb went off screen (bottom)
            else if (by > SCREEN_HEIGHT)
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
// Player parameter indicates who triggered the blast (for scoring)
void applyBlastWave(s16 bx, s16 by, u8 player)
{
    // Apply blast effects to bombs
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
                applyBlastWave(other_bx, other_by, player);
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

    // Apply blast damage to enemies
    for (u8 k = 0; k < MAX_ENEMIES; k++)
    {
        if (enemies[k].active)
        {
            // Get position of this enemy
            s16 enemy_x = (s16)(enemies[k].x >> FIX16_FRAC_BITS);
            s16 enemy_y = (s16)(enemies[k].y >> FIX16_FRAC_BITS);

            // Calculate distance from impact point
            s16 dx = enemy_x - bx;
            s16 dy = enemy_y - by;
            s16 dist = abs(dx) + abs(dy);  // Manhattan distance (faster than sqrt)

            // If within blast radius, deal 1 HP damage
            if (dist < BOMB_BLAST_RADIUS)
            {
                enemies[k].hp -= 1;

                // Check if enemy is defeated
                if (enemies[k].hp <= 0)
                {
                    // Award half points (50) to the player who triggered the blast
                    if (player == 1)
                        score_p1 += 50;
                    else
                        score_p2 += 50;

                    // Destroy enemy
                    enemies[k].active = FALSE;
                    SPR_releaseSprite(enemies[k].sprite);
                    enemies[k].sprite = NULL;
                }
            }
        }
    }

    // Apply blast damage to large enemies
    for (u8 k = 0; k < MAX_LARGE_ENEMIES; k++)
    {
        if (large_enemies[k].active)
        {
            // Get position of this large enemy
            s16 enemy_x = (s16)(large_enemies[k].x >> FIX16_FRAC_BITS);
            s16 enemy_y = (s16)(large_enemies[k].y >> FIX16_FRAC_BITS);

            // Calculate distance from impact point
            s16 dx = enemy_x - bx;
            s16 dy = enemy_y - by;
            s16 dist = abs(dx) + abs(dy);  // Manhattan distance (faster than sqrt)

            // If within blast radius, deal 1 HP damage
            if (dist < BOMB_BLAST_RADIUS)
            {
                large_enemies[k].hp -= 1;

                // Show hurt sprite
                large_enemies[k].hurt_timer = LARGE_ENEMY_HURT_DURATION;

                // Check if large enemy is defeated
                if (large_enemies[k].hp <= 0)
                {
                    // Award half points (100) to the player who triggered the blast
                    if (player == 1)
                        score_p1 += 100;
                    else
                        score_p2 += 100;

                    // Destroy large enemy
                    large_enemies[k].active = FALSE;
                    SPR_releaseSprite(large_enemies[k].sprite);
                    large_enemies[k].sprite = NULL;
                }
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
    // Don't spawn if already active or pending
    if (powerup_truck.active || powerup_truck.spawn_pending) return;

    // Set up delayed spawn with random delay 0-5 seconds (0-300 frames at 60fps)
    powerup_truck.spawn_pending = TRUE;
    powerup_truck.spawn_timer = random() % 301;  // 0 to 300 frames

    // Store spawn direction for later
    powerup_truck.from_left = random() % 2;
}

void updatePowerupTruck()
{
    // Handle spawn delay countdown
    if (powerup_truck.spawn_pending)
    {
        if (powerup_truck.spawn_timer > 0)
        {
            powerup_truck.spawn_timer--;
        }
        else
        {
            // Time to spawn the truck!
            powerup_truck.spawn_pending = FALSE;
            powerup_truck.y = FIX16(TRUCK_Y);

            if (powerup_truck.from_left)
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
                                                  TILE_ATTR(PAL1, 0, FALSE, powerup_truck.from_left ? FALSE : TRUE));
        }
        return;
    }

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

u8 shouldSpawnPolarBear(u16 wave)
{
    // First polar bear on wave 4
    if (wave < 4) return FALSE;

    // Waves 4-20: every 4 waves (4, 8, 12, 16, 20)
    if (wave <= 20)
    {
        return (wave % 4) == 0;
    }

    // After wave 20: every 5 waves (25, 30, 35...)
    return (wave % 5) == 0;
}

void spawnPolarBear()
{
    // Don't spawn if already active or pending
    if (polar_bear.active || polar_bear.spawn_pending) return;

    // Set up delayed spawn with random delay 0-5 seconds (0-300 frames at 60fps)
    polar_bear.spawn_pending = TRUE;
    polar_bear.spawn_timer = random() % 301;  // 0 to 300 frames

    // Store spawn direction for later
    polar_bear.from_left = random() % 2;

    // Reset click count for this appearance
    polar_bear.click_count = 0;
}

void updatePolarBear()
{
    // Handle spawn delay countdown
    if (polar_bear.spawn_pending)
    {
        if (polar_bear.spawn_timer > 0)
        {
            polar_bear.spawn_timer--;
        }
        else
        {
            // Time to spawn the polar bear!
            polar_bear.spawn_pending = FALSE;
            polar_bear.y = FIX16(POLAR_BEAR_Y);

            if (polar_bear.from_left)
            {
                polar_bear.x = FIX16(-20);  // Start off left edge
                polar_bear.vx = POLAR_BEAR_SPEED;  // Move right
            }
            else
            {
                polar_bear.x = FIX16(SCREEN_WIDTH + 20);  // Start off right edge
                polar_bear.vx = -POLAR_BEAR_SPEED;  // Move left
            }

            polar_bear.active = TRUE;

            // Create sprite (flip horizontally if coming from right)
            s16 sprite_x = (s16)(polar_bear.x >> FIX16_FRAC_BITS) - 8;
            s16 sprite_y = POLAR_BEAR_Y - 8;

            polar_bear.sprite = SPR_addSprite(&sprite_polarbear,
                                              sprite_x,
                                              sprite_y,
                                              TILE_ATTR(PAL1, 0, FALSE, polar_bear.from_left ? FALSE : TRUE));
        }
        return;
    }

    if (!polar_bear.active) return;

    // Move polar bear
    polar_bear.x = polar_bear.x + polar_bear.vx;

    s16 px = (s16)(polar_bear.x >> FIX16_FRAC_BITS);

    // Check if polar bear went off screen
    if ((polar_bear.from_left && px > SCREEN_WIDTH + 20) ||
        (!polar_bear.from_left && px < -20))
    {
        // Polar bear left screen
        polar_bear.active = FALSE;
        SPR_releaseSprite(polar_bear.sprite);
        polar_bear.sprite = NULL;
    }
    else
    {
        // Update sprite position
        SPR_setPosition(polar_bear.sprite, px - 8, POLAR_BEAR_Y - 8);
    }
}
