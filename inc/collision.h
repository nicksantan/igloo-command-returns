#ifndef COLLISION_H
#define COLLISION_H

#include "common.h"

// Functions
void checkCollisions();
void checkPolarBearClick(s16 crosshair_x, s16 crosshair_y, u8 player);
void checkPowerupTruckClick(s16 crosshair_x, s16 crosshair_y, u8 player);

#endif // COLLISION_H
