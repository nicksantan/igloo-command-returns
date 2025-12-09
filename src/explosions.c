#include "explosions.h"
#include "resources.h"

void initExplosions()
{
    // Initialize explosion pool
    for (u8 i = 0; i < MAX_EXPLOSIONS; i++)
    {
        explosions[i].active = FALSE;
        explosions[i].sprite = NULL;
    }
}

void updateExplosions()
{
    for (u8 i = 0; i < MAX_EXPLOSIONS; i++)
    {
        if (explosions[i].active)
        {
            explosions[i].timer--;

            if (explosions[i].timer == 0)
            {
                // Time's up - remove the explosion
                explosions[i].active = FALSE;
                SPR_releaseSprite(explosions[i].sprite);
                explosions[i].sprite = NULL;
            }
            // No position update needed for static explosions
        }
    }
}

void spawnExplosion(s16 x, s16 y)
{
    // Find an inactive explosion slot
    for (u8 i = 0; i < MAX_EXPLOSIONS; i++)
    {
        if (!explosions[i].active)
        {
            explosions[i].x = x;
            explosions[i].y = y;
            explosions[i].active = TRUE;
            explosions[i].timer = EXPLOSION_DURATION;

            explosions[i].sprite = SPR_addSprite(&sprite_explosion,
                                                  x - 8,  // Center the 16x16 sprite
                                                  y - 8,
                                                  TILE_ATTR(PAL2, 0, FALSE, FALSE));
            break;  // Only spawn one
        }
    }
}
