#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "api.h"
#include "randombytes.h"
#include "performance.h"

#define MAXMLEN 2048

// static void printbytes(const uint8_t *x, uint32_t xlen) {
//     uint32_t i;
//     for (i = 0; i < xlen; i++) {
//         printf("%02x", x[i]);
//     }
//     printf("\n");
// }

// void sprint_ken(uint8_t* key, uint32_t klen, uint32_t if_sk)
// {
//     uint8_t sk[PQCLEAN_FALCON512_CLEAN_CRYPTO_SECRETKEYBYTES] = "5923f0bc0c5102f0017e077e83101ff61be0bf000ffe085fbf03b1b81c207e17d03efff001ffbf77004fcce7ddc2e3ff81f0313e006fbd0fcf820090c00fef43ffef820fefbaefedc0f7fec60430be102f01fba17e1c4fbbffe000f84181e3f07ef00180f83ffff7dffcf7af430850061bd082104fbc342fc0141fc41bef7fd00049147107d861821ff2bc2fbfbef051010bf07c07efff00203d0c4efefbb083081040f00f04008f7e2400c1e800800460400fbfbcfbce82040184102f7d002fc60bbf7fec0f02ffcf43080f3f2410400c7041f8510107f0c2fc6ef700103a0c0f3e07b07e13f1c00baf7ffbef8003df41f03f7de8403e03df4723ffb9075f08180142f400800bd00207e1010c2f7df4107e0bbffd07c0421070be1c0efdf3ef3f13f0860060ff07a03cfc5e40ff92fefbfec003e07e0ff0befc2ebee41dbc03df7b03e005f83efdf3cf80fbcec0ebe1801c003fec7078fff0001420c20040bbf45ffe1421c600308003a07bf8607fec00c0f43ffff83fc10030b9e44f49f03efe1faff913edc4f42f7b004fbe1c0f80080f79e7f0411c503b18013c00303df3efbcf43ec9245e81ffc03f0450fd0420c1fb92c4fc1ebef0007f03ff81e40e792fb03df3ee45ffcefd03f08d0441c21850bbeff141ff80baf4313f1fa03b143f4103c17d1bf041080efbe82f84081143fc7040f3b1bf280f8623f13cf02f781bde051fe183ec01440fbf8823d001e7f0040bd080fc613a084f83fc4f76f7ffbe0c6e80e8213eec1f7c03ffc004003ce42001e44fc40f313df46ffaf86f7b0c6f03105fc2f3fec110218107efbef3f1071fdfbd03b08503dfc207a004f40f7a07defb085ef7ec0042084046f7fdb91bd13c201f8303bf7d0400400c0f40f4208407d0f800003dfbe042fc1fb91780baec2ec1e0007cec00c2043000040f790fe144f010050c2eb8ffcf430bf101ec1fc1efef3e13af800bc0c707c04204ad0408208217e0400bf17de01dbb086f3eec414013be41f7e0c103ff3e004ec3f7f080fc1f02fc107b079ebef04efd204fc7ec1145fbef82fff0c7ebf144104f82139105000af804e90925f8eb05f5def2ef0f0cfffa1910e9f9001cff0414f728174f1914dafd14fbb6160cf5082c2b2811ff2f1d1a0f020eee0afb30fced2e1d30dbf821eee813e8e6e900e8ecbc01190f17e1bfdd12ffdbfcf3f5d2260f2f15e9d5e6e7dbe5fbe7ebf20bf904f40204cfe207da0ce5220904fa0afe2b2026350bf70ef4f902202d11d8cdfedefa1026031e000e03160b14e439fe1235110218effffd2a0718d6ed170f06faf1f3f7082209d6dae9dd04f80230e5f911f5ea0b0715def3fc1edfdde0cd1c08ea0fed03f203fbd5e6f129fcf0f82be70505f3fc080733fb30f0211508eee6f20b22fd17f4f409ebf214fc1111e50f1d19fafee9f1e3f02bfa0700efee030e0efccd0210ef11f622f926edf9cfec0cfbfb3c32ff18ddf2f301bd29e2a9eaf41117f7f62eed05f00815ef1b05fee9cc0ace12262606f3f0fef8ea0c3b1e00ca2c02f70ff526df0e01f1d9ea2d1b1305e509110818e6caeb1b0af9e7f417fd08e4ffead108ff08d703e51ddc030d0302e801f3faf8fc0edcdf0f1707ead9f5da570819ff0ef7191bfaf83d2cf4e51d0c2618cb0ff8e20920ee0be8ea14f1f82b0332f90a12080309140efddbeb0411e2f825ce0be3dd34efea4fbb30b7c7f83011ea03efd4fdeb1b1ffb08ddeb13e31a02d101030efff4f602dd06f011f8d5fff3eef822f4dbe7120afce4111a3f1230170112f3fc20f2d2";
//     uint8_t pk[PQCLEAN_FALCON512_CLEAN_CRYPTO_PUBLICKEYBYTES] = "0945194158bb8b5fb6188515de418db28a3901e80e7e26cebeb08c99873da615781e613c3cf9e45085e3988104c0c3c94079a9f86c47dc092772298111176f0e0b7c1cb471d6800d202a293c99239bbc65a60eca790224f5199c1caea0106cf15c618e1bfa91fbddd78f83740fcb3b4c4a39d24d569367e309adfc72a2ed8c80e18c281ceedc1019c053a9a51fb2ac6a7af114ca14d0647084c5500e1af3507414fdbae9eb3383cfd77e1972986e9dea0b782f9365ad375c7050d2f5ef303d8a2ee3cf1b109e76b01bef82e91bf5f6c2ab2b8f796d4861b8ea60163485c504e35e6899701856cade739c3cb54389f124f046e04a9b1b9fd1e5c7f0e85f9d9e6fd57e8a737a106a546f5819adf49d3401862212004963e15f88325aa2d38dcb2273fada60e04e6e503e7c152fdd44239819835d5e65364a80b3c5e12b2eb5bd2834850572b8c099999c4f81c09ec0e611183e6dcfe34549783ac613ab2ba828a6d097d6d558ff11304e4a29d3c42d0085b81616f7747599610668386108d0950b64b9bb11b04061e56465c8c75243e0757e8564e64d6c371efd18e7dfaab92d9a628bc39d8552018ee9196ff5899942c44520dea3a64b22029f1c240ef756ae6c3776c93106ff4fe5349daa57cf5cfd8a34b1f927174b5ce637677ae43a8eac9c45be4e75a9d5ce5ae066afa6c1b5137806748b611c4bd667c3b79262ebbbe7273b8c18354c84a8b2256586a6d55c47fd97302163016e02bfaadad30d9d32fb96f32ef21e1db7c4b72e6cad2e16da9eae8294801d86de676c853633281645e235ae9d6bd9c25e91497c4bf35d381261eba32fe95e6ec928fa4316559d9ce4b4c840d1689192234a4b6339e67074f3c428be51c815b4158f26858ff14803d31e1d99508f45a51c55a2829ecc7718604db2c24b48046d54d16c47c48d4a557aa5a7406b14f7c9531ea0940160c832099b7228a7703801c1512412258cac6a98c287cbb50fd01b456c4a760d1bad2e6f34f2cc0e6776ee52d5083f3ac4fd2396c2952d9e435b9555cf4ecaaef901dd6f9c8626474557394f28f5464fdc4f591d1656add9550e9e74198c997093c9dd4455685cbf7e89d700a03c619866894e1d8d35096ea8e36a538f25f65a4117e685152bf144e3fc987c4a919ba0f05522d2f19265294c61d476aa84337839d925cdc1b521896768e0da8b03047f094d630d3db644cac590d73546bad761172c7f7109b8fd56b9337651d80ce5c929247f8696d37d";
//     if (if_sk)
//     {
//         strcpy((char*)key,(char*)sk);
//     }
//     else
//     {
//         strcpy((char*)key,(char*)pk);
//     }
// }

