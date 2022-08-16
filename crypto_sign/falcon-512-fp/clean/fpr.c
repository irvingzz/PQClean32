#include "inner.h"
#include "struct64.h"

/*
 * Floating-point operations.
 *
 * This file implements the non-inline functions declared in
 * fpr.h, as well as the constants for FFT / iFFT.
 *
 * ==========================(LICENSE BEGIN)============================
 *
 * Copyright (c) 2017-2019  Falcon Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * ===========================(LICENSE END)=============================
 *
 * @author   Thomas Pornin <thomas.pornin@nccgroup.com>
 */



/*
 * Normalize a provided unsigned integer to the 2^63..2^64-1 range by
 * left-shifting it if necessary. The exponent e is adjusted accordingly
 * (i.e. if the value was left-shifted by n bits, then n is subtracted
 * from e). If source m is 0, then it remains 0, but e is altered.
 * Both m and e must be simple variables (no expressions allowed).
 */

#define ADD64(x,y)      (uint64_s_add(x,y))
#define SHL64(x,c)      (shl(x,c))
#define SHR64(x,c)      (shr(x,c))
#define XOR64(x,y)      (uint64_s_xor(x,y))
#define  OR64(x,y)      (uint64_s_or(x,y))
#define AND64(x,y)      (uint64_s_and(x,y))


#define FPR_NORM64(m, e)   do { \
        uint64_s nt = {{0,0}}; \
        uint64_s mask;  \
        uint64_s tmp; \
        (e) -= 63; \
        \
        nt.t[0] = m.t[1]; \
        nt.t[0] = (nt.t[0] | -nt.t[0]) >> 31; \
        mask = uint64_s_sub(nt,UINT64_S_ONE); \
        tmp = shl(m,32); \
        tmp = XOR64(m,tmp); \
        tmp = AND64(tmp,mask); \
        m = XOR64(m,tmp); \
        (e) += (int)(nt.t[0] << 5); \
        \
        nt = (SHR64(m,48)); \
        nt.t[0] = (nt.t[0] | -nt.t[0]) >> 31; \
        mask = ADD64(nt,((uint64_s){{0xffffffff,0xffffffff}})); \
        tmp = SHL64(m,16); \
        tmp = XOR64(m,tmp); \
        tmp = AND64(tmp,mask); \
        m = XOR64(m,tmp); \
        (e) += (int)(nt.t[0] << 4); \
        \
        nt = (SHR64(m,56)); \
        nt.t[0] = (nt.t[0] | -nt.t[0]) >> 31; \
        mask = ADD64(nt,((uint64_s){{0xffffffff,0xffffffff}})); \
        tmp = SHL64(m,8); \
        tmp = XOR64(m,tmp); \
        tmp = AND64(tmp,mask); \
        m = XOR64(m,tmp); \
        (e) += (int)(nt.t[0] << 3); \
        \
        nt = (SHR64(m,60)); \
        nt.t[0] = (nt.t[0] | -nt.t[0]) >> 31; \
        mask = ADD64(nt,((uint64_s){{0xffffffff,0xffffffff}})); \
        tmp = SHL64(m,4); \
        tmp = XOR64(m,tmp); \
        tmp = AND64(tmp,mask); \
        m = XOR64(m,tmp); \
        (e) += (int)(nt.t[0] << 2); \
        \
        nt = (SHR64(m,62)); \
        nt.t[0] = (nt.t[0] | -nt.t[0]) >> 31; \
        mask = ADD64(nt,((uint64_s){{0xffffffff,0xffffffff}})); \
        tmp = SHL64(m,2); \
        tmp = XOR64(m,tmp); \
        tmp = AND64(tmp,mask); \
        m = XOR64(m,tmp); \
        (e) += (int)(nt.t[0] << 1); \
        \
        nt = (SHR64(m,63)); \
        mask = ADD64(nt,((uint64_s){{0xffffffff,0xffffffff}})); \
        tmp = SHL64(m,1); \
        tmp = XOR64(m,tmp); \
        tmp = AND64(tmp,mask); \
        m = XOR64(m,tmp); \
        (e) += (int)(nt.t[0]); \
    } while (0)

uint64_s
fpr_ursh(uint64_s x, int n) {
    return shr(x,(uint32_t)n);
}

int64_s
fpr_irsh(int64_s x, int n) {
    int64_s tmp;
    uint64_s mask;
    tmp = shr(x,(uint32_t)n);
    mask = shl((uint64_s){{0xffffffff,0xffffffff}},(uint32_t)(64 - n));
    if ((x.t[1] & 0x80000000) != 0)
    {
        tmp = uint64_s_or(tmp,mask);
    }
    
    return tmp;
}

uint64_s
fpr_ulsh(uint64_s x, int n) {
	// uint64_s tmp;
	// printf("x\n");
	// print_uint64_s(&x,1);
	// tmp = shl(x,(uint32_t)n);
	// print_uint64_s(&tmp,1);
	// return tmp;
    return shl(x,(uint32_t)n);
}

fpr
FPR(int s, int e, uint64_s m) {
    fpr x;
    uint64_s t;
    unsigned f;

    /*
     * If e >= -1076, then the value is "normal"; otherwise, it
     * should be a subnormal, which we clamp down to zero.
     */
    t.t[1] = 0;
    t.t[0] = 0;
    e += 1076;
    t.t[0] = (uint32_t)e >> 31;
    m = uint64_s_and(m,uint64_s_add(t,(uint64_s){{0xffffffff,0xffffffff}}));

    /*
     * If m = 0 then we want a zero; make e = 0 too, but conserve
     * the sign.
     */
    t = shr(m,54);
    e &= -(int)t.t[0];

    /*
     * The 52 mantissa bits come from m. Value m has its top bit set
     * (unless it is a zero); we leave it "as is": the top bit will
     * increment the exponent by 1, except when m = 0, which is
     * exactly what we want.
     */
    t.t[0] = (uint32_t)s;
    t = shl(t,63);
    x = uint64_s_or(t,shr(m,2));
    t.t[0] = (uint32_t)e;
    t = shl(t,52);
    x = uint64_s_add(x,t);

    /*
     * Rounding: if the low three bits of m are 011, 110 or 111,
     * then the value should be incremented to get the next
     * representable value. This implements the usual
     * round-to-nearest rule (with preference to even values in case
     * of a tie). Note that the increment may make a carry spill
     * into the exponent field, which is again exactly what we want
     * in that case.
     */
    f = (unsigned)m.t[0] & 7U;
    t.t[1] = 0;
    t.t[0] = (0xC8U >> f) & 1;
    x = uint64_s_add(t,x); 
    return x;
}

fpr
fpr_scaled(int64_s i, int sc) {
    /*
     * To convert from int to float, we have to do the following:
     *  1. Get the absolute value of the input, and its sign
     *  2. Shift right or left the value as appropriate
     *  3. Pack the result
     *
     * We can assume that the source integer is not -2^63.
     */
    int s, e;
    uint64_s m;

    uint64_s tmp;

    /*
     * Extract sign bit.
     * We have: -i = 1 + ~i
     */

    s = ((i.t[1] & 0x80000000)==0)?0:1;
    tmp.t[1] = -(uint32_t)s;
    tmp.t[0] = -(uint32_t)s;
    i = uint64_s_xor(tmp,i);
    tmp.t[1] = 0;
    tmp.t[0] = (uint32_t)s;
    i = uint64_s_add(i,tmp);
	// printf("\t\ts = %d\n",s);
    // print_uint64_s(&i,1);

    /*
     * For now we suppose that i != 0.
     * Otherwise, we set m to i and left-shift it as much as needed
     * to get a 1 in the top bit. We can do that in a logarithmic
     * numbe(0xC8U >> f) & 1;r of conditional shifts.
     */
    m = i;
	// print_uint64_s(&m,1);
    e = 9 + sc;
    // printf("\t\te = %d\n",e);
    FPR_NORM64(m, e);
    // printf("\t\te = %d\n",e);
    // print_uint64_s(&m,1);

    /*
     * Now m is in the 2^63..2^64-1 range. We must divide it by 512;
     * if one of the dropped bits is a 1, this should go into the
     * "sticky bit".
     */
    m.t[0] |= ((uint32_t)m.t[0] & 0x1FF) + 0x1FF;
    m = shr(m,9);

    // print_uint64_s(&m,1);
    /*
     * Corrective action: if i = 0 then all of the above was
     * incorrect, and we clamp e and m down to zero.
     */
    tmp = uint64_s_or(i,uint64_s_neg(i));
    tmp = shr(tmp,63);
    e &= -(int)tmp.t[0];
    tmp = uint64_s_neg(tmp);
    m = uint64_s_and(m,tmp);
    
	// printf("\t\tt = %d\n",(int)t);
    // printf("\t\te = %d\n",e);
    // print_uint64_s(&m,1);

    /*
     * Assemble back everything. The FPR() function will handle cases
     * where e is too low.
     */
	tmp = FPR(s, e, m);
	// print_uint64_s(&tmp,1);
    return tmp;
}

fpr
fpr_of(int64_s i) {
	// print_uint64_s(&i,1);
    return fpr_scaled(i, 0);
}

int64_s
fpr_rint(fpr x) {
    uint64_s m, d;
    int e;
    uint32_t dd, f;

    uint64_s tmp;

    /*
     * We assume that the value fits in -(2^63-1)..+(2^63-1). We can
     * thus extract the mantissa as a 63-bit integer, then right-shift
     * it as needed.
     */
    tmp = shl(x,10);
    m = uint64_s_or(tmp, shl(UINT64_S_ONE,62));
    tmp = shl(UINT64_S_ONE,63);
    tmp = uint64_s_sub(tmp,UINT64_S_ONE);
    m = uint64_s_and(m,tmp);
    tmp = shr(x,52);
    e = 1085 - ((int)(tmp.t[0]) & 0x7FF);

	// if(x.t[1] == 0x4152abb0 && x.t[0] == 0x2005c7fb)
	// {
	// 	printf("\t\tm\n");
	// 	print_uint64_s(&m,1);
	// 	printf("\t\te = %d\n",e);
	// }
    /*
     * If a shift of more than 63 bits is needed, then simply set m
     * to zero. This also covers the case of an input operand equal
     * to zero.
     */

    tmp.t[1] = 0;
    tmp.t[0] = ((uint32_t)(e - 64) >> 31);
    tmp = uint64_s_neg(tmp);
    m = uint64_s_and(m,tmp);
    e &= 63;

	// if(x.t[1] == 0x4152abb0 && x.t[0] == 0x2005c7fb)
	// {
	// 	printf("\t\tm\n");
	// 	print_uint64_s(&m,1);
	// 	printf("\t\te = %d\n",e);
	// }
    /*
     * Right-shift m as needed. Shift count is e. Proper rounding
     * mandates that:
     *   - If the highest dropped bit is zero, then round low.
     *   - If the highest dropped bit is one, and at least one of the
     *     other dropped bits is one, then round up.
     *   - If the highest dropped bit is one, and all other dropped
     *     bits are zero, then round up if the lowest kept bit is 1,
     *     or low otherwise (i.e. ties are broken by "rounding to even").
     *
     * We thus first extract a word consisting of all the dropped bit
     * AND the lowest kept bit; then we shrink it down to three bits,
     * the lowest being "sticky".
     */
    d = fpr_ulsh(m, 63 - e);
    dd = (uint32_t)d.t[0] | ((uint32_t)(d.t[1]) & 0x1FFFFFFF);

    tmp = shr(d,61);
    f = (uint32_t)(tmp.t[0]) | ((dd | -dd) >> 31);
    tmp.t[1] = 0;
    tmp.t[0] = ((0xC8U >> f) & 1U);
    m = uint64_s_add(fpr_ursh(m, e),tmp);

    /*
     * Apply the sign bit.
     */
    tmp = shr(x,63);
    tmp =uint64_s_add(uint64_s_xor(m,uint64_s_add(uint64_s_not(tmp),(uint64_s){{1,0}})),tmp);
    return tmp;
}

int64_s
fpr_floor(fpr x) {
    uint64_s t;
    int64_s xi;
    int e, cc;
    
    uint64_s tmp;
    uint64_s mask;
    /*
     * We extract the integer as a _signed_ 64-bit integer with
     * a scaling factor. Since we assume that the value fits
     * in the -(2^63-1)..+(2^63-1) range, we can left-shift the
     * absolute value to make it in the 2^62..2^63-1 range: we
     * will only need a right-shift afterwards.
     */
    tmp = shr(x,52);
    e = (int)(tmp.t[0]) & 0x7FF;
    t = shr(x,63);
    tmp = uint64_s_or(shl(x,10),shl(UINT64_S_ONE,62));
    mask = uint64_s_sub(shl(UINT64_S_ONE,63),UINT64_S_ONE);
    xi = uint64_s_and(tmp,mask);
    tmp = uint64_s_neg(t);
    xi = uint64_s_add(uint64_s_xor(xi,tmp),t);
    cc = 1085 - e;

	// if (x.t[1] == 0x40476585 && x.t[0] == 0xe28b47c4)
    // {
    //     printf("\t\tfpr_floor\n");
    //     printf("\t\te = %d\tcc = %d\n",e,cc);
    //     printf("\t\tt:\n");
    //     print_uint64_s(&t,1);
    //     printf("\t\txi:\n");
    //     print_uint64_s(&xi,1);
    // }
    

    /*
     * We perform an arithmetic right-shift on the value. This
     * applies floor() semantics on both positive and negative values
     * (rounding toward minus infinity).
     */
    xi = fpr_irsh(xi, cc & 63);

    /*
     * If the true shift count was 64 or more, then we should instead
     * replace xi with 0 (if nonnegative) or -1 (if negative). Edge
     * case: -0 will be floored to -1, not 0 (whether this is correct
     * is debatable; in any case, the other functions normalize zero
     * to +0).
     *
     * For an input of zero, the non-shifted xi was incorrect (we used
     * a top implicit bit of value 1, not 0), but this does not matter
     * since this operation will clamp it down.
     */
    mask = uint64_s_add(uint64_s_not(t),(uint64_s){{1,0}});
    tmp.t[1] = 0;
    tmp.t[0] = (uint32_t)(63 - cc) >> 31;
    tmp = uint64_s_add(uint64_s_not(tmp),(uint64_s){{1,0}});
    tmp = uint64_s_and(uint64_s_xor(xi,mask),tmp);
    xi = uint64_s_xor(xi,tmp);
    return xi;
}

int64_s
fpr_trunc(fpr x) {
    uint64_s t, xu;
    int e, cc;
    
    uint64_s tmp, mask;

    /*
     * Extract the absolute value. Since we assume that the value
     * fits in the -(2^63-1)..+(2^63-1) range, we can left-shift
     * the absolute value into the 2^62..2^63-1 range, and then
     * do a right shift afterwards.
     */
    tmp = shr(x,52);
    e = (int)(tmp.t[0]) & 0x7FF;
    tmp = uint64_s_or(shl(x,10),shl((uint64_s){{1,0}},62));
    mask = uint64_s_add(shl((uint64_s){{1,0}},63),(uint64_s){{0xffffffff,0xffffffff}});
    xu = uint64_s_and(tmp,mask);
    cc = 1085 - e;
    xu = fpr_ursh(xu, cc & 63);

    /*
     * If the exponent is too low (cc > 63), then the shift was wrong
     * and we must clamp the value to 0. This also covers the case
     * of an input equal to zero.
     */
    tmp.t[1] = 0;
    tmp.t[0] = (uint32_t)(cc - 64) >> 31;
    tmp = uint64_s_add(uint64_s_not(tmp),(uint64_s){{1,0}});
    xu = uint64_s_and(xu,tmp);

    /*
     * Apply back the sign, if the source value is negative.
     */
    t = shr(x, 63);
    mask = uint64_s_add(uint64_s_not(t),(uint64_s){{1,0}});
    tmp = uint64_s_xor(xu,mask);
    xu = uint64_s_add(tmp,t);
    return *(int64_s *)&xu;
}

fpr
fpr_add(fpr x, fpr y) {
    uint64_s m, xu, yu, za;
    uint32_t cs;
    int ex, ey, sx, sy, cc;

    uint64_s tmp,mask;
    /*
     * Make sure that the first operand (x) has the larger absolute
     * value. This guarantees that the exponent of y is less than
     * or equal to the exponent of x, and, if they are equal, then
     * the mantissa of y will not be greater than the mantissa of x.
     *
     * After this swap, the result will have the sign x, except in
     * the following edge case: abs(x) = abs(y), and x and y have
     * opposite sign bits; in that case, the result shall be +0
     * even if the sign bit of x is 1. To handle this case properly,
     * we do the swap is abs(x) = abs(y) AND the sign of x is 1.
     */


    tmp = shl(UINT64_S_ONE,63);
    m = uint64_s_sub(tmp,UINT64_S_ONE);
    za = uint64_s_sub(uint64_s_and(x,m),uint64_s_and(y,m));
    tmp = uint64_s_sub((uint64_s){{0,0}},za);
    cs = (uint32_t)(za.t[1] >> 31)
         | ((1U - (uint32_t)(tmp.t[1] >> 31)) & (uint32_t)(x.t[1] >> 31));
    tmp.t[1] = 0;
    tmp.t[0] = cs;
    tmp = uint64_s_sub((uint64_s){{0,0}},tmp);
    m = uint64_s_and(uint64_s_xor(x,y),tmp);
    x = uint64_s_xor(x,m);
    y = uint64_s_xor(y,m);
	// printf("x + y\n");
    // print_uint64_s(&x,1);
    // print_uint64_s(&y,1);

    /*
     * Extract sign bits, exponents and mantissas. The mantissas are
     * scaled up to 2^55..2^56-1, and the exponent is unbiased. If
     * an operand is zero, its mantissa is set to 0 at this step, and
     * its exponent will be -1078.
     */
    tmp = shr(x,52);
    ex = (int)(tmp.t[0]);
    sx = ex >> 11;
    ex &= 0x7FF;
    tmp.t[1] = 0;
    tmp.t[0] = (uint32_t)((ex + 0x7FF) >> 11);
    m = shl(tmp,52);
    mask = uint64_s_sub(shl(UINT64_S_ONE,52),UINT64_S_ONE);
	tmp = uint64_s_or(uint64_s_and(x,mask),m);
    xu = shl(tmp,3);
    ex -= 1078;
    tmp = shr(y,52);
    ey = (int)(tmp.t[0]);
    sy = ey >> 11;
    ey &= 0x7FF;
    tmp.t[1] = 0;
    tmp.t[0] = (uint32_t)((ey + 0x7FF) >> 11);
    m = shl(tmp,52);
    mask = uint64_s_sub(shl(UINT64_S_ONE,52),UINT64_S_ONE);
	tmp = uint64_s_or(uint64_s_and(y,mask),m);
    yu = shl(tmp,3);
    ey -= 1078;


    
    /*
     * x has the larger exponent; hence, we only need to right-shift y.
     * If the shift count is larger than 59 bits then we clamp the
     * value to zero.
     */
    cc = ex - ey;
    tmp.t[1] = 0;
    tmp.t[0] = ((uint32_t)(cc - 60) >> 31);
    tmp = uint64_s_sub((uint64_s){{0,0}},tmp);
    yu = uint64_s_and(yu,tmp);
    cc &= 63;
    /*
     * The lowest bit of yu is "sticky".
     */
    
    tmp = fpr_ulsh(UINT64_S_ONE, cc);
    m = uint64_s_sub(tmp,UINT64_S_ONE);
    tmp = uint64_s_and(yu,m);
    tmp = uint64_s_add(tmp,m);
    yu = uint64_s_or(yu,tmp);
    yu = fpr_ursh(yu, cc);

	// printf("m\n");
    // print_uint64_s(&m,1);
    // print_uint64_s(&xu,1);
	// print_uint64_s(&yu,1);
    // printf(" ey = %d\tex = %d\tsx = %d\tsy = %d\tcc = %d\n",ey,ex,sx,sy,cc);
    /*
     * If the operands have the same sign, then we add the mantissas;
     * otherwise, we subtract the mantissas.
     */
    tmp.t[1] = 0;
    tmp.t[0] = (uint32_t)(sx ^ sy);
    mask = uint64_s_sub((uint64_s){{0,0}},tmp);
    tmp = uint64_s_and(shl(yu,1),mask);
    tmp = uint64_s_sub(yu,tmp);
    xu = uint64_s_add(xu,tmp);

    /*
     * The result may be smaller, or slightly larger. We normalize
     * it to the 2^63..2^64-1 range (if xu is zero, then it stays
     * at zero).
     */
    FPR_NORM64(xu, ex);

    /*
     * Scale down the value to 2^54..s^55-1, handling the last bit
     * as sticky.
     */
    tmp.t[1] = 0;
    tmp.t[0] = ((uint32_t)xu.t[0] & 0x1FF) + 0x1FF;
    xu = uint64_s_or(xu,tmp);
    xu = shr(xu,9);
    ex += 9;

    /*
     * In general, the result has the sign of x. However, if the
     * result is exactly zero, then the following situations may
     * be encountered:
     *   x > 0, y = -x   -> result should be +0
     *   x < 0, y = -x   -> result should be +0
     *   x = +0, y = +0  -> result should be +0
     *   x = -0, y = +0  -> result should be +0
     *   x = +0, y = -0  -> result should be +0
     *   x = -0, y = -0  -> result should be -0
     *
     * But at the conditional swap step at the start of the
     * function, we ensured that if abs(x) = abs(y) and the
     * sign of x was 1, then x and y were swapped. Thus, the
     * two following cases cannot actually happen:
     *   x < 0, y = -x
     *   x = -0, y = +0
     * In all other cases, the sign bit of x is conserved, which
     * is what the FPR() function does. The FPR() function also
     * properly clamps values to zero when the exponent is too
     * low, but does not alter the sign in that case.
     */
	tmp = FPR(sx, ex, xu);
    // printf("tmp\n");
    // print_uint64_s(&tmp,1);
    return tmp;
}

