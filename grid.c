#include "grid.h"

Tile **grid_array = NULL;
uint16_t grid_width = 0;
uint16_t grid_height = 0;

uint8_t max_matter = 8;

uint8_t timer = 0;
uint32_t long_timer = 0;
uint8_t debug = 0;

uint8_t global_time = 0;
uint8_t phero_life = 1;
uint8_t border = 1;

uint8_t gravity = 0;
uint16_t grav_period = 1;
uint32_t grav_rate;
uint32_t max_strength;

uint8_t lighting = 0;
uint16_t lighting_period = 10;
uint16_t max_light_strength;
uint16_t direction;

uint16_t max_light = 65535;
uint16_t sun_height = 255;
uint16_t sun_light = 0;
uint32_t day_length = 10000;

uint8_t max_states = 4;
uint8_t max_links = 3;

void Grid_Init(uint16_t w, uint16_t h)
{
    grid_width = w;
    grid_height = h;
    
    grid_array = (Tile**)malloc(h * sizeof(Tile*));
    
    for(int i = 0; i < h; i++)
    {
        grid_array[i] = (Tile*)malloc(w * sizeof(Tile));
    }
    
    grav_rate = grid_width * grid_height / 100;
    max_strength = min(grid_height * grid_width, 100);
    
    Grid_Reset(0, 1000);
}

void Grid_Quit()
{
    for(int i = 0; i < grid_height; i++)
    {
        free(grid_array[i]);
    }
    
    free(grid_array);
}

void Grid_Reset(uint8_t type, uint16_t chance)
{
    Tile *tile;
    int16_t energy_delta;
    if(debug) fprintf(stderr, "\nGrid_Reset"), fflush(stderr);
    for(int i = 0; i < grid_height; i++)
    {
        for(int j = 0; j < grid_width; j++)
        {
            if(rnd() % 1000 < chance)
            {   
                Grid_Set(j, i, type);
            }
        }
    }
    Border();
}

void Border()
{
    if(border)
    {
        for(int i = 0; i < grid_height; i++)
        {
            Grid_Set(0, i, 2);
            Grid_Set(grid_width - 1, i, 2);
        }
        for(int j = 0; j < grid_width; j++)
        {
            Grid_Set(j, 0, 2);
            Grid_Set(j, grid_height - 1, 2);
        }
    }
}

void Grid_Set(int16_t x, int16_t y, uint8_t type)
{
    if(debug) fprintf(stderr, "\nGrid_Set"), fflush(stderr);
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    
    grid_array[y1][x1].type = type;
    grid_array[y1][x1].id = 0;
}

void Grid_Move(int16_t x, int16_t y, int16_t dx, int16_t dy)
{
    if(debug) fprintf(stderr, "\nGrid_Set"), fflush(stderr);
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    uint16_t x2 = mod(x + dx, grid_width);
    uint16_t y2 = mod(y + dy, grid_height);
    
    if(grid_array[y2][x2].type != 0) return;
    
    uint32_t id = grid_array[y1][x1].id;
    
    particles[id].x = x2;
    particles[id].y = y2;
    
    grid_array[y2][x2].type = grid_array[y1][x1].type;
    grid_array[y2][x2].id = grid_array[y1][x1].id;
    
    for(int i = 0; i < 8; i++)
    {
        particles[grid_array[y2][x2].id].links[i] = particles[grid_array[y1][x1].id].links[i];
        particles[grid_array[y2][x2].id].buf_links[i] = 0;
    }
    
    grid_array[y1][x1].type = 0;
    grid_array[y1][x1].id = 0;
    
    for(int i = 0; i < 8; i++)
    {
        particles[grid_array[y1][x1].id].links[i] = 0;
        particles[grid_array[y1][x1].id].buf_links[i] = 0;
    }
    
    particles[grid_array[y1][x1].id].rec_str = 0;
    particles[grid_array[y2][x2].id].rec_str = 0;
    particles[grid_array[y1][x1].id].on_edge = 0;
    particles[grid_array[y2][x2].id].on_edge = 0;
    
    if(x1 == grab_x && y1 == grab_y && lmb_held)
    {
        grab_x = x2;
        grab_y = y2;
    }
}

