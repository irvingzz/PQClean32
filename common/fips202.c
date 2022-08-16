/* Based on the public domain implementation in
 * crypto_hash/keccakc512/simple/ from http://bench.cr.yp.to/supercop.html
 * by Ronny Van Keer
 * and the public domain "TweetFips202" implementation
 * from https://twitter.com/tweetfips202
 * by Gilles Van Assche, Daniel J. Bernstein, and Peter Schwabe */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "fips202.h"
#include "performance.h"

#define NROUNDS 24
#define ROL(a, offset) (rol(a, offset))

/*************************************************
 * Name:        load64
 *
 * Description: Load 8 bytes into uint64_s in little-endian order
 *
 * Arguments:   - const uint8_t *x: pointer to input byte array
 *
 * Returns the loaded 64-bit unsigned integer in the former of uint64_s
 **************************************************/
static uint64_s load64(const uint8_t *x) {
    uint64_s r;
    r.t[1] = 0;
    r.t[0] = 0;
    for (uint32_t i = 0; i < 4; ++i) {
        r.t[0] |= (uint32_t)x[i] << 8 * i;
        r.t[1] |= (uint32_t)x[i+4] << 8 * i;
    }
    return r;
}

/*************************************************
 * Name:        store64
 *
 * Description: Store a uint64_s to a byte array in little-endian order
 *
 * Arguments:   - uint8_t *x: pointer to the output byte array
 *              - uint64_s u: input 64-bit unsigned integer in the former of uint64_s
 **************************************************/
static void store64(uint8_t *x, uint64_s u) {
    for (uint32_t i = 0; i < 4; ++i) {
        x[i]   = (uint8_t) (u.t[0] >> 8 * i);
        x[i+4] = (uint8_t) (u.t[1] >> 8 * i);
    }
}

/* Keccak round constants */
static const uint64_s KeccakF_RoundConstants[NROUNDS] = {
    (uint64_s){0x00000001,0x00000000},
    (uint64_s){0x00008082,0x00000000},
    (uint64_s){0x0000808a,0x80000000},
    (uint64_s){0x80008000,0x80000000},
    (uint64_s){0x0000808b,0x00000000},
    (uint64_s){0x80000001,0x00000000},
    (uint64_s){0x80008081,0x80000000},
    (uint64_s){0x00008009,0x80000000},
    (uint64_s){0x0000008a,0x00000000},
    (uint64_s){0x00000088,0x00000000},
    (uint64_s){0x80008009,0x00000000},
    (uint64_s){0x8000000a,0x00000000},
    (uint64_s){0x8000808b,0x00000000},
    (uint64_s){0x0000008b,0x80000000},
    (uint64_s){0x00008089,0x80000000},
    (uint64_s){0x00008003,0x80000000},
    (uint64_s){0x00008002,0x80000000},
    (uint64_s){0x00000080,0x80000000},
    (uint64_s){0x0000800a,0x00000000},
    (uint64_s){0x8000000a,0x80000000},
    (uint64_s){0x80008081,0x80000000},
    (uint64_s){0x00008080,0x80000000},
    (uint64_s){0x80000001,0x00000000},
    (uint64_s){0x80008008,0x80000000}
};

/*************************************************
 * Name:        KeccakF1600_StatePermute
 *
 * Description: The Keccak F1600 Permutation
 *
 * Arguments:   - uint64_s *state: pointer to input/output Keccak state
 **************************************************/
