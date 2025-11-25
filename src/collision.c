#include "collision.h"
#include "enemies.h"
#include "scoring.h"

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
                        // Award points to the player who fired the missile
                        if (missiles[i].player == 1)
                            score_p1 += 100;
                        else
                            score_p2 += 100;

                        // Check for bonus igloo earned
                        checkBonusIgloo();

                        // Hit! Destroy both
                        missiles[i].active = FALSE;
                        SPR_releaseSprite(missiles[i].sprite);
                        missiles[i].sprite = NULL;

                        enemies[j].active = FALSE;
                        SPR_releaseSprite(enemies[j].sprite);
                        enemies[j].sprite = NULL;

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
                        // Award points to the player who fired the missile
                        if (missiles[i].player == 1)
                            score_p1 += 10;
                        else
                            score_p2 += 10;

                        // Check for bonus igloo earned
                        checkBonusIgloo();

                        // Destroy missile
                        missiles[i].active = FALSE;
                        SPR_releaseSprite(missiles[i].sprite);
                        missiles[i].sprite = NULL;

                        // Destroy bomb
                        bombs[j].active = FALSE;
                        SPR_releaseSprite(bombs[j].sprite);
                        bombs[j].sprite = NULL;

                        // Apply blast wave (can trigger chain reactions)
                        applyBlastWave(bx, by);

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

void checkPolarBearClick(s16 crosshair_x, s16 crosshair_y, u8 player)
{
    if (!polar_bear.active) return;

    s16 px = (s16)(polar_bear.x >> FIX16_FRAC_BITS);
    s16 py = (s16)(polar_bear.y >> FIX16_FRAC_BITS);

    // Check if crosshair is over polar bear (24x16 sprite)
    if (abs(crosshair_x - px) < 12 && abs(crosshair_y - py) < 12)
    {
        // Clicked on polar bear!
        polar_bear.click_count++;

        // Calculate score based on click count
        // 200, 300, 400, 500, etc.
        u16 score_award = POLAR_BEAR_BASE_SCORE + (polar_bear.click_count - 1) * 100;

        // Award points to the player who clicked
        if (player == 1)
            score_p1 += score_award;
        else
            score_p2 += score_award;

        // Boost speed on each of the first 3 clicks
        // Click 1: 0.6 -> 1.2 (base + boost)
        // Click 2: 1.2 -> 1.8 (base + 2*boost)
        // Click 3: 1.8 -> 2.4 (base + 3*boost)
        // Click 4+: stays at 2.4 (no more boosts)
        if (polar_bear.click_count <= POLAR_BEAR_MAX_CLICKS)
        {
            if (polar_bear.from_left)
            {
                polar_bear.vx = polar_bear.vx + POLAR_BEAR_SPEED_BOOST;
            }
            else
            {
                polar_bear.vx = polar_bear.vx - POLAR_BEAR_SPEED_BOOST;
            }
        }
    }
}
