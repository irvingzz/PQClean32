#ifndef AES_H
#define AES_H

#include <stdint.h>
#include <stdlib.h>

#define AES_MAXNR 14

/*!
@brief AES Key type
@details taken from
    https://github.com/openssl/openssl/blob/master/include/openssl/aes.h#L31
*/
struct aes_key_st {
    uint32_t rd_key[4 * (AES_MAXNR + 1)];
    int rounds;
};
typedef struct aes_key_st AES_KEY;

/*!
@brief Set the key to be used for an AES context.
@note Stubbed version of AES_set_encrypt_key from openssl.
*/
int pqriscv_aes_set_encrypt_key (
    const unsigned char *userKey,
    const int bits,
    AES_KEY *key
);

/*!
@brief Perform a 256 bit ECB mode encryption of a single AES block under the
    given key.
@param in key   - The 32-byte key.
@param in plain - The 32-byte plaintext.
@param out ctxt - The output ciphertext.
*/
void pqriscv_aes_256_ecb (
    unsigned char *key, 
    unsigned char *ctr,
    unsigned char *ctxt
);


#endif
