#ifndef PLAYER_H
#define PLAYER_H

#include "common.h"

// Player crosshair positions
extern s16 crosshair1_x;
extern s16 crosshair1_y;
extern s16 crosshair2_x;
extern s16 crosshair2_y;

// Crosshair sprites
extern Sprite* crosshair1_sprite;
extern Sprite* crosshair2_sprite;

// Cannon sprites
extern Sprite* cannon_left_sprite;
extern Sprite* cannon_right_sprite;

// Previous button states (for edge detection)
extern u16 prev_joy1;
extern u16 prev_joy2;

// Functions
void initPlayer();
void updateCrosshair();
void handleInput();

#endif // PLAYER_H
