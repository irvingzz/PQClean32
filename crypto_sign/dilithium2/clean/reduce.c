#include "params.h"
#include "reduce.h"
#include <stdint.h>
#include "performance.h"

/*************************************************
* Name:        PQCLEAN_DILITHIUM2_CLEAN_montgomery_reduce
*
* Description: For finite field element a with -2^{31}Q <= a <= Q*2^31,
*              compute r \equiv a*2^{-32} (mod Q) such that -Q < r < Q.
*
* Arguments:   - s: finite field element a
*
* Returns r.
**************************************************/
int32_t PQCLEAN_DILITHIUM2_CLEAN_montgomery_reduce(uint64_s a) {
    int32_t t;
    uint64_s tmp;

    // printf("  - a:\n");
    // print_uint64_s(&a,1);

    t = (int32_t)((uint32_t)a.t[0] * (uint32_t)QINV);
    tmp = int32_t_mul(t,Q);
    tmp.t[1] = ~tmp.t[1];
    tmp.t[0] = ~tmp.t[0];
    tmp = uint64_s_add (tmp, (uint64_s){1,0});
    tmp = uint64_s_add (tmp, a);
    t = (int32_t)tmp.t[1];

    // printf("  - t:\n");
    // printf("%08x\n",t);
    return t;
}

/*************************************************
* Name:        PQCLEAN_DILITHIUM2_CLEAN_reduce32
*
* Description: For finite field element a with a <= 2^{31} - 2^{22} - 1,
*              compute r \equiv a (mod Q) such that -6283009 <= r <= 6283007.
*
* Arguments:   - int32_t: finite field element a
*
* Returns r.
**************************************************/
int32_t PQCLEAN_DILITHIUM2_CLEAN_reduce32(int32_t a) {
    int32_t t;

    t = (a + (1 << 22)) >> 23;
    t = a - t * Q;
    return t;
}

/*************************************************
* Name:        PQCLEAN_DILITHIUM2_CLEAN_caddq
*
* Description: Add Q if input coefficient is negative.
*
* Arguments:   - int32_t: finite field element a
*
* Returns r.
**************************************************/
int32_t PQCLEAN_DILITHIUM2_CLEAN_caddq(int32_t a) {
    a += (a >> 31) & Q;
    return a;
}

/*************************************************
* Name:        PQCLEAN_DILITHIUM2_CLEAN_freeze
*
* Description: For finite field element a, compute standard
*              representative r = a mod^+ Q.
*
* Arguments:   - int32_t: finite field element a
*
* Returns r.
**************************************************/
int32_t PQCLEAN_DILITHIUM2_CLEAN_freeze(int32_t a) {
    a = PQCLEAN_DILITHIUM2_CLEAN_reduce32(a);
    a = PQCLEAN_DILITHIUM2_CLEAN_caddq(a);
    return a;
}