fpr
fpr_sub(fpr x, fpr y) {
	uint64_s tmp;
	// printf("x - y\n");
    // print_uint64_s(&x,1);
    // print_uint64_s(&y,1);
    y = uint64_s_xor(y,(uint64_s){{0x00000000,0x80000000}});
	tmp = fpr_add(x, y);
	// printf("tmp\n");
    // print_uint64_s(&tmp,1);
    return tmp;
}

fpr
fpr_neg(fpr x) {
    x = uint64_s_xor(x,(uint64_s){{0x00000000,0x80000000}});
    return x;
}

fpr
fpr_half(fpr x) {
    /*
     * To divide a value by 2, we just have to subtract 1 from its
     * exponent, but we have to take care of zero.
     */
    uint32_t t;

    uint64_s tmp;

    tmp = shl((uint64_s){{1,0}},52);
    x = uint64_s_sub(x,tmp);
    t = (((uint32_t)(x.t[1] >> 20) & 0x7FF) + 1) >> 11;
    tmp.t[1] = 0;
    tmp.t[0] = t;
    tmp = uint64_s_sub(tmp,(uint64_s){{1,0}});
    x = uint64_s_and(x,tmp);
    return x;
}

fpr
fpr_double(fpr x) {
    /*
     * To double a value, we just increment by one the exponent. We
     * don't care about infinites or NaNs; however, 0 is a
     * special case.
     */
    uint64_s tmp;
	tmp = shr(x,52);
	tmp.t[0] = ((((unsigned)(tmp.t[0]) & 0x7FFU) + 0x7FFU) >> 11);
    tmp = shl(tmp,52);
    x = uint64_s_add(x,tmp);
    return x;
}

fpr
fpr_mul(fpr x, fpr y) {
    uint64_s xu, yu, w, zu, zv;
    uint32_t x0, x1, y0, y1, z0, z1, z2;
    int ex, ey, d, e, s;

    uint64_s tmp,mask;
    /*
     * Extract absolute values as scaled unsigned integers. We
     * don't extract exponents yet.
     */
	// printf("x * y\n");
    // print_uint64_s(&x,1);
    // print_uint64_s(&y,1);
    mask = shl((uint64_s){{1,0}},52);
    tmp = uint64_s_sub(mask,(uint64_s){{1,0}});
    xu = uint64_s_or(uint64_s_and(x,tmp),mask);
    yu = uint64_s_or(uint64_s_and(y,tmp),mask);

    /*
     * We have two 53-bit integers to multiply; we need to split
     * each into a lower half and a upper half. Moreover, we
     * prefer to have lower halves to be of 25 bits each, for
     * reasons explained later on.
     */
    x0 = (uint32_t)xu.t[0] & 0x01FFFFFF;
    tmp = shr(xu,25);
    x1 = (uint32_t)(tmp.t[0]);
    y0 = (uint32_t)yu.t[0] & 0x01FFFFFF;
    tmp = shr(yu,25);
    y1 = (uint32_t)(tmp.t[0]);
    w = uint32_t_mul(x0,y0);
    z0 = (uint32_t)w.t[0] & 0x01FFFFFF;
    tmp = shr(w,25);
    z1 = (uint32_t)(tmp.t[0]);
    w = uint32_t_mul(x0,y1);
    z1 += (uint32_t)w.t[0] & 0x01FFFFFF;
    tmp = shr(w,25);
    z2 = (uint32_t)(tmp.t[0]);
    w = uint32_t_mul(x1,y0);
    z1 += (uint32_t)w.t[0] & 0x01FFFFFF;
    tmp = shr(w,25);
    z2 += (uint32_t)(tmp.t[0]);
    zu = uint32_t_mul(x1,y1);
    z2 += (z1 >> 25);
    z1 &= 0x01FFFFFF;
    tmp.t[1] = 0;
    tmp.t[0] = z2;
    zu = uint64_s_add(zu,tmp);

    /*
     * Since xu and yu are both in the 2^52..2^53-1 range, the
     * product is in the 2^104..2^106-1 range. We first reassemble
     * it and round it into the 2^54..2^56-1 range; the bottom bit
     * is made "sticky". Since the low limbs z0 and z1 are 25 bits
     * each, we just take the upper part (zu), and consider z0 and
     * z1 only for purposes of stickiness.
     * (This is the reason why we chose 25-bit limbs above.)
     */
    tmp.t[1] = 0;
    tmp.t[0] = ((z0 | z1) + 0x01FFFFFF) >> 25;
    zu = uint64_s_or(zu,tmp);

    /*
     * We normalize zu to the 2^54..s^55-1 range: it could be one
     * bit too large at this point. This is done with a conditional
     * right-shift that takes into account the sticky bit.
     */
    zv = uint64_s_or(shr(zu,1),uint64_s_and(zu,(uint64_s){{1,0}}));
    w = shr(zu,55);
    mask= uint64_s_sub((uint64_s){{0,0}},w);
    tmp = uint64_s_xor(zu,zv);
    tmp = uint64_s_and(tmp,mask);
    zu = uint64_s_xor(zu,tmp);

    /*
     * Get the aggregate scaling factor:
     *
     *   - Each exponent is biased by 1023.
     *
     *   - Integral mantissas are scaled by 2^52, hence an
     *     extra 52 bias for each exponent.
     *
     *   - However, we right-shifted z by 50 bits, and then
     *     by 0 or 1 extra bit (depending on the value of w).
     *
     * In total, we must add the exponents, then subtract
     * 2 * (1023 + 52), then add 50 + w.
     */
    tmp = shr(x,52);
    ex = (int)((tmp.t[0]) & 0x7FF);
    tmp = shr(y,52);
    ey = (int)((tmp.t[0]) & 0x7FF);
    e = ex + ey - 2100 + (int)w.t[0];

    /*
     * Sign bit is the XOR of the operand sign bits.
     */
    tmp = uint64_s_xor(x,y);
    tmp = shr(tmp,63);
    s = (int)(tmp.t[0]);

    /*
     * Corrective actions for zeros: if either of the operands is
     * zero, then the computations above were wrong. Test for zero
     * is whether ex or ey is zero. We just have to set the mantissa
     * (zu) to zero, the FPR() function will normalize e.
     */
    d = ((ex + 0x7FF) & (ey + 0x7FF)) >> 11;
    tmp.t[1] = 0;
    tmp.t[0] = (uint32_t)d;
    tmp = uint64_s_add(uint64_s_not(tmp),(uint64_s){{1,0}});
    zu = uint64_s_and(zu,tmp);

    /*
     * FPR() packs the result and applies proper rounding.
     */
	tmp = FPR(s, e, zu);
	// if (y.t[0] == 0xf1e88b1f && y.t[1] == 0xc16c89ec)
	// {
	// 	printf("tmp\n");
	// 	print_uint64_s(&tmp,1);
	// }
    return tmp;
}

fpr
fpr_sqr(fpr x) {
    return fpr_mul(x, x);
}

/* 2022/7/27 make change here */

fpr
fpr_div(fpr x, fpr y) {
    uint64_s xu, yu, q, q2, w;
    int i, ex, ey, e, d, s;

    uint64_s tmp,mask;
    /*
     * Extract mantissas of x and y (unsigned).
     */
	// printf("fpr_div\n");
	// print_uint64_s(&x,1);
	// print_uint64_s(&y,1);
    mask = shl((uint64_s){{1,0}},52);
    tmp = uint64_s_sub(mask,(uint64_s){{1,0}});
    xu = uint64_s_or(uint64_s_and(x,tmp),mask);
    yu = uint64_s_or(uint64_s_and(y,tmp),mask);

    /*
     * Perform bit-by-bit division of xu by yu. We run it for 55 bits.
     */
    q = (uint64_s){{0,0}};
    for (i = 0; i < 55; i ++) {
        /*
         * If yu is less than or equal xu, then subtract it and
         * push a 1 in the quotient; otherwise, leave xu unchanged
         * and push a 0.
         */
        uint64_s b;

        tmp = uint64_s_sub(xu,yu);
        tmp = shr(tmp,63);
        b = uint64_s_sub(tmp,UINT64_S_ONE);
        tmp = uint64_s_and(b,yu);
        xu = uint64_s_sub(xu,tmp);
        tmp = uint64_s_and(b,(uint64_s){{1,0}});
        q = uint64_s_or(q,tmp);
        xu = shl(xu,1);
        q = shl(q,1);
    }
	// printf("xu-yu\n");
	// print_uint64_s(&xu,1);
	// print_uint64_s(&yu,1);

    /*
     * We got 55 bits in the quotient, followed by an extra zero. We
     * want that 56th bit to be "sticky": it should be a 1 if and
     * only if the remainder (xu) is non-zero.
     */
    tmp = uint64_s_sub((uint64_s){{0,0}},xu);
    tmp = uint64_s_or(xu,tmp);
    tmp = shr(tmp,63);
    q = uint64_s_or(q,tmp);

    /*
     * Quotient is at most 2^56-1. Its top bit may be zero, but in
     * that case the next-to-top bit will be a one, since the
     * initial xu and yu were both in the 2^52..2^53-1 range.
     * We perform a conditional shift to normalize q to the
     * 2^54..2^55-1 range (with the bottom bit being sticky).
     */
    tmp = uint64_s_and(q,(uint64_s){{1,0}});
    q2 = uint64_s_or(shr(q,1),tmp);
    w = shr(q,55);
    tmp = uint64_s_xor(q,q2);
    mask = uint64_s_sub((uint64_s){{0,0}},w);
    tmp = uint64_s_and(tmp,mask);
    q = uint64_s_xor(q,tmp);

    /*
     * Extract exponents to compute the scaling factor:
     *
     *   - Each exponent is biased and we scaled them up by
     *     52 bits; but these biases will cancel out.
     *
     *   - The division loop produced a 55-bit shifted result,
     *     so we must scale it down by 55 bits.
     *
     *   - If w = 1, we right-shifted the integer by 1 bit,
     *     hence we must add 1 to the scaling.
     */
    tmp = shr(x,52);
    ex = (int)((tmp.t[0]) & 0x7FF);
    tmp = shr(y,52);
    ey = (int)((tmp.t[0]) & 0x7FF);
    e = ex - ey - 55 + (int)w.t[0];

    /*
     * Sign is the XOR of the signs of the operands.
     */
    tmp = uint64_s_xor(x,y);
    tmp = shr(tmp,63);
    s = (int)(tmp.t[0]);

    /*
     * Corrective actions for zeros: if x = 0, then the computation
     * is wrong, and we must clamp e and q to 0. We do not care
     * about the case y = 0 (as per assumptions in this module,
     * the caller does not perform divisions by zero).
     */
    d = (ex + 0x7FF) >> 11;
    s &= d;
    e &= -d;
    tmp.t[1] = 0;
    tmp.t[0] = (uint32_t)d;
    tmp = uint64_s_sub((uint64_s){{0,0}},tmp);
    q = uint64_s_and(q,tmp);
    

    /*
     * FPR() packs the result and applies proper rounding.
     */
    return FPR(s, e, q);
}

fpr
fpr_inv(fpr x) {
    uint64_s tmp = {{0x00000000,0x3FF00000}};   // 4607182418800017408u
	tmp = fpr_div(tmp, x);
	// printf("fpr_inv\n");
	// print_uint64_s(&x,1);
	// print_uint64_s(&tmp,1);
	return tmp;
    // return fpr_div(tmp, x);
}

fpr
fpr_sqrt(fpr x) {
    uint64_s xu, q, s, r;
    int ex, e;

    uint64_s tmp,mask;
    /*
     * Extract the mantissa and the exponent. We don't care about
     * the sign: by assumption, the operand is nonnegative.
     * We want the "true" exponent corresponding to a mantissa
     * in the 1..2 range.
     */
    mask = shl((uint64_s){{1,0}},52);
    tmp = uint64_s_sub(mask,(uint64_s){{1,0}});
    xu = uint64_s_or(uint64_s_and(x,tmp),mask);
    tmp = shr(x,52);
    ex = (int)((tmp.t[0]) & 0x7FF);
    e = ex - 1023;

    /*
     * If the exponent is odd, double the mantissa and decrement
     * the exponent. The exponent is then halved to account for
     * the square root.
     */
    tmp.t[1] = 0;
    tmp.t[0] = e & 1;
    mask = uint64_s_sub((uint64_s){{0,0}},tmp);
    tmp = uint64_s_and(xu,mask);
    xu = uint64_s_add(xu,tmp);
    e >>= 1;

    /*
     * Double the mantissa.
     */
    xu = shl(xu,1);

    /*
     * We now have a mantissa in the 2^53..2^55-1 range. It
     * represents a value between 1 (inclusive) and 4 (exclusive)
     * in fixed point notation (with 53 fractional bits). We
     * compute the square root bit by bit.
     */
    q = (uint64_s){{0,0}};
    s = (uint64_s){{0,0}};
    r = shl((uint64_s){{1,0}},53);
    for (int i = 0; i < 54; i ++) {
        uint64_s t, b;

        t = uint64_s_add(s,r);
        tmp = uint64_s_sub(xu,t);
        tmp = shr(tmp,63);
        b = uint64_s_sub(tmp,(uint64_s){{1,0}});
        tmp = shl(r,1);
        tmp = uint64_s_and(tmp,b);
        s = uint64_s_add(s,tmp);
        tmp = uint64_s_and(t,b);
        xu = uint64_s_sub(xu,tmp);
        tmp = uint64_s_and(r,b);
        q = uint64_s_add(q,tmp);
        xu = shl(xu,1);
        r = shr(r,1);
    }

    /*
     * Now, q is a rounded-low 54-bit value, with a leading 1,
     * 52 fractional digits, and an additional guard bit. We add
     * an extra sticky bit to account for what remains of the operand.
     */
    q = shl(q,1);
    tmp = uint64_s_neg(xu);
    tmp = uint64_s_or(xu,tmp);
    tmp = shr(tmp,63);
    q = uint64_s_or(q,tmp);

    /*
     * Result q is in the 2^54..2^55-1 range; we bias the exponent
     * by 54 bits (the value e at that point contains the "true"
     * exponent, but q is now considered an integer, i.e. scaled
     * up.
     */
    e -= 54;

    /*
     * Corrective action for an operand of value zero.
     */
    tmp.t[1] = 0;
    tmp.t[0] = (uint32_t)((ex + 0x7FF) >> 11);
    tmp = uint64_s_neg(tmp);
    q = uint64_s_and(q,tmp);

    /*
     * Apply rounding and back result.
     */
    return FPR(0, e, q);
}

int
fpr_lt(fpr x, fpr y) {
    /*
     * If both x and y are positive, then a signed comparison yields
     * the proper result:
     *   - For positive values, the order is preserved.
     *   - The sign bit is at the same place as in integers, so
     *     sign is preserved.
     * Moreover, we can compute [x < y] as sgn(x-y) and the computation
     * of x-y will not overflow.
     *
     * If the signs differ, then sgn(x) gives the proper result.
     *
     * If both x and y are negative, then the order is reversed.
     * Hence [x < y] = sgn(y-x). We must compute this separately from
     * sgn(x-y); simply inverting sgn(x-y) would not handle the edge
     * case x = y properly.
     */
    int cc0, cc1;
    int64_s sx;
    int64_s sy;

    int64_s tmp;

    sx = *(int64_s *)&x;                /* set sy=0 if signs differ */
    sy = *(int64_s *)&y;
    tmp = uint64_s_xor(sx,sy);
    tmp = int64_s_shr(tmp,63);
    tmp = uint64_s_not(tmp);
    sy = uint64_s_and(sy,tmp);

	// if (x.t[1] == 0xb4750eb8 && x.t[0] == 0xe84631fc && y.t[1] == 0x41dfffff && y.t[0] == 0xffc00000)
	// {
	// 	printf("\t\tsy = ");
	// 	print_uint64_s(&sy,1);
	// }
    
    tmp = uint64_s_sub(sx,sy);
    tmp = int64_s_shr(tmp,63);
    cc0 = (int)(tmp.t[0]) & 1; /* Neither subtraction overflows when */
    tmp = uint64_s_sub(sy,sx);
    tmp = shr(tmp,63);
    cc1 = (int)(tmp.t[0]) & 1; /* the signs are the same. */

    tmp = uint64_s_and(x,y);
    tmp = int64_s_shr(tmp,63);
    return cc0 ^ ((cc0 ^ cc1) & (int)(tmp.t[0]));
}

