#ifndef ___STRUCT64_H___
#define ___STRUCT64_H___
#include <stdint.h>

#define uint64_s_shr(x,c)   shr(x,c)
#define uint64_s_shl(x,c)   shl(x,c)

typedef struct 
{
    /* data */
    uint32_t t[2];
}uint64_s;

typedef uint64_s int64_s;

#define UINT64_S_ZERO       ((uint64_s){{0,0}})
#define UINT64_S_ONE        ((uint64_s){{1,0}})


uint64_s rol(uint64_s a, int offset);

uint64_s uint64_s_add(uint64_s x, uint64_s y);

uint64_s uint32_t_mul(uint32_t x, uint32_t y);

uint64_s int32_t_mul(int32_t x, int32_t y);

uint64_s shr(uint64_s x,uint32_t c);

uint64_s shl(uint64_s x,uint32_t c);

uint64_s uint64_s_or(uint64_s x, uint64_s y);

uint64_s uint64_s_and(uint64_s x, uint64_s y);

uint64_s uint64_s_xor(uint64_s x, uint64_s y);

uint64_s uint64_s_not(uint64_s x);

uint64_s uint64_s_sub(uint64_s x, uint64_s y);

uint64_s uint64_s_neg(uint64_s x);

uint64_s uint64_s_mul(uint64_s x, uint64_s y);

int64_s int64_s_mul(int64_s x, int64_s y);

int64_s int64_s_shr(int64_s x,uint32_t c);

int uint64_s_eq(uint64_s x, uint64_s y);


#endif