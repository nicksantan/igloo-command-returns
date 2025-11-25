#include <genesis.h>
#include "common.h"
#include "player.h"
#include "weapons.h"
#include "enemies.h"
#include "collision.h"
#include "scoring.h"
#include "hud.h"
#include "resources.h"

// Global game state (definitions)
u8 two_player_mode = TRUE;
u16 current_wave = 1;
u8 enemies_spawned = 0;
u8 wave_complete = FALSE;
u8 game_over = FALSE;
u32 score_p1 = 0;
u32 score_p2 = 0;
u16 ammo_p1 = 0;
u16 ammo_p2 = 0;
u8 bonus_igloos_queued = 0;
u32 next_bonus_threshold = 5000;

// Global object pools (definitions)
Missile missiles[MAX_MISSILES];
Enemy enemies[MAX_ENEMIES];
Bomb bombs[MAX_BOMBS];
Igloo igloos[NUM_IGLOOS];
PowerupTruck powerup_truck;

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

    // Initialize subsystems
    initPlayer();
    initWeapons();
    initEnemies();

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

            // If wave is complete, spawn next wave after a brief delay
            if (wave_complete && enemies_spawned == 0)
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
            }

            // Display HUD
            drawHUD();
        }
        else
        {
            // Game over - display message and final scores
            drawGameOver();
        }

        // Update all sprites (sends sprite data to VDP)
        SPR_update();

        // Wait for VBlank (60 FPS sync)
        SYS_doVBlankProcess();
    }

    return 0;
}