void KeccakF1600_StatePermute(uint64_s * state)
{
  // printf("KeccakF1600_StatePermute:\n");
  // print_uint64_s(state, 25);
  int round;

        uint64_s Aba, Abe, Abi, Abo, Abu;
        uint64_s Aga, Age, Agi, Ago, Agu;
        uint64_s Aka, Ake, Aki, Ako, Aku;
        uint64_s Ama, Ame, Ami, Amo, Amu;
        uint64_s Asa, Ase, Asi, Aso, Asu;
        uint64_s BCa, BCe, BCi, BCo, BCu;
        uint64_s Da, De, Di, Do, Du;
        uint64_s Eba, Ebe, Ebi, Ebo, Ebu;
        uint64_s Ega, Ege, Egi, Ego, Egu;
        uint64_s Eka, Eke, Eki, Eko, Eku;
        uint64_s Ema, Eme, Emi, Emo, Emu;
        uint64_s Esa, Ese, Esi, Eso, Esu;

        //copyFromState(A, state)
        Aba = state[ 0];
        Abe = state[ 1];
        Abi = state[ 2];
        Abo = state[ 3];
        Abu = state[ 4];
        Aga = state[ 5];
        Age = state[ 6];
        Agi = state[ 7];
        Ago = state[ 8];
        Agu = state[ 9];
        Aka = state[10];
        Ake = state[11];
        Aki = state[12];
        Ako = state[13];
        Aku = state[14];
        Ama = state[15];
        Ame = state[16];
        Ami = state[17];
        Amo = state[18];
        Amu = state[19];
        Asa = state[20];
        Ase = state[21];
        Asi = state[22];
        Aso = state[23];
        Asu = state[24];

        for( round = 0; round < NROUNDS; round += 2 )
        {
            //    prepareTheta
            BCa.t[0] = Aba.t[0]^Aga.t[0]^Aka.t[0]^Ama.t[0]^Asa.t[0];
            BCe.t[0] = Abe.t[0]^Age.t[0]^Ake.t[0]^Ame.t[0]^Ase.t[0];
            BCi.t[0] = Abi.t[0]^Agi.t[0]^Aki.t[0]^Ami.t[0]^Asi.t[0];
            BCo.t[0] = Abo.t[0]^Ago.t[0]^Ako.t[0]^Amo.t[0]^Aso.t[0];
            BCu.t[0] = Abu.t[0]^Agu.t[0]^Aku.t[0]^Amu.t[0]^Asu.t[0];

            BCa.t[1] = Aba.t[1]^Aga.t[1]^Aka.t[1]^Ama.t[1]^Asa.t[1];
            BCe.t[1] = Abe.t[1]^Age.t[1]^Ake.t[1]^Ame.t[1]^Ase.t[1];
            BCi.t[1] = Abi.t[1]^Agi.t[1]^Aki.t[1]^Ami.t[1]^Asi.t[1];
            BCo.t[1] = Abo.t[1]^Ago.t[1]^Ako.t[1]^Amo.t[1]^Aso.t[1];
            BCu.t[1] = Abu.t[1]^Agu.t[1]^Aku.t[1]^Amu.t[1]^Asu.t[1];
            // printf("ROUND %2d:\n\n",round);
            // printf("BCa-u ^ :\n");
            // print_uint64_s(&BCa, 1);
            // print_uint64_s(&BCe, 1);
            // print_uint64_s(&BCi, 1);
            // print_uint64_s(&BCo, 1);
            // print_uint64_s(&BCu, 1);

            //thetaRhoPiChiIotaPrepareTheta(round  , A, E)
            Da = rol(BCe, 1);
            De = rol(BCi, 1);
            Di = rol(BCo, 1);
            Do = rol(BCu, 1);
            Du = rol(BCa, 1);

            Da.t[0] ^= BCu.t[0];
            De.t[0] ^= BCa.t[0];
            Di.t[0] ^= BCe.t[0];
            Do.t[0] ^= BCi.t[0];
            Du.t[0] ^= BCo.t[0];

            Da.t[1] ^= BCu.t[1];
            De.t[1] ^= BCa.t[1];
            Di.t[1] ^= BCe.t[1];
            Do.t[1] ^= BCi.t[1];
            Du.t[1] ^= BCo.t[1];

            // printf("Da-u rol&^ :\n");
            // print_uint64_s(&Da, 1);
            // print_uint64_s(&De, 1);
            // print_uint64_s(&Di, 1);
            // print_uint64_s(&Do, 1);
            // print_uint64_s(&Du, 1);

            Aba.t[0] ^= Da.t[0];
            Aba.t[1] ^= Da.t[1];
            BCa = Aba;
            Age.t[0] ^= De.t[0];
            Age.t[1] ^= De.t[1];
            BCe = rol(Age, 44);
            Aki.t[0] ^= Di.t[0];
            Aki.t[1] ^= Di.t[1];
            BCi = rol(Aki, 43);
            Amo.t[0] ^= Do.t[0];
            Amo.t[1] ^= Do.t[1];
            BCo = rol(Amo, 21);
            Asu.t[0] ^= Du.t[0];
            Asu.t[1] ^= Du.t[1];
            BCu = rol(Asu, 14);
            // printf("BCa-i &^~ :\n");
            // print_uint64_s(&BCa, 1);
            // print_uint64_s(&BCe, 1);
            // print_uint64_s(&BCi, 1);
            Eba.t[0] =   BCa.t[0] ^((~BCe.t[0])&  BCi.t[0] );
            Eba.t[1] =   BCa.t[1] ^((~BCe.t[1])&  BCi.t[1] );
            Eba.t[0] ^= KeccakF_RoundConstants[round].t[0];
            Eba.t[1] ^= KeccakF_RoundConstants[round].t[1];
            Ebe.t[0] =   BCe.t[0] ^((~BCi.t[0])&  BCo.t[0] );
            Ebe.t[1] =   BCe.t[1] ^((~BCi.t[1])&  BCo.t[1] );
            Ebi.t[0] =   BCi.t[0] ^((~BCo.t[0])&  BCu.t[0] );
            Ebi.t[1] =   BCi.t[1] ^((~BCo.t[1])&  BCu.t[1] );
            Ebo.t[0] =   BCo.t[0] ^((~BCu.t[0])&  BCa.t[0] );
            Ebo.t[1] =   BCo.t[1] ^((~BCu.t[1])&  BCa.t[1] );
            Ebu.t[0] =   BCu.t[0] ^((~BCa.t[0])&  BCe.t[0] );
            Ebu.t[1] =   BCu.t[1] ^((~BCa.t[1])&  BCe.t[1] );

            // printf("Eba-u rol&^~ :\n");
            // print_uint64_s(&Eba, 1);
            // print_uint64_s(&Ebe, 1);
            // print_uint64_s(&Ebi, 1);
            // print_uint64_s(&Ebo, 1);
            // print_uint64_s(&Ebu, 1);

            Abo.t[0] ^= Do.t[0];
            Abo.t[1] ^= Do.t[1];
            BCa = rol(Abo, 28);
            Agu.t[0] ^= Du.t[0];
            Agu.t[1] ^= Du.t[1];
            BCe = rol(Agu, 20);
            Aka.t[0] ^= Da.t[0];
            Aka.t[1] ^= Da.t[1];
            BCi = rol(Aka,  3);
            Ame.t[0] ^= De.t[0];
            Ame.t[1] ^= De.t[1];
            BCo = rol(Ame, 45);
            Asi.t[0] ^= Di.t[0];
            Asi.t[1] ^= Di.t[1];
            BCu = rol(Asi, 61);
            Ega.t[0] =   BCa.t[0] ^((~BCe.t[0])&  BCi.t[0] );
            Ega.t[1] =   BCa.t[1] ^((~BCe.t[1])&  BCi.t[1] );
            Ege.t[0] =   BCe.t[0] ^((~BCi.t[0])&  BCo.t[0] );
            Ege.t[1] =   BCe.t[1] ^((~BCi.t[1])&  BCo.t[1] );
            Egi.t[0] =   BCi.t[0] ^((~BCo.t[0])&  BCu.t[0] );
            Egi.t[1] =   BCi.t[1] ^((~BCo.t[1])&  BCu.t[1] );
            Ego.t[0] =   BCo.t[0] ^((~BCu.t[0])&  BCa.t[0] );
            Ego.t[1] =   BCo.t[1] ^((~BCu.t[1])&  BCa.t[1] );
            Egu.t[0] =   BCu.t[0] ^((~BCa.t[0])&  BCe.t[0] );
            Egu.t[1] =   BCu.t[1] ^((~BCa.t[1])&  BCe.t[1] );

            // printf("Ega-u rol&^~ :\n");
            // print_uint64_s(&Ega, 1);
            // print_uint64_s(&Ege, 1);
            // print_uint64_s(&Egi, 1);
            // print_uint64_s(&Ego, 1);
            // print_uint64_s(&Egu, 1);

            Abe.t[0] ^= De.t[0];
            Abe.t[1] ^= De.t[1];
            BCa = rol(Abe,  1);
            Agi.t[0] ^= Di.t[0];
            Agi.t[1] ^= Di.t[1];
            BCe = rol(Agi,  6);
            Ako.t[0] ^= Do.t[0];
            Ako.t[1] ^= Do.t[1];
            BCi = rol(Ako, 25);
            Amu.t[0] ^= Du.t[0];
            Amu.t[1] ^= Du.t[1];
            BCo = rol(Amu,  8);
            Asa.t[0] ^= Da.t[0];
            Asa.t[1] ^= Da.t[1];
            BCu = rol(Asa, 18);
            Eka.t[0] =   BCa.t[0] ^((~BCe.t[0])&  BCi.t[0] );
            Eka.t[1] =   BCa.t[1] ^((~BCe.t[1])&  BCi.t[1] );
            Eke.t[0] =   BCe.t[0] ^((~BCi.t[0])&  BCo.t[0] );
            Eke.t[1] =   BCe.t[1] ^((~BCi.t[1])&  BCo.t[1] );
            Eki.t[0] =   BCi.t[0] ^((~BCo.t[0])&  BCu.t[0] );
            Eki.t[1] =   BCi.t[1] ^((~BCo.t[1])&  BCu.t[1] );
            Eko.t[0] =   BCo.t[0] ^((~BCu.t[0])&  BCa.t[0] );
            Eko.t[1] =   BCo.t[1] ^((~BCu.t[1])&  BCa.t[1] );
            Eku.t[0] =   BCu.t[0] ^((~BCa.t[0])&  BCe.t[0] );
            Eku.t[1] =   BCu.t[1] ^((~BCa.t[1])&  BCe.t[1] );

            // printf("Eka-u rol&^~ :\n");
            // print_uint64_s(&Eka, 1);
            // print_uint64_s(&Eke, 1);
            // print_uint64_s(&Eki, 1);
            // print_uint64_s(&Eko, 1);
            // print_uint64_s(&Eku, 1);

            Abu.t[0] ^= Du.t[0];
            Abu.t[1] ^= Du.t[1];
            BCa = rol(Abu, 27);
            Aga.t[0] ^= Da.t[0];
            Aga.t[1] ^= Da.t[1];
            BCe = rol(Aga, 36);
            Ake.t[0] ^= De.t[0];
            Ake.t[1] ^= De.t[1];
            BCi = rol(Ake, 10);
            Ami.t[0] ^= Di.t[0];
            Ami.t[1] ^= Di.t[1];
            BCo = rol(Ami, 15);
            Aso.t[0] ^= Do.t[0];
            Aso.t[1] ^= Do.t[1];
            BCu = rol(Aso, 56);
            Ema.t[0] =   BCa.t[0] ^((~BCe.t[0])&  BCi.t[0] );
            Ema.t[1] =   BCa.t[1] ^((~BCe.t[1])&  BCi.t[1] );
            Eme.t[0] =   BCe.t[0] ^((~BCi.t[0])&  BCo.t[0] );
            Eme.t[1] =   BCe.t[1] ^((~BCi.t[1])&  BCo.t[1] );
            Emi.t[0] =   BCi.t[0] ^((~BCo.t[0])&  BCu.t[0] );
            Emi.t[1] =   BCi.t[1] ^((~BCo.t[1])&  BCu.t[1] );
            Emo.t[0] =   BCo.t[0] ^((~BCu.t[0])&  BCa.t[0] );
            Emo.t[1] =   BCo.t[1] ^((~BCu.t[1])&  BCa.t[1] );
            Emu.t[0] =   BCu.t[0] ^((~BCa.t[0])&  BCe.t[0] );
            Emu.t[1] =   BCu.t[1] ^((~BCa.t[1])&  BCe.t[1] );

            // printf("Ema-u rol&^~ :\n");
            // print_uint64_s(&Ema, 1);
            // print_uint64_s(&Eme, 1);
            // print_uint64_s(&Emi, 1);
            // print_uint64_s(&Emo, 1);
            // print_uint64_s(&Emu, 1);

            Abi.t[0] ^= Di.t[0];
            Abi.t[1] ^= Di.t[1];
            BCa = rol(Abi, 62);
            Ago.t[0] ^= Do.t[0];
            Ago.t[1] ^= Do.t[1];
            BCe = rol(Ago, 55);
            Aku.t[0] ^= Du.t[0];
            Aku.t[1] ^= Du.t[1];
            BCi = rol(Aku, 39);
            Ama.t[0] ^= Da.t[0];
            Ama.t[1] ^= Da.t[1];
            BCo = rol(Ama, 41);
            Ase.t[0] ^= De.t[0];
            Ase.t[1] ^= De.t[1];
            BCu = rol(Ase,  2);
            Esa.t[0] =   BCa.t[0] ^((~BCe.t[0])&  BCi.t[0] );
            Esa.t[1] =   BCa.t[1] ^((~BCe.t[1])&  BCi.t[1] );
            Ese.t[0] =   BCe.t[0] ^((~BCi.t[0])&  BCo.t[0] );
            Ese.t[1] =   BCe.t[1] ^((~BCi.t[1])&  BCo.t[1] );
            Esi.t[0] =   BCi.t[0] ^((~BCo.t[0])&  BCu.t[0] );
            Esi.t[1] =   BCi.t[1] ^((~BCo.t[1])&  BCu.t[1] );
            Eso.t[0] =   BCo.t[0] ^((~BCu.t[0])&  BCa.t[0] );
            Eso.t[1] =   BCo.t[1] ^((~BCu.t[1])&  BCa.t[1] );
            Esu.t[0] =   BCu.t[0] ^((~BCa.t[0])&  BCe.t[0] );
            Esu.t[1] =   BCu.t[1] ^((~BCa.t[1])&  BCe.t[1] );

            // printf("Esa-u rol&^~ :\n");
            // print_uint64_s(&Esa, 1);
            // print_uint64_s(&Ese, 1);
            // print_uint64_s(&Esi, 1);
            // print_uint64_s(&Eso, 1);
            // print_uint64_s(&Esu, 1);

            //    prepareTheta
            BCa.t[0] = Eba.t[0]^Ega.t[0]^Eka.t[0]^Ema.t[0]^Esa.t[0];
            BCe.t[0] = Ebe.t[0]^Ege.t[0]^Eke.t[0]^Eme.t[0]^Ese.t[0];
            BCi.t[0] = Ebi.t[0]^Egi.t[0]^Eki.t[0]^Emi.t[0]^Esi.t[0];
            BCo.t[0] = Ebo.t[0]^Ego.t[0]^Eko.t[0]^Emo.t[0]^Eso.t[0];
            BCu.t[0] = Ebu.t[0]^Egu.t[0]^Eku.t[0]^Emu.t[0]^Esu.t[0];
            BCa.t[1] = Eba.t[1]^Ega.t[1]^Eka.t[1]^Ema.t[1]^Esa.t[1];
            BCe.t[1] = Ebe.t[1]^Ege.t[1]^Eke.t[1]^Eme.t[1]^Ese.t[1];
            BCi.t[1] = Ebi.t[1]^Egi.t[1]^Eki.t[1]^Emi.t[1]^Esi.t[1];
            BCo.t[1] = Ebo.t[1]^Ego.t[1]^Eko.t[1]^Emo.t[1]^Eso.t[1];
            BCu.t[1] = Ebu.t[1]^Egu.t[1]^Eku.t[1]^Emu.t[1]^Esu.t[1];

            // printf("ROUND %2d:\n\n",round);
            // printf("BCa-u ^ :\n");
            // print_uint64_s(&BCa, 1);
            // print_uint64_s(&BCe, 1);
            // print_uint64_s(&BCi, 1);
            // print_uint64_s(&BCo, 1);
            // print_uint64_s(&BCu, 1);

            //thetaRhoPiChiIotaPrepareTheta(round+1, E, A)
            Da = rol(BCe, 1);
            De = rol(BCi, 1);
            Di = rol(BCo, 1);
            Do = rol(BCu, 1);
            Du = rol(BCa, 1);

            Da.t[0] ^= BCu.t[0];
            De.t[0] ^= BCa.t[0];
            Di.t[0] ^= BCe.t[0];
            Do.t[0] ^= BCi.t[0];
            Du.t[0] ^= BCo.t[0];

            Da.t[1] ^= BCu.t[1];
            De.t[1] ^= BCa.t[1];
            Di.t[1] ^= BCe.t[1];
            Do.t[1] ^= BCi.t[1];
            Du.t[1] ^= BCo.t[1];

            // printf("Da-u rol&^ :\n");
            // print_uint64_s(&Da, 1);
            // print_uint64_s(&De, 1);
            // print_uint64_s(&Di, 1);
            // print_uint64_s(&Do, 1);
            // print_uint64_s(&Du, 1);

            Eba.t[0] ^= Da.t[0];
            Eba.t[1] ^= Da.t[1];
            BCa = Eba;
            Ege.t[0] ^= De.t[0];
            Ege.t[1] ^= De.t[1];
            BCe = rol(Ege, 44);
            Eki.t[0] ^= Di.t[0];
            Eki.t[1] ^= Di.t[1];
            BCi = rol(Eki, 43);
            Emo.t[0] ^= Do.t[0];
            Emo.t[1] ^= Do.t[1];
            BCo = rol(Emo, 21);
            Esu.t[0] ^= Du.t[0];
            Esu.t[1] ^= Du.t[1];
            BCu = rol(Esu, 14);
            Aba.t[0] =   BCa.t[0] ^((~BCe.t[0])&  BCi.t[0] );
            Aba.t[1] =   BCa.t[1] ^((~BCe.t[1])&  BCi.t[1] );
            Aba.t[0] ^= KeccakF_RoundConstants[round+1].t[0];
            Aba.t[1] ^= KeccakF_RoundConstants[round+1].t[1];
            Abe.t[0] =   BCe.t[0] ^((~BCi.t[0])&  BCo.t[0] );
            Abe.t[1] =   BCe.t[1] ^((~BCi.t[1])&  BCo.t[1] );
            Abi.t[0] =   BCi.t[0] ^((~BCo.t[0])&  BCu.t[0] );
            Abi.t[1] =   BCi.t[1] ^((~BCo.t[1])&  BCu.t[1] );
            Abo.t[0] =   BCo.t[0] ^((~BCu.t[0])&  BCa.t[0] );
            Abo.t[1] =   BCo.t[1] ^((~BCu.t[1])&  BCa.t[1] );
            Abu.t[0] =   BCu.t[0] ^((~BCa.t[0])&  BCe.t[0] );
            Abu.t[1] =   BCu.t[1] ^((~BCa.t[1])&  BCe.t[1] );

            // printf("Aba-u rol&^~ :\n");
            // print_uint64_s(&Aba, 1);
            // print_uint64_s(&Abe, 1);
            // print_uint64_s(&Abi, 1);
            // print_uint64_s(&Abo, 1);
            // print_uint64_s(&Abu, 1);

            Ebo.t[0] ^= Do.t[0];
            Ebo.t[1] ^= Do.t[1];
            BCa = rol(Ebo, 28);
            Egu.t[0] ^= Du.t[0];
            Egu.t[1] ^= Du.t[1];
            BCe = rol(Egu, 20);
            Eka.t[0] ^= Da.t[0];
            Eka.t[1] ^= Da.t[1];
            BCi = rol(Eka,  3);
            Eme.t[0] ^= De.t[0];
            Eme.t[1] ^= De.t[1];
            BCo = rol(Eme, 45);
            Esi.t[0] ^= Di.t[0];
            Esi.t[1] ^= Di.t[1];
            BCu = rol(Esi, 61);
            Aga.t[0] =   BCa.t[0] ^((~BCe.t[0])&  BCi.t[0] );
            Aga.t[1] =   BCa.t[1] ^((~BCe.t[1])&  BCi.t[1] );
            Age.t[0] =   BCe.t[0] ^((~BCi.t[0])&  BCo.t[0] );
            Age.t[1] =   BCe.t[1] ^((~BCi.t[1])&  BCo.t[1] );
            Agi.t[0] =   BCi.t[0] ^((~BCo.t[0])&  BCu.t[0] );
            Agi.t[1] =   BCi.t[1] ^((~BCo.t[1])&  BCu.t[1] );
            Ago.t[0] =   BCo.t[0] ^((~BCu.t[0])&  BCa.t[0] );
            Ago.t[1] =   BCo.t[1] ^((~BCu.t[1])&  BCa.t[1] );
            Agu.t[0] =   BCu.t[0] ^((~BCa.t[0])&  BCe.t[0] );
            Agu.t[1] =   BCu.t[1] ^((~BCa.t[1])&  BCe.t[1] );

            // printf("Aga-u rol&^~ :\n");
            // print_uint64_s(&Aga, 1);
            // print_uint64_s(&Age, 1);
            // print_uint64_s(&Agi, 1);
            // print_uint64_s(&Ago, 1);
            // print_uint64_s(&Agu, 1);

            Ebe.t[0] ^= De.t[0];
            Ebe.t[1] ^= De.t[1];
            BCa = rol(Ebe,  1);
            Egi.t[0] ^= Di.t[0];
            Egi.t[1] ^= Di.t[1];
            BCe = rol(Egi,  6);
            Eko.t[0] ^= Do.t[0];
            Eko.t[1] ^= Do.t[1];
            BCi = rol(Eko, 25);
            Emu.t[0] ^= Du.t[0];
            Emu.t[1] ^= Du.t[1];
            BCo = rol(Emu,  8);
            Esa.t[0] ^= Da.t[0];
            Esa.t[1] ^= Da.t[1];
            BCu = rol(Esa, 18);
            Aka.t[0] =   BCa.t[0] ^((~BCe.t[0])&  BCi.t[0] );
            Aka.t[1] =   BCa.t[1] ^((~BCe.t[1])&  BCi.t[1] );
            Ake.t[0] =   BCe.t[0] ^((~BCi.t[0])&  BCo.t[0] );
            Ake.t[1] =   BCe.t[1] ^((~BCi.t[1])&  BCo.t[1] );
            Aki.t[0] =   BCi.t[0] ^((~BCo.t[0])&  BCu.t[0] );
            Aki.t[1] =   BCi.t[1] ^((~BCo.t[1])&  BCu.t[1] );
            Ako.t[0] =   BCo.t[0] ^((~BCu.t[0])&  BCa.t[0] );
            Ako.t[1] =   BCo.t[1] ^((~BCu.t[1])&  BCa.t[1] );
            Aku.t[0] =   BCu.t[0] ^((~BCa.t[0])&  BCe.t[0] );
            Aku.t[1] =   BCu.t[1] ^((~BCa.t[1])&  BCe.t[1] );

            // printf("Aka-u rol&^~ :\n");
            // print_uint64_s(&Aka, 1);
            // print_uint64_s(&Ake, 1);
            // print_uint64_s(&Aki, 1);
            // print_uint64_s(&Ako, 1);
            // print_uint64_s(&Aku, 1);

            Ebu.t[0] ^= Du.t[0];
            Ebu.t[1] ^= Du.t[1];
            BCa = rol(Ebu, 27);
            Ega.t[0] ^= Da.t[0];
            Ega.t[1] ^= Da.t[1];
            BCe = rol(Ega, 36);
            Eke.t[0] ^= De.t[0];
            Eke.t[1] ^= De.t[1];
            BCi = rol(Eke, 10);
            Emi.t[0] ^= Di.t[0];
            Emi.t[1] ^= Di.t[1];
            BCo = rol(Emi, 15);
            Eso.t[0] ^= Do.t[0];
            Eso.t[1] ^= Do.t[1];
            BCu = rol(Eso, 56);
            Ama.t[0] =   BCa.t[0] ^((~BCe.t[0])&  BCi.t[0] );
            Ama.t[1] =   BCa.t[1] ^((~BCe.t[1])&  BCi.t[1] );
            Ame.t[0] =   BCe.t[0] ^((~BCi.t[0])&  BCo.t[0] );
            Ame.t[1] =   BCe.t[1] ^((~BCi.t[1])&  BCo.t[1] );
            Ami.t[0] =   BCi.t[0] ^((~BCo.t[0])&  BCu.t[0] );
            Ami.t[1] =   BCi.t[1] ^((~BCo.t[1])&  BCu.t[1] );
            Amo.t[0] =   BCo.t[0] ^((~BCu.t[0])&  BCa.t[0] );
            Amo.t[1] =   BCo.t[1] ^((~BCu.t[1])&  BCa.t[1] );
            Amu.t[0] =   BCu.t[0] ^((~BCa.t[0])&  BCe.t[0] );
            Amu.t[1] =   BCu.t[1] ^((~BCa.t[1])&  BCe.t[1] );

            // printf("Ama-u rol&^~ :\n");
            // print_uint64_s(&Ama, 1);
            // print_uint64_s(&Ame, 1);
            // print_uint64_s(&Ami, 1);
            // print_uint64_s(&Amo, 1);
            // print_uint64_s(&Amu, 1);

            Ebi.t[0] ^= Di.t[0];
            Ebi.t[1] ^= Di.t[1];
            BCa = rol(Ebi, 62);
            Ego.t[0] ^= Do.t[0];
            Ego.t[1] ^= Do.t[1];
            BCe = rol(Ego, 55);
            Eku.t[0] ^= Du.t[0];
            Eku.t[1] ^= Du.t[1];
            BCi = rol(Eku, 39);
            Ema.t[0] ^= Da.t[0];
            Ema.t[1] ^= Da.t[1];
            BCo = rol(Ema, 41);
            Ese.t[0] ^= De.t[0];
            Ese.t[1] ^= De.t[1];
            BCu = rol(Ese,  2);
            Asa.t[0] =   BCa.t[0] ^((~BCe.t[0])&  BCi.t[0] );
            Asa.t[1] =   BCa.t[1] ^((~BCe.t[1])&  BCi.t[1] );
            Ase.t[0] =   BCe.t[0] ^((~BCi.t[0])&  BCo.t[0] );
            Ase.t[1] =   BCe.t[1] ^((~BCi.t[1])&  BCo.t[1] );
            Asi.t[0] =   BCi.t[0] ^((~BCo.t[0])&  BCu.t[0] );
            Asi.t[1] =   BCi.t[1] ^((~BCo.t[1])&  BCu.t[1] );
            Aso.t[0] =   BCo.t[0] ^((~BCu.t[0])&  BCa.t[0] );
            Aso.t[1] =   BCo.t[1] ^((~BCu.t[1])&  BCa.t[1] );
            Asu.t[0] =   BCu.t[0] ^((~BCa.t[0])&  BCe.t[0] );
            Asu.t[1] =   BCu.t[1] ^((~BCa.t[1])&  BCe.t[1] );

            // printf("Asa-u rol&^~ :\n");
            // print_uint64_s(&Asa, 1);
            // print_uint64_s(&Ase, 1);
            // print_uint64_s(&Asi, 1);
            // print_uint64_s(&Aso, 1);
            // print_uint64_s(&Asu, 1);
        }

        //copyToState(state, A)
        state[ 0] = Aba;
        state[ 1] = Abe;
        state[ 2] = Abi;
        state[ 3] = Abo;
        state[ 4] = Abu;
        state[ 5] = Aga;
        state[ 6] = Age;
        state[ 7] = Agi;
        state[ 8] = Ago;
        state[ 9] = Agu;
        state[10] = Aka;
        state[11] = Ake;
        state[12] = Aki;
        state[13] = Ako;
        state[14] = Aku;
        state[15] = Ama;
        state[16] = Ame;
        state[17] = Ami;
        state[18] = Amo;
        state[19] = Amu;
        state[20] = Asa;
        state[21] = Ase;
        state[22] = Asi;
        state[23] = Aso;
        state[24] = Asu;
  // printf("KeccakF1600_StatePermute:\n");
  // print_uint64_s(state, 25);
        #undef    round
}

