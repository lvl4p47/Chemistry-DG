#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "life.h"

#define WINDOW_WIDTH 1920 // for debugging
#define WINDOW_HEIGHT 720
#define CELL_SIZE 5

typedef enum
{
    RESOURCES,
    STATES,
    PHERO,
    LIGHT,
    DISP_MODE_COUNT
} disp_modes;

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern uint8_t display_mode;
extern uint32_t total_matter;
extern uint32_t total_energy;
extern uint8_t draw_links, draw_dots;

void Graphics_Init();
void Graphics_Quit();
void Screen_Clear();
void Screen_Draw();
void Grid_Draw();

#endif