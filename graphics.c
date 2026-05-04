#include "graphics.h"
#include <SDL2/SDL_image.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

uint8_t draw_links = 01, draw_dots = 0;
uint8_t display_mode = STATES;
uint32_t prev_matter = 0, prev_energy = 0;
uint32_t total_matter = 0;
uint32_t total_energy = 0;


void Graphics_Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return ;
    }

    window = SDL_CreateWindow("templateSDL", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
    SDL_WINDOW_BORDERLESS);
    if (window == NULL) {
        fprintf(stderr, "Window could not be created! SDL Error: %s\n", SDL_GetError());
        SDL_Quit();
        return ;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return ;
    }
    
    if(draw_links)
    {
        if(CELL_SIZE > 2) draw_links = 1;
    }
    if(draw_dots)
    {
        if(CELL_SIZE > 6) draw_dots = 1;
    }
}

void Graphics_Quit()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Screen_Clear()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void Screen_Draw()
{
    Grid_Draw();
    SDL_RenderPresent(renderer);
}

void Grid_Draw()
{
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = CELL_SIZE;
    rect.h = CELL_SIZE;
    
    SDL_Rect dot;
    dot.x = 0;
    dot.y = 0;
    dot.w = 3;
    dot.h = 3;
    
    Tile *tile;
    
    int16_t x, y;
    
    switch (display_mode)
    {
    case RESOURCES:
        total_matter = 0;
        total_energy = 0;
        
        for(int i = 0; i < grid_height; i++)
        {
            for(int j = 0; j < grid_width; j++)
            {
                tile = Grid_Get(j, i);
                int type = tile->type;
                int matter = tile->matter;
                int energy = tile->energy;
                uint32_t light = 0 * tile->light * 255 / max_light;
                uint8_t *links = tile->links;
                
                int r = 0, g = 0, b = 0;
                
                if(type == 1) total_matter++;
                total_matter += matter;
                total_energy += energy;
                    
                r = (matter + (type == 1)) * 255 / (max_matter + 1);
                g = (type == 1) * 127;
                b = energy;
                
                rect.x = j * CELL_SIZE;
                rect.y = i * CELL_SIZE;
                
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderFillRect(renderer, &rect);
                
                if(draw_links == 0) continue;
                
                
                uint8_t mask;
                uint8_t draw_dot = 0;
                int16_t dx, dy, cx, cy;
                cx = rect.x + CELL_SIZE / 2;
                cy = rect.y + CELL_SIZE / 2;
                for(uint8_t dir = 0; dir < 8; dir++)
                {
                    dx = dir_to_coords[dir][0] * CELL_SIZE / 2;
                    dy = dir_to_coords[dir][1] * CELL_SIZE / 2;
                    mask = (uint8_t)1 << dir;
                    
                    if(links[dir])
                    {
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                        SDL_RenderDrawLineF(renderer, cx, cy, cx + dx, cy + dy);
                        draw_dot = 1;
                    }
                }
                
                if(draw_dot && draw_dots)
                {
                    dot.x = j * CELL_SIZE + CELL_SIZE / 2 - 1;
                    dot.y = i * CELL_SIZE + CELL_SIZE / 2 - 1;
                    SDL_RenderFillRect(renderer, &dot);
                }
            }
        }
        break;
    case STATES:
        for(int i = 0; i < grid_height; i++)
        {
            for(int j = 0; j < grid_width; j++)
            {
                tile = Grid_Get(j, i);
                int state = tile->state;
                int type = tile->type;
                int id = tile->id;
                int Z = cells[id].Z;
                int per = Level(Z);
                int val = Valence(Z);
                int req = Required_Electrons(Z);
                uint8_t *links = tile->links;
                
                int r = 0, g = 0, b = 0;
                
                if(id == 0) continue;
                    
                // printf("%d\n", per);
                    
                r = (val - 1) * 35;
                g = (per - 1) * 255;
                b = (8 - val) * 35;
                
                if(per == 1)
                {
                    r = (val - 1) * 255;
                    b = (2 - val) * 255;
                }
                
                switch (Z)
                {
                case 1:
                    r = 0;
                    g = 0;
                    b = 255;
                    break;
                case 6:
                    r = 255;
                    g = 255;
                    b = 0;
                    break;
                case 7:
                    r = 0;
                    g = 255;
                    b = 255;
                    break;
                case 8:
                    r = 255;
                    g = 0;
                    b = 0;
                    break;
                
                default:
                    break;
                }
                
                rect.x = j * CELL_SIZE;
                rect.y = i * CELL_SIZE;
                
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderFillRect(renderer, &rect);
                
                if(draw_links == 0) continue;
                
                uint8_t mask;
                uint8_t draw_dot = 0;
                int16_t dx, dy, cx, cy;
                cx = rect.x + CELL_SIZE / 2;
                cy = rect.y + CELL_SIZE / 2;
                for(uint8_t dir = 0; dir < 8; dir++)
                {
                    dx = dir_to_coords[dir][0] * CELL_SIZE / 2;
                    dy = dir_to_coords[dir][1] * CELL_SIZE / 2;
                    
                    if(links[dir])
                    {
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                        SDL_RenderDrawLineF(renderer, cx, cy, cx + dx, cy + dy);
                        draw_dot = 1;
                    }
                }
                
                if(draw_dot && draw_dots)
                {
                    dot.x = j * CELL_SIZE + CELL_SIZE / 2 - 1;
                    dot.y = i * CELL_SIZE + CELL_SIZE / 2 - 1;
                    SDL_RenderFillRect(renderer, &dot);
                }
            }
        }
        break;
    case PHERO:
        for(int i = 0; i < grid_height; i++)
        {
            for(int j = 0; j < grid_width; j++)
            {
                tile = Grid_Get(j, i);
                
                int c_0 = 0, c_1 = 0, c_2 = 0;
                int r = 0, g = 0, b = 0;
                c_0 = Phero_Get(j, i, 0, 0);
                
                c_1 = Phero_Get(j, i, 1, 0);
                int count = 0;
                for(int p = 2; p < MAX_PHEROMONES; p++)
                {
                    count++;
                    c_2 += Phero_Get(j, i, p, 0);
                }
                
                if(c_2 != 0) 
                {
                    c_2 = c_2 / count;
                }
                
                r = c_0;
                g = c_1;
                b = c_2;
                
                rect.x = j * CELL_SIZE;
                rect.y = i * CELL_SIZE;
                
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderFillRect(renderer, &rect);
            }
        }
        break;
    case LIGHT:
        for(int i = 0; i < grid_height; i++)
        {
            for(int j = 0; j < grid_width; j++)
            {
                tile = Grid_Get(j, i);
                
                int r = 0, g = 0, b = 0;
                
                int light = 0;
                if(max_light > 0)
                    light = tile->light * 255 / max_light;
                
                r = light, g = light;
                
                rect.x = j * CELL_SIZE;
                rect.y = i * CELL_SIZE;
                
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderFillRect(renderer, &rect);
            }
        }
        break;
    default:
        break;
    }
    
}

