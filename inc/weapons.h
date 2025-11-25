#ifndef WEAPONS_H
#define WEAPONS_H

#include "common.h"

extern u8 active_missile_count;

// Functions
void initWeapons();
void fireMissile(u8 player);
void updateMissiles();

#endif // WEAPONS_H
