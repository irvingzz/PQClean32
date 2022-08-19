#include <stddef.h>
#include <stdint.h>

#include "sp800-185.h"

static uint32_t left_encode(uint8_t *encbuf, uint64_s value) {
    uint32_t n, i;
    uint64_s v, tmp;

    for (v = value, n = 0; v.t[0] && v.t[1] && (n < sizeof(uint64_s)); n++, v = uint64_s_shr(v, 8)) {
        ; /* empty */
    }
    if (n == 0) {
        n = 1;
    }
    for (i = 1; i <= n; i++) {
        tmp = uint64_s_shr(value, (8 * (n-i)));
        encbuf[i] = (uint8_t)tmp.t[0];
    }
    encbuf[0] = (uint8_t)n;
    return n + 1;
}

void cshake128_inc_init(shake128incctx *state, const uint8_t *name, uint32_t namelen, const uint8_t *cstm, uint32_t cstmlen) {
    uint8_t encbuf[sizeof(uint64_s)+1];

    shake128_inc_init(state);

    shake128_inc_absorb(state, encbuf, left_encode(encbuf, (uint64_s){SHAKE128_RATE,0}));

    shake128_inc_absorb(state, encbuf, left_encode(encbuf, (uint64_s){namelen * 8, 0}));
    shake128_inc_absorb(state, name, namelen);

    shake128_inc_absorb(state, encbuf, left_encode(encbuf, (uint64_s){cstmlen * 8, 0}));
    shake128_inc_absorb(state, cstm, cstmlen);

    if (state->ctx[25].t[0] != 0) {
        state->ctx[25].t[0] = SHAKE128_RATE - 1;
        encbuf[0] = 0;
        shake128_inc_absorb(state, encbuf, 1);
    }
}

void cshake128_inc_absorb(shake128incctx *state, const uint8_t *input, uint32_t inlen) {
    shake128_inc_absorb(state, input, inlen);
}

void cshake128_inc_finalize(shake128incctx *state) {
    state->ctx[state->ctx[25].t[0] >> 3].t[(state->ctx[25].t[0] & 0x04) >> 2] ^= (uint32_t)0x04 << (8 * (state->ctx[25].t[0] & 0x03));
    state->ctx[(SHAKE128_RATE - 1) >> 3].t[((SHAKE128_RATE - 1) & 0x04) >> 2] ^= (uint32_t)128 << (8 * ((SHAKE128_RATE - 1) & 0x03));
    state->ctx[25].t[0] = 0;
}

void cshake128_inc_squeeze(uint8_t *output, uint32_t outlen, shake128incctx *state) {
    shake128_inc_squeeze(output, outlen, state);
}

void cshake128_inc_ctx_release(shake128incctx *state) {
    shake128_inc_ctx_release(state);
}

void cshake128_inc_ctx_clone(shake128incctx *dest, const shake128incctx *src) {
    shake128_inc_ctx_clone(dest, src);
}

void cshake256_inc_init(shake256incctx *state, const uint8_t *name, uint32_t namelen, const uint8_t *cstm, uint32_t cstmlen) {
    uint8_t encbuf[sizeof(uint64_s)+1];

    shake256_inc_init(state);

    shake256_inc_absorb(state, encbuf, left_encode(encbuf, (uint64_s){SHAKE256_RATE, 0}));

    shake256_inc_absorb(state, encbuf, left_encode(encbuf, (uint64_s){namelen * 8, 0}));
    shake256_inc_absorb(state, name, namelen);

    shake256_inc_absorb(state, encbuf, left_encode(encbuf, (uint64_s){cstmlen * 8, 0}));
    shake256_inc_absorb(state, cstm, cstmlen);

    if (state->ctx[25].t[0] != 0) {
        state->ctx[25].t[0] = SHAKE256_RATE - 1;
        encbuf[0] = 0;
        shake256_inc_absorb(state, encbuf, 1);
    }
}

void cshake256_inc_absorb(shake256incctx *state, const uint8_t *input, uint32_t inlen) {
    shake256_inc_absorb(state, input, inlen);
}

void cshake256_inc_finalize(shake256incctx *state) {
    state->ctx[state->ctx[25].t[0] >> 3].t[(state->ctx[25].t[0] & 0x04) >> 2] ^= (uint32_t)0x04 << (8 * (state->ctx[25].t[0] & 0x03));
    state->ctx[(SHAKE256_RATE - 1) >> 3].t[((SHAKE256_RATE - 1) & 0x04) >> 2] ^= (uint32_t)128 << (8 * ((SHAKE256_RATE - 1) & 0x03));
    state->ctx[25].t[0] = 0;
}

void cshake256_inc_squeeze(uint8_t *output, uint32_t outlen, shake256incctx *state) {
    shake256_inc_squeeze(output, outlen, state);
}

void cshake256_inc_ctx_release(shake256incctx *state) {
    shake256_inc_ctx_release(state);
}

void cshake256_inc_ctx_clone(shake256incctx *dest, const shake256incctx *src) {
    shake256_inc_ctx_clone(dest, src);
}

/*************************************************
 * Name:        cshake128
 *
 * Description: cSHAKE128 XOF with non-incremental API
 *
 * Arguments:   - uint8_t *output: pointer to output
 *              - uint32_t outlen: requested output length in bytes
 *              - const uint8_t *name: pointer to function-name string
 *              - uint32_t namelen: length of function-name string in bytes
 *              - const uint8_t *cstm: pointer to non-empty customization string
 *              - uint32_t cstmlen: length of customization string in bytes
 *              - const uint8_t *input: pointer to input
 *              - uint32_t inlen: length of input in bytes
 **************************************************/
void cshake128(uint8_t *output, uint32_t outlen,
               const uint8_t *name, uint32_t namelen,
               const uint8_t *cstm, uint32_t cstmlen,
               const uint8_t *input, uint32_t inlen) {
    shake128incctx state;
    cshake128_inc_init(&state, name, namelen, cstm, cstmlen);
    cshake128_inc_absorb(&state, input, inlen);
    cshake128_inc_finalize(&state);
    cshake128_inc_squeeze(output, outlen, &state);
    cshake128_inc_ctx_release(&state);
}

/*************************************************
 * Name:        cshake256
 *
 * Description: cSHAKE256 XOF with non-incremental API
 *
 * Arguments:   - uint8_t *output: pointer to output
 *              - uint32_t outlen: requested output length in bytes
 *              - const uint8_t *name: pointer to function-name string
 *              - uint32_t namelen: length of function-name string in bytes
 *              - const uint8_t *cstm: pointer to non-empty customization string
 *              - uint32_t cstmlen: length of customization string in bytes
 *              - const uint8_t *input: pointer to input
 *              - uint32_t inlen: length of input in bytes
 **************************************************/
void cshake256(uint8_t *output, uint32_t outlen,
               const uint8_t *name, uint32_t namelen,
               const uint8_t *cstm, uint32_t cstmlen,
               const uint8_t *input, uint32_t inlen) {
    shake256incctx state;
    cshake256_inc_init(&state, name, namelen, cstm, cstmlen);
    cshake256_inc_absorb(&state, input, inlen);
    cshake256_inc_finalize(&state);
    cshake256_inc_squeeze(output, outlen, &state);
    cshake256_inc_ctx_release(&state);
}