/*************************************************
 * Name:        keccak_absorb
 *
 * Description: Absorb step of Keccak;
 *              non-incremental, starts by zeroeing the state.
 *
 * Arguments:   - uint64_s *s: pointer to (uninitialized) output Keccak state
 *              - uint32_t r: rate in bytes (e.g., 168 for SHAKE128)
 *              - const uint8_t *m: pointer to input to be absorbed into s
 *              - uint32_t mlen: length of input in bytes
 *              - uint8_t p: domain-separation byte for different
 *                                 Keccak-derived functions
 **************************************************/
static void keccak_absorb(uint64_s *s, uint32_t r, const uint8_t *m,
                          uint32_t mlen, uint8_t p) {
    uint32_t i;
    uint8_t t[200];
    uint64_s tmp;
    // printf("   - m:\n");
    // print_hex(m,mlen);
    // printf("\n");
    
    /* Zero state */
    for (i = 0; i < 25; ++i) {
        s[i].t[0] = 0;
        s[i].t[1] = 0;
    }

    while (mlen >= r) {
        for (i = 0; i < r / 8; ++i) {
            tmp = load64(m + 8 * i);
            s[i].t[0] ^= tmp.t[0];
            s[i].t[1] ^= tmp.t[1];
        }

        KeccakF1600_StatePermute(s);
        mlen -= r;
        m += r;
    }

    for (i = 0; i < r; ++i) {
        t[i] = 0;
    }
    for (i = 0; i < mlen; ++i) {
        t[i] = m[i];
    }
    t[i] = p;
    t[r - 1] |= 128;
    // printf("   - t:\n");
    // print_hex(t,200);
    // printf("\n");
    for (i = 0; i < r / 8; ++i) {
        tmp = load64(t + 8 * i);
        s[i].t[0] ^= tmp.t[0];
        s[i].t[1] ^= tmp.t[1];
    }
    // printf("   - s:\n");
    // print_uint64_s(s,25);
    // printf("\n");
}

