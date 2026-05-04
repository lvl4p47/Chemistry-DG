#include "life.h"

Cell cells[MAX_CELLS];

uint32_t next_id, free_id;

uint32_t mutation_rarity = 1000000;

int16_t temp_control = 0;
uint8_t temp_gradient = 10;

uint32_t kinetic_energy, amount_of_active;

uint32_t Find_Free_Id()
{
    uint32_t counter = 0;
    while((cells[free_id].used != 0 || free_id == 0)
    && counter < MAX_CELLS)
    {
        free_id = mod(free_id + 1, MAX_CELLS);
        counter++;
    }
    
    if(cells[free_id].used == 0) return free_id;
    else 
    {
        printf("no free cells\n");
        return 0;
    }
}

void Cells_Init()
{
    for(uint32_t id = 0; id < MAX_CELLS; id++)
    {
        cells[id].x = 0;
        cells[id].y = 0;
        cells[id].prev = 0;
        cells[id].next = 0;
        cells[id].used = 0;
        
        cells[id].Z = 0;
        cells[id].e = 0;
        cells[id].energy = 0;
        cells[id].shared = 0;
        cells[id].dir = 8;
    }
    
    free_id = 1;
    
    // for(int z = 1; z < 37; z++)
    // {
    //     printf("z %d exs %d req %d\n", z, Valence(z), Required_Electrons(z));
    // }
    
    // printf("%d\n", slater_sigma(8));
    // printf("%d\n", fast_root(4250 * 8));
}

void Cell_Create(int16_t x, int16_t y, uint32_t parent, uint8_t Z)
{
    if(Grid_Get(x, y)->type != 1 || Grid_Get(x, y)->id != 0) return;
    
    uint32_t id = Find_Free_Id();

    if(id == 0) return;
    
    cells[id].x = mod(x, grid_width);
    cells[id].y = mod(y, grid_height);
    cells[id].prev = cells[parent].prev;
    cells[id].next = parent;
    cells[id].used = 1;
    
    cells[id].Z = Z;
    cells[id].e = Z;
    cells[id].energy = 0;
    cells[id].shared = 0;
    cells[id].dir = 8;
    
    cells[cells[id].prev].next = id;
    cells[parent].prev = id;
    
    Grid_Get(x, y)->id = id;
}

void Cell_Destroy(uint32_t id)
{
    if(id == 0) return;
    
    cells[cells[id].prev].next = cells[id].next;
    cells[cells[id].next].prev = cells[id].prev;
    
    cells[id].used = 0;
    
    Grid_Get(cells[id].x, cells[id].y)->id = 0;
    
    free_id = id;
}

void Cells_Update()
{
    kinetic_energy = 0, amount_of_active = 0;
    uint32_t id = 0;
    next_id = 0;
    do
    {
        next_id = cells[id].next;
        if(cells[id].used
        )
        {
            Cell_Update_Start(id);
        }
    
        id = next_id;
    }
    while(id != 0);
    
    id = 0;
    next_id = 0;
    do
    {
        next_id = cells[id].next;
        if(cells[id].used
        )
        {
            Cell_Update_Finish(id);
        }
    
        id = next_id;
    }
    while(id != 0);
    
    if(amount_of_active != 0) printf("total kinetic %5d\tenergy per active %3d\n", kinetic_energy, kinetic_energy / amount_of_active);
}

void Cell_Update_Start(uint32_t id)
{   
    if(rnd() % 1000 < abs(temp_control))
    {
        cells[id].energy = max(cells[id].energy + sign(temp_control), 0);
    }
    
    if(temp_gradient)
    {
        if(cells[id].x > grid_width / 2
        && rnd() % (grid_width * 1000) < temp_gradient * (cells[id].x - grid_width / 2))
        {
            cells[id].energy = max(cells[id].energy + 1, 0);
        }
        if(cells[id].x < grid_width / 2
        && rnd() % (grid_width * 1000) < temp_gradient * (grid_width / 2 - cells[id].x))
        {
            cells[id].energy = max(cells[id].energy - 1, 0);
        }
    }
    
    uint8_t dir;
    uint16_t vel = fast_root(2 * cells[id].energy * 256 / (cells[id].Z * 2));
    uint16_t move = rnd() % 256 < vel + 1;
    uint16_t x, y;
    x = cells[id].x;
    y = cells[id].y;
    int16_t dx, dy;
    
    kinetic_energy += cells[id].energy;
    amount_of_active++;
    
    if(move == 0) return;
    if(cells[id].dir == 8 || 1) cells[id].dir = rnd() % 8;
    
    dir = cells[id].dir;
    
    dx = dir_to_coords[dir][0];
    dy = dir_to_coords[dir][1];
    
    Rec_Push_Flexible(x, y, dx, dy, cells[id].Z);
}

