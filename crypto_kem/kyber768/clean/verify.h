#ifndef PQCLEAN_KYBER768_CLEAN_VERIFY_H
#define PQCLEAN_KYBER768_CLEAN_VERIFY_H
#include "params.h"
#include <stddef.h>
#include <stdint.h>

int PQCLEAN_KYBER768_CLEAN_verify(const uint8_t *a, const uint8_t *b, uint32_t len);

void PQCLEAN_KYBER768_CLEAN_cmov(uint8_t *r, const uint8_t *x, uint32_t len, uint8_t b);

#endif