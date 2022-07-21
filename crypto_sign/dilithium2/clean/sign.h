#ifndef PQCLEAN_DILITHIUM2_CLEAN_SIGN_H
#define PQCLEAN_DILITHIUM2_CLEAN_SIGN_H
#include "params.h"
#include "poly.h"
#include "polyvec.h"
#include <stddef.h>
#include <stdint.h>

void PQCLEAN_DILITHIUM2_CLEAN_challenge(poly *c, const uint8_t seed[SEEDBYTES]);

int PQCLEAN_DILITHIUM2_CLEAN_crypto_sign_keypair(uint8_t *pk, uint8_t *sk);

int PQCLEAN_DILITHIUM2_CLEAN_crypto_sign_signature(uint8_t *sig, uint32_t *siglen,
        const uint8_t *m, uint32_t mlen,
        const uint8_t *sk);

int PQCLEAN_DILITHIUM2_CLEAN_crypto_sign(uint8_t *sm, uint32_t *smlen,
        const uint8_t *m, uint32_t mlen,
        const uint8_t *sk);

int PQCLEAN_DILITHIUM2_CLEAN_crypto_sign_verify(const uint8_t *sig, uint32_t siglen,
        const uint8_t *m, uint32_t mlen,
        const uint8_t *pk);

int PQCLEAN_DILITHIUM2_CLEAN_crypto_sign_open(uint8_t *m, uint32_t *mlen,
        const uint8_t *sm, uint32_t smlen,
        const uint8_t *pk);

#endif
