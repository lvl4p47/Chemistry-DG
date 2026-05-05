#include "init.h"

uint64_t prev_tick, cur_tick;
uint16_t cycles = 0, cps = 0, threshold = 1000;
uint32_t total_cycles = 0;
uint16_t ms_per_fast_frame = 32, ms_per_slow_frame = 320;
uint16_t ms_per_frame;

uint8_t leak_detector = 0;

int main(int argc, char* args[])
{   
    freopen("debug.log", "w", stderr);
    
    state = (uint32_t)time(NULL);
    
    All_Init();
    
    prev_tick = SDL_GetTicks64();

    while (!quit) {
        cycles++;
        
        if(!pause) 
        {
            Global_Time_Update();
            Gravity();
            Cells_Update();
            
        }
        
        Events_Handle();

        Events_Process();
        
        Illuminate();
        // SDL_Delay(100);
        cur_tick = SDL_GetTicks64();
        
        if(slow_mode == 0)
        {
            ms_per_frame = ms_per_fast_frame;
        }
        else
        {
            ms_per_frame = ms_per_slow_frame;
        }
        if(cur_tick - last_frame > ms_per_frame || leak_detector)
        {
            last_frame = cur_tick;
            Screen_Clear();
            Screen_Draw();
        }
        
        if(cur_tick - prev_tick > threshold || leak_detector)
        {
            
            cps = cycles * 1000 / threshold;
            total_cycles += cycles;
            cycles = 0;
            prev_tick = cur_tick;
            
            printf("cps %4d\n", cps);
        }
    }
    
    All_Quit();

    return 0;
}