/*************************************************
 * Name:        keccak_squeezeblocks
 *
 * Description: Squeeze step of Keccak. Squeezes full blocks of r bytes each.
 *              Modifies the state. Can be called multiple times to keep
 *              squeezing, i.e., is incremental.
 *
 * Arguments:   - uint8_t *h: pointer to output blocks
 *              - uint32_t nblocks: number of blocks to be
 *                                                squeezed (written to h)
 *              - uint64_s *s: pointer to input/output Keccak state
 *              - uint32_t r: rate in bytes (e.g., 168 for SHAKE128)
 **************************************************/
static void keccak_squeezeblocks(uint8_t *h, uint32_t nblocks,
                                 uint64_s *s, uint32_t r) {
    while (nblocks > 0) {
        KeccakF1600_StatePermute(s);
        for (uint32_t i = 0; i < (r >> 3); i++) {
            store64(h + 8 * i, s[i]);
        }
        h += r;
        nblocks--;
    }
}

/*************************************************
 * Name:        keccak_inc_init
 *
 * Description: Initializes the incremental Keccak state to zero.
 *
 * Arguments:   - uint64_s *s_inc: pointer to input/output incremental state
 *                First 25 values represent Keccak state.
 *                26th value represents either the number of absorbed bytes
 *                that have not been permuted, or not-yet-squeezed bytes.
 **************************************************/