void Cell_Update_Finish(uint32_t id)
{
    uint8_t req = Required_Electrons(cells[id].e);
    if(req == 0) return;

    uint16_t x, y;
    x = cells[id].x;
    y = cells[id].y;
    
    Tile *itself, *neighbor;
    int8_t dx, dy;
    uint8_t mask, n_state;
    int16_t temp1;
    uint8_t membrane = Is_Membrane(x, y);
    int32_t minus_energy = 0, plus_energy, sum_energy, energy1;
    
    itself = Grid_Get(x, y);
    
    for(int dir = 0; dir < 8; dir++)
    {
        dx = dir_to_coords[dir][0];
        dy = dir_to_coords[dir][1];
        
        neighbor = Grid_Get(x + dx, y + dy);
        if(neighbor->type != 1) continue;
        
        sum_energy = cells[id].energy + cells[neighbor->id].energy;
        if(sum_energy != 0)
        {
            cells[id].energy = 0;
            cells[neighbor->id].energy = 0;
        } 
        uint8_t stop = 0, broken = 0;
        
        if(itself->links[dir] > 0 && neighbor->links[mod(dir + 4, 8)] > 0
        )
        {
            
            minus_energy = Energy(id, neighbor->id, itself->links[dir])
            - Energy(id, neighbor->id, max(itself->links[dir] - 1, 0));
            while(
            (sum_energy > minus_energy && minus_energy >= 0
            || minus_energy <= 0) && !stop && itself->links[dir] > 0
            )
            {
                if( minus_energy <= 0
                || rnd() % 1000 < 1000 * (sum_energy - minus_energy) / minus_energy
                && minus_energy > 0)
                {
                    // printf("breaking %d %d %d %d\n", sum_energy, minus_energy, id, neighbor->id);
                    cells[id].shared--;
                    cells[neighbor->id].shared--;
                    Unlink_Two(x, y, x + dx, y + dy);
                    sum_energy -= minus_energy;
                    
                    minus_energy = Energy(id, neighbor->id, itself->links[dir])
                    - Energy(id, neighbor->id, max(itself->links[dir] - 1, 0));
            
                    broken = 1;
                }
                else
                {
                    stop = 1;
                }
            }
        }
        if(!broken)
        {
            stop = 0;
            if(
            cells[id].shared < Valence(cells[id].e)
            && Required_Electrons(cells[id].e) - cells[id].shared > 0
            && cells[neighbor->id].shared < Valence(cells[neighbor->id].e)
            && Required_Electrons(cells[neighbor->id].e) - cells[neighbor->id].shared > 0
            && 
            !stop)
            {
                plus_energy = Energy(id, neighbor->id, itself->links[dir] + 1)
                 - Energy(id, neighbor->id, itself->links[dir]);
                if(plus_energy >= 0
                || (rnd() % 1000 < 1000 * (sum_energy + plus_energy) / -plus_energy
                && plus_energy < 0)
                )
                {
                    
                    
                    cells[id].shared++;
                    cells[neighbor->id].shared++;
                    Link_Two(x, y, x + dx, y + dy);
                    
                    sum_energy += plus_energy;
                
                    plus_energy = Energy(id, neighbor->id, itself->links[dir] + 1)
                     - Energy(id, neighbor->id, itself->links[dir]);
                }
                else
                {
                    stop = 1;
                }
            }
        }
        
        if(sum_energy >= 0)
        {
            energy1 = rnd() % (sum_energy + 1);
            cells[id].energy = energy1;
            cells[neighbor->id].energy = sum_energy - energy1;
            
            // cells[id].dir = rnd() % 9;
            // cells[neighbor->id].dir = rnd() % 9;
        }
    }
}

uint8_t Valence(uint8_t e)
{
    uint8_t total_e = e;
    uint8_t level = 1;
    uint8_t valence = 0;
    uint8_t max_on_level;
    
    while(total_e > 0)
    {
        max_on_level = 2 * level * level;
        if(total_e <= max_on_level)
        {
            valence = total_e;
            break;
        }
        total_e -= max_on_level;
        level += 1;
    }
    
    return valence;
}

