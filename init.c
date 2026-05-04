#include "init.h"

void All_Init()
{
    Grid_Init(WINDOW_WIDTH / CELL_SIZE, WINDOW_HEIGHT / CELL_SIZE);
    Cells_Init();
    Graphics_Init();
}

void All_Quit()
{
    Graphics_Quit();
    Grid_Quit();
}