void Grid_Maintain()
{
    if(debug) 
    {
        freopen("debug.log", "w", stderr);
        fprintf(stderr, "\nGrid_Maintain"), fflush(stderr);
    }
    Tile *tile;
    
    for(int i = 0; i < grid_height; i++)
    {
        for(int j = 0; j < grid_width; j++)
        {
            tile = Grid_Get(j, i);
            for(int type = 0; type < MAX_PHEROMONES; type++)
            {
                Phero_Get(j, i, type, 1);
            }
        }
    }
}

int32_t Rec_Can_Move(int16_t x, int16_t y, int8_t dx, int8_t dy, int32_t strength, uint8_t rigid, uint8_t linked)
{
    uint8_t local_debug = 0;
    
    if(local_debug) printf("\n");
    
    Tile *center = Grid_Get(x, y);
    Tile *neighbor;
    
    Particle *center_part = &particles[center->id];
    Particle *neighbor_part;
    
    int32_t problems = 0;
    int32_t ret;
    
    if(local_debug) printf("start x %d y %d str %d c %d\n", x, y, strength, center_part->links);
    
    if(strength == 0 || center->type == 0) 
    {
        if(local_debug) printf("too weak to move str %d\n", strength);
        if(center_part->rec_str == 0 && center->type != 0)
        {
            
            center_part->rec_str = -1; // this tile was not reached
            if(local_debug) printf("add solvable problem\n");
            return 1; // too weak to move, add solvable problem
        }
        else return 0; // can't reach now, but it was reached by some other branch
    }
    
    center_part->on_edge = 2;
    
    if(center_part->rec_str == -1) // reached previously unreachable, str != 0
    {
        if(local_debug) printf("solution\n");
        problems = -1;
        center_part->rec_str = strength;
    }
    
    center_part->rec_str = strength;
    
    neighbor = Grid_Get(x + dx, y + dy);
    neighbor_part = &particles[neighbor->id];
    if(neighbor->type == 1)
    {
        if(neighbor_part->rec_str < strength - 1
        || neighbor_part->rec_str == 0
        )
        {
            ret = Rec_Can_Move(x + dx, y + dy, dx, dy, strength - 1, rigid, linked);
            problems += ret;
            if(ret < 0)
            {
                if(local_debug) printf("more solutions than problems in front str %d\n", strength);
            }
            if(ret > 0)
            {
                if(local_debug) printf("My neighbor in the direction can't move str%d\n", strength);
            }
            
        }
    }
    else if(neighbor->type != 0)
    {
        problems += 1; // add unsolvable problem
        if(local_debug) printf("add unsolvable problem\n");
    }
    
    int16_t Dx, Dy;
    int16_t nx, ny;
    uint8_t mask, opposite;
    uint8_t is_energy_out, is_energy_out_n;
    uint8_t push_dir = coords_to_dir[1 + dy][1 + dx];
    for(uint8_t dir = 0; dir < 8; dir++)
    {
        mask = (uint8_t)1 << dir;
        opposite = (uint8_t)1 << mod(dir + 4, 8);
        Dx = dir_to_coords[dir][0];
        Dy = dir_to_coords[dir][1];
        nx = x + Dx;
        ny = y + Dy;
        neighbor = Grid_Get(nx, ny);
        neighbor_part = &particles[neighbor->id];
        
        if(neighbor->type == 1
        && (neighbor_part->rec_str < strength - 1
        || neighbor_part->rec_str == 0
        )
        )
        {
            if(center_part->links[dir]
             || (linked == 0
             && (
             dir == mod(push_dir + 4, 8)
             )
             )
             )
            {
                ret = Rec_Can_Move(nx, ny, dx, dy, strength - 1, rigid, linked);
                problems += ret;
                if(local_debug) printf("str %d problems %d\n", strength, problems);
                
                if(rigid == 0)
                {
                    if(ret > 0 && strength == 1// && problems - ret <= 0
                    )
                    {
                        if(
                        max(abs(nx - x - dx), abs(ny - y - dy)) < 2
                        )
                        {
                            if(neighbor_part->on_edge == 0)
                                neighbor_part->on_edge = 1;
                            neighbor_part->rec_str = 0;
                            if(local_debug) printf("no tearing dir %d\n", dir);
                            problems -= ret;
                        }
                        else
                        {
                            if(local_debug) printf("tearing\n");
                        }
                    }
                }
                if(local_debug) printf("str %d problems %d\n", strength, problems);
                if(ret < 0)
                {
                    if(local_debug) printf("more solutions than problems from side str %d\n", strength);
                }
                if(ret > 0) 
                {
                    if(local_debug) printf("My x %d y %d neighbor x %d y %d to the side can't move str %d\n", x, y, nx, ny, strength);
                }
            }
        }
    }
    
    if(local_debug) printf("finish str %d problems %d c %d edge %d\n", strength, problems, center_part->links, center_part->on_edge);
    
    return problems;
}

