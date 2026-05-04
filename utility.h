#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define RED_BG   "\x1b[41m"
#define GREEN_BG "\x1b[42m"
#define RESET    "\x1b[0m"

typedef enum
{
    NW,
    N,
    NE,
    E,
    SE,
    S,
    SW,
    W,
    C
} Moore_Directions;

extern int8_t dir_to_coords[9][2];
extern uint8_t coords_to_dir[3][3];
extern uint32_t my_state;
extern uint32_t last_frame;

extern int pause;

static inline int64_t mod(int64_t x, int64_t m) {
    int64_t r = x % m;
    return r >= 0 ? r : r + m;
}
int32_t min(int32_t a, int32_t b);
int32_t max(int32_t a, int32_t b);
static inline int8_t sign(int16_t x) {
    return (x > 0) - (x < 0);
}
float power(int a, int n);
static inline uint32_t fast_root(uint32_t x)
{
    uint32_t min = 1, max = x;
    uint32_t ans = (min + max) / 2;
    uint32_t sqr = ans * ans;
    uint32_t step = 0;
    
    while(abs(sqr - x) > 1 && step < 100)
    {
        if(sqr < x) min = ans;
        if(sqr > x) max = ans;
        
        ans = (min + max) / 2;
        sqr = ans * ans;
        step++;
    }

    return ans;
}

static inline uint32_t cubic_root(uint32_t x)
{
    uint32_t min = 1, max = x;
    uint32_t ans = (min + max) / 2;
    uint32_t cube = ans * ans * ans;
    uint32_t step = 0;
    
    while(abs(cube - x) > 1 && step < 100)
    {
        if(cube < x) min = ans;
        if(cube > x) max = ans;
        
        ans = (min + max) / 2;
        cube = ans * ans * ans;
        step++;
    }

    return ans;
}

static inline uint32_t lg(uint32_t a, uint32_t pow)
{
    int ans = 1;
    int step = 0;
    while(ans < a && step < 1000)
    {
        ans *= pow;
        step++;
    }
    
    return step;
}

static inline uint8_t Count_Bits_8(uint8_t x)
{
    uint8_t n = 0;

    while (x != 0) {
        n += (uint8_t)(x & 1u);
        x >>= 1;
    }

    return n;
}

extern uint32_t state;

uint32_t rnd();

// STACK

#define STACK_SIZE 32

typedef struct Stack {
    uint8_t data[STACK_SIZE];
    uint8_t size;
} Stack;

uint8_t Stack_Reset(Stack *s);
uint8_t Stack_Is_Empty(const Stack *s);
uint8_t Stack_Push(Stack *s, uint8_t value);
uint8_t Stack_Pop(Stack *s, uint8_t *value);
uint8_t Stack_Peek(const Stack *s, uint8_t *value);

#endif