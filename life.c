#include "life.h"

Particle particles[MAX_PARTICLES];

uint32_t next_id, free_id;

uint32_t mutation_rarity = 1000000;

int16_t temp_control = 0;
uint8_t temp_gradient = 1;
uint8_t chem_bonds = 01;

uint32_t kinetic_energy, amount_of_particles;

uint32_t Find_Free_Id()
{
    uint32_t counter = 0;
    while((particles[free_id].used != 0 || free_id == 0)
    && counter < MAX_PARTICLES)
    {
        free_id = mod(free_id + 1, MAX_PARTICLES);
        counter++;
    }
    
    if(particles[free_id].used == 0) return free_id;
    else 
    {
        printf("no free cells\n");
        return 0;
    }
}

void Cells_Init()
{
    for(uint32_t id = 0; id < MAX_PARTICLES; id++)
    {
        particles[id].prev = 0;
        particles[id].next = 0;
        particles[id].used = 0;
        
        particles[id].rec_str = 0;
        particles[id].on_edge = 0;
        
        for(int i = 0; i < 8; i++)
        {
            particles[id].links[i] = 0;
            particles[id].buf_links[i] = 0;
        }
        
        particles[id].x = 0;
        particles[id].y = 0;
        
        particles[id].Z = 0;
        particles[id].e = 0;
        particles[id].energy = 0;
        particles[id].shared = 0;
        particles[id].dir = 8;
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
    
    particles[id].x = mod(x, grid_width);
    particles[id].y = mod(y, grid_height);
    particles[id].prev = particles[parent].prev;
    particles[id].next = parent;
    particles[id].used = 1;
    
    particles[id].Z = Z;
    particles[id].e = Z;
    particles[id].energy = 0;
    particles[id].shared = 0;
    particles[id].dir = rnd() % 8;
    
    particles[particles[id].prev].next = id;
    particles[parent].prev = id;
    
    Tile *center = Grid_Get(x, y);
    Particle *tile_part = &particles[center->id];
        
    for(int dir = 0; dir < 8; dir++)
    {
        tile_part->links[dir] = 0;
    }
    
    Grid_Get(x, y)->id = id;
}

void Cell_Destroy(uint32_t id)
{
    if(id == 0) return;
    
    particles[particles[id].prev].next = particles[id].next;
    particles[particles[id].next].prev = particles[id].prev;
    
    particles[id].used = 0;
    
    Grid_Get(particles[id].x, particles[id].y)->id = 0;
    
    free_id = id;
}

void Cells_Update()
{
    kinetic_energy = 0, amount_of_particles = 0;
    uint32_t id = 0;
    next_id = 0;
    do
    {
        next_id = particles[id].next;
        if(particles[id].used
        )
        {
            Cell_Update_Movement(id);
        }
    
        id = next_id;
    }
    while(id != 0);
    
    id = 0;
    next_id = 0;
    do
    {
        next_id = particles[id].next;
        if(particles[id].used
        )
        {
            Cell_Update_Collision(id);
        }
    
        id = next_id;
    }
    while(id != 0);
    
    // if(amount_of_particles != 0) printf("total kinetic %5d\tenergy per particle %3d\n", kinetic_energy, kinetic_energy / amount_of_particles);
}

void Cell_Update_Movement(uint32_t id)
{   
    if(rnd() % 1000 < abs(temp_control))
    {
        particles[id].energy = max(particles[id].energy + sign(temp_control), 0);
    }
    
    if(temp_gradient)
    {
        if(particles[id].x > grid_width / 2
        && rnd() % (grid_width * 1000) < temp_gradient * (particles[id].x - grid_width / 2))
        {
            particles[id].energy = max(particles[id].energy + 1, 0);
        }
        if(particles[id].x < grid_width / 2
        && rnd() % (grid_width * 1000) < temp_gradient * (grid_width / 2 - particles[id].x))
        {
            particles[id].energy = max(particles[id].energy - 1, 0);
        }
    }
    
    uint8_t dir;
    uint16_t vel = fast_root(2 * particles[id].energy * 256 / (particles[id].Z * 2));
    // uint16_t move = rnd() % 256 < vel + 1;
    uint16_t move = (particles[id].energy > 0);
    uint16_t x, y;
    x = particles[id].x;
    y = particles[id].y;
    int16_t dx, dy;
    
    kinetic_energy += particles[id].energy;
    amount_of_particles++;
    
    if(move == 0) return;
    if(particles[id].dir == 8) return;
    
    dir = particles[id].dir;
    
    dx = dir_to_coords[dir][0];
    dy = dir_to_coords[dir][1];
    
    Rec_Push_Flexible(x, y, dx, dy, 100);
}

void Cell_Update_Collision(uint32_t id)
{
    uint16_t x, y;
    x = particles[id].x;
    y = particles[id].y;
    
    Tile *itself, *neighbor;
    Particle *itself_part, *neighbor_part;
    int8_t dx, dy;
    int8_t vx1, vy1, vx2, vy2;
    int8_t nx1, ny1, nx2, ny2;
    int8_t dvx, dvy;
    int16_t temp1;
    uint8_t membrane = Is_Membrane(x, y);
    int32_t minus_energy = 0, plus_energy, sum_energy, energy1;
    
    itself = Grid_Get(x, y);
    itself_part = &particles[itself->id];
    
    for(int dir = 0; dir < 8; dir++)
    {
        dx = dir_to_coords[dir][0];
        dy = dir_to_coords[dir][1];
        
        neighbor = Grid_Get(x + dx, y + dy);
        neighbor_part = &particles[neighbor->id];
        
        if(neighbor->type == 2)
        {
            vx1 = dir_to_coords[particles[id].dir][0];
            vy1 = dir_to_coords[particles[id].dir][1];
            
            nx1 = vx1, ny1 = vy1;
            
            if( dx == 0 )
            {
                ny1 = -ny1;
            }
            if( dy == 0 )
            {
                nx1 = -nx1;
            }
            
            particles[id].dir = coords_to_dir[1 + ny1][1 + nx1];
        }
        
        if(neighbor->type != 1) continue;
        
        sum_energy = particles[id].energy + particles[neighbor->id].energy;
        if(sum_energy != 0)
        {
            particles[id].energy = 0;
            particles[neighbor->id].energy = 0;
        }
        
        if(chem_bonds)
        {
            uint8_t stop = 0, broken = 0;
            
            if(itself_part->links[dir] > 0 && neighbor_part->links[mod(dir + 4, 8)] > 0
            )
            {
                
                minus_energy = Energy(id, neighbor->id, itself_part->links[dir])
                - Energy(id, neighbor->id, max(itself_part->links[dir] - 1, 0));
                while(
                (sum_energy > minus_energy && minus_energy >= 0
                || minus_energy <= 0) && !stop && itself_part->links[dir] > 0
                )
                {
                    if( minus_energy <= 0
                    || rnd() % 1000 < 1000 * (sum_energy - minus_energy) / minus_energy
                    && minus_energy > 0)
                    {
                        // printf("breaking %d %d %d %d\n", sum_energy, minus_energy, id, neighbor->id);
                        particles[id].shared--;
                        particles[neighbor->id].shared--;
                        Unlink_Two(x, y, x + dx, y + dy);
                        sum_energy -= minus_energy;
                        
                        minus_energy = Energy(id, neighbor->id, itself_part->links[dir])
                        - Energy(id, neighbor->id, max(itself_part->links[dir] - 1, 0));
                
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
                particles[id].shared < Valence(particles[id].e)
                && Required_Electrons(particles[id].e) - particles[id].shared > 0
                && particles[neighbor->id].shared < Valence(particles[neighbor->id].e)
                && Required_Electrons(particles[neighbor->id].e) - particles[neighbor->id].shared > 0
                && 
                !stop)
                {
                    plus_energy = Energy(id, neighbor->id, itself_part->links[dir] + 1)
                    - Energy(id, neighbor->id, itself_part->links[dir]);
                    if(plus_energy >= 0
                    || (rnd() % 1000 < 1000 * (sum_energy + plus_energy) / -plus_energy
                    && plus_energy < 0)
                    )
                    {
                        
                        
                        particles[id].shared++;
                        particles[neighbor->id].shared++;
                        Link_Two(x, y, x + dx, y + dy);
                        
                        sum_energy += plus_energy;
                    
                        plus_energy = Energy(id, neighbor->id, itself_part->links[dir] + 1)
                        - Energy(id, neighbor->id, itself_part->links[dir]);
                    }
                    else
                    {
                        stop = 1;
                    }
                }
            }
        }
        {
            energy1 = rnd() % (sum_energy + 1);
            particles[id].energy = energy1;
            particles[neighbor->id].energy = sum_energy - energy1;
            
            if(itself_part->links[dir] == 0 || 1)
            {
                vx1 = dir_to_coords[particles[id].dir][0];
                vy1 = dir_to_coords[particles[id].dir][1];
                vx2 = dir_to_coords[particles[neighbor->id].dir][0];
                vy2 = dir_to_coords[particles[neighbor->id].dir][1];
                
                nx1 = vx1, ny1 = vy1, nx2 = vx2, ny2 = vy2;
                
                dvx = nx2 - nx1;
                dvy = ny2 - ny1;
                
                while(
                abs(nx1 - dx) < 2 &&
                abs(ny1 - dy) < 2 &&
                abs(nx2 + dx) < 2 &&
                abs(ny2 + dy) < 2 &&
                dvx * dx + dvy * dy < 0
                )
                {
                    nx1 -= dx;
                    ny1 -= dy;
                    nx2 += dx;
                    ny2 += dy;
                    
                    dvx = nx2 - nx1;
                    dvy = ny2 - ny1;
                }
                
                particles[id].dir = coords_to_dir[1 + ny1][1 + nx1];
                particles[neighbor->id].dir = coords_to_dir[1 + ny2][1 + nx2];
            }
            if(itself_part->links[dir] != 0 && 0)
            {
                vx1 = dir_to_coords[particles[id].dir][0];
                vy1 = dir_to_coords[particles[id].dir][1];
                vx2 = dir_to_coords[particles[neighbor->id].dir][0];
                vy2 = dir_to_coords[particles[neighbor->id].dir][1];
                
                nx1 = vx1, ny1 = vy1, nx2 = vx2, ny2 = vy2;
                
                dvx = nx2 - nx1;
                dvy = ny2 - ny1;
                
                while(
                abs(nx1 - dx) < 2 &&
                abs(ny1 - dy) < 2 &&
                abs(nx2 + dx) < 2 &&
                abs(ny2 + dy) < 2 &&
                dvx * dx + dvy * dy > 0
                )
                {
                    nx1 += dx;
                    ny1 += dy;
                    nx2 -= dx;
                    ny2 -= dy;
                    
                    dvx = nx2 - nx1;
                    dvy = ny2 - ny1;
                }
                
                particles[id].dir = coords_to_dir[1 + ny1][1 + nx1];
                particles[neighbor->id].dir = coords_to_dir[1 + ny2][1 + nx2];
            }
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
    
    uint32_t sigma100 = 0;
    for (uint32_t g = 0; g <= target_group; g++) {
        if (g == target_group)
        {
            if(target_group == 1)
                sigma100 += 30 * (group_electrons[g] - 1);
            else
                sigma100 += 35 * (group_electrons[g] - 1);
        } else if (g == target_group - 1)
        {
            sigma100 += 85 * group_electrons[g];
        } else
        {
            sigma100 += 100 * group_electrons[g];
        }
    }
    
    return sigma100;
}

uint32_t Energy(uint32_t id1, uint32_t id2, uint8_t links)
{
    uint32_t radius = cubic_root(50653 * particles[id1].Z) + cubic_root(50653 * particles[id2].Z);
    
    // printf("rad %d\n", radius);
    
    uint32_t en1 = fast_root(4250 * particles[id1].Z) * (8 - Valence(particles[id1].e));
    uint32_t en2 = fast_root(4250 * particles[id2].Z) * (8 - Valence(particles[id2].e));
    uint32_t delta_en = abs(en1 - en2);
    
    uint32_t valence_repulsion = (Valence(particles[id1].e) + Valence(particles[id2].e));
    
    // printf("rad %d\n", valence_repulsion);
    
    
    uint32_t e1 = 100 * particles[id1].e;
    uint32_t e2 = 100 * particles[id2].e;
    
    uint32_t sigma1 = slater_sigma(particles[id1].e);
    uint32_t sigma2 = slater_sigma(particles[id2].e);
    
    uint32_t Z1 = 100 * particles[id1].Z - sigma1;
    uint32_t Z2 = 100 * particles[id2].Z - sigma2;
    
    uint32_t N1 = Valence(particles[id1].e) - links;
    uint32_t N2 = Valence(particles[id2].e) - links;
    
    uint32_t k = 57;
    
    uint32_t energy = 100 * links * fast_root(Z1 * Z2) / radius
    - 1000 * links * k * (N1 + N2) * (N1 + N2) / (radius * radius)
    + links * delta_en * delta_en / radius;
    
    // energy = energy * 436 / 135;
    
    // if(particles[id1].Z == 1 && particles[id2].Z == 7)
    // {
    //     printf("Z1 %d Z2 %d links %d delta_en %d\n", Z1, Z2, links, delta_en);
    //     printf("%d - %d + %d = %d\n", 100 * links * fast_root(Z1 * Z2) / radius
    //     , 1000 * links * k * (N1 + N2) * (N1 + N2) / (radius * radius)
    //     , links * delta_en * delta_en / radius, energy);
    // }
    
    return energy;
}