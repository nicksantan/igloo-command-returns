#include "weapons.h"
#include "player.h"
#include "resources.h"

u8 active_missile_count = 0;

void initWeapons()
{
    // Initialize missile pool
    for (u8 i = 0; i < MAX_MISSILES; i++)
    {
        missiles[i].active = FALSE;
        missiles[i].sprite = NULL;
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
            s32 vx_scaled = ((s32)vx_normalized * (s32)MISSILE_SPEED) >> 8;
            s32 vy_scaled = ((s32)vy_normalized * (s32)MISSILE_SPEED) >> 8;

            // Velocities are already in fix16 format
            missiles[i].vx = vx_scaled;
            missiles[i].vy = vy_scaled;

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
            missiles[i].player = player;

            // Decrement ammo for this player
            if (player == 1)
            {
                ammo_p1--;
            }
            else
            {
                ammo_p2--;
            }

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

            // Apply slight gravity to vertical velocity
            missiles[i].vy = missiles[i].vy + MISSILE_GRAVITY;

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
