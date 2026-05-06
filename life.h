#ifndef LIFE_H
#define LIFE_H

#include "grid.h"

extern int16_t temp_control;

uint32_t Find_Free_Id();

void Cells_Init();
void Cell_Create(int16_t x, int16_t y, uint32_t parent, uint8_t Z);
void Cell_Destroy(uint32_t id);
void Cells_Update();
void Cell_Update_Movement(uint32_t id);
void Cell_Update_Collision(uint32_t id);

uint8_t Valence(uint8_t e);
uint8_t Level(uint8_t e);
uint8_t Required_Electrons(uint8_t e);
uint32_t slater_sigma(uint8_t e);
uint32_t Energy(uint32_t id1, uint32_t id2, uint8_t links);

#endif