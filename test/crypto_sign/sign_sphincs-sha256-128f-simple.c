#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "api.h"
#include "randombytes.h"
#include "performance.h"

#define MAXMLEN 2048



int main(void) {
    uint8_t sk[PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_CRYPTO_SECRETKEYBYTES];
    uint8_t pk[PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_CRYPTO_PUBLICKEYBYTES];

    uint8_t mi[MAXMLEN];
    uint8_t sig[PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_CRYPTO_BYTES];

    uint32_t smlen;
    uint32_t siglen;
    uint32_t mlen;

    int r;
    uint32_t i=1024, k;

    printf("           CRYPTO_BYTES: %10lu\n",i);
    printf("  CRYPTO_SIGNATUREBYTES: %10lu\n",PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_CRYPTO_BYTES);
    printf("  CRYPTO_SECRETKEYBYTES: %10lu\n",PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_CRYPTO_SECRETKEYBYTES);
    printf("  CRYPTO_PUBLICKEYBYTES: %10lu\n",PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_CRYPTO_PUBLICKEYBYTES);

    randombytes(mi, i);
    //printbytes(mi,i);

    cycles_keypair = perf_rdcycle();
    instrs_keypair = perf_rdinstret();
    PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_crypto_sign_keypair(pk, sk);
    // printf("   - pk:\n");
    // print_hex(pk,PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_CRYPTO_PUBLICKEYBYTES);
    // printf("\n");
    // printf("   - sk:\n");
    // print_hex(sk,PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_CRYPTO_SECRETKEYBYTES);
    // printf("\n");
    printf("         keypair_cycles: %10lu\n",cycles_keypair);
    printf("         keypair_instrs: %10lu\n",instrs_keypair);
    cycles_keypair = perf_rdcycle() - cycles_keypair;
    instrs_keypair = perf_rdinstret() - instrs_keypair;

    cycles_signature = perf_rdcycle();
    instrs_signature = perf_rdinstret();
    PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_crypto_sign_signature(sig, &siglen, mi, i, sk);
    printf("       signature_cycles: %10lu\n",cycles_signature);
    printf("       signature_instrs: %10lu\n",instrs_signature);
    // printf("   - sig:\n");
    // print_hex(sig,siglen);
    // printf("\n");
    // printf("   - mi:\n");
    // print_hex(mi,i);
    // printf("\n");

    cycles_signature = perf_rdcycle() - cycles_signature;
    instrs_signature = perf_rdinstret() - instrs_signature;

    cycles_verify = perf_rdcycle() - cycles_verify;
    instrs_verify = perf_rdinstret() - instrs_verify;
    r = PQCLEAN_SPHINCSSHA256128FSIMPLE_CLEAN_crypto_sign_verify(sig, siglen, mi, i, pk);
    cycles_verify = perf_rdcycle() - cycles_verify;
    instrs_verify = perf_rdinstret() - instrs_verify;
    if (r) {
        printf("ERROR: signature verification failed\n");
        return -1;
    }
    printf("          verify_cycles: %10lu\n",cycles_verify);
    printf("          verify_instrs: %10lu\n",instrs_verify);
    printf("-----\n");

    printf("                   fft:  %10lu\n",times_fft); 
    printf("             fft_cycles: %10lu\n",cycles_fft);
    printf("             fft_instrs: %10lu\n",instrs_fft);

    printf("                  ifft:  %10lu\n",times_ifft); 
    printf("            ifft_cycles: %10lu\n",cycles_ifft);
    printf("            ifft_instrs: %10lu\n",instrs_ifft);

    printf("                   ntt:  %10lu\n",times_ntt); 
    printf("             ntt_cycles: %10lu\n",cycles_ntt);
    printf("             ntt_instrs: %10lu\n",instrs_ntt);

    printf("                  intt:  %10lu\n",times_intt); 
    printf("            intt_cycles: %10lu\n",cycles_intt);
    printf("            intt_instrs: %10lu\n",instrs_intt);
    printf("-----\n");

    return 0;
}
