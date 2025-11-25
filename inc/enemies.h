#ifndef ENEMIES_H
#define ENEMIES_H

#include "common.h"

// Functions
void initEnemies();
void spawnWave();
void updateEnemies();
void updateBombs();
u8 shouldSpawnTruck(u16 wave);
void spawnPowerupTruck();
void updatePowerupTruck();
void applyBlastWave(s16 bx, s16 by);

#endif // ENEMIES_H