void Rec_Move(int16_t x, int16_t y, int8_t dx, int8_t dy, uint32_t *moved)
{
    uint8_t local_debug = 0;
    x = mod(x, grid_width);
    y = mod(y, grid_height);
    Tile *center = Grid_Get(x, y);
    Tile *neighbor, *linked;
    Particle *center_part = &particles[center->id];
    Particle *neighbor_part;
    
    int32_t problems = 0;
    int32_t str = center_part->rec_str;
    if(str < -1) str = -1 - str;
    
    if(str <= 0) 
    {
        if(center_part->rec_str == -1)
            center_part->rec_str = 0;
        
        return;
    }
    // printf("2\n");
    // if(local_debug) printf("x %d y %d str %d on_edge %d\n", x, y, str, center->on_edge);
    
    neighbor = Grid_Get(x + dx, y + dy);
    neighbor_part = &particles[neighbor->id];
    if(neighbor->type == 1)
    {
        if(neighbor_part->rec_str != 0 && neighbor_part->on_edge != 1)
        {
            // if(local_debug) printf("x %d y %d str %d front reaching x %d y %d str %d moved %d\n", x, y, str, x + dx, y + dy, neighbor->rec_str, *moved);
            
            Rec_Move(x + dx, y + dy, dx, dy, moved);
        }
        
    }
    else if(neighbor->type == 1)
        problems = 1;
    
    if(neighbor->type == 0)
    {
        int16_t Dx, Dy;
        int16_t nx, ny;
        uint8_t mask, opposite;
        uint8_t temp_links, n_temp_links;
        uint8_t is_energy_out, is_energy_out_n;
        uint8_t is_matter_out, is_matter_out_n;
        for(uint8_t dir = 0; dir < 8; dir++)
        {
            mask = (uint8_t)1 << dir;
            opposite = (uint8_t)1 << mod(dir + 4, 8);
            Dx = dir_to_coords[dir][0];
            Dy = dir_to_coords[dir][1];
            nx = x + Dx;
            ny = y + Dy;
            neighbor = Grid_Get(nx, ny);
            neighbor_part = &particles[neighbor->id];
            if(
            (center_part->links[dir] || center_part->buf_links[dir])
            && neighbor->type == 1
            && neighbor_part->on_edge == 1
            && max(abs(nx - x - dx), abs(ny - y - dy)) < 2
            )
            {
                if(local_debug) printf("x %d y %d str %d updating links with x %d y %d str %d on edge %d\n", x, y, str, x + Dx, y + Dy, neighbor_part->rec_str, neighbor_part->on_edge);
                
                temp_links = center_part->links[dir];
                n_temp_links = neighbor_part->links[mod(dir + 4, 8)];
                
                center_part->links[dir] = 0; 
                neighbor_part->links[mod(dir + 4, 8)] = 0;
                
                uint8_t new_dir = coords_to_dir[Dy - dy + 1][Dx - dx + 1];
                
                mask = (uint8_t)1 << new_dir;
                opposite = (uint8_t)1 << mod(new_dir + 4, 8);
                
                center_part->buf_links[new_dir] = temp_links;
                neighbor_part->buf_links[mod(new_dir + 4, 8)] = n_temp_links;
            }
        }
        
        for(int i = 0; i < 8; i++)
        {
            center_part->links[i] += center_part->buf_links[i];
            center_part->buf_links[i] = 0;
        }
        
        Grid_Move(x, y, dx, dy);
        *moved = *moved + 1;
        center_part->on_edge = 0;
        if(local_debug) printf("x %d y %d str %d moved %d\n", x, y, str, *moved);
        center = Grid_Get(x + dx, y + dy);
    }
    else
    {
        if(local_debug) printf("x %d y %d can't move forward\n", x, y);
        for(int ny = y - 1; ny <= y + 1; ny++)
        {
            for(int nx = x - 1; nx <= x + 1; nx++)
            {
                neighbor = Grid_Get(nx, ny);
                neighbor_part = &particles[neighbor->id];
                
                if(Active_Neighbors(nx, ny) == 0 && neighbor_part->on_edge == 1)
                {
                    if(local_debug) printf("x %d y %d str %d resetting 2 x %d y %d str %d\n", x, y, str, nx, ny, neighbor_part->rec_str);
                    neighbor_part->on_edge = 0;
                    
                    for(int i = 0; i < 8; i++)
                    {
                        neighbor_part->links[i] += neighbor_part->buf_links[i];
                        neighbor_part->buf_links[i] = 0;
                    }
                }
            }
        }
    
        return;
    }
    
    for(int ny = y - 1; ny <= y + 1; ny++)
    {
        for(int nx = x - 1; nx <= x + 1; nx++)
        {
            neighbor = Grid_Get(nx, ny);
            neighbor_part = &particles[neighbor->id];
            
            if(neighbor->type == 1
            && (neighbor_part->rec_str >= str - 1
            || neighbor_part->rec_str <= 0)
            )
            {
                // if(local_debug) if(neighbor->rec_str > str) printf("%d > %d ", neighbor->rec_str, str - 1);
                if(local_debug) printf("x %d y %d str %d side reaching x %d y %d str %d\n", x, y, str, nx, ny, neighbor_part->rec_str);
                Rec_Move(nx, ny, dx, dy, moved);
            }
        }
    }
    
    for(int ny = y - 1; ny <= y + 1; ny++)
    {
        for(int nx = x - 1; nx <= x + 1; nx++)
        {
            neighbor = Grid_Get(nx, ny);
            neighbor_part = &particles[neighbor->id];
            
            if(Active_Neighbors(nx, ny) == 0 && neighbor_part->on_edge == 1)
            {
                if(local_debug) printf("x %d y %d str %d resetting 3 x %d y %d str %d\n", x, y, str, nx, ny, neighbor_part->rec_str);
                neighbor_part->on_edge = 0;
                
                for(int i = 0; i < 8; i++)
                {
                    neighbor_part->links[i] += neighbor_part->buf_links[i];
                    neighbor_part->buf_links[i] = 0;
                }
            }
        }
    }
    
    return;
}