uint64_s
fpr_expm_p63(fpr x, fpr ccs) {
    /*
     * Polynomial approximation of exp(-x) is taken from FACCT:
     *   https://eprint.iacr.org/2018/1234
     * Specifically, values are extracted from the implementation
     * referenced from the FACCT article, and available at:
     *   https://github.com/raykzhao/gaussian
     * Here, the coefficients have been scaled up by 2^63 and
     * converted to integers.
     *
     * Tests over more than 24 billions of random inputs in the
     * 0..log(2) range have never shown a deviation larger than
     * 2^(-50) from the true mathematical value.
     */
    // static const uint64_t C[] = {
    //     0x00000004741183A3u,
    //     0x00000036548CFC06u,
    //     0x0000024FDCBF140Au,
    //     0x0000171D939DE045u,
    //     0x0000D00CF58F6F84u,
    //     0x000680681CF796E3u,
    //     0x002D82D8305B0FEAu,
    //     0x011111110E066FD0u,
    //     0x0555555555070F00u,
    //     0x155555555581FF00u,
    //     0x400000000002B400u,
    //     0x7FFFFFFFFFFF4800u,
    //     0x8000000000000000u
    // };

static const uint64_s C[] = {
		{{0x741183a3, 0x00000004}},
		{{0x548cfc06, 0x00000036}},
		{{0xdcbf140a, 0x0000024f}},
		{{0x939de045, 0x0000171d}},
		{{0xf58f6f84, 0x0000d00c}},
		{{0x1cf796e3, 0x00068068}},
		{{0x305b0fea, 0x002d82d8}},
		{{0x0e066fd0, 0x01111111}},
		{{0x55070f00, 0x05555555}},
		{{0x5581ff00, 0x15555555}},
		{{0x0002b400, 0x40000000}},
		{{0xffff4800, 0x7fffffff}},
		{{0x00000000, 0x80000000}}
};

    uint64_s z, y;
    uint32_t u;
    uint32_t z0, z1, y0, y1;
    uint64_s a, b;

    uint64_s tmp;

    y = C[0];
    tmp = fpr_trunc(fpr_mul(x, fpr_ptwo63));
    z = shl(tmp, 1);
    for (u = 1; u < (sizeof C) / sizeof(C[0]); u ++) {
        /*
         * Compute product z * y over 128 bits, but keep only
         * the top 64 bits.
         *
         * TODO: On some architectures/compilers we could use
         * some intrinsics (__umulh() on MSVC) or other compiler
         * extensions (unsigned __int128 on GCC / Clang) for
         * improved speed; however, most 64-bit architectures
         * also have appropriate IEEE754 floating-point support,
         * which is better.
         */
        uint64_s c;

        z0 = (uint32_t)z.t[0];
        z1 = (uint32_t)(z.t[1]);
        y0 = (uint32_t)y.t[0];
        y1 = (uint32_t)(y.t[1]);
        tmp = uint32_t_mul(z0,y0);
        tmp = shr(tmp,32);
        a = uint64_s_add(uint32_t_mul(z0,y1),tmp);
        b = uint32_t_mul(z1,y0);
        c = uint64_s_add(shr(a,32),shr(b,32));
        a.t[1] = 0;
        b.t[1] = 0;
        tmp = uint64_s_add(a,b);
        tmp = shr(tmp,32);
        c = uint64_s_add(c,tmp);
        tmp = uint32_t_mul(z1,y1);
        c = uint64_s_add(c,tmp);
        y = uint64_s_sub(C[u],c);
    }

    /*
     * The scaling factor must be applied at the end. Since y is now
     * in fixed-point notation, we have to convert the factor to the
     * same format, and do an extra integer multiplication.
     */
    tmp = fpr_trunc(fpr_mul(ccs, fpr_ptwo63));
    z = shl(tmp,1);
    z0 = (uint32_t)z.t[0];
    z1 = (uint32_t)(z.t[1]);
    y0 = (uint32_t)y.t[0];
    y1 = (uint32_t)(y.t[1]);
    tmp = uint32_t_mul(z0,y0);
    tmp = shr(tmp,32);
    a = uint64_s_add(uint32_t_mul(z0,y1),tmp);
    b = uint32_t_mul(z1,y0);
    y = uint64_s_add(shr(a,32),shr(b,32));
    a.t[1] = 0;
    b.t[1] = 0;
    tmp = uint64_s_add(a,b);
    tmp = shr(tmp,32);
    y = uint64_s_add(y,tmp);
    tmp = uint32_t_mul(z1,y1);
    y = uint64_s_add(y,tmp);

    return y;
}

