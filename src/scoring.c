#include "scoring.h"
#include "resources.h"

void checkBonusIgloo()
{
    // In two-player mode, use the higher score of the two players
    u32 check_score = two_player_mode ? (score_p1 > score_p2 ? score_p1 : score_p2) : score_p1;

    // Check if we've crossed the threshold
    if (check_score >= next_bonus_threshold)
    {
        bonus_igloos_queued++;

        // Calculate next threshold based on current threshold
        if (next_bonus_threshold < 20000)
        {
            // Up to 20000: every 5000 points (5000, 10000, 15000, 20000)
            next_bonus_threshold += 5000;
        }
        else if (next_bonus_threshold < 50000)
        {
            // 20000 to 50000: every 7500 points (27500, 35000, 42500, 50000)
            next_bonus_threshold += 7500;
        }
        else
        {
            // After 50000: every 10000 points (60000, 70000, ...)
            next_bonus_threshold += 10000;
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

void restoreBonusIgloo()
{
    // Restore one bonus igloo if available and needed (at most one per wave)
    if (bonus_igloos_queued > 0)
    {
        // Check if any igloos are missing
        for (u8 i = 0; i < NUM_IGLOOS; i++)
        {
            if (!igloos[i].alive)
            {
                // Restore this igloo
                igloos[i].alive = TRUE;
                igloos[i].sprite = SPR_addSprite(&sprite_igloo,
                                                  igloos[i].x - 8,
                                                  igloos[i].y - 8,
                                                  TILE_ATTR(PAL1, 0, FALSE, FALSE));
                bonus_igloos_queued--;
                break;  // Only restore one igloo per wave
            }
        }
    }
}
