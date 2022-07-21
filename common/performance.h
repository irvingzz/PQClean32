
#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include "struct64.h"

#define print_hex(char,l)  (printbytes(char,l))

extern uint32_t cycles_keypair;
extern uint32_t instrs_keypair;

extern uint32_t cycles_signature;
extern uint32_t instrs_signature;

extern uint32_t cycles_verify;
extern uint32_t instrs_verify;

extern unsigned int times_ntt;
extern uint32_t cycles_ntt;
extern uint32_t instrs_ntt;

extern unsigned int times_fft;
extern uint32_t cycles_fft;
extern uint32_t instrs_fft;

extern unsigned int times_intt;
extern uint32_t cycles_intt;
extern uint32_t instrs_intt;

extern unsigned int times_ifft;
extern uint32_t cycles_ifft;
extern uint32_t instrs_ifft;

//! Sample the clock cycle counter (used for timing checks)
uint32_t perf_rdcycle();

//! Sample the clock cycle counter (used for timing checks)
uint32_t perf_rdinstret();

void print_uint64_s(uint64_s *s, int length);

void printbytes(const uint8_t *x, uint32_t xlen);

#endif
