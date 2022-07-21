

#include <stdio.h>
#include <stdlib.h>

// The KEM API header as specified by NIST. See the "-I" flags in the
// build string to see where this comes from: candidates/<scheme>/<var>/
#include "api.h"

// Performance measurement
#include "performance.h"

// #ifndef KEM_NAME
//     #error Please define KEM_NAME
// #endif

// #ifndef KEM_VARIANT
//     #error Please define KEM_VARIANT
// #endif

int main(int argc, char ** argv) {

    unsigned int repeat_count = 100;

    printf("---\n");
    printf("# PQRISCV Test Harness Program\n");

    //printf("- kem: "); printf(KEM_NAME); printf("\n");
    printf("  CRYPTO_SECRETKEYBYTES : %d\n", PQCLEAN_KYBER768_CLEAN_CRYPTO_SECRETKEYBYTES );
    printf("  CRYPTO_PUBLICKEYBYTES : %d\n", PQCLEAN_KYBER768_CLEAN_CRYPTO_PUBLICKEYBYTES );
    printf("  CRYPTO_CIPHERTEXTBYTES: %d\n", PQCLEAN_KYBER768_CLEAN_CRYPTO_CIPHERTEXTBYTES);
    printf("  CRYPTO_BYTES          : %d\n", PQCLEAN_KYBER768_CLEAN_CRYPTO_BYTES          );
    //printf("  - var: "); printf(KEM_VARIANT); printf("\n");

    unsigned char sk [PQCLEAN_KYBER768_CLEAN_CRYPTO_SECRETKEYBYTES  ];
    unsigned char pk [PQCLEAN_KYBER768_CLEAN_CRYPTO_PUBLICKEYBYTES  ];
    unsigned char ct [PQCLEAN_KYBER768_CLEAN_CRYPTO_CIPHERTEXTBYTES ];
    unsigned char ss [PQCLEAN_KYBER768_CLEAN_CRYPTO_BYTES           ];

    // read_random(ss, PQCLEAN_KYBER768_CLEAN_CRYPTO_BYTES);
    // printf("   - ss:\n");
    // print_hex(ss,PQCLEAN_KYBER768_CLEAN_CRYPTO_BYTES);
    // printf("\n");

    uint32_t cycles_keypair;
    uint32_t cycles_enc;
    uint32_t cycles_dec;
    uint32_t instrs_keypair;
    uint32_t instrs_enc;
    uint32_t instrs_dec;
    
    printf("      - keypair:\n");
    cycles_keypair = perf_rdcycle();
    instrs_keypair = perf_rdinstret();
    PQCLEAN_KYBER768_CLEAN_crypto_kem_keypair(pk, sk);
    cycles_keypair = perf_rdcycle()   - cycles_keypair;
    instrs_keypair = perf_rdinstret() - instrs_keypair;

    printf("          cycles: %10lu\n", cycles_keypair);
    printf("          instrs: %10lu\n", instrs_keypair);

    printf("   - sk:\n");
    print_hex(sk,PQCLEAN_KYBER768_CLEAN_CRYPTO_SECRETKEYBYTES);
    printf("\n");
    printf("   - pk:\n");
    print_hex(pk,PQCLEAN_KYBER768_CLEAN_CRYPTO_PUBLICKEYBYTES);
    printf("\n");

    printf("        encrypt:\n");
    cycles_enc     = perf_rdcycle();
    instrs_enc     = perf_rdinstret();
    PQCLEAN_KYBER768_CLEAN_crypto_kem_enc(ct ,ss, pk);
    cycles_enc     = perf_rdcycle()   - cycles_enc    ;
    instrs_enc     = perf_rdinstret() - instrs_enc    ;

    printf("          cycles: %10lu\n", cycles_enc);
    printf("          instrs: %10lu\n", instrs_enc);

    printf("   - ct:\n");
    print_hex(ct,PQCLEAN_KYBER768_CLEAN_CRYPTO_CIPHERTEXTBYTES);
    printf("\n");
    printf("  - ss1:\n");
    print_hex(ss,PQCLEAN_KYBER768_CLEAN_CRYPTO_BYTES);
    printf("\n");

    printf("        decrypt:\n");
    cycles_dec     = perf_rdcycle();
    instrs_dec     = perf_rdinstret();
    PQCLEAN_KYBER768_CLEAN_crypto_kem_dec(ss, ct, sk);
    cycles_dec     = perf_rdcycle()   - cycles_dec    ;
    instrs_dec     = perf_rdinstret() - instrs_dec    ;
    
    printf("          cycles: %10lu\n", cycles_dec);
    printf("          instrs: %10lu\n", instrs_dec);

    printf("  - ss2:\n");
    print_hex(ss,PQCLEAN_KYBER768_CLEAN_CRYPTO_BYTES);
    printf("\n");
    printf("---\nn");

    return 0;

}