uint8_t Active_Neighbors(int16_t x, int16_t y)
{
    Tile *neighbor;
    Particle *neighbor_part;
    if(Grid_Get(x, y)->type != 1) return 1;
    for(int ny = y - 1; ny <= y + 1; ny++)
    {
        for(int nx = x - 1; nx <= x + 1; nx++)
        {
            if(nx != x || ny != y)
            {
                neighbor = Grid_Get(nx, ny);
                neighbor_part = &particles[neighbor->id];
                if(neighbor_part->rec_str != 0)
                    return 1;
            }
        }
    }
    return 0;
}

void Rec_Clean(int16_t x, int16_t y, int8_t dx, int8_t dy, int32_t depth)
{
    uint8_t local_debug = 0;
    Tile *center = Grid_Get(x, y);
    Tile *neighbor;
    Particle *center_part = &particles[center->id];
    Particle *neighbor_part;
    
    int32_t problems = 0;
    int32_t str = center_part->rec_str;
    // center->will_move = 0;
    if(depth < -1)
    {
        // if(center->rec_str != 0)
        //     center->will_move = 2;
        return;
    }
    
    if(local_debug) printf("x %d y %d str %d\n", x, y, str);
    
    neighbor = Grid_Get(x + dx, y + dy);
    neighbor_part = &particles[neighbor->id];
    
    if(neighbor->type == 1)
    {
        if(neighbor_part->rec_str == depth - 1
        || neighbor_part->on_edge == 1)
        {
            if(local_debug) printf("x %d y %d str %d front reaching x %d y %d str %d\n", x, y, str, x + dx, y + dy, neighbor_part->rec_str);
            Rec_Clean(x + dx, y + dy, dx, dy, depth - 1);
        }
    }
    if(center_part->rec_str == depth
    || center_part->rec_str < 0)
    {
        center_part->rec_str = 0;
        center_part->on_edge = 0;
    }
    
    for(int ny = y - 1; ny <= y + 1; ny++)
    {
        for(int nx = x - 1; nx <= x + 1; nx++)
        {
            neighbor = Grid_Get(nx, ny);
            neighbor_part = &particles[neighbor->id];
            
            if(neighbor->type == 1
            && (neighbor_part->rec_str == depth - 1
            || neighbor_part->on_edge == 1)
            )
            {
                if(local_debug) if(neighbor_part->rec_str > str) printf("%d > %d ", neighbor_part->rec_str, str - 1);
                if(local_debug) printf("x %d y %d str %d side reaching x %d y %d str %d\n", x, y, str, nx, ny, neighbor_part->rec_str);
                Rec_Clean(nx, ny, dx, dy, depth - 1);
            }
        }
    }
    return;
}

