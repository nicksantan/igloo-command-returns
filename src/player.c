#include "player.h"
#include "weapons.h"
#include "collision.h"
#include "resources.h"

// Player crosshair positions
s16 crosshair1_x = SCREEN_WIDTH / 2;
s16 crosshair1_y = SCREEN_HEIGHT / 2;
s16 crosshair2_x = SCREEN_WIDTH / 2;
s16 crosshair2_y = SCREEN_HEIGHT / 2;

// Sprites
Sprite* crosshair1_sprite = NULL;
Sprite* crosshair2_sprite = NULL;
Sprite* cannon_left_sprite = NULL;
Sprite* cannon_right_sprite = NULL;

// Previous button states (for edge detection)
u16 prev_joy1 = 0;
u16 prev_joy2 = 0;

void initPlayer()
{
    // Load crosshair sprites (16x16)
    crosshair1_sprite = SPR_addSprite(&sprite_crosshair,
                                       crosshair1_x - 8,
                                       crosshair1_y - 8,
                                       TILE_ATTR(PAL1, 0, FALSE, FALSE));

    if (two_player_mode)
    {
        crosshair2_sprite = SPR_addSprite(&sprite_crosshair,
                                           crosshair2_x - 8,
                                           crosshair2_y - 8,
                                           TILE_ATTR(PAL1, 0, FALSE, FALSE));
    }

    // Load cannon sprites (16x16 each)
    cannon_left_sprite = SPR_addSprite(&sprite_cannon,
                                        CANNON_LEFT_X - 8,
                                        CANNON_Y - 8,
                                        TILE_ATTR(PAL1, 0, FALSE, FALSE));

    cannon_right_sprite = SPR_addSprite(&sprite_cannon,
                                         CANNON_RIGHT_X - 8,
                                         CANNON_Y - 8,
                                         TILE_ATTR(PAL1, 0, FALSE, FALSE));
}

void updateCrosshair()
{
    // Player 1 input (JOY_1)
    u16 joy1 = JOY_readJoypad(JOY_1);

    // Determine speed based on B button
    s16 speed1 = (joy1 & BUTTON_B) ? CROSSHAIR_SPEED_BOOST : CROSSHAIR_SPEED_NORMAL;

    // Handle D-pad input for Player 1
    if (joy1 & BUTTON_UP)
        crosshair1_y -= speed1;
    if (joy1 & BUTTON_DOWN)
        crosshair1_y += speed1;
    if (joy1 & BUTTON_LEFT)
        crosshair1_x -= speed1;
    if (joy1 & BUTTON_RIGHT)
        crosshair1_x += speed1;

    // Keep Player 1 crosshair within screen bounds (16x16 sprite)
    if (crosshair1_x < 16)
        crosshair1_x = 16;
    if (crosshair1_x > SCREEN_WIDTH - 16)
        crosshair1_x = SCREEN_WIDTH - 16;
    if (crosshair1_y < 32)
        crosshair1_y = 32;
    if (crosshair1_y > SCREEN_HEIGHT - 16)
        crosshair1_y = SCREEN_HEIGHT - 16;

    // Update Player 1 sprite position
    SPR_setPosition(crosshair1_sprite, crosshair1_x - 8, crosshair1_y - 8);

    // Player 2 input (JOY_2) - only in 2-player mode
    if (two_player_mode)
    {
        u16 joy2 = JOY_readJoypad(JOY_2);

        // Determine speed based on B button
        s16 speed2 = (joy2 & BUTTON_B) ? CROSSHAIR_SPEED_BOOST : CROSSHAIR_SPEED_NORMAL;

        // Handle D-pad input for Player 2
        if (joy2 & BUTTON_UP)
            crosshair2_y -= speed2;
        if (joy2 & BUTTON_DOWN)
            crosshair2_y += speed2;
        if (joy2 & BUTTON_LEFT)
            crosshair2_x -= speed2;
        if (joy2 & BUTTON_RIGHT)
            crosshair2_x += speed2;

        // Keep Player 2 crosshair within screen bounds (16x16 sprite)
        if (crosshair2_x < 16)
            crosshair2_x = 16;
        if (crosshair2_x > SCREEN_WIDTH - 16)
            crosshair2_x = SCREEN_WIDTH - 16;
        if (crosshair2_y < 32)
            crosshair2_y = 32;
        if (crosshair2_y > SCREEN_HEIGHT - 16)
            crosshair2_y = SCREEN_HEIGHT - 16;

        // Update Player 2 sprite position
        SPR_setPosition(crosshair2_sprite, crosshair2_x - 8, crosshair2_y - 8);
    }
}

void handleInput()
{
    // Player 1 input
    u16 joy1 = JOY_readJoypad(JOY_1);

    // Check for A button press (edge detection - only fire once per press)
    if ((joy1 & BUTTON_A) && !(prev_joy1 & BUTTON_A))
    {
        // Check if clicking on polar bear first
        checkPolarBearClick(crosshair1_x, crosshair1_y, 1);
        // Then fire missile
        fireMissile(1);  // Player 1 fires from left cannon
    }

    prev_joy1 = joy1;

    // Player 2 input (only in 2-player mode)
    if (two_player_mode)
    {
        u16 joy2 = JOY_readJoypad(JOY_2);

        // Check for A button press (edge detection - only fire once per press)
        if ((joy2 & BUTTON_A) && !(prev_joy2 & BUTTON_A))
        {
            // Check if clicking on polar bear first
            checkPolarBearClick(crosshair2_x, crosshair2_y, 2);
            // Then fire missile
            fireMissile(2);  // Player 2 fires from right cannon
        }

        prev_joy2 = joy2;
    }
}