const fpr fpr_gm_tab[] = {
	{{0x00000000, 0x00000000}}, {{0x00000000, 0x00000000}},
	{{0x00000000, 0x80000000}}, {{0x00000000, 0x3ff00000}},
	{{0x667f3bcd, 0x3fe6a09e}}, {{0x667f3bcd, 0x3fe6a09e}},
	{{0x667f3bcd, 0xbfe6a09e}}, {{0x667f3bcd, 0x3fe6a09e}},
	{{0xcf328d46, 0x3fed906b}}, {{0xa6aea963, 0x3fd87de2}},
	{{0xa6aea963, 0xbfd87de2}}, {{0xcf328d46, 0x3fed906b}},
	{{0xa6aea963, 0x3fd87de2}}, {{0xcf328d46, 0x3fed906b}},
	{{0xcf328d46, 0xbfed906b}}, {{0xa6aea963, 0x3fd87de2}},
	{{0xcff75cb0, 0x3fef6297}}, {{0x3c69a60b, 0x3fc8f8b8}},
	{{0x3c69a60b, 0xbfc8f8b8}}, {{0xcff75cb0, 0x3fef6297}},
	{{0x39ae68c8, 0x3fe1c73b}}, {{0x290ea1a3, 0x3fea9b66}},
	{{0x290ea1a3, 0xbfea9b66}}, {{0x39ae68c8, 0x3fe1c73b}},
	{{0x290ea1a3, 0x3fea9b66}}, {{0x39ae68c8, 0x3fe1c73b}},
	{{0x39ae68c8, 0xbfe1c73b}}, {{0x290ea1a3, 0x3fea9b66}},
	{{0x3c69a60b, 0x3fc8f8b8}}, {{0xcff75cb0, 0x3fef6297}},
	{{0xcff75cb0, 0xbfef6297}}, {{0x3c69a60b, 0x3fc8f8b8}},
	{{0xa3d12526, 0x3fefd88d}}, {{0xbc29b42c, 0x3fb917a6}},
	{{0xbc29b42c, 0xbfb917a6}}, {{0xa3d12526, 0x3fefd88d}},
	{{0x25091dd6, 0x3fe44cf3}}, {{0x6b151741, 0x3fe8bc80}},
	{{0x6b151741, 0xbfe8bc80}}, {{0x25091dd6, 0x3fe44cf3}},
	{{0xf180bdb1, 0x3fec38b2}}, {{0x3806f63b, 0x3fde2b5d}},
	{{0x3806f63b, 0xbfde2b5d}}, {{0xf180bdb1, 0x3fec38b2}},
	{{0x2ed59f06, 0x3fd29406}}, {{0x56c62dda, 0x3fee9f41}},
	{{0x56c62dda, 0xbfee9f41}}, {{0x2ed59f06, 0x3fd29406}},
	{{0x56c62dda, 0x3fee9f41}}, {{0x2ed59f06, 0x3fd29406}},
	{{0x2ed59f06, 0xbfd29406}}, {{0x56c62dda, 0x3fee9f41}},
	{{0x3806f63b, 0x3fde2b5d}}, {{0xf180bdb1, 0x3fec38b2}},
	{{0xf180bdb1, 0xbfec38b2}}, {{0x3806f63b, 0x3fde2b5d}},
	{{0x6b151741, 0x3fe8bc80}}, {{0x25091dd6, 0x3fe44cf3}},
	{{0x25091dd6, 0xbfe44cf3}}, {{0x6b151741, 0x3fe8bc80}},
	{{0xbc29b42c, 0x3fb917a6}}, {{0xa3d12526, 0x3fefd88d}},
	{{0xa3d12526, 0xbfefd88d}}, {{0xbc29b42c, 0x3fb917a6}},
	{{0xe3796d7e, 0x3feff621}}, {{0xf10dd814, 0x3fa91f65}},
	{{0xf10dd814, 0xbfa91f65}}, {{0xe3796d7e, 0x3feff621}},
	{{0x348ceca0, 0x3fe57d69}}, {{0x226aafaf, 0x3fe7b5df}},
	{{0x226aafaf, 0xbfe7b5df}}, {{0x348ceca0, 0x3fe57d69}},
	{{0xf43cc773, 0x3feced7a}}, {{0x09e15cc0, 0x3fdb5d10}},
	{{0x09e15cc0, 0xbfdb5d10}}, {{0xf43cc773, 0x3feced7a}},
	{{0x75ab1fdd, 0x3fd58f9a}}, {{0x04f686e5, 0x3fee2121}},
	{{0x04f686e5, 0xbfee2121}}, {{0x75ab1fdd, 0x3fd58f9a}},
	{{0xfb9230d7, 0x3fef0a7e}}, {{0x7b215f1b, 0x3fcf19f9}},
	{{0x7b215f1b, 0xbfcf19f9}}, {{0xfb9230d7, 0x3fef0a7e}},
	{{0x9922ffee, 0x3fe07387}}, {{0x45196e3e, 0x3feb7283}},
	{{0x45196e3e, 0xbfeb7283}}, {{0x9922ffee, 0x3fe07387}},
	{{0x47f38741, 0x3fe9b3e0}}, {{0xfce17035, 0x3fe30ff7}},
	{{0xfce17035, 0xbfe30ff7}}, {{0x47f38741, 0x3fe9b3e0}},
	{{0x6e8e613a, 0x3fc2c810}}, {{0x7f08a517, 0x3fefa755}},
	{{0x7f08a517, 0xbfefa755}}, {{0x6e8e613a, 0x3fc2c810}},
	{{0x7f08a517, 0x3fefa755}}, {{0x6e8e613a, 0x3fc2c810}},
	{{0x6e8e613a, 0xbfc2c810}}, {{0x7f08a517, 0x3fefa755}},
	{{0xfce17035, 0x3fe30ff7}}, {{0x47f38741, 0x3fe9b3e0}},
	{{0x47f38741, 0xbfe9b3e0}}, {{0xfce17035, 0x3fe30ff7}},
	{{0x45196e3e, 0x3feb7283}}, {{0x9922ffee, 0x3fe07387}},
	{{0x9922ffee, 0xbfe07387}}, {{0x45196e3e, 0x3feb7283}},
	{{0x7b215f1b, 0x3fcf19f9}}, {{0xfb9230d7, 0x3fef0a7e}},
	{{0xfb9230d7, 0xbfef0a7e}}, {{0x7b215f1b, 0x3fcf19f9}},
	{{0x04f686e5, 0x3fee2121}}, {{0x75ab1fdd, 0x3fd58f9a}},
	{{0x75ab1fdd, 0xbfd58f9a}}, {{0x04f686e5, 0x3fee2121}},
	{{0x09e15cc0, 0x3fdb5d10}}, {{0xf43cc773, 0x3feced7a}},
	{{0xf43cc773, 0xbfeced7a}}, {{0x09e15cc0, 0x3fdb5d10}},
	{{0x226aafaf, 0x3fe7b5df}}, {{0x348ceca0, 0x3fe57d69}},
	{{0x348ceca0, 0xbfe57d69}}, {{0x226aafaf, 0x3fe7b5df}},
	{{0xf10dd814, 0x3fa91f65}}, {{0xe3796d7e, 0x3feff621}},
	{{0xe3796d7e, 0xbfeff621}}, {{0xf10dd814, 0x3fa91f65}},
	{{0x6084cd0d, 0x3feffd88}}, {{0xf7a3667e, 0x3f992155}},
	{{0xf7a3667e, 0xbf992155}}, {{0x6084cd0d, 0x3feffd88}},
	{{0x551d2cdf, 0x3fe610b7}}, {{0x37efff96, 0x3fe72d08}},
	{{0x37efff96, 0xbfe72d08}}, {{0x551d2cdf, 0x3fe610b7}},
	{{0xd14dc93a, 0x3fed4134}}, {{0x43a8ed8a, 0x3fd9ef79}},
	{{0x43a8ed8a, 0xbfd9ef79}}, {{0xd14dc93a, 0x3fed4134}},
	{{0x30fa459f, 0x3fd70885}}, {{0xb6ccc23c, 0x3feddb13}},
	{{0xb6ccc23c, 0xbfeddb13}}, {{0x30fa459f, 0x3fd70885}},
	{{0xac64e589, 0x3fef38f3}}, {{0x6a7e4f63, 0x3fcc0b82}},
	{{0x6a7e4f63, 0xbfcc0b82}}, {{0xac64e589, 0x3fef38f3}},
	{{0x541b4b23, 0x3fe11eb3}}, {{0x58150200, 0x3feb090a}},
	{{0x58150200, 0xbfeb090a}}, {{0x541b4b23, 0x3fe11eb3}},
	{{0xa0462782, 0x3fea29a7}}, {{0x4cdd12df, 0x3fe26d05}},
	{{0x4cdd12df, 0xbfe26d05}}, {{0xa0462782, 0x3fea29a7}},
	{{0x448b3fc6, 0x3fc5e214}}, {{0xfa714ba9, 0x3fef8764}},
	{{0xfa714ba9, 0xbfef8764}}, {{0x448b3fc6, 0x3fc5e214}},
	{{0x70e19fd3, 0x3fefc264}}, {{0x56a9730e, 0x3fbf564e}},
	{{0x56a9730e, 0xbfbf564e}}, {{0x70e19fd3, 0x3fefc264}},
	{{0x292050b9, 0x3fe3affa}}, {{0x499263fb, 0x3fe93a22}},
	{{0x499263fb, 0xbfe93a22}}, {{0x292050b9, 0x3fe3affa}},
	{{0xac6f952a, 0x3febd7c0}}, {{0xdbf89aba, 0x3fdf8ba4}},
	{{0xdbf89aba, 0xbfdf8ba4}}, {{0xac6f952a, 0x3febd7c0}},
	{{0x62b1f677, 0x3fd111d2}}, {{0xe7684963, 0x3feed740}},
	{{0xe7684963, 0xbfeed740}}, {{0x62b1f677, 0x3fd111d2}},
	{{0xec48e112, 0x3fee6288}}, {{0x94176601, 0x3fd4135c}},
	{{0x94176601, 0xbfd4135c}}, {{0xec48e112, 0x3fee6288}},
	{{0x9931c45e, 0x3fdcc66e}}, {{0x213411f5, 0x3fec954b}},
	{{0x213411f5, 0xbfec954b}}, {{0x9931c45e, 0x3fdcc66e}},
	{{0x0bff976e, 0x3fe83b0e}}, {{0xbbe3e5e9, 0x3fe4e6ca}},
	{{0xbbe3e5e9, 0xbfe4e6ca}}, {{0x0bff976e, 0x3fe83b0e}},
	{{0x92ce19f6, 0x3fb2d520}}, {{0xad01883a, 0x3fefe9cd}},
	{{0xad01883a, 0xbfefe9cd}}, {{0x92ce19f6, 0x3fb2d520}},
	{{0xad01883a, 0x3fefe9cd}}, {{0x92ce19f6, 0x3fb2d520}},
	{{0x92ce19f6, 0xbfb2d520}}, {{0xad01883a, 0x3fefe9cd}},
	{{0xbbe3e5e9, 0x3fe4e6ca}}, {{0x0bff976e, 0x3fe83b0e}},
	{{0x0bff976e, 0xbfe83b0e}}, {{0xbbe3e5e9, 0x3fe4e6ca}},
	{{0x213411f5, 0x3fec954b}}, {{0x9931c45e, 0x3fdcc66e}},
	{{0x9931c45e, 0xbfdcc66e}}, {{0x213411f5, 0x3fec954b}},
	{{0x94176601, 0x3fd4135c}}, {{0xec48e112, 0x3fee6288}},
	{{0xec48e112, 0xbfee6288}}, {{0x94176601, 0x3fd4135c}},
	{{0xe7684963, 0x3feed740}}, {{0x62b1f677, 0x3fd111d2}},
	{{0x62b1f677, 0xbfd111d2}}, {{0xe7684963, 0x3feed740}},
	{{0xdbf89aba, 0x3fdf8ba4}}, {{0xac6f952a, 0x3febd7c0}},
	{{0xac6f952a, 0xbfebd7c0}}, {{0xdbf89aba, 0x3fdf8ba4}},
	{{0x499263fb, 0x3fe93a22}}, {{0x292050b9, 0x3fe3affa}},
	{{0x292050b9, 0xbfe3affa}}, {{0x499263fb, 0x3fe93a22}},
	{{0x56a9730e, 0x3fbf564e}}, {{0x70e19fd3, 0x3fefc264}},
	{{0x70e19fd3, 0xbfefc264}}, {{0x56a9730e, 0x3fbf564e}},
	{{0xfa714ba9, 0x3fef8764}}, {{0x448b3fc6, 0x3fc5e214}},
	{{0x448b3fc6, 0xbfc5e214}}, {{0xfa714ba9, 0x3fef8764}},
	{{0x4cdd12df, 0x3fe26d05}}, {{0xa0462782, 0x3fea29a7}},
	{{0xa0462782, 0xbfea29a7}}, {{0x4cdd12df, 0x3fe26d05}},
	{{0x58150200, 0x3feb090a}}, {{0x541b4b23, 0x3fe11eb3}},
	{{0x541b4b23, 0xbfe11eb3}}, {{0x58150200, 0x3feb090a}},
	{{0x6a7e4f63, 0x3fcc0b82}}, {{0xac64e589, 0x3fef38f3}},
	{{0xac64e589, 0xbfef38f3}}, {{0x6a7e4f63, 0x3fcc0b82}},
	{{0xb6ccc23c, 0x3feddb13}}, {{0x30fa459f, 0x3fd70885}},
	{{0x30fa459f, 0xbfd70885}}, {{0xb6ccc23c, 0x3feddb13}},
	{{0x43a8ed8a, 0x3fd9ef79}}, {{0xd14dc93a, 0x3fed4134}},
	{{0xd14dc93a, 0xbfed4134}}, {{0x43a8ed8a, 0x3fd9ef79}},
	{{0x37efff96, 0x3fe72d08}}, {{0x551d2cdf, 0x3fe610b7}},
	{{0x551d2cdf, 0xbfe610b7}}, {{0x37efff96, 0x3fe72d08}},
	{{0xf7a3667e, 0x3f992155}}, {{0x6084cd0d, 0x3feffd88}},
	{{0x6084cd0d, 0xbfeffd88}}, {{0xf7a3667e, 0x3f992155}},
	{{0x169b92db, 0x3fefff62}}, {{0xfcdec784, 0x3f8921d1}},
	{{0xfcdec784, 0xbf8921d1}}, {{0x169b92db, 0x3fefff62}},
	{{0x25f0783d, 0x3fe65919}}, {{0x54eaa8af, 0x3fe6e744}},
	{{0x54eaa8af, 0xbfe6e744}}, {{0x25f0783d, 0x3fe65919}},
	{{0x73c9e68b, 0x3fed6961}}, {{0x63bc93d7, 0x3fd9372a}},
	{{0x63bc93d7, 0xbfd9372a}}, {{0x73c9e68b, 0x3fed6961}},
	{{0x311dcce7, 0x3fd7c3a9}}, {{0x6238a09b, 0x3fedb652}},
	{{0x6238a09b, 0xbfedb652}}, {{0x311dcce7, 0x3fd7c3a9}},
	{{0x3b0b2f2d, 0x3fef4e60}}, {{0x25b00451, 0x3fca82a0}},
	{{0x25b00451, 0xbfca82a0}}, {{0x3b0b2f2d, 0x3fef4e60}},
	{{0x63dedb49, 0x3fe1734d}}, {{0x9e21d511, 0x3fead2bc}},
	{{0x9e21d511, 0xbfead2bc}}, {{0x63dedb49, 0x3fe1734d}},
	{{0x1b02fae2, 0x3fea6309}}, {{0x9933eb59, 0x3fe21a79}},
	{{0x9933eb59, 0xbfe21a79}}, {{0x1b02fae2, 0x3fea6309}},
	{{0xde50bf31, 0x3fc76dd9}}, {{0xa3a12077, 0x3fef7599}},
	{{0xa3a12077, 0xbfef7599}}, {{0xde50bf31, 0x3fc76dd9}},
	{{0xfd6da67b, 0x3fefce15}}, {{0xc79ec2d5, 0x3fbc3785}},
	{{0xc79ec2d5, 0xbfbc3785}}, {{0xfd6da67b, 0x3fefce15}},
	{{0x534556d4, 0x3fe3fed9}}, {{0xa3ef940d, 0x3fe8fbcc}},
	{{0xa3ef940d, 0xbfe8fbcc}}, {{0x534556d4, 0x3fe3fed9}},
	{{0x26725549, 0x3fec08c4}}, {{0x52ef78d6, 0x3fdedc19}},
	{{0x52ef78d6, 0xbfdedc19}}, {{0x26725549, 0x3fec08c4}},
	{{0x3f4cdb3e, 0x3fd1d344}}, {{0xc8df0b74, 0x3feebbd8}},
	{{0xc8df0b74, 0xbfeebbd8}}, {{0x3f4cdb3e, 0x3fd1d344}},
	{{0xab4cd10d, 0x3fee817b}}, {{0xc2e18152, 0x3fd35410}},
	{{0xc2e18152, 0xbfd35410}}, {{0xab4cd10d, 0x3fee817b}},
	{{0x5b86e389, 0x3fdd7977}}, {{0x3488739b, 0x3fec678b}},
	{{0x3488739b, 0xbfec678b}}, {{0x5b86e389, 0x3fdd7977}},
	{{0x0fba2ebf, 0x3fe87c40}}, {{0x9b9b0939, 0x3fe49a44}},
	{{0x9b9b0939, 0xbfe49a44}}, {{0x0fba2ebf, 0x3fe87c40}},
	{{0x0a9aa419, 0x3fb5f6d0}}, {{0xfcbd5b09, 0x3fefe1ca}},
	{{0xfcbd5b09, 0xbfefe1ca}}, {{0x0a9aa419, 0x3fb5f6d0}},
	{{0x658e71ad, 0x3feff095}}, {{0x79f820e0, 0x3faf656e}},
	{{0x79f820e0, 0xbfaf656e}}, {{0x658e71ad, 0x3feff095}},
	{{0x92a35596, 0x3fe53282}}, {{0xe3571771, 0x3fe7f8ec}},
	{{0xe3571771, 0xbfe7f8ec}}, {{0x92a35596, 0x3fe53282}},
	{{0xf3fcfc5c, 0x3fecc1f0}}, {{0xd8011ee7, 0x3fdc1249}},
	{{0xd8011ee7, 0xbfdc1249}}, {{0xf3fcfc5c, 0x3fecc1f0}},
	{{0x4278e76a, 0x3fd4d1e2}}, {{0x4b2bc17e, 0x3fee426a}},
	{{0x4b2bc17e, 0xbfee426a}}, {{0x4278e76a, 0x3fd4d1e2}},
	{{0xa3e473c2, 0x3feef178}}, {{0x0e37fdae, 0x3fd04fb8}},
	{{0x0e37fdae, 0xbfd04fb8}}, {{0xa3e473c2, 0x3feef178}},
	{{0x874c3eb7, 0x3fe01cfc}}, {{0x673590d2, 0x3feba5aa}},
	{{0x673590d2, 0xbfeba5aa}}, {{0x874c3eb7, 0x3fe01cfc}},
	{{0xf4c7d742, 0x3fe9777e}}, {{0xb10659f3, 0x3fe36058}},
	{{0xb10659f3, 0xbfe36058}}, {{0xf4c7d742, 0x3fe9777e}},
	{{0xcedaf577, 0x3fc139f0}}, {{0x7195d741, 0x3fefb579}},
	{{0x7195d741, 0xbfefb579}}, {{0xcedaf577, 0x3fc139f0}},
	{{0x24c9099b, 0x3fef97f9}}, {{0xb1293e5a, 0x3fc45576}},
	{{0xb1293e5a, 0xbfc45576}}, {{0x24c9099b, 0x3fef97f9}},
	{{0x25faf3ea, 0x3fe2bedb}}, {{0xef29af94, 0x3fe9ef43}},
	{{0xef29af94, 0xbfe9ef43}}, {{0x25faf3ea, 0x3fe2bedb}},
	{{0x3ef55712, 0x3feb3e4d}}, {{0x4d5d898f, 0x3fe0c970}},
	{{0x4d5d898f, 0xbfe0c970}}, {{0x3ef55712, 0x3feb3e4d}},
	{{0xe5454311, 0x3fcd934f}}, {{0xf7763ada, 0x3fef2252}},
	{{0xf7763ada, 0xbfef2252}}, {{0xe5454311, 0x3fcd934f}},
	{{0x622dbe2b, 0x3fedfeae}}, {{0xdd3f27c6, 0x3fd64c7d}},
	{{0xdd3f27c6, 0xbfd64c7d}}, {{0x622dbe2b, 0x3fedfeae}},
	{{0x2b6d3fca, 0x3fdaa6c8}}, {{0x743e35dc, 0x3fed17e7}},
	{{0x743e35dc, 0xbfed17e7}}, {{0x2b6d3fca, 0x3fdaa6c8}},
	{{0x5f037261, 0x3fe771e7}}, {{0xbe65018c, 0x3fe5c77b}},
	{{0xbe65018c, 0xbfe5c77b}}, {{0x5f037261, 0x3fe771e7}},
	{{0x759455cd, 0x3fa2d865}}, {{0xeffef75d, 0x3feffa72}},
	{{0xeffef75d, 0xbfeffa72}}, {{0x759455cd, 0x3fa2d865}},
	{{0xeffef75d, 0x3feffa72}}, {{0x759455cd, 0x3fa2d865}},
	{{0x759455cd, 0xbfa2d865}}, {{0xeffef75d, 0x3feffa72}},
	{{0xbe65018c, 0x3fe5c77b}}, {{0x5f037261, 0x3fe771e7}},
	{{0x5f037261, 0xbfe771e7}}, {{0xbe65018c, 0x3fe5c77b}},
	{{0x743e35dc, 0x3fed17e7}}, {{0x2b6d3fca, 0x3fdaa6c8}},
	{{0x2b6d3fca, 0xbfdaa6c8}}, {{0x743e35dc, 0x3fed17e7}},
	{{0xdd3f27c6, 0x3fd64c7d}}, {{0x622dbe2b, 0x3fedfeae}},
	{{0x622dbe2b, 0xbfedfeae}}, {{0xdd3f27c6, 0x3fd64c7d}},
	{{0xf7763ada, 0x3fef2252}}, {{0xe5454311, 0x3fcd934f}},
	{{0xe5454311, 0xbfcd934f}}, {{0xf7763ada, 0x3fef2252}},
	{{0x4d5d898f, 0x3fe0c970}}, {{0x3ef55712, 0x3feb3e4d}},
	{{0x3ef55712, 0xbfeb3e4d}}, {{0x4d5d898f, 0x3fe0c970}},
	{{0xef29af94, 0x3fe9ef43}}, {{0x25faf3ea, 0x3fe2bedb}},
	{{0x25faf3ea, 0xbfe2bedb}}, {{0xef29af94, 0x3fe9ef43}},
	{{0xb1293e5a, 0x3fc45576}}, {{0x24c9099b, 0x3fef97f9}},
	{{0x24c9099b, 0xbfef97f9}}, {{0xb1293e5a, 0x3fc45576}},
	{{0x7195d741, 0x3fefb579}}, {{0xcedaf577, 0x3fc139f0}},
	{{0xcedaf577, 0xbfc139f0}}, {{0x7195d741, 0x3fefb579}},
	{{0xb10659f3, 0x3fe36058}}, {{0xf4c7d742, 0x3fe9777e}},
	{{0xf4c7d742, 0xbfe9777e}}, {{0xb10659f3, 0x3fe36058}},
	{{0x673590d2, 0x3feba5aa}}, {{0x874c3eb7, 0x3fe01cfc}},
	{{0x874c3eb7, 0xbfe01cfc}}, {{0x673590d2, 0x3feba5aa}},
	{{0x0e37fdae, 0x3fd04fb8}}, {{0xa3e473c2, 0x3feef178}},
	{{0xa3e473c2, 0xbfeef178}}, {{0x0e37fdae, 0x3fd04fb8}},
	{{0x4b2bc17e, 0x3fee426a}}, {{0x4278e76a, 0x3fd4d1e2}},
	{{0x4278e76a, 0xbfd4d1e2}}, {{0x4b2bc17e, 0x3fee426a}},
	{{0xd8011ee7, 0x3fdc1249}}, {{0xf3fcfc5c, 0x3fecc1f0}},
	{{0xf3fcfc5c, 0xbfecc1f0}}, {{0xd8011ee7, 0x3fdc1249}},
	{{0xe3571771, 0x3fe7f8ec}}, {{0x92a35596, 0x3fe53282}},
	{{0x92a35596, 0xbfe53282}}, {{0xe3571771, 0x3fe7f8ec}},
	{{0x79f820e0, 0x3faf656e}}, {{0x658e71ad, 0x3feff095}},
	{{0x658e71ad, 0xbfeff095}}, {{0x79f820e0, 0x3faf656e}},
	{{0xfcbd5b09, 0x3fefe1ca}}, {{0x0a9aa419, 0x3fb5f6d0}},
	{{0x0a9aa419, 0xbfb5f6d0}}, {{0xfcbd5b09, 0x3fefe1ca}},
	{{0x9b9b0939, 0x3fe49a44}}, {{0x0fba2ebf, 0x3fe87c40}},
	{{0x0fba2ebf, 0xbfe87c40}}, {{0x9b9b0939, 0x3fe49a44}},
	{{0x3488739b, 0x3fec678b}}, {{0x5b86e389, 0x3fdd7977}},
	{{0x5b86e389, 0xbfdd7977}}, {{0x3488739b, 0x3fec678b}},
	{{0xc2e18152, 0x3fd35410}}, {{0xab4cd10d, 0x3fee817b}},
	{{0xab4cd10d, 0xbfee817b}}, {{0xc2e18152, 0x3fd35410}},
	{{0xc8df0b74, 0x3feebbd8}}, {{0x3f4cdb3e, 0x3fd1d344}},
	{{0x3f4cdb3e, 0xbfd1d344}}, {{0xc8df0b74, 0x3feebbd8}},
	{{0x52ef78d6, 0x3fdedc19}}, {{0x26725549, 0x3fec08c4}},
	{{0x26725549, 0xbfec08c4}}, {{0x52ef78d6, 0x3fdedc19}},
	{{0xa3ef940d, 0x3fe8fbcc}}, {{0x534556d4, 0x3fe3fed9}},
	{{0x534556d4, 0xbfe3fed9}}, {{0xa3ef940d, 0x3fe8fbcc}},
	{{0xc79ec2d5, 0x3fbc3785}}, {{0xfd6da67b, 0x3fefce15}},
	{{0xfd6da67b, 0xbfefce15}}, {{0xc79ec2d5, 0x3fbc3785}},
	{{0xa3a12077, 0x3fef7599}}, {{0xde50bf31, 0x3fc76dd9}},
	{{0xde50bf31, 0xbfc76dd9}}, {{0xa3a12077, 0x3fef7599}},
	{{0x9933eb59, 0x3fe21a79}}, {{0x1b02fae2, 0x3fea6309}},
	{{0x1b02fae2, 0xbfea6309}}, {{0x9933eb59, 0x3fe21a79}},
	{{0x9e21d511, 0x3fead2bc}}, {{0x63dedb49, 0x3fe1734d}},
	{{0x63dedb49, 0xbfe1734d}}, {{0x9e21d511, 0x3fead2bc}},
	{{0x25b00451, 0x3fca82a0}}, {{0x3b0b2f2d, 0x3fef4e60}},
	{{0x3b0b2f2d, 0xbfef4e60}}, {{0x25b00451, 0x3fca82a0}},
	{{0x6238a09b, 0x3fedb652}}, {{0x311dcce7, 0x3fd7c3a9}},
	{{0x311dcce7, 0xbfd7c3a9}}, {{0x6238a09b, 0x3fedb652}},
	{{0x63bc93d7, 0x3fd9372a}}, {{0x73c9e68b, 0x3fed6961}},
	{{0x73c9e68b, 0xbfed6961}}, {{0x63bc93d7, 0x3fd9372a}},
	{{0x54eaa8af, 0x3fe6e744}}, {{0x25f0783d, 0x3fe65919}},
	{{0x25f0783d, 0xbfe65919}}, {{0x54eaa8af, 0x3fe6e744}},
	{{0xfcdec784, 0x3f8921d1}}, {{0x169b92db, 0x3fefff62}},
	{{0x169b92db, 0xbfefff62}}, {{0xfcdec784, 0x3f8921d1}},
	{{0x858e8a92, 0x3fefffd8}}, {{0xfe670071, 0x3f7921f0}},
	{{0xfe670071, 0xbf7921f0}}, {{0x858e8a92, 0x3fefffd8}},
	{{0x8491af10, 0x3fe67cf7}}, {{0x73c18275, 0x3fe6c40d}},
	{{0x73c18275, 0xbfe6c40d}}, {{0x8491af10, 0x3fe67cf7}},
	{{0x02b8ecf9, 0x3fed7d0b}}, {{0x2ec8a4b0, 0x3fd8daa5}},
	{{0x2ec8a4b0, 0xbfd8daa5}}, {{0x02b8ecf9, 0x3fed7d0b}},
	{{0xb04eaac4, 0x3fd820e3}}, {{0xa9668988, 0x3feda383}},
	{{0xa9668988, 0xbfeda383}}, {{0xb04eaac4, 0x3fd820e3}},
	{{0xb1789e84, 0x3fef58a2}}, {{0xf2dc4366, 0x3fc9bdcb}},
	{{0xf2dc4366, 0xbfc9bdcb}}, {{0xb1789e84, 0x3fef58a2}},
	{{0x09f2b9b8, 0x3fe19d5a}}, {{0x5916c0d4, 0x3feab732}},
	{{0x5916c0d4, 0xbfeab732}}, {{0x09f2b9b8, 0x3fe19d5a}},
	{{0x529fe69d, 0x3fea7f58}}, {{0x8bbc861b, 0x3fe1f0f0}},
	{{0x8bbc861b, 0xbfe1f0f0}}, {{0x529fe69d, 0x3fea7f58}},
	{{0xe89c64c6, 0x3fc83366}}, {{0x7df5bbb7, 0x3fef6c3f}},
	{{0x7df5bbb7, 0xbfef6c3f}}, {{0xe89c64c6, 0x3fc83366}},
	{{0x14220b84, 0x3fefd379}}, {{0x24495c03, 0x3fbaa7b7}},
	{{0x24495c03, 0xbfbaa7b7}}, {{0x14220b84, 0x3fefd379}},
	{{0x178e6bb1, 0x3fe425ff}}, {{0x331698cc, 0x3fe8dc45}},
	{{0x331698cc, 0xbfe8dc45}}, {{0x178e6bb1, 0x3fe425ff}},
	{{0x3fa971b0, 0x3fec20de}}, {{0xeaf85114, 0x3fde83e0}},
	{{0xeaf85114, 0xbfde83e0}}, {{0x3fa971b0, 0x3fec20de}},
	{{0xabc3bb71, 0x3fd233bb}}, {{0xe8e7a88e, 0x3feeadb2}},
	{{0xe8e7a88e, 0xbfeeadb2}}, {{0xabc3bb71, 0x3fd233bb}},
	{{0x361df7f2, 0x3fee9084}}, {{0xdaec0387, 0x3fd2f422}},
	{{0xdaec0387, 0xbfd2f422}}, {{0x361df7f2, 0x3fee9084}},
	{{0x1481cc58, 0x3fddd28f}}, {{0x012b6907, 0x3fec5042}},
	{{0x012b6907, 0xbfec5042}}, {{0x1481cc58, 0x3fddd28f}},
	{{0x9a4dd4aa, 0x3fe89c7e}}, {{0x1b987347, 0x3fe473b5}},
	{{0x1b987347, 0xbfe473b5}}, {{0x9a4dd4aa, 0x3fe89c7e}},
	{{0x6a5d5b21, 0x3fb78758}}, {{0x9ff1f456, 0x3fefdd53}},
	{{0x9ff1f456, 0xbfefdd53}}, {{0x6a5d5b21, 0x3fb78758}},
	{{0x0f8d575c, 0x3feff383}}, {{0x12c0d7e3, 0x3fac428d}},
	{{0x12c0d7e3, 0xbfac428d}}, {{0x0f8d575c, 0x3feff383}},
	{{0x38975137, 0x3fe55810}}, {{0x6cc33db2, 0x3fe7d783}},
	{{0x6cc33db2, 0xbfe7d783}}, {{0x38975137, 0x3fe55810}},
	{{0x898b32f6, 0x3fecd7d9}}, {{0x2304bd01, 0x3fdbb7cf}},
	{{0x2304bd01, 0xbfdbb7cf}}, {{0x898b32f6, 0x3fecd7d9}},
	{{0x80af3c24, 0x3fd530d8}}, {{0xe870ce25, 0x3fee31ea}},
	{{0xe870ce25, 0xbfee31ea}}, {{0x80af3c24, 0x3fd530d8}},
	{{0x0c0b95ec, 0x3feefe22}}, {{0x1adfedf9, 0x3fcfdcdc}},
	{{0x1adfedf9, 0xbfcfdcdc}}, {{0x0c0b95ec, 0x3feefe22}},
	{{0x26ae221a, 0x3fe04856}}, {{0xd27504e9, 0x3feb8c38}},
	{{0xd27504e9, 0xbfeb8c38}}, {{0x26ae221a, 0x3fe04856}},
	{{0x2ed80d22, 0x3fe995cf}}, {{0x0d0c8e57, 0x3fe33840}},
	{{0x0d0c8e57, 0xbfe33840}}, {{0x2ed80d22, 0x3fe995cf}},
	{{0xd4ec7bcf, 0x3fc20116}}, {{0x8e46cfbb, 0x3fefae8e}},
	{{0x8e46cfbb, 0xbfefae8e}}, {{0xd4ec7bcf, 0x3fc20116}},
	{{0x55adb2c8, 0x3fef9fce}}, {{0xb0cd8d14, 0x3fc38edb}},
	{{0xb0cd8d14, 0xbfc38edb}}, {{0x55adb2c8, 0x3fef9fce}},
	{{0xe3e8ea17, 0x3fe2e780}}, {{0xf5ea80d5, 0x3fe9d1b1}},
	{{0xf5ea80d5, 0xbfe9d1b1}}, {{0xe3e8ea17, 0x3fe2e780}},
	{{0xfe921405, 0x3feb5889}}, {{0x7417c5e1, 0x3fe09e90}},
	{{0x7417c5e1, 0xbfe09e90}}, {{0xfe921405, 0x3feb5889}},
	{{0x1e101a1b, 0x3fce56ca}}, {{0x53f7205d, 0x3fef168f}},
	{{0x53f7205d, 0xbfef168f}}, {{0x1e101a1b, 0x3fce56ca}},
	{{0xca2980ac, 0x3fee100c}}, {{0x379ea693, 0x3fd5ee27}},
	{{0x379ea693, 0xbfd5ee27}}, {{0xca2980ac, 0x3fee100c}},
	{{0x6c7f4009, 0x3fdb020d}}, {{0xfeb2bd92, 0x3fed02d4}},
	{{0xfeb2bd92, 0xbfed02d4}}, {{0x6c7f4009, 0x3fdb020d}},
	{{0x574f55e5, 0x3fe79400}}, {{0x2a5d7250, 0x3fe5a28d}},
	{{0x2a5d7250, 0xbfe5a28d}}, {{0x574f55e5, 0x3fe79400}},
	{{0xd290cd43, 0x3fa5fc00}}, {{0xdadb81df, 0x3feff871}},
	{{0xdadb81df, 0xbfeff871}}, {{0xd290cd43, 0x3fa5fc00}},
	{{0x1df1d3f8, 0x3feffc25}}, {{0x31d1cf01, 0x3f9f6937}},
	{{0x31d1cf01, 0xbf9f6937}}, {{0x1df1d3f8, 0x3feffc25}},
	{{0x95837074, 0x3fe5ec34}}, {{0x8da8d28d, 0x3fe74f94}},
	{{0x8da8d28d, 0xbfe74f94}}, {{0x95837074, 0x3fe5ec34}},
	{{0x20e0ef9f, 0x3fed2cb2}}, {{0x27dea1e5, 0x3fda4b41}},
	{{0x27dea1e5, 0xbfda4b41}}, {{0x20e0ef9f, 0x3fed2cb2}},
	{{0x7dc77e17, 0x3fd6aa9d}}, {{0xf7de47da, 0x3feded05}},
	{{0xf7de47da, 0xbfeded05}}, {{0x7dc77e17, 0x3fd6aa9d}},
	{{0xc9089a9d, 0x3fef2dc9}}, {{0xb312b286, 0x3fcccf8c}},
	{{0xb312b286, 0xbfcccf8c}}, {{0xc9089a9d, 0x3fef2dc9}},
	{{0xbb2a8e7e, 0x3fe0f426}}, {{0x470013b4, 0x3feb23cd}},
	{{0x470013b4, 0xbfeb23cd}}, {{0xbb2a8e7e, 0x3fe0f426}},
	{{0xeabaf937, 0x3fea0c95}}, {{0x27629ca8, 0x3fe29607}},
	{{0x27629ca8, 0xbfe29607}}, {{0xeabaf937, 0x3fea0c95}},
	{{0x8597c5f2, 0x3fc51bdf}}, {{0xffae41db, 0x3fef8fd5}},
	{{0xffae41db, 0xbfef8fd5}}, {{0x8597c5f2, 0x3fc51bdf}},
	{{0x17e44186, 0x3fefbc16}}, {{0x47ba831d, 0x3fc072a0}},
	{{0x47ba831d, 0xbfc072a0}}, {{0x17e44186, 0x3fefbc16}},
	{{0x85dfeb22, 0x3fe38841}}, {{0xe48e6dd7, 0x3fe958ef}},
	{{0xe48e6dd7, 0xbfe958ef}}, {{0x85dfeb22, 0x3fe38841}},
	{{0xc49380ea, 0x3febbed7}}, {{0x4be71210, 0x3fdfe2f6}},
	{{0x4be71210, 0xbfdfe2f6}}, {{0xc49380ea, 0x3febbed7}},
	{{0xcfdbdb90, 0x3fd0b0d9}}, {{0xe25a9dbc, 0x3feee482}},
	{{0xe25a9dbc, 0xbfeee482}}, {{0xcfdbdb90, 0x3fd0b0d9}},
	{{0x04729ffc, 0x3fee529f}}, {{0xa5571054, 0x3fd472b8}},
	{{0xa5571054, 0xbfd472b8}}, {{0x04729ffc, 0x3fee529f}},
	{{0x4997000b, 0x3fdc6c7f}}, {{0x69a0b900, 0x3fecabc1}},
	{{0x69a0b900, 0xbfecabc1}}, {{0x4997000b, 0x3fdc6c7f}},
	{{0x33b57acc, 0x3fe81a1b}}, {{0x9f59a09b, 0x3fe50cc0}},
	{{0x9f59a09b, 0xbfe50cc0}}, {{0x33b57acc, 0x3fe81a1b}},
	{{0x34d709b3, 0x3fb14401}}, {{0xecb673c4, 0x3fefed58}},
	{{0xecb673c4, 0xbfefed58}}, {{0x34d709b3, 0x3fb14401}},
	{{0xaf2e3940, 0x3fefe5f3}}, {{0x79272096, 0x3fb46611}},
	{{0x79272096, 0xbfb46611}}, {{0xaf2e3940, 0x3fefe5f3}},
	{{0x45ec0004, 0x3fe4c0a1}}, {{0x1ae958cc, 0x3fe85bc5}},
	{{0x1ae958cc, 0xbfe85bc5}}, {{0x45ec0004, 0x3fe4c0a1}},
	{{0x52233cf3, 0x3fec7e8e}}, {{0xe8e9db5b, 0x3fdd2016}},
	{{0xe8e9db5b, 0xbfdd2016}}, {{0x52233cf3, 0x3fec7e8e}},
	{{0xfa0414b7, 0x3fd3b3ce}}, {{0xdb6a9744, 0x3fee7227}},
	{{0xdb6a9744, 0xbfee7227}}, {{0xfa0414b7, 0x3fd3b3ce}},
	{{0xd3c3bf84, 0x3feec9b2}}, {{0xd7765177, 0x3fd172a0}},
	{{0xd7765177, 0xbfd172a0}}, {{0xd3c3bf84, 0x3feec9b2}},
	{{0x963fd067, 0x3fdf3405}}, {{0xe15377dd, 0x3febf064}},
	{{0xe15377dd, 0xbfebf064}}, {{0x963fd067, 0x3fdf3405}},
	{{0x6fd49da2, 0x3fe91b16}}, {{0x38c58344, 0x3fe3d782}},
	{{0x38c58344, 0xbfe3d782}}, {{0x6fd49da2, 0x3fe91b16}},
	{{0xcbae9fc9, 0x3fbdc70e}}, {{0x6cfeb721, 0x3fefc864}},
	{{0x6cfeb721, 0xbfefc864}}, {{0xcbae9fc9, 0x3fbdc70e}},
	{{0x29e63d6e, 0x3fef7ea6}}, {{0x04f64ab2, 0x3fc6a813}},
	{{0x04f64ab2, 0xbfc6a813}}, {{0x29e63d6e, 0x3fef7ea6}},
	{{0xfb98ac1f, 0x3fe243d5}}, {{0xc8119ac8, 0x3fea4678}},
	{{0xc8119ac8, 0xbfea4678}}, {{0xfb98ac1f, 0x3fe243d5}},
	{{0xb43c1474, 0x3feaee04}}, {{0xaf336ceb, 0x3fe14915}},
	{{0xaf336ceb, 0xbfe14915}}, {{0xb43c1474, 0x3feaee04}},
	{{0xef3d6722, 0x3fcb4732}}, {{0x85ff92dd, 0x3fef43d0}},
	{{0x85ff92dd, 0xbfef43d0}}, {{0xef3d6722, 0x3fcb4732}},
	{{0xcb410260, 0x3fedc8d7}}, {{0x0f2418f6, 0x3fd76634}},
	{{0x0f2418f6, 0xbfd76634}}, {{0xcb410260, 0x3fedc8d7}},
	{{0x6141bdff, 0x3fd99371}}, {{0x52e93eb1, 0x3fed556f}},
	{{0x52e93eb1, 0xbfed556f}}, {{0x6141bdff, 0x3fd99371}},
	{{0xb3176d7a, 0x3fe70a42}}, {{0xa31c1be9, 0x3fe63503}},
	{{0xa31c1be9, 0xbfe63503}}, {{0xb3176d7a, 0x3fe70a42}},
	{{0xbbe30efd, 0x3f92d936}}, {{0xb44b51a1, 0x3feffe9c}},
	{{0xb44b51a1, 0xbfeffe9c}}, {{0xbbe30efd, 0x3f92d936}},
	{{0xb44b51a1, 0x3feffe9c}}, {{0xbbe30efd, 0x3f92d936}},
	{{0xbbe30efd, 0xbf92d936}}, {{0xb44b51a1, 0x3feffe9c}},
	{{0xa31c1be9, 0x3fe63503}}, {{0xb3176d7a, 0x3fe70a42}},
	{{0xb3176d7a, 0xbfe70a42}}, {{0xa31c1be9, 0x3fe63503}},
	{{0x52e93eb1, 0x3fed556f}}, {{0x6141bdff, 0x3fd99371}},
	{{0x6141bdff, 0xbfd99371}}, {{0x52e93eb1, 0x3fed556f}},
	{{0x0f2418f6, 0x3fd76634}}, {{0xcb410260, 0x3fedc8d7}},
	{{0xcb410260, 0xbfedc8d7}}, {{0x0f2418f6, 0x3fd76634}},
	{{0x85ff92dd, 0x3fef43d0}}, {{0xef3d6722, 0x3fcb4732}},
	{{0xef3d6722, 0xbfcb4732}}, {{0x85ff92dd, 0x3fef43d0}},
	{{0xaf336ceb, 0x3fe14915}}, {{0xb43c1474, 0x3feaee04}},
	{{0xb43c1474, 0xbfeaee04}}, {{0xaf336ceb, 0x3fe14915}},
	{{0xc8119ac8, 0x3fea4678}}, {{0xfb98ac1f, 0x3fe243d5}},
	{{0xfb98ac1f, 0xbfe243d5}}, {{0xc8119ac8, 0x3fea4678}},
	{{0x04f64ab2, 0x3fc6a813}}, {{0x29e63d6e, 0x3fef7ea6}},
	{{0x29e63d6e, 0xbfef7ea6}}, {{0x04f64ab2, 0x3fc6a813}},
	{{0x6cfeb721, 0x3fefc864}}, {{0xcbae9fc9, 0x3fbdc70e}},
	{{0xcbae9fc9, 0xbfbdc70e}}, {{0x6cfeb721, 0x3fefc864}},
	{{0x38c58344, 0x3fe3d782}}, {{0x6fd49da2, 0x3fe91b16}},
	{{0x6fd49da2, 0xbfe91b16}}, {{0x38c58344, 0x3fe3d782}},
	{{0xe15377dd, 0x3febf064}}, {{0x963fd067, 0x3fdf3405}},
	{{0x963fd067, 0xbfdf3405}}, {{0xe15377dd, 0x3febf064}},
	{{0xd7765177, 0x3fd172a0}}, {{0xd3c3bf84, 0x3feec9b2}},
	{{0xd3c3bf84, 0xbfeec9b2}}, {{0xd7765177, 0x3fd172a0}},
	{{0xdb6a9744, 0x3fee7227}}, {{0xfa0414b7, 0x3fd3b3ce}},
	{{0xfa0414b7, 0xbfd3b3ce}}, {{0xdb6a9744, 0x3fee7227}},
	{{0xe8e9db5b, 0x3fdd2016}}, {{0x52233cf3, 0x3fec7e8e}},
	{{0x52233cf3, 0xbfec7e8e}}, {{0xe8e9db5b, 0x3fdd2016}},
	{{0x1ae958cc, 0x3fe85bc5}}, {{0x45ec0004, 0x3fe4c0a1}},
	{{0x45ec0004, 0xbfe4c0a1}}, {{0x1ae958cc, 0x3fe85bc5}},
	{{0x79272096, 0x3fb46611}}, {{0xaf2e3940, 0x3fefe5f3}},
	{{0xaf2e3940, 0xbfefe5f3}}, {{0x79272096, 0x3fb46611}},
	{{0xecb673c4, 0x3fefed58}}, {{0x34d709b3, 0x3fb14401}},
	{{0x34d709b3, 0xbfb14401}}, {{0xecb673c4, 0x3fefed58}},
	{{0x9f59a09b, 0x3fe50cc0}}, {{0x33b57acc, 0x3fe81a1b}},
	{{0x33b57acc, 0xbfe81a1b}}, {{0x9f59a09b, 0x3fe50cc0}},
	{{0x69a0b900, 0x3fecabc1}}, {{0x4997000b, 0x3fdc6c7f}},
	{{0x4997000b, 0xbfdc6c7f}}, {{0x69a0b900, 0x3fecabc1}},
	{{0xa5571054, 0x3fd472b8}}, {{0x04729ffc, 0x3fee529f}},
	{{0x04729ffc, 0xbfee529f}}, {{0xa5571054, 0x3fd472b8}},
	{{0xe25a9dbc, 0x3feee482}}, {{0xcfdbdb90, 0x3fd0b0d9}},
	{{0xcfdbdb90, 0xbfd0b0d9}}, {{0xe25a9dbc, 0x3feee482}},
	{{0x4be71210, 0x3fdfe2f6}}, {{0xc49380ea, 0x3febbed7}},
	{{0xc49380ea, 0xbfebbed7}}, {{0x4be71210, 0x3fdfe2f6}},
	{{0xe48e6dd7, 0x3fe958ef}}, {{0x85dfeb22, 0x3fe38841}},
	{{0x85dfeb22, 0xbfe38841}}, {{0xe48e6dd7, 0x3fe958ef}},
	{{0x47ba831d, 0x3fc072a0}}, {{0x17e44186, 0x3fefbc16}},
	{{0x17e44186, 0xbfefbc16}}, {{0x47ba831d, 0x3fc072a0}},
	{{0xffae41db, 0x3fef8fd5}}, {{0x8597c5f2, 0x3fc51bdf}},
	{{0x8597c5f2, 0xbfc51bdf}}, {{0xffae41db, 0x3fef8fd5}},
	{{0x27629ca8, 0x3fe29607}}, {{0xeabaf937, 0x3fea0c95}},
	{{0xeabaf937, 0xbfea0c95}}, {{0x27629ca8, 0x3fe29607}},
	{{0x470013b4, 0x3feb23cd}}, {{0xbb2a8e7e, 0x3fe0f426}},
	{{0xbb2a8e7e, 0xbfe0f426}}, {{0x470013b4, 0x3feb23cd}},
	{{0xb312b286, 0x3fcccf8c}}, {{0xc9089a9d, 0x3fef2dc9}},
	{{0xc9089a9d, 0xbfef2dc9}}, {{0xb312b286, 0x3fcccf8c}},
	{{0xf7de47da, 0x3feded05}}, {{0x7dc77e17, 0x3fd6aa9d}},
	{{0x7dc77e17, 0xbfd6aa9d}}, {{0xf7de47da, 0x3feded05}},
	{{0x27dea1e5, 0x3fda4b41}}, {{0x20e0ef9f, 0x3fed2cb2}},
	{{0x20e0ef9f, 0xbfed2cb2}}, {{0x27dea1e5, 0x3fda4b41}},
	{{0x8da8d28d, 0x3fe74f94}}, {{0x95837074, 0x3fe5ec34}},
	{{0x95837074, 0xbfe5ec34}}, {{0x8da8d28d, 0x3fe74f94}},
	{{0x31d1cf01, 0x3f9f6937}}, {{0x1df1d3f8, 0x3feffc25}},
	{{0x1df1d3f8, 0xbfeffc25}}, {{0x31d1cf01, 0x3f9f6937}},
	{{0xdadb81df, 0x3feff871}}, {{0xd290cd43, 0x3fa5fc00}},
	{{0xd290cd43, 0xbfa5fc00}}, {{0xdadb81df, 0x3feff871}},
	{{0x2a5d7250, 0x3fe5a28d}}, {{0x574f55e5, 0x3fe79400}},
	{{0x574f55e5, 0xbfe79400}}, {{0x2a5d7250, 0x3fe5a28d}},
	{{0xfeb2bd92, 0x3fed02d4}}, {{0x6c7f4009, 0x3fdb020d}},
	{{0x6c7f4009, 0xbfdb020d}}, {{0xfeb2bd92, 0x3fed02d4}},
	{{0x379ea693, 0x3fd5ee27}}, {{0xca2980ac, 0x3fee100c}},
	{{0xca2980ac, 0xbfee100c}}, {{0x379ea693, 0x3fd5ee27}},
	{{0x53f7205d, 0x3fef168f}}, {{0x1e101a1b, 0x3fce56ca}},
	{{0x1e101a1b, 0xbfce56ca}}, {{0x53f7205d, 0x3fef168f}},
	{{0x7417c5e1, 0x3fe09e90}}, {{0xfe921405, 0x3feb5889}},
	{{0xfe921405, 0xbfeb5889}}, {{0x7417c5e1, 0x3fe09e90}},
	{{0xf5ea80d5, 0x3fe9d1b1}}, {{0xe3e8ea17, 0x3fe2e780}},
	{{0xe3e8ea17, 0xbfe2e780}}, {{0xf5ea80d5, 0x3fe9d1b1}},
	{{0xb0cd8d14, 0x3fc38edb}}, {{0x55adb2c8, 0x3fef9fce}},
	{{0x55adb2c8, 0xbfef9fce}}, {{0xb0cd8d14, 0x3fc38edb}},
	{{0x8e46cfbb, 0x3fefae8e}}, {{0xd4ec7bcf, 0x3fc20116}},
	{{0xd4ec7bcf, 0xbfc20116}}, {{0x8e46cfbb, 0x3fefae8e}},
	{{0x0d0c8e57, 0x3fe33840}}, {{0x2ed80d22, 0x3fe995cf}},
	{{0x2ed80d22, 0xbfe995cf}}, {{0x0d0c8e57, 0x3fe33840}},
	{{0xd27504e9, 0x3feb8c38}}, {{0x26ae221a, 0x3fe04856}},
	{{0x26ae221a, 0xbfe04856}}, {{0xd27504e9, 0x3feb8c38}},
	{{0x1adfedf9, 0x3fcfdcdc}}, {{0x0c0b95ec, 0x3feefe22}},
	{{0x0c0b95ec, 0xbfeefe22}}, {{0x1adfedf9, 0x3fcfdcdc}},
	{{0xe870ce25, 0x3fee31ea}}, {{0x80af3c24, 0x3fd530d8}},
	{{0x80af3c24, 0xbfd530d8}}, {{0xe870ce25, 0x3fee31ea}},
	{{0x2304bd01, 0x3fdbb7cf}}, {{0x898b32f6, 0x3fecd7d9}},
	{{0x898b32f6, 0xbfecd7d9}}, {{0x2304bd01, 0x3fdbb7cf}},
	{{0x6cc33db2, 0x3fe7d783}}, {{0x38975137, 0x3fe55810}},
	{{0x38975137, 0xbfe55810}}, {{0x6cc33db2, 0x3fe7d783}},
	{{0x12c0d7e3, 0x3fac428d}}, {{0x0f8d575c, 0x3feff383}},
	{{0x0f8d575c, 0xbfeff383}}, {{0x12c0d7e3, 0x3fac428d}},
	{{0x9ff1f456, 0x3fefdd53}}, {{0x6a5d5b21, 0x3fb78758}},
	{{0x6a5d5b21, 0xbfb78758}}, {{0x9ff1f456, 0x3fefdd53}},
	{{0x1b987347, 0x3fe473b5}}, {{0x9a4dd4aa, 0x3fe89c7e}},
	{{0x9a4dd4aa, 0xbfe89c7e}}, {{0x1b987347, 0x3fe473b5}},
	{{0x012b6907, 0x3fec5042}}, {{0x1481cc58, 0x3fddd28f}},
	{{0x1481cc58, 0xbfddd28f}}, {{0x012b6907, 0x3fec5042}},
	{{0xdaec0387, 0x3fd2f422}}, {{0x361df7f2, 0x3fee9084}},
	{{0x361df7f2, 0xbfee9084}}, {{0xdaec0387, 0x3fd2f422}},
	{{0xe8e7a88e, 0x3feeadb2}}, {{0xabc3bb71, 0x3fd233bb}},
	{{0xabc3bb71, 0xbfd233bb}}, {{0xe8e7a88e, 0x3feeadb2}},
	{{0xeaf85114, 0x3fde83e0}}, {{0x3fa971b0, 0x3fec20de}},
	{{0x3fa971b0, 0xbfec20de}}, {{0xeaf85114, 0x3fde83e0}},
	{{0x331698cc, 0x3fe8dc45}}, {{0x178e6bb1, 0x3fe425ff}},
	{{0x178e6bb1, 0xbfe425ff}}, {{0x331698cc, 0x3fe8dc45}},
	{{0x24495c03, 0x3fbaa7b7}}, {{0x14220b84, 0x3fefd379}},
	{{0x14220b84, 0xbfefd379}}, {{0x24495c03, 0x3fbaa7b7}},
	{{0x7df5bbb7, 0x3fef6c3f}}, {{0xe89c64c6, 0x3fc83366}},
	{{0xe89c64c6, 0xbfc83366}}, {{0x7df5bbb7, 0x3fef6c3f}},
	{{0x8bbc861b, 0x3fe1f0f0}}, {{0x529fe69d, 0x3fea7f58}},
	{{0x529fe69d, 0xbfea7f58}}, {{0x8bbc861b, 0x3fe1f0f0}},
	{{0x5916c0d4, 0x3feab732}}, {{0x09f2b9b8, 0x3fe19d5a}},
	{{0x09f2b9b8, 0xbfe19d5a}}, {{0x5916c0d4, 0x3feab732}},
	{{0xf2dc4366, 0x3fc9bdcb}}, {{0xb1789e84, 0x3fef58a2}},
	{{0xb1789e84, 0xbfef58a2}}, {{0xf2dc4366, 0x3fc9bdcb}},
	{{0xa9668988, 0x3feda383}}, {{0xb04eaac4, 0x3fd820e3}},
	{{0xb04eaac4, 0xbfd820e3}}, {{0xa9668988, 0x3feda383}},
	{{0x2ec8a4b0, 0x3fd8daa5}}, {{0x02b8ecf9, 0x3fed7d0b}},
	{{0x02b8ecf9, 0xbfed7d0b}}, {{0x2ec8a4b0, 0x3fd8daa5}},
	{{0x73c18275, 0x3fe6c40d}}, {{0x8491af10, 0x3fe67cf7}},
	{{0x8491af10, 0xbfe67cf7}}, {{0x73c18275, 0x3fe6c40d}},
	{{0xfe670071, 0x3f7921f0}}, {{0x858e8a92, 0x3fefffd8}},
	{{0x858e8a92, 0xbfefffd8}}, {{0xfe670071, 0x3f7921f0}},
	{{0x21621d02, 0x3feffff6}}, {{0xbecca4ba, 0x3f6921f8}},
	{{0xbecca4ba, 0xbf6921f8}}, {{0x21621d02, 0x3feffff6}},
	{{0xeaa19c71, 0x3fe68ed1}}, {{0xed2fe29c, 0x3fe6b25c}},
	{{0xed2fe29c, 0xbfe6b25c}}, {{0xeaa19c71, 0x3fe68ed1}},
	{{0x8445a44f, 0x3fed86c4}}, {{0x86d5ed44, 0x3fd8ac4b}},
	{{0x86d5ed44, 0xbfd8ac4b}}, {{0x8445a44f, 0x3fed86c4}},
	{{0xaaf3903f, 0x3fd84f6a}}, {{0xdd8b3d46, 0x3fed9a00}},
	{{0xdd8b3d46, 0xbfed9a00}}, {{0xaaf3903f, 0x3fd84f6a}},
	{{0xed43685d, 0x3fef5da6}}, {{0xe9b62afa, 0x3fc95b49}},
	{{0xe9b62afa, 0xbfc95b49}}, {{0xed43685d, 0x3fef5da6}},
	{{0x171373bf, 0x3fe1b250}}, {{0x7a2cb98e, 0x3feaa954}},
	{{0x7a2cb98e, 0xbfeaa954}}, {{0x171373bf, 0x3fe1b250}},
	{{0x6e545ad2, 0x3fea8d67}}, {{0x64dc4872, 0x3fe1dc1b}},
	{{0x64dc4872, 0xbfe1dc1b}}, {{0x6e545ad2, 0x3fea8d67}},
	{{0x27c41804, 0x3fc89617}}, {{0x56883cee, 0x3fef6775}},
	{{0x56883cee, 0xbfef6775}}, {{0x27c41804, 0x3fc89617}},
	{{0x2da75c9e, 0x3fefd60d}}, {{0xeb24a85c, 0x3fb9dfb6}},
	{{0xeb24a85c, 0xbfb9dfb6}}, {{0x2da75c9e, 0x3fefd60d}},
	{{0x5b2a4380, 0x3fe4397f}}, {{0x75184655, 0x3fe8cc6a}},
	{{0x75184655, 0xbfe8cc6a}}, {{0x5b2a4380, 0x3fe4397f}},
	{{0x4931e3f1, 0x3fec2cd1}}, {{0x6d3cd825, 0x3fde57a8}},
	{{0x6d3cd825, 0xbfde57a8}}, {{0x4931e3f1, 0x3fec2cd1}},
	{{0x995554ba, 0x3fd263e6}}, {{0x93e65800, 0x3feea683}},
	{{0x93e65800, 0xbfeea683}}, {{0x995554ba, 0x3fd263e6}},
	{{0x36016b30, 0x3fee97ec}}, {{0x4e954520, 0x3fd2c41a}},
	{{0x4e954520, 0xbfd2c41a}}, {{0x36016b30, 0x3fee97ec}},
	{{0x66a941de, 0x3fddfeff}}, {{0x3141c004, 0x3fec4483}},
	{{0x3141c004, 0xbfec4483}}, {{0x66a941de, 0x3fddfeff}},
	{{0x1ede1d88, 0x3fe8ac87}}, {{0x692b32a2, 0x3fe4605a}},
	{{0x692b32a2, 0xbfe4605a}}, {{0x1ede1d88, 0x3fe8ac87}},
	{{0x12c130a1, 0x3fb84f87}}, {{0x7514538c, 0x3fefdafa}},
	{{0x7514538c, 0xbfefdafa}}, {{0x12c130a1, 0x3fb84f87}},
	{{0x54b1bed3, 0x3feff4dc}}, {{0xbd5f8317, 0x3faab101}},
	{{0xbd5f8317, 0xbfaab101}}, {{0x54b1bed3, 0x3feff4dc}},
	{{0x5197649f, 0x3fe56ac3}}, {{0x9ce2d333, 0x3fe7c6b8}},
	{{0x9ce2d333, 0xbfe7c6b8}}, {{0x5197649f, 0x3fe56ac3}},
	{{0x2799a060, 0x3fece2b3}}, {{0x14fd5693, 0x3fdb8a78}},
	{{0x14fd5693, 0xbfdb8a78}}, {{0x2799a060, 0x3fece2b3}},
	{{0x12f467b4, 0x3fd56040}}, {{0x4439197a, 0x3fee298f}},
	{{0x4439197a, 0xbfee298f}}, {{0x12f467b4, 0x3fd56040}},
	{{0x14cf738c, 0x3fef045a}}, {{0x80bd3802, 0x3fcf7b74}},
	{{0x80bd3802, 0xbfcf7b74}}, {{0x14cf738c, 0x3fef045a}},
	{{0xec31b8b7, 0x3fe05df3}}, {{0x86e792e9, 0x3feb7f66}},
	{{0x86e792e9, 0xbfeb7f66}}, {{0xec31b8b7, 0x3fe05df3}},
	{{0xa42b06b2, 0x3fe9a4df}}, {{0xec49a61f, 0x3fe32421}},
	{{0xec49a61f, 0xbfe32421}}, {{0xa42b06b2, 0x3fe9a4df}},
	{{0x4dfd3409, 0x3fc26499}}, {{0xcb0cfddc, 0x3fefaafb}},
	{{0xcb0cfddc, 0xbfefaafb}}, {{0x4dfd3409, 0x3fc26499}},
	{{0xac7a1791, 0x3fefa39b}}, {{0xf94516a7, 0x3fc32b7b}},
	{{0xf94516a7, 0xbfc32b7b}}, {{0xac7a1791, 0x3fefa39b}},
	{{0x4b441015, 0x3fe2fbc2}}, {{0x10f075c2, 0x3fe9c2d1}},
	{{0x10f075c2, 0xbfe9c2d1}}, {{0x4b441015, 0x3fe2fbc2}},
	{{0x14fdbc47, 0x3feb658f}}, {{0x2032b08c, 0x3fe08911}},
	{{0x2032b08c, 0xbfe08911}}, {{0x14fdbc47, 0x3feb658f}},
	{{0x462de348, 0x3fceb86b}}, {{0xbc898f5f, 0x3fef1090}},
	{{0xbc898f5f, 0xbfef1090}}, {{0x462de348, 0x3fceb86b}},
	{{0x2fdc66d9, 0x3fee18a0}}, {{0x8b9db3b6, 0x3fd5bee7}},
	{{0x8b9db3b6, 0xbfd5bee7}}, {{0x2fdc66d9, 0x3fee18a0}},
	{{0x1db31972, 0x3fdb2f97}}, {{0xe8ce467b, 0x3fecf830}},
	{{0xe8ce467b, 0xbfecf830}}, {{0x1db31972, 0x3fdb2f97}},
	{{0x07bf97d2, 0x3fe7a4f7}}, {{0xd5f723df, 0x3fe59001}},
	{{0xd5f723df, 0xbfe59001}}, {{0x07bf97d2, 0x3fe7a4f7}},
	{{0xa5874686, 0x3fa78dba}}, {{0xbb1b9164, 0x3feff753}},
	{{0xbb1b9164, 0xbfeff753}}, {{0xa5874686, 0x3fa78dba}},
	{{0x9ce2a679, 0x3feffce0}}, {{0x4ce53b1d, 0x3f9c454f}},
	{{0x4ce53b1d, 0xbf9c454f}}, {{0x9ce2a679, 0x3feffce0}},
	{{0xbde56a10, 0x3fe5fe7c}}, {{0x8e079942, 0x3fe73e55}},
	{{0x8e079942, 0xbfe73e55}}, {{0xbde56a10, 0x3fe5fe7c}},
	{{0x7bcbfbdc, 0x3fed36fc}}, {{0x43b50ac0, 0x3fda1d65}},
	{{0x43b50ac0, 0xbfda1d65}}, {{0x7bcbfbdc, 0x3fed36fc}},
	{{0x638a0cb6, 0x3fd6d998}}, {{0x0f6d8d81, 0x3fede416}},
	{{0x0f6d8d81, 0xbfede416}}, {{0x638a0cb6, 0x3fd6d998}},
	{{0x5a3aaef0, 0x3fef3368}}, {{0x535d74dd, 0x3fcc6d90}},
	{{0x535d74dd, 0xbfcc6d90}}, {{0x5a3aaef0, 0x3fef3368}},
	{{0x48d0a957, 0x3fe10972}}, {{0x2a4ca2f5, 0x3feb1674}},
	{{0x2a4ca2f5, 0xbfeb1674}}, {{0x48d0a957, 0x3fe10972}},
	{{0xd2c0a75e, 0x3fea1b26}}, {{0xef4d3cba, 0x3fe2818b}},
	{{0xef4d3cba, 0xbfe2818b}}, {{0xd2c0a75e, 0x3fea1b26}},
	{{0x8654cbde, 0x3fc57f00}}, {{0x37cb4b78, 0x3fef8ba7}},
	{{0x37cb4b78, 0xbfef8ba7}}, {{0x8654cbde, 0x3fc57f00}},
	{{0x0f0a8d88, 0x3fefbf47}}, {{0xad6fb85b, 0x3fc00ee8}},
	{{0xad6fb85b, 0xbfc00ee8}}, {{0x0f0a8d88, 0x3fefbf47}},
	{{0xe3d63029, 0x3fe39c23}}, {{0xe3ac4a6c, 0x3fe94990}},
	{{0xe3ac4a6c, 0xbfe94990}}, {{0xe3d63029, 0x3fe39c23}},
	{{0xcb0d2327, 0x3febcb54}}, {{0x5c24d2de, 0x3fdfb757}},
	{{0x5c24d2de, 0xbfdfb757}}, {{0xcb0d2327, 0x3febcb54}},
	{{0x4e1749ce, 0x3fd0e15b}}, {{0x6a078651, 0x3feeddeb}},
	{{0x6a078651, 0xbfeeddeb}}, {{0x4e1749ce, 0x3fd0e15b}},
	{{0x550467d3, 0x3fee5a9d}}, {{0xdc8936f0, 0x3fd44310}},
	{{0xdc8936f0, 0xbfd44310}}, {{0x550467d3, 0x3fee5a9d}},
	{{0xc3865389, 0x3fdc997f}}, {{0x19b9c449, 0x3feca08f}},
	{{0x19b9c449, 0xbfeca08f}}, {{0xc3865389, 0x3fdc997f}},
	{{0x13f545ff, 0x3fe82a9c}}, {{0x25cca486, 0x3fe4f9cc}},
	{{0x25cca486, 0xbfe4f9cc}}, {{0x13f545ff, 0x3fe82a9c}},
	{{0x74ed444d, 0x3fb20c96}}, {{0x2530410f, 0x3fefeb9d}},
	{{0x2530410f, 0xbfefeb9d}}, {{0x74ed444d, 0x3fb20c96}},
	{{0x85482d60, 0x3fefe7ea}}, {{0x12c5a299, 0x3fb39d9f}},
	{{0x12c5a299, 0xbfb39d9f}}, {{0x85482d60, 0x3fefe7ea}},
	{{0x6d589f7f, 0x3fe4d3bc}}, {{0x11af83fa, 0x3fe84b71}},
	{{0x11af83fa, 0xbfe84b71}}, {{0x6d589f7f, 0x3fe4d3bc}},
	{{0x87029c13, 0x3fec89f5}}, {{0xaee1cd21, 0x3fdcf34b}},
	{{0xaee1cd21, 0xbfdcf34b}}, {{0x87029c13, 0x3fec89f5}},
	{{0xe96ec271, 0x3fd3e39b}}, {{0xc55d53a7, 0x3fee6a61}},
	{{0xc55d53a7, 0xbfee6a61}}, {{0xe96ec271, 0x3fd3e39b}},
	{{0x5e999009, 0x3feed083}}, {{0xefc69378, 0x3fd1423e}},
	{{0xefc69378, 0xbfd1423e}}, {{0x5e999009, 0x3feed083}},
	{{0xe656cda3, 0x3fdf5fde}}, {{0x611154c1, 0x3febe41b}},
	{{0x611154c1, 0xbfebe41b}}, {{0xe656cda3, 0x3fdf5fde}},
	{{0x1fc5a815, 0x3fe92aa4}}, {{0x4981c518, 0x3fe3c3c4}},
	{{0x4981c518, 0xbfe3c3c4}}, {{0x1fc5a815, 0x3fe92aa4}},
	{{0xfde4aa3f, 0x3fbe8eb7}}, {{0x3b7d9af6, 0x3fefc56e}},
	{{0x3b7d9af6, 0xbfefc56e}}, {{0xfde4aa3f, 0x3fbe8eb7}},
	{{0x4a40c60c, 0x3fef830f}}, {{0x831d830d, 0x3fc6451a}},
	{{0x831d830d, 0xbfc6451a}}, {{0x4a40c60c, 0x3fef830f}},
	{{0x4cbb7110, 0x3fe25873}}, {{0x4a593bc6, 0x3fea3818}},
	{{0x4a593bc6, 0xbfea3818}}, {{0x4cbb7110, 0x3fe25873}},
	{{0xd89f57b6, 0x3feafb8f}}, {{0xcfee254f, 0x3fe133e9}},
	{{0xcfee254f, 0xbfe133e9}}, {{0xd89f57b6, 0x3feafb8f}},
	{{0x34f15dad, 0x3fcba963}}, {{0xbc1bbc65, 0x3fef3e6b}},
	{{0xbc1bbc65, 0xbfef3e6b}}, {{0x34f15dad, 0x3fcba963}},
	{{0xf38a915a, 0x3fedd1fe}}, {{0xc9261092, 0x3fd73763}},
	{{0xc9261092, 0xbfd73763}}, {{0xf38a915a, 0x3fedd1fe}},
	{{0x440df9f2, 0x3fd9c17d}}, {{0x1b187524, 0x3fed4b5b}},
	{{0x1b187524, 0xbfed4b5b}}, {{0x440df9f2, 0x3fd9c17d}},
	{{0x960e41bf, 0x3fe71bac}}, {{0x4fec22ff, 0x3fe622e4}},
	{{0x4fec22ff, 0xbfe622e4}}, {{0x960e41bf, 0x3fe71bac}},
	{{0x21fab226, 0x3f95fd4d}}, {{0x6870cb77, 0x3feffe1c}},
	{{0x6870cb77, 0xbfeffe1c}}, {{0x21fab226, 0x3f95fd4d}},
	{{0x43c53bd1, 0x3fefff09}}, {{0x6ab997cb, 0x3f8f6a29}},
	{{0x6ab997cb, 0xbf8f6a29}}, {{0x43c53bd1, 0x3fefff09}},
	{{0x437f535b, 0x3fe64715}}, {{0x99c95b75, 0x3fe6f8ca}},
	{{0x99c95b75, 0xbfe6f8ca}}, {{0x437f535b, 0x3fe64715}},
	{{0x72888a7f, 0x3fed5f71}}, {{0xb7ab948f, 0x3fd96555}},
	{{0xb7ab948f, 0xbfd96555}}, {{0x72888a7f, 0x3fed5f71}},
	{{0xe613dfae, 0x3fd794f5}}, {{0x4395759a, 0x3fedbf9e}},
	{{0x4395759a, 0xbfedbf9e}}, {{0xe613dfae, 0x3fd794f5}},
	{{0x06bcabb4, 0x3fef4922}}, {{0xd5f3b9ab, 0x3fcae4f1}},
	{{0xd5f3b9ab, 0xbfcae4f1}}, {{0x06bcabb4, 0x3fef4922}},
	{{0xe4dbe2bc, 0x3fe15e36}}, {{0xf345ecef, 0x3feae068}},
	{{0xf345ecef, 0xbfeae068}}, {{0xe4dbe2bc, 0x3fe15e36}},
	{{0x1090f523, 0x3fea54c9}}, {{0x662c13e2, 0x3fe22f2d}},
	{{0x662c13e2, 0xbfe22f2d}}, {{0x1090f523, 0x3fea54c9}},
	{{0x8d08c4ff, 0x3fc70afd}}, {{0x9c1a322a, 0x3fef7a29}},
	{{0x9c1a322a, 0xbfef7a29}}, {{0x8d08c4ff, 0x3fc70afd}},
	{{0x03914354, 0x3fefcb47}}, {{0x3b307dc1, 0x3fbcff53}},
	{{0x3b307dc1, 0xbfbcff53}}, {{0x03914354, 0x3fefcb47}},
	{{0xeabe0680, 0x3fe3eb33}}, {{0x43575efe, 0x3fe90b79}},
	{{0x43575efe, 0xbfe90b79}}, {{0xeabe0680, 0x3fe3eb33}},
	{{0x25a1b147, 0x3febfc9d}}, {{0x06bff7fe, 0x3fdf0819}},
	{{0x06bff7fe, 0xbfdf0819}}, {{0x25a1b147, 0x3febfc9d}},
	{{0xfbe8f243, 0x3fd1a2f7}}, {{0x4b1af6b2, 0x3feec2cf}},
	{{0x4b1af6b2, 0xbfeec2cf}}, {{0xfbe8f243, 0x3fd1a2f7}},
	{{0x29a5165a, 0x3fee79db}}, {{0xe353b6ab, 0x3fd383f5}},
	{{0xe353b6ab, 0xbfd383f5}}, {{0x29a5165a, 0x3fee79db}},
	{{0x2ba8609d, 0x3fdd4cd0}}, {{0x899eaad7, 0x3fec7315}},
	{{0x899eaad7, 0xbfec7315}}, {{0x2ba8609d, 0x3fdd4cd0}},
	{{0x1d9aa195, 0x3fe86c0a}}, {{0x516722f1, 0x3fe4ad79}},
	{{0x516722f1, 0xbfe4ad79}}, {{0x1d9aa195, 0x3fe86c0a}},
	{{0x4a4d4d0a, 0x3fb52e77}}, {{0x2be9d886, 0x3fefe3e9}},
	{{0x2be9d886, 0xbfefe3e9}}, {{0x4a4d4d0a, 0x3fb52e77}},
	{{0x02826191, 0x3fefef01}}, {{0x4e463064, 0x3fb07b61}},
	{{0x4e463064, 0xbfb07b61}}, {{0x02826191, 0x3fefef01}},
	{{0x1cd99aa6, 0x3fe51fa8}}, {{0x756e52fa, 0x3fe8098b}},
	{{0x756e52fa, 0xbfe8098b}}, {{0x1cd99aa6, 0x3fe51fa8}},
	{{0x0a00da99, 0x3fecb6e2}}, {{0x47263129, 0x3fdc3f6d}},
	{{0x47263129, 0xbfdc3f6d}}, {{0x0a00da99, 0x3fecb6e2}},
	{{0xd11b82f3, 0x3fd4a253}}, {{0xff81ce5e, 0x3fee4a8d}},
	{{0xff81ce5e, 0xbfee4a8d}}, {{0xd11b82f3, 0x3fd4a253}},
	{{0x4c50a544, 0x3feeeb07}}, {{0x05eb661e, 0x3fd0804e}},
	{{0x05eb661e, 0xbfd0804e}}, {{0x4c50a544, 0x3feeeb07}},
	{{0xc82b82e1, 0x3fe00740}}, {{0xa0b6c40d, 0x3febb249}},
	{{0xa0b6c40d, 0xbfebb249}}, {{0xc82b82e1, 0x3fe00740}},
	{{0x42bd7fe1, 0x3fe9683f}}, {{0x1b817f8d, 0x3fe37453}},
	{{0x1b817f8d, 0xbfe37453}}, {{0x42bd7fe1, 0x3fe9683f}},
	{{0xbcb26786, 0x3fc0d64d}}, {{0x8d66adb7, 0x3fefb8d1}},
	{{0x8d66adb7, 0xbfefb8d1}}, {{0xbcb26786, 0x3fc0d64d}},
	{{0x4f85ac08, 0x3fef93f1}}, {{0x7f79fa88, 0x3fc4b8b1}},
	{{0x7f79fa88, 0xbfc4b8b1}}, {{0x4f85ac08, 0x3fef93f1}},
	{{0xe87aeb58, 0x3fe2aa76}}, {{0xf13149de, 0x3fe9fdf4}},
	{{0xf13149de, 0xbfe9fdf4}}, {{0xe87aeb58, 0x3fe2aa76}},
	{{0xa5f37bf3, 0x3feb3115}}, {{0xb84bc4b6, 0x3fe0ded0}},
	{{0xb84bc4b6, 0xbfe0ded0}}, {{0xa5f37bf3, 0x3feb3115}},
	{{0x4d2cbdee, 0x3fcd3177}}, {{0xfc4609ce, 0x3fef2817}},
	{{0xfc4609ce, 0xbfef2817}}, {{0x4d2cbdee, 0x3fcd3177}},
	{{0x6a9ba59c, 0x3fedf5e3}}, {{0x9cad63cb, 0x3fd67b94}},
	{{0x9cad63cb, 0xbfd67b94}}, {{0x6a9ba59c, 0x3fedf5e3}},
	{{0xd3dbf31b, 0x3fda790c}}, {{0xc6e5a4e1, 0x3fed2255}},
	{{0xc6e5a4e1, 0xbfed2255}}, {{0xd3dbf31b, 0x3fda790c}},
	{{0x2c304764, 0x3fe760c5}}, {{0xe73e345c, 0x3fe5d9de}},
	{{0xe73e345c, 0xbfe5d9de}}, {{0x2c304764, 0x3fe760c5}},
	{{0xdb42c17f, 0x3fa14685}}, {{0xe425fdae, 0x3feffb55}},
	{{0xe425fdae, 0xbfeffb55}}, {{0xdb42c17f, 0x3fa14685}},
	{{0x4208c014, 0x3feff97c}}, {{0x6ff86179, 0x3fa46a39}},
	{{0x6ff86179, 0xbfa46a39}}, {{0x4208c014, 0x3feff97c}},
	{{0x264f7448, 0x3fe5b50b}}, {{0x1b90b35b, 0x3fe782fb}},
	{{0x1b90b35b, 0xbfe782fb}}, {{0x264f7448, 0x3fe5b50b}},
	{{0x2f59d2b9, 0x3fed0d67}}, {{0x125cdc09, 0x3fdad473}},
	{{0x125cdc09, 0xbfdad473}}, {{0x2f59d2b9, 0x3fed0d67}},
	{{0x5c88c202, 0x3fd61d59}}, {{0xd9280f54, 0x3fee0766}},
	{{0xd9280f54, 0xbfee0766}}, {{0x5c88c202, 0x3fd61d59}},
	{{0xbe284708, 0x3fef1c7a}}, {{0x3f01099a, 0x3fcdf516}},
	{{0x3f01099a, 0xbfcdf516}}, {{0xbe284708, 0x3fef1c7a}},
	{{0x878f85ec, 0x3fe0b405}}, {{0x09de7925, 0x3feb4b74}},
	{{0x09de7925, 0xbfeb4b74}}, {{0x878f85ec, 0x3fe0b405}},
	{{0xedb42472, 0x3fe9e082}}, {{0xd34e9bb8, 0x3fe2d333}},
	{{0xd34e9bb8, 0xbfe2d333}}, {{0xedb42472, 0x3fe9e082}},
	{{0x57db4893, 0x3fc3f22f}}, {{0x7cfbde29, 0x3fef9bed}},
	{{0x7cfbde29, 0xbfef9bed}}, {{0x57db4893, 0x3fc3f22f}},
	{{0xc681d54d, 0x3fefb20d}}, {{0x40be24e7, 0x3fc19d89}},
	{{0x40be24e7, 0xbfc19d89}}, {{0xc681d54d, 0x3fefb20d}},
	{{0x52c14de1, 0x3fe34c52}}, {{0xf1457594, 0x3fe986ae}},
	{{0xf1457594, 0xbfe986ae}}, {{0x52c14de1, 0x3fe34c52}},
	{{0x1fd9155e, 0x3feb98fa}}, {{0x55edbd96, 0x3fe032ae}},
	{{0x55edbd96, 0xbfe032ae}}, {{0x1fd9155e, 0x3feb98fa}},
	{{0x06b9fdd2, 0x3fd01f18}}, {{0xe51ca3c0, 0x3feef7d6}},
	{{0xe51ca3c0, 0xbfeef7d6}}, {{0x06b9fdd2, 0x3fd01f18}},
	{{0xec75ce85, 0x3fee3a33}}, {{0xdc197048, 0x3fd50163}},
	{{0xdc197048, 0xbfd50163}}, {{0xec75ce85, 0x3fee3a33}},
	{{0x17ffc0d9, 0x3fdbe515}}, {{0x20c2dea0, 0x3fecccee}},
	{{0x20c2dea0, 0xbfecccee}}, {{0x17ffc0d9, 0x3fdbe515}},
	{{0x87b03686, 0x3fe7e83f}}, {{0xf5159dfc, 0x3fe5454f}},
	{{0xf5159dfc, 0xbfe5454f}}, {{0x87b03686, 0x3fe7e83f}},
	{{0xf9808ec9, 0x3fadd406}}, {{0x14e131ed, 0x3feff216}},
	{{0x14e131ed, 0xbfeff216}}, {{0xf9808ec9, 0x3fadd406}},
	{{0x22f73307, 0x3fefdf99}}, {{0x3e79b129, 0x3fb6bf1b}},
	{{0x3e79b129, 0xbfb6bf1b}}, {{0x22f73307, 0x3fefdf99}},
	{{0x306091ff, 0x3fe48703}}, {{0xe7481ba1, 0x3fe88c66}},
	{{0xe7481ba1, 0xbfe88c66}}, {{0x306091ff, 0x3fe48703}},
	{{0x59fef85a, 0x3fec5bef}}, {{0x5cfa10d9, 0x3fdda60c}},
	{{0x5cfa10d9, 0xbfdda60c}}, {{0x59fef85a, 0x3fec5bef}},
	{{0xb638baaf, 0x3fd3241f}}, {{0x5bad6025, 0x3fee8909}},
	{{0x5bad6025, 0xbfee8909}}, {{0xb638baaf, 0x3fd3241f}},
	{{0x515b8811, 0x3feeb4cf}}, {{0x83d727be, 0x3fd20385}},
	{{0x83d727be, 0xbfd20385}}, {{0x515b8811, 0x3feeb4cf}},
	{{0x95f25620, 0x3fdeb006}}, {{0xdc465e57, 0x3fec14d9}},
	{{0xdc465e57, 0xbfec14d9}}, {{0x95f25620, 0x3fdeb006}},
	{{0x9b486c49, 0x3fe8ec10}}, {{0x663d108c, 0x3fe41272}},
	{{0x663d108c, 0xbfe41272}}, {{0x9b486c49, 0x3fe8ec10}},
	{{0xec38f64c, 0x3fbb6fa6}}, {{0x58d86087, 0x3fefd0d1}},
	{{0x58d86087, 0xbfefd0d1}}, {{0xec38f64c, 0x3fbb6fa6}},
	{{0x434b7eb7, 0x3fef70f6}}, {{0xbbd2cb1c, 0x3fc7d0a7}},
	{{0xbbd2cb1c, 0xbfc7d0a7}}, {{0x434b7eb7, 0x3fef70f6}},
	{{0xa17560d6, 0x3fe205ba}}, {{0xde9d60f5, 0x3fea7138}},
	{{0xde9d60f5, 0xbfea7138}}, {{0xa17560d6, 0x3fe205ba}},
	{{0xbd3efac8, 0x3feac4ff}}, {{0x1f3a46e5, 0x3fe18859}},
	{{0x1f3a46e5, 0xbfe18859}}, {{0xbd3efac8, 0x3feac4ff}},
	{{0x1b1831da, 0x3fca203e}}, {{0x1faf2d07, 0x3fef538b}},
	{{0x1faf2d07, 0xbfef538b}}, {{0x1b1831da, 0x3fca203e}},
	{{0x2ce68ab9, 0x3fedacf4}}, {{0xd37341e4, 0x3fd7f24d}},
	{{0xd37341e4, 0xbfd7f24d}}, {{0x2ce68ab9, 0x3fedacf4}},
	{{0x81ef7bd1, 0x3fd908ef}}, {{0x508c0dff, 0x3fed733f}},
	{{0x508c0dff, 0xbfed733f}}, {{0x81ef7bd1, 0x3fd908ef}},
	{{0xef4aafcd, 0x3fe6d5af}}, {{0x3f52b386, 0x3fe66b0f}},
	{{0x3f52b386, 0xbfe66b0f}}, {{0xef4aafcd, 0x3fe6d5af}},
	{{0x0e509703, 0x3f82d96b}}, {{0x2c978c4f, 0x3fefffa7}},
	{{0x2c978c4f, 0xbfefffa7}}, {{0x0e509703, 0x3f82d96b}},
	{{0x2c978c4f, 0x3fefffa7}}, {{0x0e509703, 0x3f82d96b}},
	{{0x0e509703, 0xbf82d96b}}, {{0x2c978c4f, 0x3fefffa7}},
	{{0x3f52b386, 0x3fe66b0f}}, {{0xef4aafcd, 0x3fe6d5af}},
	{{0xef4aafcd, 0xbfe6d5af}}, {{0x3f52b386, 0x3fe66b0f}},
	{{0x508c0dff, 0x3fed733f}}, {{0x81ef7bd1, 0x3fd908ef}},
	{{0x81ef7bd1, 0xbfd908ef}}, {{0x508c0dff, 0x3fed733f}},
	{{0xd37341e4, 0x3fd7f24d}}, {{0x2ce68ab9, 0x3fedacf4}},
	{{0x2ce68ab9, 0xbfedacf4}}, {{0xd37341e4, 0x3fd7f24d}},
	{{0x1faf2d07, 0x3fef538b}}, {{0x1b1831da, 0x3fca203e}},
	{{0x1b1831da, 0xbfca203e}}, {{0x1faf2d07, 0x3fef538b}},
	{{0x1f3a46e5, 0x3fe18859}}, {{0xbd3efac8, 0x3feac4ff}},
	{{0xbd3efac8, 0xbfeac4ff}}, {{0x1f3a46e5, 0x3fe18859}},
	{{0xde9d60f5, 0x3fea7138}}, {{0xa17560d6, 0x3fe205ba}},
	{{0xa17560d6, 0xbfe205ba}}, {{0xde9d60f5, 0x3fea7138}},
	{{0xbbd2cb1c, 0x3fc7d0a7}}, {{0x434b7eb7, 0x3fef70f6}},
	{{0x434b7eb7, 0xbfef70f6}}, {{0xbbd2cb1c, 0x3fc7d0a7}},
	{{0x58d86087, 0x3fefd0d1}}, {{0xec38f64c, 0x3fbb6fa6}},
	{{0xec38f64c, 0xbfbb6fa6}}, {{0x58d86087, 0x3fefd0d1}},
	{{0x663d108c, 0x3fe41272}}, {{0x9b486c49, 0x3fe8ec10}},
	{{0x9b486c49, 0xbfe8ec10}}, {{0x663d108c, 0x3fe41272}},
	{{0xdc465e57, 0x3fec14d9}}, {{0x95f25620, 0x3fdeb006}},
	{{0x95f25620, 0xbfdeb006}}, {{0xdc465e57, 0x3fec14d9}},
	{{0x83d727be, 0x3fd20385}}, {{0x515b8811, 0x3feeb4cf}},
	{{0x515b8811, 0xbfeeb4cf}}, {{0x83d727be, 0x3fd20385}},
	{{0x5bad6025, 0x3fee8909}}, {{0xb638baaf, 0x3fd3241f}},
	{{0xb638baaf, 0xbfd3241f}}, {{0x5bad6025, 0x3fee8909}},
	{{0x5cfa10d9, 0x3fdda60c}}, {{0x59fef85a, 0x3fec5bef}},
	{{0x59fef85a, 0xbfec5bef}}, {{0x5cfa10d9, 0x3fdda60c}},
	{{0xe7481ba1, 0x3fe88c66}}, {{0x306091ff, 0x3fe48703}},
	{{0x306091ff, 0xbfe48703}}, {{0xe7481ba1, 0x3fe88c66}},
	{{0x3e79b129, 0x3fb6bf1b}}, {{0x22f73307, 0x3fefdf99}},
	{{0x22f73307, 0xbfefdf99}}, {{0x3e79b129, 0x3fb6bf1b}},
	{{0x14e131ed, 0x3feff216}}, {{0xf9808ec9, 0x3fadd406}},
	{{0xf9808ec9, 0xbfadd406}}, {{0x14e131ed, 0x3feff216}},
	{{0xf5159dfc, 0x3fe5454f}}, {{0x87b03686, 0x3fe7e83f}},
	{{0x87b03686, 0xbfe7e83f}}, {{0xf5159dfc, 0x3fe5454f}},
	{{0x20c2dea0, 0x3fecccee}}, {{0x17ffc0d9, 0x3fdbe515}},
	{{0x17ffc0d9, 0xbfdbe515}}, {{0x20c2dea0, 0x3fecccee}},
	{{0xdc197048, 0x3fd50163}}, {{0xec75ce85, 0x3fee3a33}},
	{{0xec75ce85, 0xbfee3a33}}, {{0xdc197048, 0x3fd50163}},
	{{0xe51ca3c0, 0x3feef7d6}}, {{0x06b9fdd2, 0x3fd01f18}},
	{{0x06b9fdd2, 0xbfd01f18}}, {{0xe51ca3c0, 0x3feef7d6}},
	{{0x55edbd96, 0x3fe032ae}}, {{0x1fd9155e, 0x3feb98fa}},
	{{0x1fd9155e, 0xbfeb98fa}}, {{0x55edbd96, 0x3fe032ae}},
	{{0xf1457594, 0x3fe986ae}}, {{0x52c14de1, 0x3fe34c52}},
	{{0x52c14de1, 0xbfe34c52}}, {{0xf1457594, 0x3fe986ae}},
	{{0x40be24e7, 0x3fc19d89}}, {{0xc681d54d, 0x3fefb20d}},
	{{0xc681d54d, 0xbfefb20d}}, {{0x40be24e7, 0x3fc19d89}},
	{{0x7cfbde29, 0x3fef9bed}}, {{0x57db4893, 0x3fc3f22f}},
	{{0x57db4893, 0xbfc3f22f}}, {{0x7cfbde29, 0x3fef9bed}},
	{{0xd34e9bb8, 0x3fe2d333}}, {{0xedb42472, 0x3fe9e082}},
	{{0xedb42472, 0xbfe9e082}}, {{0xd34e9bb8, 0x3fe2d333}},
	{{0x09de7925, 0x3feb4b74}}, {{0x878f85ec, 0x3fe0b405}},
	{{0x878f85ec, 0xbfe0b405}}, {{0x09de7925, 0x3feb4b74}},
	{{0x3f01099a, 0x3fcdf516}}, {{0xbe284708, 0x3fef1c7a}},
	{{0xbe284708, 0xbfef1c7a}}, {{0x3f01099a, 0x3fcdf516}},
	{{0xd9280f54, 0x3fee0766}}, {{0x5c88c202, 0x3fd61d59}},
	{{0x5c88c202, 0xbfd61d59}}, {{0xd9280f54, 0x3fee0766}},
	{{0x125cdc09, 0x3fdad473}}, {{0x2f59d2b9, 0x3fed0d67}},
	{{0x2f59d2b9, 0xbfed0d67}}, {{0x125cdc09, 0x3fdad473}},
	{{0x1b90b35b, 0x3fe782fb}}, {{0x264f7448, 0x3fe5b50b}},
	{{0x264f7448, 0xbfe5b50b}}, {{0x1b90b35b, 0x3fe782fb}},
	{{0x6ff86179, 0x3fa46a39}}, {{0x4208c014, 0x3feff97c}},
	{{0x4208c014, 0xbfeff97c}}, {{0x6ff86179, 0x3fa46a39}},
	{{0xe425fdae, 0x3feffb55}}, {{0xdb42c17f, 0x3fa14685}},
	{{0xdb42c17f, 0xbfa14685}}, {{0xe425fdae, 0x3feffb55}},
	{{0xe73e345c, 0x3fe5d9de}}, {{0x2c304764, 0x3fe760c5}},
	{{0x2c304764, 0xbfe760c5}}, {{0xe73e345c, 0x3fe5d9de}},
	{{0xc6e5a4e1, 0x3fed2255}}, {{0xd3dbf31b, 0x3fda790c}},
	{{0xd3dbf31b, 0xbfda790c}}, {{0xc6e5a4e1, 0x3fed2255}},
	{{0x9cad63cb, 0x3fd67b94}}, {{0x6a9ba59c, 0x3fedf5e3}},
	{{0x6a9ba59c, 0xbfedf5e3}}, {{0x9cad63cb, 0x3fd67b94}},
	{{0xfc4609ce, 0x3fef2817}}, {{0x4d2cbdee, 0x3fcd3177}},
	{{0x4d2cbdee, 0xbfcd3177}}, {{0xfc4609ce, 0x3fef2817}},
	{{0xb84bc4b6, 0x3fe0ded0}}, {{0xa5f37bf3, 0x3feb3115}},
	{{0xa5f37bf3, 0xbfeb3115}}, {{0xb84bc4b6, 0x3fe0ded0}},
	{{0xf13149de, 0x3fe9fdf4}}, {{0xe87aeb58, 0x3fe2aa76}},
	{{0xe87aeb58, 0xbfe2aa76}}, {{0xf13149de, 0x3fe9fdf4}},
	{{0x7f79fa88, 0x3fc4b8b1}}, {{0x4f85ac08, 0x3fef93f1}},
	{{0x4f85ac08, 0xbfef93f1}}, {{0x7f79fa88, 0x3fc4b8b1}},
	{{0x8d66adb7, 0x3fefb8d1}}, {{0xbcb26786, 0x3fc0d64d}},
	{{0xbcb26786, 0xbfc0d64d}}, {{0x8d66adb7, 0x3fefb8d1}},
	{{0x1b817f8d, 0x3fe37453}}, {{0x42bd7fe1, 0x3fe9683f}},
	{{0x42bd7fe1, 0xbfe9683f}}, {{0x1b817f8d, 0x3fe37453}},
	{{0xa0b6c40d, 0x3febb249}}, {{0xc82b82e1, 0x3fe00740}},
	{{0xc82b82e1, 0xbfe00740}}, {{0xa0b6c40d, 0x3febb249}},
	{{0x05eb661e, 0x3fd0804e}}, {{0x4c50a544, 0x3feeeb07}},
	{{0x4c50a544, 0xbfeeeb07}}, {{0x05eb661e, 0x3fd0804e}},
	{{0xff81ce5e, 0x3fee4a8d}}, {{0xd11b82f3, 0x3fd4a253}},
	{{0xd11b82f3, 0xbfd4a253}}, {{0xff81ce5e, 0x3fee4a8d}},
	{{0x47263129, 0x3fdc3f6d}}, {{0x0a00da99, 0x3fecb6e2}},
	{{0x0a00da99, 0xbfecb6e2}}, {{0x47263129, 0x3fdc3f6d}},
	{{0x756e52fa, 0x3fe8098b}}, {{0x1cd99aa6, 0x3fe51fa8}},
	{{0x1cd99aa6, 0xbfe51fa8}}, {{0x756e52fa, 0x3fe8098b}},
	{{0x4e463064, 0x3fb07b61}}, {{0x02826191, 0x3fefef01}},
	{{0x02826191, 0xbfefef01}}, {{0x4e463064, 0x3fb07b61}},
	{{0x2be9d886, 0x3fefe3e9}}, {{0x4a4d4d0a, 0x3fb52e77}},
	{{0x4a4d4d0a, 0xbfb52e77}}, {{0x2be9d886, 0x3fefe3e9}},
	{{0x516722f1, 0x3fe4ad79}}, {{0x1d9aa195, 0x3fe86c0a}},
	{{0x1d9aa195, 0xbfe86c0a}}, {{0x516722f1, 0x3fe4ad79}},
	{{0x899eaad7, 0x3fec7315}}, {{0x2ba8609d, 0x3fdd4cd0}},
	{{0x2ba8609d, 0xbfdd4cd0}}, {{0x899eaad7, 0x3fec7315}},
	{{0xe353b6ab, 0x3fd383f5}}, {{0x29a5165a, 0x3fee79db}},
	{{0x29a5165a, 0xbfee79db}}, {{0xe353b6ab, 0x3fd383f5}},
	{{0x4b1af6b2, 0x3feec2cf}}, {{0xfbe8f243, 0x3fd1a2f7}},
	{{0xfbe8f243, 0xbfd1a2f7}}, {{0x4b1af6b2, 0x3feec2cf}},
	{{0x06bff7fe, 0x3fdf0819}}, {{0x25a1b147, 0x3febfc9d}},
	{{0x25a1b147, 0xbfebfc9d}}, {{0x06bff7fe, 0x3fdf0819}},
	{{0x43575efe, 0x3fe90b79}}, {{0xeabe0680, 0x3fe3eb33}},
	{{0xeabe0680, 0xbfe3eb33}}, {{0x43575efe, 0x3fe90b79}},
	{{0x3b307dc1, 0x3fbcff53}}, {{0x03914354, 0x3fefcb47}},
	{{0x03914354, 0xbfefcb47}}, {{0x3b307dc1, 0x3fbcff53}},
	{{0x9c1a322a, 0x3fef7a29}}, {{0x8d08c4ff, 0x3fc70afd}},
	{{0x8d08c4ff, 0xbfc70afd}}, {{0x9c1a322a, 0x3fef7a29}},
	{{0x662c13e2, 0x3fe22f2d}}, {{0x1090f523, 0x3fea54c9}},
	{{0x1090f523, 0xbfea54c9}}, {{0x662c13e2, 0x3fe22f2d}},
	{{0xf345ecef, 0x3feae068}}, {{0xe4dbe2bc, 0x3fe15e36}},
	{{0xe4dbe2bc, 0xbfe15e36}}, {{0xf345ecef, 0x3feae068}},
	{{0xd5f3b9ab, 0x3fcae4f1}}, {{0x06bcabb4, 0x3fef4922}},
	{{0x06bcabb4, 0xbfef4922}}, {{0xd5f3b9ab, 0x3fcae4f1}},
	{{0x4395759a, 0x3fedbf9e}}, {{0xe613dfae, 0x3fd794f5}},
	{{0xe613dfae, 0xbfd794f5}}, {{0x4395759a, 0x3fedbf9e}},
	{{0xb7ab948f, 0x3fd96555}}, {{0x72888a7f, 0x3fed5f71}},
	{{0x72888a7f, 0xbfed5f71}}, {{0xb7ab948f, 0x3fd96555}},
	{{0x99c95b75, 0x3fe6f8ca}}, {{0x437f535b, 0x3fe64715}},
	{{0x437f535b, 0xbfe64715}}, {{0x99c95b75, 0x3fe6f8ca}},
	{{0x6ab997cb, 0x3f8f6a29}}, {{0x43c53bd1, 0x3fefff09}},
	{{0x43c53bd1, 0xbfefff09}}, {{0x6ab997cb, 0x3f8f6a29}},
	{{0x6870cb77, 0x3feffe1c}}, {{0x21fab226, 0x3f95fd4d}},
	{{0x21fab226, 0xbf95fd4d}}, {{0x6870cb77, 0x3feffe1c}},
	{{0x4fec22ff, 0x3fe622e4}}, {{0x960e41bf, 0x3fe71bac}},
	{{0x960e41bf, 0xbfe71bac}}, {{0x4fec22ff, 0x3fe622e4}},
	{{0x1b187524, 0x3fed4b5b}}, {{0x440df9f2, 0x3fd9c17d}},
	{{0x440df9f2, 0xbfd9c17d}}, {{0x1b187524, 0x3fed4b5b}},
	{{0xc9261092, 0x3fd73763}}, {{0xf38a915a, 0x3fedd1fe}},
	{{0xf38a915a, 0xbfedd1fe}}, {{0xc9261092, 0x3fd73763}},
	{{0xbc1bbc65, 0x3fef3e6b}}, {{0x34f15dad, 0x3fcba963}},
	{{0x34f15dad, 0xbfcba963}}, {{0xbc1bbc65, 0x3fef3e6b}},
	{{0xcfee254f, 0x3fe133e9}}, {{0xd89f57b6, 0x3feafb8f}},
	{{0xd89f57b6, 0xbfeafb8f}}, {{0xcfee254f, 0x3fe133e9}},
	{{0x4a593bc6, 0x3fea3818}}, {{0x4cbb7110, 0x3fe25873}},
	{{0x4cbb7110, 0xbfe25873}}, {{0x4a593bc6, 0x3fea3818}},
	{{0x831d830d, 0x3fc6451a}}, {{0x4a40c60c, 0x3fef830f}},
	{{0x4a40c60c, 0xbfef830f}}, {{0x831d830d, 0x3fc6451a}},
	{{0x3b7d9af6, 0x3fefc56e}}, {{0xfde4aa3f, 0x3fbe8eb7}},
	{{0xfde4aa3f, 0xbfbe8eb7}}, {{0x3b7d9af6, 0x3fefc56e}},
	{{0x4981c518, 0x3fe3c3c4}}, {{0x1fc5a815, 0x3fe92aa4}},
	{{0x1fc5a815, 0xbfe92aa4}}, {{0x4981c518, 0x3fe3c3c4}},
	{{0x611154c1, 0x3febe41b}}, {{0xe656cda3, 0x3fdf5fde}},
	{{0xe656cda3, 0xbfdf5fde}}, {{0x611154c1, 0x3febe41b}},
	{{0xefc69378, 0x3fd1423e}}, {{0x5e999009, 0x3feed083}},
	{{0x5e999009, 0xbfeed083}}, {{0xefc69378, 0x3fd1423e}},
	{{0xc55d53a7, 0x3fee6a61}}, {{0xe96ec271, 0x3fd3e39b}},
	{{0xe96ec271, 0xbfd3e39b}}, {{0xc55d53a7, 0x3fee6a61}},
	{{0xaee1cd21, 0x3fdcf34b}}, {{0x87029c13, 0x3fec89f5}},
	{{0x87029c13, 0xbfec89f5}}, {{0xaee1cd21, 0x3fdcf34b}},
	{{0x11af83fa, 0x3fe84b71}}, {{0x6d589f7f, 0x3fe4d3bc}},
	{{0x6d589f7f, 0xbfe4d3bc}}, {{0x11af83fa, 0x3fe84b71}},
	{{0x12c5a299, 0x3fb39d9f}}, {{0x85482d60, 0x3fefe7ea}},
	{{0x85482d60, 0xbfefe7ea}}, {{0x12c5a299, 0x3fb39d9f}},
	{{0x2530410f, 0x3fefeb9d}}, {{0x74ed444d, 0x3fb20c96}},
	{{0x74ed444d, 0xbfb20c96}}, {{0x2530410f, 0x3fefeb9d}},
	{{0x25cca486, 0x3fe4f9cc}}, {{0x13f545ff, 0x3fe82a9c}},
	{{0x13f545ff, 0xbfe82a9c}}, {{0x25cca486, 0x3fe4f9cc}},
	{{0x19b9c449, 0x3feca08f}}, {{0xc3865389, 0x3fdc997f}},
	{{0xc3865389, 0xbfdc997f}}, {{0x19b9c449, 0x3feca08f}},
	{{0xdc8936f0, 0x3fd44310}}, {{0x550467d3, 0x3fee5a9d}},
	{{0x550467d3, 0xbfee5a9d}}, {{0xdc8936f0, 0x3fd44310}},
	{{0x6a078651, 0x3feeddeb}}, {{0x4e1749ce, 0x3fd0e15b}},
	{{0x4e1749ce, 0xbfd0e15b}}, {{0x6a078651, 0x3feeddeb}},
	{{0x5c24d2de, 0x3fdfb757}}, {{0xcb0d2327, 0x3febcb54}},
	{{0xcb0d2327, 0xbfebcb54}}, {{0x5c24d2de, 0x3fdfb757}},
	{{0xe3ac4a6c, 0x3fe94990}}, {{0xe3d63029, 0x3fe39c23}},
	{{0xe3d63029, 0xbfe39c23}}, {{0xe3ac4a6c, 0x3fe94990}},
	{{0xad6fb85b, 0x3fc00ee8}}, {{0x0f0a8d88, 0x3fefbf47}},
	{{0x0f0a8d88, 0xbfefbf47}}, {{0xad6fb85b, 0x3fc00ee8}},
	{{0x37cb4b78, 0x3fef8ba7}}, {{0x8654cbde, 0x3fc57f00}},
	{{0x8654cbde, 0xbfc57f00}}, {{0x37cb4b78, 0x3fef8ba7}},
	{{0xef4d3cba, 0x3fe2818b}}, {{0xd2c0a75e, 0x3fea1b26}},
	{{0xd2c0a75e, 0xbfea1b26}}, {{0xef4d3cba, 0x3fe2818b}},
	{{0x2a4ca2f5, 0x3feb1674}}, {{0x48d0a957, 0x3fe10972}},
	{{0x48d0a957, 0xbfe10972}}, {{0x2a4ca2f5, 0x3feb1674}},
	{{0x535d74dd, 0x3fcc6d90}}, {{0x5a3aaef0, 0x3fef3368}},
	{{0x5a3aaef0, 0xbfef3368}}, {{0x535d74dd, 0x3fcc6d90}},
	{{0x0f6d8d81, 0x3fede416}}, {{0x638a0cb6, 0x3fd6d998}},
	{{0x638a0cb6, 0xbfd6d998}}, {{0x0f6d8d81, 0x3fede416}},
	{{0x43b50ac0, 0x3fda1d65}}, {{0x7bcbfbdc, 0x3fed36fc}},
	{{0x7bcbfbdc, 0xbfed36fc}}, {{0x43b50ac0, 0x3fda1d65}},
	{{0x8e079942, 0x3fe73e55}}, {{0xbde56a10, 0x3fe5fe7c}},
	{{0xbde56a10, 0xbfe5fe7c}}, {{0x8e079942, 0x3fe73e55}},
	{{0x4ce53b1d, 0x3f9c454f}}, {{0x9ce2a679, 0x3feffce0}},
	{{0x9ce2a679, 0xbfeffce0}}, {{0x4ce53b1d, 0x3f9c454f}},
	{{0xbb1b9164, 0x3feff753}}, {{0xa5874686, 0x3fa78dba}},
	{{0xa5874686, 0xbfa78dba}}, {{0xbb1b9164, 0x3feff753}},
	{{0xd5f723df, 0x3fe59001}}, {{0x07bf97d2, 0x3fe7a4f7}},
	{{0x07bf97d2, 0xbfe7a4f7}}, {{0xd5f723df, 0x3fe59001}},
	{{0xe8ce467b, 0x3fecf830}}, {{0x1db31972, 0x3fdb2f97}},
	{{0x1db31972, 0xbfdb2f97}}, {{0xe8ce467b, 0x3fecf830}},
	{{0x8b9db3b6, 0x3fd5bee7}}, {{0x2fdc66d9, 0x3fee18a0}},
	{{0x2fdc66d9, 0xbfee18a0}}, {{0x8b9db3b6, 0x3fd5bee7}},
	{{0xbc898f5f, 0x3fef1090}}, {{0x462de348, 0x3fceb86b}},
	{{0x462de348, 0xbfceb86b}}, {{0xbc898f5f, 0x3fef1090}},
	{{0x2032b08c, 0x3fe08911}}, {{0x14fdbc47, 0x3feb658f}},
	{{0x14fdbc47, 0xbfeb658f}}, {{0x2032b08c, 0x3fe08911}},
	{{0x10f075c2, 0x3fe9c2d1}}, {{0x4b441015, 0x3fe2fbc2}},
	{{0x4b441015, 0xbfe2fbc2}}, {{0x10f075c2, 0x3fe9c2d1}},
	{{0xf94516a7, 0x3fc32b7b}}, {{0xac7a1791, 0x3fefa39b}},
	{{0xac7a1791, 0xbfefa39b}}, {{0xf94516a7, 0x3fc32b7b}},
	{{0xcb0cfddc, 0x3fefaafb}}, {{0x4dfd3409, 0x3fc26499}},
	{{0x4dfd3409, 0xbfc26499}}, {{0xcb0cfddc, 0x3fefaafb}},
	{{0xec49a61f, 0x3fe32421}}, {{0xa42b06b2, 0x3fe9a4df}},
	{{0xa42b06b2, 0xbfe9a4df}}, {{0xec49a61f, 0x3fe32421}},
	{{0x86e792e9, 0x3feb7f66}}, {{0xec31b8b7, 0x3fe05df3}},
	{{0xec31b8b7, 0xbfe05df3}}, {{0x86e792e9, 0x3feb7f66}},
	{{0x80bd3802, 0x3fcf7b74}}, {{0x14cf738c, 0x3fef045a}},
	{{0x14cf738c, 0xbfef045a}}, {{0x80bd3802, 0x3fcf7b74}},
	{{0x4439197a, 0x3fee298f}}, {{0x12f467b4, 0x3fd56040}},
	{{0x12f467b4, 0xbfd56040}}, {{0x4439197a, 0x3fee298f}},
	{{0x14fd5693, 0x3fdb8a78}}, {{0x2799a060, 0x3fece2b3}},
	{{0x2799a060, 0xbfece2b3}}, {{0x14fd5693, 0x3fdb8a78}},
	{{0x9ce2d333, 0x3fe7c6b8}}, {{0x5197649f, 0x3fe56ac3}},
	{{0x5197649f, 0xbfe56ac3}}, {{0x9ce2d333, 0x3fe7c6b8}},
	{{0xbd5f8317, 0x3faab101}}, {{0x54b1bed3, 0x3feff4dc}},
	{{0x54b1bed3, 0xbfeff4dc}}, {{0xbd5f8317, 0x3faab101}},
	{{0x7514538c, 0x3fefdafa}}, {{0x12c130a1, 0x3fb84f87}},
	{{0x12c130a1, 0xbfb84f87}}, {{0x7514538c, 0x3fefdafa}},
	{{0x692b32a2, 0x3fe4605a}}, {{0x1ede1d88, 0x3fe8ac87}},
	{{0x1ede1d88, 0xbfe8ac87}}, {{0x692b32a2, 0x3fe4605a}},
	{{0x3141c004, 0x3fec4483}}, {{0x66a941de, 0x3fddfeff}},
	{{0x66a941de, 0xbfddfeff}}, {{0x3141c004, 0x3fec4483}},
	{{0x4e954520, 0x3fd2c41a}}, {{0x36016b30, 0x3fee97ec}},
	{{0x36016b30, 0xbfee97ec}}, {{0x4e954520, 0x3fd2c41a}},
	{{0x93e65800, 0x3feea683}}, {{0x995554ba, 0x3fd263e6}},
	{{0x995554ba, 0xbfd263e6}}, {{0x93e65800, 0x3feea683}},
	{{0x6d3cd825, 0x3fde57a8}}, {{0x4931e3f1, 0x3fec2cd1}},
	{{0x4931e3f1, 0xbfec2cd1}}, {{0x6d3cd825, 0x3fde57a8}},
	{{0x75184655, 0x3fe8cc6a}}, {{0x5b2a4380, 0x3fe4397f}},
	{{0x5b2a4380, 0xbfe4397f}}, {{0x75184655, 0x3fe8cc6a}},
	{{0xeb24a85c, 0x3fb9dfb6}}, {{0x2da75c9e, 0x3fefd60d}},
	{{0x2da75c9e, 0xbfefd60d}}, {{0xeb24a85c, 0x3fb9dfb6}},
	{{0x56883cee, 0x3fef6775}}, {{0x27c41804, 0x3fc89617}},
	{{0x27c41804, 0xbfc89617}}, {{0x56883cee, 0x3fef6775}},
	{{0x64dc4872, 0x3fe1dc1b}}, {{0x6e545ad2, 0x3fea8d67}},
	{{0x6e545ad2, 0xbfea8d67}}, {{0x64dc4872, 0x3fe1dc1b}},
	{{0x7a2cb98e, 0x3feaa954}}, {{0x171373bf, 0x3fe1b250}},
	{{0x171373bf, 0xbfe1b250}}, {{0x7a2cb98e, 0x3feaa954}},
	{{0xe9b62afa, 0x3fc95b49}}, {{0xed43685d, 0x3fef5da6}},
	{{0xed43685d, 0xbfef5da6}}, {{0xe9b62afa, 0x3fc95b49}},
	{{0xdd8b3d46, 0x3fed9a00}}, {{0xaaf3903f, 0x3fd84f6a}},
	{{0xaaf3903f, 0xbfd84f6a}}, {{0xdd8b3d46, 0x3fed9a00}},
	{{0x86d5ed44, 0x3fd8ac4b}}, {{0x8445a44f, 0x3fed86c4}},
	{{0x8445a44f, 0xbfed86c4}}, {{0x86d5ed44, 0x3fd8ac4b}},
	{{0xed2fe29c, 0x3fe6b25c}}, {{0xeaa19c71, 0x3fe68ed1}},
	{{0xeaa19c71, 0xbfe68ed1}}, {{0xed2fe29c, 0x3fe6b25c}},
	{{0xbecca4ba, 0x3f6921f8}}, {{0x21621d02, 0x3feffff6}},
	{{0x21621d02, 0xbfeffff6}}, {{0xbecca4ba, 0x3f6921f8}}
};

const fpr fpr_p2_tab[] = {
	{{0x00000000, 0x40000000}},
	{{0x00000000, 0x3ff00000}},
	{{0x00000000, 0x3fe00000}},
	{{0x00000000, 0x3fd00000}},
	{{0x00000000, 0x3fc00000}},
	{{0x00000000, 0x3fb00000}},
	{{0x00000000, 0x3fa00000}},
	{{0x00000000, 0x3f900000}},
	{{0x00000000, 0x3f800000}},
	{{0x00000000, 0x3f700000}},
	{{0x00000000, 0x3f600000}}
};