uint32_t Rec_Push(int16_t x, int16_t y, int8_t dx, int8_t dy, int32_t strength, uint8_t rigid)
{
    int32_t cur_str = strength;
    int32_t ret;
    uint32_t moved = 0;
    
    do
    {
        
        ret = Rec_Can_Move(x, y, dx, dy, cur_str, rigid, 1);
        
        if(ret <= 0) 
        {
            Rec_Move(x, y, dx, dy, &moved);
            cur_str = -1;
            
        }
        else
        {
            Rec_Clean(x, y, dx, dy, cur_str);
            cur_str--; 
            
        }
        
    }
    while(cur_str > 0 && rigid == 0);
    return moved;
}

uint32_t Rec_Push_Away(int16_t x, int16_t y, int8_t dx, int8_t dy, int32_t strength, uint8_t rigid)
{
    int32_t cur_str = strength;
    int32_t ret;
    uint32_t moved = 0;
    while(cur_str > 0)
    {
        ret = Rec_Can_Move(x, y, -dx, -dy, cur_str, 1, 0);
        Rec_Clean(x, y, -dx, -dy, cur_str);
        if(ret > 0) 
        {
            ret = Rec_Can_Move(x, y, dx, dy, cur_str, rigid, 1);
            if(ret <= 0) 
            {
                Rec_Move(x, y, dx, dy, &moved);
                cur_str = -1;
            }
            else
            {
                Rec_Clean(x, y, dx, dy, cur_str);
                cur_str -= 1; 
            }
        }
        cur_str -= 1;
        
    }
    return moved;
}

uint32_t Rec_Push_Attempt(int16_t x, int16_t y, int8_t dx, int8_t dy, int32_t strength, uint8_t rigid)
{
    uint32_t ret = Rec_Push(x, y, dx, dy, strength, rigid);
    if(ret <= 0) return 0;
    if(rnd() % ret != 0)
    {
        Rec_Push(x + dx, y + dy, -dx, -dy, strength, rigid);
        return 0;
    }
    return ret;
}

uint32_t Rec_Push_Flexible(int16_t x, int16_t y, int8_t dx, int8_t dy, int32_t strength)
{
    int32_t cur_str = 1;
    int32_t ret;
    uint32_t moved = 0;
    
    do
    {
        
        ret = Rec_Can_Move(x, y, dx, dy, cur_str, 0, 1);
        
        if(ret <= 0) 
        {
            Rec_Push_Attempt(x, y, dx, dy, cur_str, 0);
            cur_str = strength + 1;
        }
        else
        {
            Rec_Clean(x, y, dx, dy, cur_str);
            cur_str++;
        }
        
    }
    while(cur_str <= strength);
    return moved;
}

int32_t Rec_Push_CoM(int16_t x, int16_t y, int8_t dx, int8_t dy, int32_t strength)
{
    uint32_t forward = Rec_Push_Away(x, y, dx, dy, strength, 0);
    int32_t attempts = 0;
    uint32_t back = 0;
    
    if(forward > 0)
    {
        while(attempts > 0)
        {
            back = Rec_Push_Attempt(x + dx, y + dy, -dx, -dy, max_strength, 1);
            if(back > 0) break;
            attempts--;
        }
    }
    else
        back = Rec_Push_Away(x, y, -dx, -dy, strength, 1);
    
    if( (forward > 0) && !(back > 0) ) return 1;
    if( !(forward > 0) && (back > 0) ) return -1;
    
    return 0;
}