static void keccak_inc_init(uint64_s *s_inc) {
    uint32_t i;

    for (i = 0; i < 25; ++i) {
        s_inc[i].t[0] = 0;
        s_inc[i].t[1] = 0;
    }
    s_inc[25].t[0] = 0;
    s_inc[25].t[1] = 0;
}

/*************************************************
 * Name:        keccak_inc_absorb
 *
 * Description: Incremental keccak absorb
 *              Preceded by keccak_inc_init, succeeded by keccak_inc_finalize
 *
 * Arguments:   - uint64_s *s_inc: pointer to input/output incremental state
 *                First 25 values represent Keccak state.
 *                26th value represents either the number of absorbed bytes
 *                that have not been permuted, or not-yet-squeezed bytes.
 *              - uint32_t r: rate in bytes (e.g., 168 for SHAKE128)
 *              - const uint8_t *m: pointer to input to be absorbed into s
 *              - uint32_t mlen: length of input in bytes
 **************************************************/
static void keccak_inc_absorb(uint64_s *s_inc, uint32_t r, const uint8_t *m,
                              uint32_t mlen) {
    uint32_t i;

    /* Recall that s_inc[25] is the non-absorbed bytes xored into the state */
    while (mlen + s_inc[25].t[0] >= r) {
        for (i = 0; i < r - (uint32_t)s_inc[25].t[0]; i++) {
            /* Take the i'th byte from message
               xor with the s_inc[25] + i'th byte of the state; little-endian */
            s_inc[(s_inc[25].t[0] + i) >> 3].t[((s_inc[25].t[0] + i) & 0x04) >> 2] ^= (uint32_t)m[i] << (8 * ((s_inc[25].t[0] + i) & 0x03));
        }
        mlen -= (uint32_t)(r - s_inc[25].t[0]);
        m += r - s_inc[25].t[0];
        s_inc[25].t[0] = 0;

        KeccakF1600_StatePermute(s_inc);
    }

    for (i = 0; i < mlen; i++) {
        s_inc[(s_inc[25].t[0] + i) >> 3].t[((s_inc[25].t[0] + i) & 0x04) >> 2] ^= (uint32_t)m[i] << (8 * ((s_inc[25].t[0] + i) & 0x03));
    }
    s_inc[25].t[0] += mlen;
}

