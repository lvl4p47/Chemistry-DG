#include "utility.h"

int8_t dir_to_coords[9][2] = 
{
    {-1, -1},
    {0, -1},
    {1, -1},
    {1, 0},
    {1, 1},
    {0, 1},
    {-1, 1},
    {-1, 0},
    {0, 0}
};

uint8_t coords_to_dir[3][3] = 
{
    {0, 1, 2},
    {7, 8, 3},
    {6, 5, 4}
};

uint32_t state;
int pause = 0;
uint32_t last_frame = 0;

int32_t min(int32_t a, int32_t b)
{
    if(a < b) return a;
    return b;
}

int32_t max(int32_t a, int32_t b)
{
    if(a > b) return a;
    return b;
}

float power(int a, int n)
{
    float ans = 1;
    if(n > 0)
    {
        for(int i = 0; i < n; i++)
        {
            ans *= a;
        }
    }
    if(n < 0)
    {
        for(int i = 0; i < -n; i++)
        {
            ans /= a;
        }
    }
    return ans;
}

uint32_t rnd() {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

// STACK

uint8_t Stack_Reset(Stack *s)
{
    if (!s) return 0;
    s->size = 0;
    return 1;
}

uint8_t Stack_Is_Empty(const Stack *s)
{
    if (!s) return 1;
    return (s->size == 0);
}

uint8_t Stack_Push(Stack *s, uint8_t value)
{
    if (!s) return 0;
    if (s->size >= STACK_SIZE) return 0;

    s->data[s->size] = value;
    s->size++;
    return 1;
}

uint8_t Stack_Pop(Stack *s, uint8_t *value)
{
    if (!s) return 0;
    if (s->size == 0) return 0;

    s->size--;
    if (value) {
        *value = s->data[s->size];
    }
    return 1;
}

uint8_t Stack_Peek(const Stack *s, uint8_t *value)
{
    if (!s) return 0;
    if (s->size == 0) return 0;
    if (!value) return 0;

    *value = s->data[s->size - 1];
    return 1;
}