void Rec_Link_All(int16_t x, int16_t y, int32_t strength)
{
    Tile *center = Grid_Get(x, y);
    Tile *neighbor;
    Particle *center_part = &particles[center->id];
    Particle *neighbor_part;
    
    uint8_t just_remove = 0;
    
    if(strength < -1 || (center->type == 0)) 
    {
        return;
    }
    
    center_part->rec_str = strength;
    
    for(int ny = y - 1; ny <= y + 1; ny++)
    {
        for(int nx = x - 1; nx <= x + 1; nx++)
        {
            neighbor = Grid_Get(nx, ny);
            neighbor_part = &particles[neighbor->id];
            
            if(neighbor->type == 1
            && neighbor_part->rec_str < strength - 1
            || just_remove)
            {
                Rec_Link_All(nx, ny, strength - 1);
            }
        }
    }
    
    uint8_t mask, opposite;
    int16_t dx, dy;
    
    for(uint8_t dir = 1; dir < 8; dir += 2)
    {
        mask = (uint8_t)1 << dir;
        opposite = (uint8_t)1 << mod(dir + 4, 8);
        dx = dir_to_coords[dir][0];
        dy = dir_to_coords[dir][1];
        neighbor = Grid_Get(x + dx, y + dy);
        neighbor_part = &particles[neighbor->id];
        
        if(just_remove)
        {
            continue;
        }
        
        if(Count_Bits_8(center_part->links) < max_links
        && Count_Bits_8(neighbor_part->links) < max_links)
        {
            if(neighbor->type == 1)
            {
                center_part->links[dir] += 1;
                neighbor_part->links[mod(dir + 4, 8)] += 1;
            }
        }
    }
    
    for(uint8_t dir = 0; dir < 8; dir += 2)
    {
        mask = (uint8_t)1 << dir;
        opposite = (uint8_t)1 << mod(dir + 4, 8);
        dx = dir_to_coords[dir][0];
        dy = dir_to_coords[dir][1];
        neighbor = Grid_Get(x + dx, y + dy);
        neighbor_part = &particles[neighbor->id];
        
        if(just_remove)
        {
            continue;
        }
        
        if(Count_Bits_8(center_part->links) < max_links
        && Count_Bits_8(neighbor_part->links) < max_links)
        {
            if(neighbor->type == 1)
            {
                center_part->links[dir] += 1;
                neighbor_part->links[mod(dir + 4, 8)] += 1;
            }
        }
    }
}

void Link_Two(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    Tile *center = Grid_Get(x1, y1);
    Tile *neighbor = Grid_Get(x2, y2);
    Particle *center_part = &particles[center->id];
    Particle *neighbor_part = &particles[neighbor->id];

    uint8_t mask, opposite;
    int16_t dx = x2 - x1;
    int16_t dy = y2 - y1;
    
    if(max(abs(dx), abs(dy)) > 1) return;
    
    uint8_t dir = coords_to_dir[dy + 1][dx + 1];
    
    mask = (uint8_t)1 << dir;
    opposite = (uint8_t)1 << mod(dir + 4, 8);
    
    if(neighbor->type == 1 && center->type == 1)
    {
        center_part->links[dir] += 1;
        neighbor_part->links[mod(dir + 4, 8)] += 1;
    }
}

void Unlink_Two(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    Tile *center = Grid_Get(x1, y1);
    Tile *neighbor = Grid_Get(x2, y2);
    Particle *center_part = &particles[center->id];
    Particle *neighbor_part = &particles[neighbor->id];

    int16_t dx = x2 - x1;
    int16_t dy = y2 - y1;
    
    if(max(abs(dx), abs(dy)) > 1) return;
    
    uint8_t dir = coords_to_dir[dy + 1][dx + 1];
    
    if(neighbor->type == 1 && center->type == 1
    && center_part->links[dir] > 0 && neighbor_part->links[mod(dir + 4, 8)] > 0)
    {
        center_part->links[dir] -= 1;
        neighbor_part->links[mod(dir + 4, 8)] -= 1;
    }
}

void Rec_Connect(int16_t x, int16_t y, int32_t strength)
{
    Rec_Link_All(x, y, strength);
    Rec_Clean(x, y, 1, 0, strength);
}

uint8_t Is_Membrane(int16_t x, int16_t y)
{
    uint8_t counter = 0;
    
    int16_t dx, dy;
    Tile *neighbor;
    for(int dir = 0; dir < 8; dir++)
    {
        dx = dir_to_coords[dir][0];
        dy = dir_to_coords[dir][1];
        neighbor = Grid_Get(x + dx, y + dy);
        
        if(neighbor->type == 0) counter++;
    }
    return counter;
}

void Global_Time_Update()
{
    timer++;
    long_timer++;
    if(timer % phero_life == 0)
    {
        global_time++;
        // printf("time %3d\n", global_time);
        
        if(global_time == 0 || global_time == 127)
        {
            Grid_Maintain();
        }
    }
}

