#ifndef INPUT_H
#define INPUT_H

#include "graphics.h"

extern uint8_t quit;
extern uint8_t slow_mode;

void Events_Handle();
void Events_Process();

#endif