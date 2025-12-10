#include "hud.h"
#include <string.h>

void drawHUD()
{
    char status[40];
    u8 igloos_alive = 0;
    for (u8 i = 0; i < NUM_IGLOOS; i++)
    {
        if (igloos[i].alive) igloos_alive++;
    }

    // Get active sprite count from SGDK
    u16 sprite_count = SPR_getNumActiveSprite();

    if (two_player_mode)
    {
        // 2-player HUD: show both scores and ammo
        sprintf(status, "WAVE:%02d IGLOOS:%d BOMBS:%d", current_wave, igloos_alive, megabombs);
        VDP_drawText(status, 1, 1);
        sprintf(status, "P1:%lu AMMO:%d", score_p1, ammo_p1);
        VDP_drawText(status, 1, 2);
        sprintf(status, "P2:%lu AMMO:%d", score_p2, ammo_p2);
        VDP_drawText(status, 1, 3);
        sprintf(status, "SPRITES:%d/80", sprite_count);
        VDP_drawText(status, 1, 4);
    }
    else
    {
        // 1-player HUD: show score and ammo
        sprintf(status, "WAVE:%02d IGLOOS:%d BOMBS:%d", current_wave, igloos_alive, megabombs);
        VDP_drawText(status, 1, 1);
        sprintf(status, "SCORE:%lu AMMO:%d", score_p1, ammo_p1);
        VDP_drawText(status, 1, 2);
        sprintf(status, "SPRITES:%d/80", sprite_count);
        VDP_drawText(status, 1, 3);
    }
}

void drawGameOver()
{
    if (two_player_mode)
    {
        char p1_score[40], p2_score[40];
        VDP_drawText("    GAME OVER!    ", 11, 13);
        VDP_drawText("  All Igloos Lost ", 11, 14);
        sprintf(p1_score, " Player 1: %lu ", score_p1);
        sprintf(p2_score, " Player 2: %lu ", score_p2);
        VDP_drawText(p1_score, 11, 15);
        VDP_drawText(p2_score, 11, 16);
    }
    else
    {
        char final_score[40];
        VDP_drawText("    GAME OVER!    ", 11, 14);
        VDP_drawText("  All Igloos Lost ", 11, 15);
        sprintf(final_score, " Final Score: %lu ", score_p1);
        VDP_drawText(final_score, 11, 16);
    }
}
