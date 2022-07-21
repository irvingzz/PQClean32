#ifndef ___STRUCT64_H___
#define ___STRUCT64_H___
#include <stdint.h>

typedef struct 
{
    /* data */
    uint32_t t[2];
}uint64_s;

uint64_s rol(uint64_s a, int offset);

uint64_s uint64_s_add(uint64_s x, uint64_s y);

uint64_s uint32_t_mul(uint32_t x, uint32_t y);

uint64_s int32_t_mul(int32_t x, int32_t y);

uint64_s shr(uint64_s x,uint32_t c);

#endif