/*************************************************
 * Name:        keccak_inc_finalize
 *
 * Description: Finalizes Keccak absorb phase, prepares for squeezing
 *
 * Arguments:   - uint64_s *s_inc: pointer to input/output incremental state
 *                First 25 values represent Keccak state.
 *                26th value represents either the number of absorbed bytes
 *                that have not been permuted, or not-yet-squeezed bytes.
 *              - uint32_t r: rate in bytes (e.g., 168 for SHAKE128)
 *              - uint8_t p: domain-separation byte for different
 *                                 Keccak-derived functions
 **************************************************/
static void keccak_inc_finalize(uint64_s *s_inc, uint32_t r, uint8_t p) {
    /* After keccak_inc_absorb, we are guaranteed that s_inc[25] < r,
       so we can always use one more byte for p in the current state. */
    s_inc[s_inc[25].t[0] >> 3].t[(s_inc[25].t[0] & 0x04) >> 2] ^= (uint32_t)p << (8 * (s_inc[25].t[0] & 0x03));
    s_inc[(r - 1) >> 3].t[((r - 1) & 0x04) >> 2] ^= (uint32_t)128 << (8 * ((r - 1) & 0x03));
    s_inc[25].t[0] = 0;
}

/*************************************************
 * Name:        keccak_inc_squeeze
 *
 * Description: Incremental Keccak squeeze; can be called on byte-level
 *
 * Arguments:   - uint8_t *h: pointer to output bytes
 *              - uint32_t outlen: number of bytes to be squeezed
 *              - uint64_s *s_inc: pointer to input/output incremental state
 *                First 25 values represent Keccak state.
 *                26th value represents either the number of absorbed bytes
 *                that have not been permuted, or not-yet-squeezed bytes.
 *              - uint32_t r: rate in bytes (e.g., 168 for SHAKE128)
 **************************************************/
static void keccak_inc_squeeze(uint8_t *h, uint32_t outlen,
                               uint64_s *s_inc, uint32_t r) {
    uint32_t i;

    /* First consume any bytes we still have sitting around */
    for (i = 0; i < outlen && i < s_inc[25].t[0]; i++) {
        /* There are s_inc[25] bytes left, so r - s_inc[25] is the first
           available byte. We consume from there, i.e., up to r. */
        h[i] = (uint8_t)(s_inc[(r - s_inc[25].t[0] + i) >> 3].t[((r - s_inc[25].t[0] + i) & 0x04) >> 2] >> (8 * ((r - s_inc[25].t[0] + i) & 0x03)));
    }
    h += i;
    outlen -= i;
    s_inc[25].t[0] -= i;

    /* Then squeeze the remaining necessary blocks */
    while (outlen > 0) {
        KeccakF1600_StatePermute(s_inc);

        for (i = 0; i < outlen && i < r; i++) {
            h[i] = (uint8_t)(s_inc[i >> 3].t[(i & 0x4) >> 2] >> (8 * (i & 0x03)));
        }
        h += i;
        outlen -= i;
        s_inc[25].t[0] = r - i;
    }
}

void shake128_inc_init(shake128incctx *state) {
    state->ctx = malloc(PQC_SHAKEINCCTX_BYTES);
    if (state->ctx == NULL) {
        exit(111);
    }
    keccak_inc_init(state->ctx);
}

void shake128_inc_absorb(shake128incctx *state, const uint8_t *input, uint32_t inlen) {
    keccak_inc_absorb(state->ctx, SHAKE128_RATE, input, inlen);
}

void shake128_inc_finalize(shake128incctx *state) {
    keccak_inc_finalize(state->ctx, SHAKE128_RATE, 0x1F);
}

void shake128_inc_squeeze(uint8_t *output, uint32_t outlen, shake128incctx *state) {
    keccak_inc_squeeze(output, outlen, state->ctx, SHAKE128_RATE);
}

void shake128_inc_ctx_clone(shake128incctx *dest, const shake128incctx *src) {
    dest->ctx = malloc(PQC_SHAKEINCCTX_BYTES);
    if (dest->ctx == NULL) {
        exit(111);
    }
    memcpy(dest->ctx, src->ctx, PQC_SHAKEINCCTX_BYTES);
}

