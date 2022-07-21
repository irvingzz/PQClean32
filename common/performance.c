#include <stdint.h>

#include "performance.h"
#include <stdio.h>

uint32_t cycles_keypair;
uint32_t instrs_keypair;

uint32_t cycles_signature;
uint32_t instrs_signature;

uint32_t cycles_verify;
uint32_t instrs_verify;

unsigned int times_ntt=0;
uint32_t cycles_ntt;
uint32_t instrs_ntt;

unsigned int times_fft=0;
uint32_t cycles_fft;
uint32_t instrs_fft;

unsigned int times_intt=0;
uint32_t cycles_intt;
uint32_t instrs_intt;

unsigned int times_ifft=0;
uint32_t cycles_ifft;
uint32_t instrs_ifft;

//! Sample the clock cycle counter (used for timing checks)
uint32_t perf_rdcycle() {
    uint32_t tr;
    asm volatile ("rdcycle %0":"=r"(tr));
    return tr;
}

//! Sample the clock cycle counter (used for timing checks)
uint32_t perf_rdinstret() {
    uint32_t tr;
    asm volatile ("rdinstret %0":"=r"(tr));
    return tr;
}

void print_uint64_s(uint64_s *s, int length) {
    for(int i = 0; i < length; i ++) {
        printf("%08x",s[i].t[1]);
        printf("%08x",s[i].t[0]);
    }
    printf("\n");
}

void printbytes(const uint8_t *x, uint32_t xlen) {
    uint32_t i;
    for (i = 0; i < xlen; i++) {
        printf("%02x", x[i]);
    }
    printf("\n");
}