uint8_t Level(uint8_t e)
{
    uint8_t total_e = e;
    uint8_t level = 1;
    uint8_t valence = 0;
    uint8_t max_on_level;
    
    while(total_e > 0)
    {
        max_on_level = 2 * level * level;
        if(total_e <= max_on_level)
        {
            valence = total_e;
            break;
        }
        total_e -= max_on_level;
        level += 1;
    }
    
    return level;
}

uint8_t Required_Electrons(uint8_t e)
{
    uint8_t total_e = e;
    uint8_t level = 1;
    uint8_t valence = 0;
    uint8_t max_on_level;
    
    while(total_e > 0)
    {
        max_on_level = 2 * level * level;
        if(total_e <= max_on_level)
        {
            valence = total_e;
            break;
        }
        total_e -= max_on_level;
        level += 1;
    }
    
    return max_on_level - valence;
}

uint32_t slater_sigma(uint8_t e) {
    uint32_t electrons_left = e;
    uint32_t n = 1, l = 0;
    uint32_t group_electrons[20] = {0};
    uint32_t group_idx = 0;
    
    while (electrons_left > 0) {
        uint32_t cap = 2 * (2 * l + 1);
        uint32_t fill = (electrons_left < cap) ? electrons_left : cap;
        
        if (l == 0)
        {
            group_idx++;
        }
        group_electrons[group_idx] += fill;
        
        electrons_left -= fill;
        l++;
        if (l >= n) { 
            l = 0; 
            n++; 
        }
    }
    
    uint32_t target_group = group_idx;
    
    uint32_t sigma10000 = 0;
    for (uint32_t g = 0; g <= target_group; g++) {
        if (g == target_group)
        {
            if(target_group == 1)
                sigma10000 += 30 * (group_electrons[g] - 1);
            else
                sigma10000 += 35 * (group_electrons[g] - 1);
        } else if (g == target_group - 1)
        {
            sigma10000 += 85 * group_electrons[g];
        } else
        {
            sigma10000 += 100 * group_electrons[g];
        }
    }
    
    return sigma10000;
}

uint32_t Energy(uint32_t id1, uint32_t id2, uint8_t links)
{
    uint32_t radius = cubic_root(50653 * cells[id1].Z) + cubic_root(50653 * cells[id2].Z);
    
    // printf("rad %d\n", radius);
    
    uint32_t en1 = fast_root(4250 * cells[id1].Z) * (8 - Valence(cells[id1].e));
    uint32_t en2 = fast_root(4250 * cells[id2].Z) * (8 - Valence(cells[id2].e));
    uint32_t delta_en = abs(en1 - en2);
    
    uint32_t valence_repulsion = (Valence(cells[id1].e) + Valence(cells[id2].e));
    
    // printf("rad %d\n", valence_repulsion);
    
    
    uint32_t e1 = 100 * cells[id1].e;
    uint32_t e2 = 100 * cells[id2].e;
    
    uint32_t sigma1 = slater_sigma(cells[id1].e);
    uint32_t sigma2 = slater_sigma(cells[id2].e);
    
    uint32_t Z1 = 100 * cells[id1].Z - sigma1;
    uint32_t Z2 = 100 * cells[id2].Z - sigma2;
    
    uint32_t N1 = Valence(cells[id1].e) - links;
    uint32_t N2 = Valence(cells[id2].e) - links;
    
    uint32_t k = 57;
    
    uint32_t energy = 100 * links * fast_root(Z1 * Z2) / radius
    - 1000 * links * k * (N1 + N2) * (N1 + N2) / (radius * radius)
    + links * delta_en * delta_en / radius;
    
    // energy = energy * 436 / 135;
    
    // if(cells[id1].Z == 1 && cells[id2].Z == 7)
    // {
    //     printf("Z1 %d Z2 %d links %d delta_en %d\n", Z1, Z2, links, delta_en);
    //     printf("%d - %d + %d = %d\n", 100 * links * fast_root(Z1 * Z2) / radius
    //     , 1000 * links * k * (N1 + N2) * (N1 + N2) / (radius * radius)
    //     , links * delta_en * delta_en / radius, energy);
    // }
    
    return energy;
}