void shake128_inc_ctx_release(shake128incctx *state) {
    free(state->ctx);
}

void shake256_inc_init(shake256incctx *state) {
    state->ctx = malloc(PQC_SHAKEINCCTX_BYTES);
    if (state->ctx == NULL) {
        exit(111);
    }
    keccak_inc_init(state->ctx);
}

void shake256_inc_absorb(shake256incctx *state, const uint8_t *input, uint32_t inlen) {
    keccak_inc_absorb(state->ctx, SHAKE256_RATE, input, inlen);
}

void shake256_inc_finalize(shake256incctx *state) {
    keccak_inc_finalize(state->ctx, SHAKE256_RATE, 0x1F);
}

void shake256_inc_squeeze(uint8_t *output, uint32_t outlen, shake256incctx *state) {
    keccak_inc_squeeze(output, outlen, state->ctx, SHAKE256_RATE);
}

void shake256_inc_ctx_clone(shake256incctx *dest, const shake256incctx *src) {
    dest->ctx = malloc(PQC_SHAKEINCCTX_BYTES);
    if (dest->ctx == NULL) {
        exit(111);
    }
    memcpy(dest->ctx, src->ctx, PQC_SHAKEINCCTX_BYTES);
}

void shake256_inc_ctx_release(shake256incctx *state) {
    free(state->ctx);
}


/*************************************************
 * Name:        shake128_absorb
 *
 * Description: Absorb step of the SHAKE128 XOF.
 *              non-incremental, starts by zeroeing the state.
 *
 * Arguments:   - uint64_s *s: pointer to (uninitialized) output Keccak state
 *              - const uint8_t *input: pointer to input to be absorbed
 *                                            into s
 *              - uint32_t inlen: length of input in bytes
 **************************************************/
void shake128_absorb(shake128ctx *state, const uint8_t *input, uint32_t inlen) {
    state->ctx = malloc(PQC_SHAKECTX_BYTES);
    if (state->ctx == NULL) {
        exit(111);
    }
    keccak_absorb(state->ctx, SHAKE128_RATE, input, inlen, 0x1F);
}

/*************************************************
 * Name:        shake128_squeezeblocks
 *
 * Description: Squeeze step of SHAKE128 XOF. Squeezes full blocks of
 *              SHAKE128_RATE bytes each. Modifies the state. Can be called
 *              multiple times to keep squeezing, i.e., is incremental.
 *
 * Arguments:   - uint8_t *output: pointer to output blocks
 *              - uint32_t nblocks: number of blocks to be squeezed
 *                                            (written to output)
 *              - shake128ctx *state: pointer to input/output Keccak state
 **************************************************/
void shake128_squeezeblocks(uint8_t *output, uint32_t nblocks, shake128ctx *state) {
    keccak_squeezeblocks(output, nblocks, state->ctx, SHAKE128_RATE);
}

void shake128_ctx_clone(shake128ctx *dest, const shake128ctx *src) {
    dest->ctx = malloc(PQC_SHAKECTX_BYTES);
    if (dest->ctx == NULL) {
        exit(111);
    }
    memcpy(dest->ctx, src->ctx, PQC_SHAKECTX_BYTES);
}

/** Release the allocated state. Call only once. */
void shake128_ctx_release(shake128ctx *state) {
    free(state->ctx);
}

/*************************************************
 * Name:        shake256_absorb
 *
 * Description: Absorb step of the SHAKE256 XOF.
 *              non-incremental, starts by zeroeing the state.
 *
 * Arguments:   - shake256ctx *state: pointer to (uninitialized) output Keccak state
 *              - const uint8_t *input: pointer to input to be absorbed
 *                                            into s
 *              - uint32_t inlen: length of input in bytes
 **************************************************/
void shake256_absorb(shake256ctx *state, const uint8_t *input, uint32_t inlen) {
    state->ctx = malloc(PQC_SHAKECTX_BYTES);
    if (state->ctx == NULL) {
        exit(111);
    }
    keccak_absorb(state->ctx, SHAKE256_RATE, input, inlen, 0x1F);
}

/*************************************************
 * Name:        shake256_squeezeblocks
 *
 * Description: Squeeze step of SHAKE256 XOF. Squeezes full blocks of
 *              SHAKE256_RATE bytes each. Modifies the state. Can be called
 *              multiple times to keep squeezing, i.e., is incremental.
 *
 * Arguments:   - uint8_t *output: pointer to output blocks
 *              - uint32_t nblocks: number of blocks to be squeezed
 *                                (written to output)
 *              - shake256ctx *state: pointer to input/output Keccak state
 **************************************************/
void shake256_squeezeblocks(uint8_t *output, uint32_t nblocks, shake256ctx *state) {
    keccak_squeezeblocks(output, nblocks, state->ctx, SHAKE256_RATE);
}

void shake256_ctx_clone(shake256ctx *dest, const shake256ctx *src) {
    dest->ctx = malloc(PQC_SHAKECTX_BYTES);
    if (dest->ctx == NULL) {
        exit(111);
    }
    memcpy(dest->ctx, src->ctx, PQC_SHAKECTX_BYTES);
}

/** Release the allocated state. Call only once. */
void shake256_ctx_release(shake256ctx *state) {
    free(state->ctx);
}

/*************************************************
 * Name:        shake128
 *
 * Description: SHAKE128 XOF with non-incremental API
 *
 * Arguments:   - uint8_t *output: pointer to output
 *              - uint32_t outlen: requested output length in bytes
 *              - const uint8_t *input: pointer to input
 *              - uint32_t inlen: length of input in bytes
 **************************************************/
void shake128(uint8_t *output, uint32_t outlen,
              const uint8_t *input, uint32_t inlen) {
    uint32_t nblocks = outlen / SHAKE128_RATE;
    uint8_t t[SHAKE128_RATE];
    shake128ctx s;

    shake128_absorb(&s, input, inlen);
    shake128_squeezeblocks(output, nblocks, &s);

    output += nblocks * SHAKE128_RATE;
    outlen -= nblocks * SHAKE128_RATE;

    if (outlen) {
        shake128_squeezeblocks(t, 1, &s);
        for (uint32_t i = 0; i < outlen; ++i) {
            output[i] = t[i];
        }
    }
    shake128_ctx_release(&s);
}

/*************************************************
 * Name:        shake256
 *
 * Description: SHAKE256 XOF with non-incremental API
 *
 * Arguments:   - uint8_t *output: pointer to output
 *              - uint32_t outlen: requested output length in bytes
 *              - const uint8_t *input: pointer to input
 *              - uint32_t inlen: length of input in bytes
 **************************************************/
