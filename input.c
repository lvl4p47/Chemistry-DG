#include "input.h"

uint8_t quit = 0;
uint8_t slow_mode = 0;
int lmb_held = 0;
int rmb_held = 0;
int mmb_held = 0;
SDL_Event e;

int grab_x, grab_y, dest_x, dest_y, str;

void Events_Handle()
{
    
    while (SDL_PollEvent(&e) != 0) 
    {
        
        if (e.type == SDL_QUIT) 
        {
            quit = 1;
        }
        
        if (e.type == SDL_KEYDOWN) 
        {
            if(e.key.keysym.sym == SDLK_q)
            {
                display_mode = mod(display_mode - 1, DISP_MODE_COUNT);
                last_frame = 0;
            }
            if(e.key.keysym.sym == SDLK_w)
            {
                display_mode = mod(display_mode + 1, DISP_MODE_COUNT);
                last_frame = 0;
            }
            if(e.key.keysym.sym == SDLK_r)
            {
                Grid_Reset(0, 1000);
            }
            if(e.key.keysym.sym == SDLK_t)
            {
                
            }
            if(e.key.keysym.sym == SDLK_p)
            {
                
            }
            if(e.key.keysym.sym == SDLK_s)
            {
                slow_mode = 1 - slow_mode;
            }
            if(e.key.keysym.sym == SDLK_d)
            {
                draw_dots = 1 - draw_dots;
            }
            if(e.key.keysym.sym == SDLK_l)
            {
                draw_links = 1 - draw_links;
            }
            if(e.key.keysym.sym == SDLK_z)
            {
                if(temp_control < 0)
                    temp_control *= 2;
                else
                    temp_control = -1;
            }
            if(e.key.keysym.sym == SDLK_x)
            {
                temp_control = 0;
            }
            if(e.key.keysym.sym == SDLK_c)
            {
                if(temp_control > 0)
                    temp_control *= 2;
                else
                    temp_control = 1;
            }
            if(e.key.keysym.sym == SDLK_ESCAPE)
            {
                // SDL_MinimizeWindow(window);
                quit = 1;
            }
            if(e.key.keysym.sym == SDLK_SPACE)
            {
                pause = 1 - pause;
            }
        }
        
        if (e.type == SDL_MOUSEBUTTONDOWN) 
        {
            int mouseX = e.button.x;
            int mouseY = e.button.y;
            
            int x = mouseX / CELL_SIZE;
            int y = mouseY / CELL_SIZE;
            
            if (e.button.button == SDL_BUTTON_RIGHT) 
            {
                rmb_held = 1;
                
            }
            if (e.button.button == SDL_BUTTON_LEFT) 
            {
                lmb_held = 1;
                
                grab_x = x;
                grab_y = y;
                dest_x = x;
                dest_y = y;
                
                // Grid_Signal(x, y, 2);
                
            }
            if (e.button.button == SDL_BUTTON_MIDDLE) 
            {
                mmb_held = 1;
                
                cells[Grid_Get(x, y)->id].energy = 60000;
                
                // Rec_Connect(x, y, 100);
                // if(Grid_Get(x, y)->id == 0)
                    
                // else
                //     Cell_Destroy(Grid_Get(x, y)->id);
            }
        }
        if (e.type == SDL_MOUSEBUTTONUP) {
            if (e.button.button == SDL_BUTTON_LEFT) 
            {
                lmb_held = 0;
            }
            if (e.button.button == SDL_BUTTON_RIGHT) 
            {
                rmb_held = 0;
            }
            if (e.button.button == SDL_BUTTON_MIDDLE) 
            {
                mmb_held = 0;
            }
        }
        if (e.type == SDL_MOUSEMOTION) {
            int mouseX = e.button.x;
            int mouseY = e.button.y;
            
            int x = mouseX / CELL_SIZE;
            int y = mouseY / CELL_SIZE;

            if (rmb_held == 1)
            {
                if(Grid_Get(x, y)->type == 0)
                {
                    Grid_Set(x, y, 1);
                    switch (rnd() % 4)
                    {
                    case 0:
                        Cell_Create(x, y, 0, 1);
                        break;
                    case 1:
                        Cell_Create(x, y, 0, 6);
                        break;
                    case 2:
                        Cell_Create(x, y, 0, 7);
                        break;
                    case 3:
                        Cell_Create(x, y, 0, 8);
                        break;
                    
                    default:
                        break;
                    }
                }
            }
            if (mmb_held == 1)
            {
                
            }
            if (lmb_held == 1)
            {
                dest_x = x;
                dest_y = y;
            }
        }
    }
}

void Events_Process()
{
    if(lmb_held == 0) return;
    
    int dx = (dest_x - grab_x);
    int dy = (dest_y - grab_y);
    
    uint8_t ax = abs(dx);
    uint8_t ay = abs(dy);
    int8_t sx = sign(dx);
    int8_t sy = sign(dy);
    uint8_t w_diag = min(ax, ay);
    uint8_t w_total = max(ax, ay);
    uint8_t w_axis = w_total - w_diag;
    
    if(w_total != 0)
    {
        uint8_t r = rand() % w_total;
        if (r >= w_diag)
        {
            if(ax > ay) 
            {
                sy = 0;
            }
            else 
            {
                sx = 0;
            }
        }
    }
    
    int str = ax * ax + ay * ay;
    
    if(Grid_Get(grab_x, grab_y)->type != 0 && str > 0)
    {
        // Rec_Push(grab_x, grab_y, sx, sy, str, 0);
        Rec_Push_Flexible(grab_x, grab_y, sx, sy, str);
    }
}
