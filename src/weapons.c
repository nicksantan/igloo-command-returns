#include "weapons.h"
#include "player.h"
#include "resources.h"
#include "explosions.h"
#include "scoring.h"

u8 active_missile_count = 0;

void initWeapons()
{
    // Initialize missile pool
    for (u8 i = 0; i < MAX_MISSILES; i++)
    {
        missiles[i].active = FALSE;
        missiles[i].sprite = NULL;
        missiles[i].type = MISSILE_TYPE_NORMAL;
    }
}

// Helper function to fire a single missile with optional angle rotation
// angle_cos and angle_sin are fix16 values for rotating the velocity vector
// missile_type: MISSILE_TYPE_NORMAL or MISSILE_TYPE_FAST
static void fireSingleMissileWithAngle(u8 player, s16 cannon_x, s16 crosshair_x, s16 crosshair_y,
                                        fix16 angle_cos, fix16 angle_sin, u8 missile_type)
{
    // Find an inactive missile slot
    for (u8 i = 0; i < MAX_MISSILES; i++)
    {
        if (!missiles[i].active)
        {
            s16 cannon_y = CANNON_Y;

            // Set missile start position (at cannon)
            missiles[i].x = FIX16(cannon_x);
            missiles[i].y = FIX16(cannon_y);

            // Set target (crosshair position)
            missiles[i].target_x = FIX16(crosshair_x);
            missiles[i].target_y = FIX16(crosshair_y);

            // Calculate velocity with proper normalization using integer math
            s32 dx = crosshair_x - cannon_x;
            s32 dy = crosshair_y - cannon_y;

            // Calculate distance squared
            s32 dist_sq = dx * dx + dy * dy;

            // Safety check
            if (dist_sq == 0)
            {
                return;
            }

            // Integer square root using bit manipulation
            s32 dist;
            if (dist_sq <= 1)
            {
                dist = dist_sq;
            }
            else
            {
                // Find the position of the highest bit in dist_sq
                s32 bit = 1 << 30;  // Start with the second-highest bit (avoid overflow)
                s32 num = dist_sq;  // Work with a copy
                while (bit > num)
                {
                    bit >>= 2;
                }

                // Build the result bit by bit
                dist = 0;
                while (bit != 0)
                {
                    if (num >= dist + bit)
                    {
                        num -= dist + bit;
                        dist = (dist >> 1) + bit;
                    }
                    else
                    {
                        dist >>= 1;
                    }
                    bit >>= 2;
                }
            }

            // Calculate velocity: normalize direction, then scale by speed
            // Avoid overflow by doing (dx / dist) first, preserving precision with shifts
            // Scale dx by 256 for precision, divide by dist, then multiply by MISSILE_SPEED and adjust
            s32 vx_normalized = ((s32)dx << 8) / dist;  // dx/dist scaled by 256
            s32 vy_normalized = ((s32)dy << 8) / dist;  // dy/dist scaled by 256

            // Now multiply by MISSILE_SPEED (fix16) and divide by 256
            // Result: (direction/distance) * MISSILE_SPEED in fix16 format
            // For fast shots, use double speed
            fix16 speed = (missile_type == MISSILE_TYPE_FAST) ? (MISSILE_SPEED * 2) : MISSILE_SPEED;
            s32 vx_scaled = ((s32)vx_normalized * (s32)speed) >> 8;
            s32 vy_scaled = ((s32)vy_normalized * (s32)speed) >> 8;

            // Apply angle rotation if not identity (cos=1, sin=0)
            // Rotation formula: new_vx = vx*cos - vy*sin, new_vy = vx*sin + vy*cos
            if (angle_cos != FIX16(1) || angle_sin != FIX16(0))
            {
                fix16 vx_orig = vx_scaled;
                fix16 vy_orig = vy_scaled;

                // Manual fixed-point multiplication (a * b) >> 16
                vx_scaled = (((s32)vx_orig * (s32)angle_cos) >> FIX16_FRAC_BITS) - (((s32)vy_orig * (s32)angle_sin) >> FIX16_FRAC_BITS);
                vy_scaled = (((s32)vx_orig * (s32)angle_sin) >> FIX16_FRAC_BITS) + (((s32)vy_orig * (s32)angle_cos) >> FIX16_FRAC_BITS);
            }

            // Velocities are in fix16 format
            missiles[i].vx = vx_scaled;
            missiles[i].vy = vy_scaled;

            // Create sprite for this missile
            s16 sprite_x = (s16)(missiles[i].x >> FIX16_FRAC_BITS) - 4;
            s16 sprite_y = (s16)(missiles[i].y >> FIX16_FRAC_BITS) - 4;

            // Use appropriate sprite based on missile type
            // For now, use snowball for both (fastshot sprite to be added)
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
            missiles[i].player = player;
            missiles[i].type = missile_type;

            // Don't decrement ammo here - let fireMissile handle it
            return;
        }
    }
}