void Phero_Set(int16_t x, int16_t y, uint8_t type, uint8_t range)
{
    Tile *tile;
    uint32_t dist;
    uint16_t time;
    uint8_t root = fast_root(range);
    
    for(int dy = -root; dy <= root; dy++)
    {
        for(int dx = -root; dx <= root; dx++)
        {
            tile = Grid_Get(x + dx, y + dy);
            dist = max(abs(dx), abs(dy));
            if(dist > root) continue;
            dist = abs(dx) * abs(dx) + abs(dy) * abs(dy);
            if(dist >= range) continue;
            
            time = global_time + range - dist;
            
            if(
                (range - dist) > Phero_Get(x + dx, y + dy, type, 0)
            )
            {
                tile->pheromone[type][0] = global_time;
                tile->pheromone[type][1] = time;
                tile->pheromone[type][2] = 1;
            }
        }
    }
}

uint8_t Phero_Get(int16_t x, int16_t y, uint8_t type, uint8_t update)
{
    Tile *tile = Grid_Get(x, y);
    
    if(tile->pheromone[type][2] == 0) return 0;
    
    uint8_t start_time = tile->pheromone[type][0];
    uint8_t end_time = tile->pheromone[type][1];
    uint8_t conc = 0;
    
    if(start_time > end_time)
    {
        
        if(end_time >= global_time || start_time <= global_time)
        {
            conc = (end_time - start_time - 256) - global_time + start_time - 256;
            if(update) tile->pheromone[type][0] = global_time;
        }
        
    }
    else
    {
        if(end_time >= global_time && start_time <= global_time)
        {
            conc = (end_time - start_time) - global_time + start_time;
            if(update) tile->pheromone[type][0] = global_time;
        }
    }
    if(conc == 0
    && update
    )
    {
        tile->pheromone[type][0] = 0;
        tile->pheromone[type][1] = 0;
        tile->pheromone[type][2] = 0;
    }
    
    return conc;
}

