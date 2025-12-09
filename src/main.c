#include <genesis.h>
#include "common.h"
#include "player.h"
#include "weapons.h"
#include "enemies.h"
#include "collision.h"
#include "scoring.h"
#include "hud.h"
#include "explosions.h"
#include "resources.h"

// Global game state (definitions)
u8 two_player_mode = FALSE;
u16 current_wave = 1;
u8 enemies_spawned = 0;
u8 large_enemies_spawned = 0;
u8 wave_complete = FALSE;
u8 game_over = FALSE;
u32 score_p1 = 0;
u32 score_p2 = 0;
u16 ammo_p1 = 0;
u16 ammo_p2 = 0;
u8 bonus_igloos_queued = 0;
u32 next_bonus_threshold = 5000;

// Title screen state
u8 title_screen_active = TRUE;
u8 menu_selection = 0;  // 0 = 1 Player, 1 = 2 Players

// Global object pools (definitions)
Missile missiles[MAX_MISSILES];
Enemy enemies[MAX_ENEMIES];
Enemy large_enemies[MAX_LARGE_ENEMIES];
Bomb bombs[MAX_BOMBS];
Igloo igloos[NUM_IGLOOS];
PowerupTruck powerup_truck;
PolarBear polar_bear;
Explosion explosions[MAX_EXPLOSIONS];

void drawTitleScreen()
{
    // Draw title
    VDP_drawText("     SNOWBALL DEFENSE     ", 7, 8);
    VDP_drawText("                          ", 7, 9);

    // Draw menu options with selection indicator
    if (menu_selection == 0)
        VDP_drawText("      > 1 PLAYER <        ", 7, 12);
    else
        VDP_drawText("        1 PLAYER          ", 7, 12);

    if (menu_selection == 1)
        VDP_drawText("      > 2 PLAYERS <       ", 7, 14);
    else
        VDP_drawText("        2 PLAYERS         ", 7, 14);

    // Draw instructions
    VDP_drawText("   UP/DOWN: Select        ", 7, 18);
    VDP_drawText("   START: Begin Game      ", 7, 19);
}

void handleTitleScreenInput()
{
    u16 joy1 = JOY_readJoypad(JOY_1);
    static u16 prev_joy1 = 0;

    // Check for button press (not held)
    u16 pressed = joy1 & ~prev_joy1;

    // UP - Move selection up
    if (pressed & BUTTON_UP)
    {
        if (menu_selection > 0)
            menu_selection--;
    }

    // DOWN - Move selection down
    if (pressed & BUTTON_DOWN)
    {
        if (menu_selection < 1)
            menu_selection++;
    }

    // START - Confirm selection and start game
    if (pressed & BUTTON_START)
    {
        // Set game mode based on selection
        two_player_mode = (menu_selection == 1) ? TRUE : FALSE;

        // Exit title screen
        title_screen_active = FALSE;

        // Clear screen for game
        VDP_clearPlane(BG_A, TRUE);
        VDP_clearPlane(BG_B, TRUE);
    }

    prev_joy1 = joy1;
}

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

    // Load the large enemy hurt sprite's palette into PAL2 (has more colors than explosion)
    PAL_setPalette(PAL2, sprite_plane_large_hurt.palette->data, CPU);

    // Initialize sprite engine
    SPR_init();

    // Initialize subsystems
    initPlayer();
    initWeapons();
    initEnemies();
    initExplosions();

    // Start background music (loop infinitely)
    XGM_setLoopNumber(-1);
  //  XGM_startPlay(bgm_music);

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

    // Initialize ammunition
    if (two_player_mode)
    {
        ammo_p1 = 25;  // Two-player mode: each player starts with 25
        ammo_p2 = 25;
    }
    else
    {
        ammo_p1 = 50;  // Single-player mode: start with 50
        ammo_p2 = 0;   // Player 2 not used in single-player
    }
}

int main()
{
    // Initialize VDP early for title screen
    VDP_setScreenWidth320();
    VDP_clearPlane(BG_A, TRUE);
    VDP_clearPlane(BG_B, TRUE);
    PAL_setColor(0, RGB24_TO_VDPCOLOR(0x001040));

    // Main game loop
    while(1)
    {
        if (title_screen_active)
        {
            // Title screen mode
            handleTitleScreenInput();
            drawTitleScreen();
        }
        else
        {
            // Game has started - initialize if this is first frame after title screen
            static u8 game_initialized = FALSE;
            if (!game_initialized)
            {
                initGame();
                spawnWave();
                game_initialized = TRUE;
            }

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

                // Update large enemies
                updateLargeEnemies();

                // Update bombs
                updateBombs();

                // Update powerup truck
                updatePowerupTruck();

                // Update polar bear
                updatePolarBear();

                // Update explosions
                updateExplosions();

                // Check collisions
                checkCollisions();

                // If wave is complete, spawn next wave after a brief delay
                if (wave_complete && enemies_spawned == 0 && large_enemies_spawned == 0)
                {
                    // Restore one bonus igloo if available and needed (at most one per wave)
                    restoreBonusIgloo();

                    // Check for game over AFTER attempting to restore bonus igloo
                    checkGameOver();

                    spawnWave();

                    // Check if we should spawn a truck on this wave
                    if (shouldSpawnTruck(current_wave))
                    {
                        spawnPowerupTruck();
                    }

                    // Check if we should spawn a polar bear on this wave
                    if (shouldSpawnPolarBear(current_wave))
                    {
                        spawnPolarBear();
                    }
                }

                // Display HUD
                drawHUD();

                // Update all sprites (sends sprite data to VDP)
                SPR_update();
            }
            else
            {
                // Game over - display message and final scores
                drawGameOver();
            }
        }

        // Wait for VBlank (60 FPS sync)
        SYS_doVBlankProcess();
    }

    return 0;
}