void fireMissile(u8 player)
{
    // player: 1 = Player 1 (left cannon), 2 = Player 2 (right cannon)

    // Check if player has ammo
    if (player == 1 && ammo_p1 == 0)
    {
        return;  // No ammo for player 1
    }
    if (player == 2 && ammo_p2 == 0)
    {
        return;  // No ammo for player 2
    }

    s16 crosshair_x, crosshair_y, cannon_x;

    if (player == 1)
    {
        crosshair_x = crosshair1_x;
        crosshair_y = crosshair1_y;

        // In 1-player mode, use closest cannon
        // In 2-player mode, always use left cannon for P1
        if (two_player_mode)
        {
            cannon_x = CANNON_LEFT_X;
        }
        else
        {
            // Determine which cannon is closer to crosshair
            s16 dist_left = abs(crosshair_x - CANNON_LEFT_X);
            s16 dist_right = abs(crosshair_x - CANNON_RIGHT_X);
            cannon_x = (dist_left <= dist_right) ? CANNON_LEFT_X : CANNON_RIGHT_X;
        }
    }
    else
    {
        crosshair_x = crosshair2_x;
        crosshair_y = crosshair2_y;
        cannon_x = CANNON_RIGHT_X;  // Player 2 always uses right cannon
    }

    // Don't fire if crosshair is below the cannon line
    // (allows clicking on powerups/trucks below cannons)
    if (crosshair_y >= CANNON_Y)
    {
        return;
    }

    // Check if triple shot is active for this player
    u8 triple_shot_active = (player == 1) ? triple_shot_active_p1 : triple_shot_active_p2;

    // Check if fast shot is active for this player
    u8 fast_shot_active = (player == 1) ? fast_shot_active_p1 : fast_shot_active_p2;

    // Determine missile type
    u8 missile_type = fast_shot_active ? MISSILE_TYPE_FAST : MISSILE_TYPE_NORMAL;

    if (triple_shot_active)
    {
        // Fire three missiles with offset targets to create spread
        // Calculate horizontal offset based on distance to target
        s16 dy_to_target = crosshair_y - CANNON_Y;
        s16 offset = 40;  // 40 pixels horizontal spread at crosshair

        // Adjust offset based on distance (closer = less spread, farther = more spread)
        if (dy_to_target < -100)
        {
            offset = 60;  // More spread for distant targets
        }
        else if (dy_to_target > -50)
        {
            offset = 20;  // Less spread for close targets
        }

        // Left shot
        fireSingleMissileWithAngle(player, cannon_x, crosshair_x - offset, crosshair_y,
                                    FIX16(1), FIX16(0), missile_type);
        // Center shot
        fireSingleMissileWithAngle(player, cannon_x, crosshair_x, crosshair_y,
                                    FIX16(1), FIX16(0), missile_type);
        // Right shot
        fireSingleMissileWithAngle(player, cannon_x, crosshair_x + offset, crosshair_y,
                                    FIX16(1), FIX16(0), missile_type);
    }
    else
    {
        // Fire single missile
        fireSingleMissileWithAngle(player, cannon_x, crosshair_x, crosshair_y,
                                    FIX16(1), FIX16(0), missile_type);
    }

    // Decrement ammo once (regardless of triple shot)
    if (player == 1)
    {
        ammo_p1--;
    }
    else
    {
        ammo_p2--;
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

            // Apply slight gravity to vertical velocity (only for normal missiles)
            if (missiles[i].type == MISSILE_TYPE_NORMAL)
            {
                missiles[i].vy = missiles[i].vy + MISSILE_GRAVITY;
            }

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

void updatePowerups()
{
    // Update triple shot timer for player 1
    if (triple_shot_active_p1)
    {
        triple_shot_timer_p1--;
        if (triple_shot_timer_p1 == 0)
        {
            triple_shot_active_p1 = FALSE;
        }
    }

    // Update triple shot timer for player 2
    if (triple_shot_active_p2)
    {
        triple_shot_timer_p2--;
        if (triple_shot_timer_p2 == 0)
        {
            triple_shot_active_p2 = FALSE;
        }
    }

    // Update fast shot timer for player 1
    if (fast_shot_active_p1)
    {
        fast_shot_timer_p1--;
        if (fast_shot_timer_p1 == 0)
        {
            fast_shot_active_p1 = FALSE;
        }
    }

    // Update fast shot timer for player 2
    if (fast_shot_active_p2)
    {
        fast_shot_timer_p2--;
        if (fast_shot_timer_p2 == 0)
        {
            fast_shot_active_p2 = FALSE;
        }
    }
}

void triggerMegabomb(u8 player)
{
    // Check if we have megabombs available
    if (megabombs == 0)
    {
        return;  // No megabombs left
    }

    // Decrement megabomb count
    megabombs--;

    u16 points_awarded = 0;

    // Destroy all active enemies
    for (u8 i = 0; i < MAX_ENEMIES; i++)
    {
        if (enemies[i].active)
        {
            s16 ex = (s16)(enemies[i].x >> FIX16_FRAC_BITS);
            s16 ey = (s16)(enemies[i].y >> FIX16_FRAC_BITS);

            // Spawn explosion at enemy position
            spawnExplosion(ex, ey);

            // Destroy enemy
            enemies[i].active = FALSE;
            SPR_releaseSprite(enemies[i].sprite);
            enemies[i].sprite = NULL;

            // Award points (100 per enemy)
            points_awarded += 100;
        }
    }

    // Destroy all active large enemies
    for (u8 i = 0; i < MAX_LARGE_ENEMIES; i++)
    {
        if (large_enemies[i].active)
        {
            s16 ex = (s16)(large_enemies[i].x >> FIX16_FRAC_BITS);
            s16 ey = (s16)(large_enemies[i].y >> FIX16_FRAC_BITS);

            // Spawn explosion at large enemy position
            spawnExplosion(ex, ey);

            // Destroy large enemy
            large_enemies[i].active = FALSE;
            SPR_releaseSprite(large_enemies[i].sprite);
            large_enemies[i].sprite = NULL;

            // Award points (200 per large enemy)
            points_awarded += 200;
        }
    }

    // Destroy all active bombs
    for (u8 i = 0; i < MAX_BOMBS; i++)
    {
        if (bombs[i].active)
        {
            s16 bx = (s16)(bombs[i].x >> FIX16_FRAC_BITS);
            s16 by = (s16)(bombs[i].y >> FIX16_FRAC_BITS);

            // Spawn explosion at bomb position
            spawnExplosion(bx, by);

            // Destroy bomb
            bombs[i].active = FALSE;
            SPR_releaseSprite(bombs[i].sprite);
            bombs[i].sprite = NULL;

            // Award points (10 per bomb)
            points_awarded += 10;
        }
    }

    // Award total points to the player who used the megabomb
    if (player == 1)
        score_p1 += points_awarded;
    else
        score_p2 += points_awarded;

    // Check for bonus igloo earned from the points
    checkBonusIgloo();
}