uint16_t Rec_Find_Light(int16_t x, int16_t y, int32_t strength, uint16_t direction, uint8_t source)
{
    uint8_t local_debug = 0;
    
    Tile *center = Grid_Get(x, y);
    Tile *neighbor;
    Particle *center_part = &particles[center->id];
    Particle *neighbor_part;
    
    if(strength == 0
    ) 
    {
        center_part->rec_str = -1;
        return 0;
    }
    if(x < 0 || y < 0
    || x > grid_width - 1 || y > grid_height - 1
    ) 
    {
        return 0;
    }
    
    if(local_debug) printf("\nx %d y %d field %d str %d start\n", x, y, center_part->rec_str, strength);
    
    if(center->light != 0 && center_part->rec_str <= 0)
    {
        if(center_part->rec_str > -strength - 1)
            center_part->rec_str = -strength - 1;
            
        if(local_debug) printf("x %d y %d field %d str %d found light\n\n", x, y, center_part->rec_str, strength);
        return center->light;
    }
    
    int16_t dx = 0, dy = -1;
    int16_t Dx, Dy;
    int16_t nx, ny;
    uint32_t light_acc = 0;
    uint16_t light_blocking;
    uint8_t unfinished_directions = 0;
    
    uint16_t factors[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint16_t maximum = max_light;
    uint16_t shift = (direction % 256) * maximum / 255;
    
    factors[mod(direction / 256 + 5, 8)] = maximum - shift;
    factors[mod(direction / 256 + 6, 8)] = shift;
    
    
    for(uint8_t dir = 0; dir < 8; dir++)
    {
        if(factors[dir] == 0) continue;
        unfinished_directions++;
    }
    
    center_part->rec_str = max(strength, center_part->rec_str);
    
    for(uint8_t dir = 0; dir < 8; dir++)
    {
        Dx = dir_to_coords[dir][0];
        Dy = dir_to_coords[dir][1];
        nx = x + Dx;
        ny = y + Dy;
        neighbor = Grid_Get(nx, ny);
        neighbor_part = &particles[neighbor->id];
        
        if(factors[dir] == 0) continue;
        
        light_blocking = (neighbor_part->Z + (neighbor->type == 1)) * maximum / (max_matter + 1);
        
        if(neighbor_part->rec_str < strength - 1 && neighbor_part->rec_str > 0
        || neighbor_part->rec_str == 0
        )
            light_acc += min(max(Rec_Find_Light(nx, ny, strength - 1, direction, 0)
            - light_blocking + sun_height, 0), max_light) * factors[dir];
        else
        {
            light_acc += min(max(neighbor->light
            - light_blocking + sun_height, 0), max_light) * factors[dir];
            if(local_debug) printf("x %d y %d field %d str %d taken from x %d y %d\n\n", x, y, center_part->rec_str, strength, nx, ny);
        }
        
        if(neighbor_part->rec_str < 0)
            unfinished_directions--;
    }
    
    light_acc /= maximum;
    
    center->light = light_acc;
    
    if(unfinished_directions == 0)
    {
        center_part->rec_str = -strength - 1;
        if(local_debug) printf("x %d y %d field %d str %d finished\n", x, y, center_part->rec_str, strength);
    }
    else
    {
        if(local_debug) printf("x %d y %d field %d str %d unfinished directions %d\n", x, y, center_part->rec_str, strength, unfinished_directions);
    }
    
    if(local_debug) printf("x %d y %d field %d str %d end\n\n", x, y, center_part->rec_str, strength);
    
    return center->light;
}

void Gravity()
{
    uint8_t local_debug = 0;
    Tile *tile;
    if(gravity && timer % grav_period == 0) 
    {
        if(local_debug) printf("\ngravity\n");
        Tile *upper_left, *upper, *upper_right;
        int32_t ret;
        uint32_t rcm_count = 0;
        uint16_t x, y;
        int8_t sx, sy;
        
        for(uint32_t n = 0; n < grav_rate; n++)
        {
            x = rnd() % grid_width;
            y = rnd() % grid_height;
            
            tile = Grid_Get(x, y);
            if(tile->type == 1)
            {
            
                if(gravity == 1) sx = 0, sy = 1;
                
                if(gravity == 2)
                {
                    int dx = (grid_width / 2 - x);
                    int dy = (grid_height / 2 - y);
                    
                    if(dx == 0 && dy == 0) continue;
                    
                    uint8_t ax = abs(dx);
                    uint8_t ay = abs(dy);
                    sx = sign(dx);
                    sy = sign(dy);
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
                }
            
            
                Rec_Push_Attempt(x, y, sx, sy, max_strength, 1);
            }
        }
    }
}

void Illuminate()
{
    if(lighting && (long_timer % lighting_period == 0 || pause))
    {
        Tile *tile;
        uint16_t new_direction;
        
        new_direction = 2048 * (long_timer % day_length) / day_length;
        
        if(new_direction == direction) return;
        direction = new_direction;
        
        sun_light = max_light;
        
        for(int y = 0; y < grid_height; y++)
        {
            for(int x = 0; x < grid_width; x++)
            {
                tile = Grid_Get(x, y);
                tile->light = 0;
            }
        }
        
        for(int y = 0; y < grid_height; y++)
        {
            tile = Grid_Get(0, y);
            tile->light = max_light;
            
            tile = Grid_Get(grid_width - 1, y);
            tile->light = max_light;
        }
        for(int x = 0; x < grid_width; x++)
        {
            tile = Grid_Get(x, 0);
            tile->light = max_light;
            
            tile = Grid_Get(x, grid_height - 1);
            tile->light = max_light;
        }
        
        max_light_strength = max(grid_width, grid_height);
        
        uint32_t calls = 0;
        
        for(int y = 0; y < grid_height; y++)
        {
            int x = 1;
            tile = Grid_Get(x, y);
            if(tile->light == 0)
            {
                Rec_Find_Light(x, y, max_light_strength, direction, 0);
                calls++;
            }
            
            x = grid_width - 2;
            tile = Grid_Get(x, y);
            if(tile->light == 0)
            {
                Rec_Find_Light(x, y, max_light_strength, direction, 0);
                calls++;
            }
        }
        for(int x = 0; x < grid_width; x++)
        {
            int y = 1;
            tile = Grid_Get(x, y);
            if(tile->light == 0)
            {
                Rec_Find_Light(x, y, max_light_strength, direction, 0);
                calls++;
            }
            
            y = grid_height - 2;
            tile = Grid_Get(x, y);
            if(tile->light == 0)
            {
                Rec_Find_Light(x, y, max_light_strength, direction, 0);
                calls++;
            }
        }
        
        for(int y = 0; y < grid_height; y++)
        {
            for(int x = 0; x < grid_width; x++)
            {
                tile = Grid_Get(x, y);
                particles[tile->id].rec_str = 0;
            }
        }
    }
}