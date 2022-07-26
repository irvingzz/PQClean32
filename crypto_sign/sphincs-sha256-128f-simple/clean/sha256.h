#ifndef PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_SHA256_H
#define PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_SHA256_H

#define PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_SHA256_BLOCK_BYTES 64
#define PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_SHA256_OUTPUT_BYTES 32  /* This does not necessarily equal PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_N */
#define PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_SHA256_ADDR_BYTES 22

#include <stddef.h>
#include <stdint.h>

#include "sha2.h"
#include "performance.h"

void PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_compress_address(unsigned char *out, const uint32_t addr[8]);

void PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_mgf1(
    unsigned char *out, unsigned int outlen,
    unsigned char *input_plus_four_bytes, unsigned int inlen);

void PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_seed_state(sha256ctx *hash_state_seeded, const unsigned char *pub_seed);

#endif