int main(void) {
    uint8_t sk[PQCLEAN_FALCON512_CLEAN_CRYPTO_SECRETKEYBYTES];
    uint8_t pk[PQCLEAN_FALCON512_CLEAN_CRYPTO_PUBLICKEYBYTES];

    uint8_t mi[MAXMLEN];
    uint8_t sig[PQCLEAN_FALCON512_CLEAN_CRYPTO_BYTES];

    uint32_t smlen;
    uint32_t siglen;
    uint32_t mlen;

    int r;
    uint32_t i=1023, k;

    randombytes(mi, i);

    cycles_keypair = perf_rdcycle();
    instrs_keypair = perf_rdinstret();
    // printf("\t\tI am here!\n");
    PQCLEAN_FALCON512_CLEAN_crypto_sign_keypair(pk, sk);
    // sprint_ken(sk,PQCLEAN_FALCON512_CLEAN_CRYPTO_SECRETKEYBYTES,1);
    // sprint_ken(pk,PQCLEAN_FALCON512_CLEAN_CRYPTO_PUBLICKEYBYTES,0);
    // printf("         keypair_cycles: %10lu\n",cycles_keypair);
    // printf("         keypair_instrs: %10lu\n",instrs_keypair);
    cycles_keypair = perf_rdcycle() - cycles_keypair;
    instrs_keypair = perf_rdinstret() - instrs_keypair;
    printf("\t\t-pk:\n");
    print_hex(pk,PQCLEAN_FALCON512_CLEAN_CRYPTO_PUBLICKEYBYTES);
    printf("\t\t-sk:\n");
    print_hex(sk,PQCLEAN_FALCON512_CLEAN_CRYPTO_SECRETKEYBYTES);


    cycles_signature = perf_rdcycle();
    instrs_signature = perf_rdinstret();
    PQCLEAN_FALCON512_CLEAN_crypto_sign_signature(sig, &siglen, mi, i, sk);
    // printf("       signature_cycles: %10lu\n",cycles_signature);
    // printf("       signature_instrs: %10lu\n",instrs_signature);
    cycles_signature = perf_rdcycle() - cycles_signature;
    instrs_signature = perf_rdinstret() - instrs_signature;
    printf("\t\t-mi:\n");
    print_hex(mi,i);
    printf("\t\t-sig:\n");
    print_hex(sig,siglen);

    cycles_verify = perf_rdcycle() - cycles_verify;
    instrs_verify = perf_rdinstret() - instrs_verify;
    r = PQCLEAN_FALCON512_CLEAN_crypto_sign_verify(sig, siglen, mi, i, pk);
    cycles_verify = perf_rdcycle() - cycles_verify;
    instrs_verify = perf_rdinstret() - instrs_verify;
    if (r) {
        printf("ERROR: signature verification failed\n");
        return -1;
    }
    printf("\t\tI am here!\n");
    // printf("          verify_cycles: %10lu\n",cycles_verify);
    // printf("          verify_instrs: %10lu\n",instrs_verify);
    // printf("-----\n");

    // printf("                   fft:  %10lu\n",times_fft); 
    // printf("             fft_cycles: %10lu\n",cycles_fft);
    // printf("             fft_instrs: %10lu\n",instrs_fft);

    // printf("                  ifft:  %10lu\n",times_ifft); 
    // printf("            ifft_cycles: %10lu\n",cycles_ifft);
    // printf("            ifft_instrs: %10lu\n",instrs_ifft);

    // printf("                   ntt:  %10lu\n",times_ntt); 
    // printf("             ntt_cycles: %10lu\n",cycles_ntt);
    // printf("             ntt_instrs: %10lu\n",instrs_ntt);

    // printf("                  intt:  %10lu\n",times_intt); 
    // printf("            intt_cycles: %10lu\n",cycles_intt);
    // printf("            intt_instrs: %10lu\n",instrs_intt);
    // printf("-----\n");

    return 0;
}