void shake256(uint8_t *output, uint32_t outlen,
              const uint8_t *input, uint32_t inlen) {
    uint32_t nblocks = outlen / SHAKE256_RATE;
    uint8_t t[SHAKE256_RATE];
    shake256ctx s;
    // printf("   - input:\n");
    // print_hex(input,inlen);
    // printf("\n");
    shake256_absorb(&s, input, inlen);
    // printf("   - s:\n");
    // print_uint64_s(s.ctx,25);
    // printf("\n");
    shake256_squeezeblocks(output, nblocks, &s);

    output += nblocks * SHAKE256_RATE;
    outlen -= nblocks * SHAKE256_RATE;

    if (outlen) {
        shake256_squeezeblocks(t, 1, &s);
        for (uint32_t i = 0; i < outlen; ++i) {
            output[i] = t[i];
        }
    }
    shake256_ctx_release(&s);
}

void sha3_256_inc_init(sha3_256incctx *state) {
    state->ctx = malloc(PQC_SHAKEINCCTX_BYTES);
    if (state->ctx == NULL) {
        exit(111);
    }
    keccak_inc_init(state->ctx);
}

void sha3_256_inc_ctx_clone(sha3_256incctx *dest, const sha3_256incctx *src) {
    dest->ctx = malloc(PQC_SHAKEINCCTX_BYTES);
    if (dest->ctx == NULL) {
        exit(111);
    }
    memcpy(dest->ctx, src->ctx, PQC_SHAKEINCCTX_BYTES);
}

void sha3_256_inc_ctx_release(sha3_256incctx *state) {
    free(state->ctx);
}

void sha3_256_inc_absorb(sha3_256incctx *state, const uint8_t *input, uint32_t inlen) {
    keccak_inc_absorb(state->ctx, SHA3_256_RATE, input, inlen);
}

void sha3_256_inc_finalize(uint8_t *output, sha3_256incctx *state) {
    uint8_t t[SHA3_256_RATE];
    keccak_inc_finalize(state->ctx, SHA3_256_RATE, 0x06);

    keccak_squeezeblocks(t, 1, state->ctx, SHA3_256_RATE);

    sha3_256_inc_ctx_release(state);

    for (uint32_t i = 0; i < 32; i++) {
        output[i] = t[i];
    }
}

/*************************************************
 * Name:        sha3_256
 *
 * Description: SHA3-256 with non-incremental API
 *
 * Arguments:   - uint8_t *output:      pointer to output
 *              - const uint8_t *input: pointer to input
 *              - uint32_t inlen:   length of input in bytes
 **************************************************/
void sha3_256(uint8_t *output, const uint8_t *input, uint32_t inlen) {
    uint64_s s[25];
    uint8_t t[SHA3_256_RATE];

    /* Absorb input */
    keccak_absorb(s, SHA3_256_RATE, input, inlen, 0x06);

    /* Squeeze output */
    keccak_squeezeblocks(t, 1, s, SHA3_256_RATE);

    for (uint32_t i = 0; i < 32; i++) {
        output[i] = t[i];
    }
}

void sha3_384_inc_init(sha3_384incctx *state) {
    state->ctx = malloc(PQC_SHAKEINCCTX_BYTES);
    if (state->ctx == NULL) {
        exit(111);
    }
    keccak_inc_init(state->ctx);
}

void sha3_384_inc_ctx_clone(sha3_384incctx *dest, const sha3_384incctx *src) {
    dest->ctx = malloc(PQC_SHAKEINCCTX_BYTES);
    if (dest->ctx == NULL) {
        exit(111);
    }
    memcpy(dest->ctx, src->ctx, PQC_SHAKEINCCTX_BYTES);
}

void sha3_384_inc_absorb(sha3_384incctx *state, const uint8_t *input, uint32_t inlen) {
    keccak_inc_absorb(state->ctx, SHA3_384_RATE, input, inlen);
}

void sha3_384_inc_ctx_release(sha3_384incctx *state) {
    free(state->ctx);
}

void sha3_384_inc_finalize(uint8_t *output, sha3_384incctx *state) {
    uint8_t t[SHA3_384_RATE];
    keccak_inc_finalize(state->ctx, SHA3_384_RATE, 0x06);

    keccak_squeezeblocks(t, 1, state->ctx, SHA3_384_RATE);

    sha3_384_inc_ctx_release(state);

    for (uint32_t i = 0; i < 48; i++) {
        output[i] = t[i];
    }
}

/*************************************************
 * Name:        sha3_384
 *
 * Description: SHA3-256 with non-incremental API
 *
 * Arguments:   - uint8_t *output:      pointer to output
 *              - const uint8_t *input: pointer to input
 *              - uint32_t inlen:   length of input in bytes
 **************************************************/
void sha3_384(uint8_t *output, const uint8_t *input, uint32_t inlen) {
    uint64_s s[25];
    uint8_t t[SHA3_384_RATE];

    /* Absorb input */
    keccak_absorb(s, SHA3_384_RATE, input, inlen, 0x06);

    /* Squeeze output */
    keccak_squeezeblocks(t, 1, s, SHA3_384_RATE);

    for (uint32_t i = 0; i < 48; i++) {
        output[i] = t[i];
    }
}

void sha3_512_inc_init(sha3_512incctx *state) {
    state->ctx = malloc(PQC_SHAKEINCCTX_BYTES);
    if (state->ctx == NULL) {
        exit(111);
    }
    keccak_inc_init(state->ctx);
}

void sha3_512_inc_ctx_clone(sha3_512incctx *dest, const sha3_512incctx *src) {
    dest->ctx = malloc(PQC_SHAKEINCCTX_BYTES);
    if (dest->ctx == NULL) {
        exit(111);
    }
    memcpy(dest->ctx, src->ctx, PQC_SHAKEINCCTX_BYTES);
}

void sha3_512_inc_absorb(sha3_512incctx *state, const uint8_t *input, uint32_t inlen) {
    keccak_inc_absorb(state->ctx, SHA3_512_RATE, input, inlen);
}

void sha3_512_inc_ctx_release(sha3_512incctx *state) {
    free(state->ctx);
}

void sha3_512_inc_finalize(uint8_t *output, sha3_512incctx *state) {
    uint8_t t[SHA3_512_RATE];
    keccak_inc_finalize(state->ctx, SHA3_512_RATE, 0x06);

    keccak_squeezeblocks(t, 1, state->ctx, SHA3_512_RATE);

    sha3_512_inc_ctx_release(state);

    for (uint32_t i = 0; i < 64; i++) {
        output[i] = t[i];
    }
}

/*************************************************
 * Name:        sha3_512
 *
 * Description: SHA3-512 with non-incremental API
 *
 * Arguments:   - uint8_t *output:      pointer to output
 *              - const uint8_t *input: pointer to input
 *              - uint32_t inlen:   length of input in bytes
 **************************************************/
void sha3_512(uint8_t *output, const uint8_t *input, uint32_t inlen) {
    uint64_s s[25];
    uint8_t t[SHA3_512_RATE];

    /* Absorb input */
    keccak_absorb(s, SHA3_512_RATE, input, inlen, 0x06);

    /* Squeeze output */
    keccak_squeezeblocks(t, 1, s, SHA3_512_RATE);

    for (uint32_t i = 0; i < 64; i++) {
        output[i] = t[i];
    }
}
