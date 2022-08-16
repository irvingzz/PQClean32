#include "struct64.h"
#include "performance.h"

#define BIT_ALU_OR    1
#define BIT_ALU_AND   2
#define BIT_ALU_XOR   3
#define BIT_ALU_NOT   4

uint64_s uint64_s_bit_alu(uint64_s x, uint64_s y, int bit_alu)
{
  uint64_s result;
  switch (bit_alu)
  {
  case BIT_ALU_OR:
    result.t[1]=x.t[1] | y.t[1];
    result.t[0]=x.t[0] | y.t[0];
    break;
  
  case BIT_ALU_AND:
    result.t[1]=x.t[1] & y.t[1];
    result.t[0]=x.t[0] & y.t[0];
    break;
  
  case BIT_ALU_XOR:
    result.t[1]=x.t[1] ^ y.t[1];
    result.t[0]=x.t[0] ^ y.t[0];
    break;

  case BIT_ALU_NOT:
    result.t[1]= ~x.t[1];
    result.t[0]= ~x.t[0];
    break;
  
  default:
    break;
  }

  return result;
}



uint64_s rol(uint64_s a, int offset)
{
  uint64_s b;
  if (offset <=32)
  {
    b.t[0] = (a.t[0] << offset) ^ (a.t[1] >> (32 - offset));
    b.t[1] = (a.t[1] << offset) ^ (a.t[0] >> (32 - offset));
  }
  else
  {
    b.t[0] = (a.t[0] >> 64 - offset) ^ (a.t[1] << (offset - 32));
    b.t[1] = (a.t[1] >> 64 - offset) ^ (a.t[0] << (offset - 32));
  }

  return b;
}

uint64_s shr(uint64_s x,uint32_t c)
{
  uint64_s a;
  if (c == 0)
  {
    a = x;
  }
  else if (c<32 && c>0)
  {
    a.t[1] = (x.t[1] >> c);
    a.t[0] = (x.t[1] << (32 - c)) | (x.t[0] >> c);
  }
  else
  {
    a.t[1] = 0;
    a.t[0] = (x.t[1] >> (c - 32));
  }
  return a;
}

uint64_s uint64_s_add(uint64_s x, uint64_s y)
{
  uint32_t c;
  uint64_s result;
  result.t[0] = x.t[0] + y.t[0];
  c = (result.t[0] < x.t[0])?1:0;
  result.t[1] = x.t[1] + y.t[1] + c;
  return result;
}

uint64_s uint32_t_mul(uint32_t x, uint32_t y)
{
  
  uint64_s a,b,c;
  uint16_t a1 = (uint16_t)(x >> 16), a2 = (uint16_t)x, b1 = (uint16_t)(y >> 16), b2 = (uint16_t)y;
  a.t[1] = (uint32_t)a1 * (uint32_t)b1;
  a.t[0] = (uint32_t)a2 * (uint32_t)b2;
  b.t[1] = 0;
  b.t[0] = (uint32_t)a1 * b2;
  c.t[1] = 0;
  c.t[0] = (uint32_t)a2 * b1;
  b = uint64_s_add(b,c);
  b = shl(b,16);
  a = uint64_s_add(a,b);
  // if (x == 0x58F2DD48 && y == 0xffffe090)
  // {
  //   printf("\t\tuint32_mul\n");
  //   print_uint64_s(&a,1);
  //   print_uint64_s(&b,1);
  // }
  return a;
}

uint64_s int32_t_mul(int32_t x, int32_t y)
{
  uint64_s a,b;
  uint32_t s1,s2,x1,y1;
  // printf("zeta = %08x\n",x );
  // printf("a[j+len] = %08x\n",y);
  s1 = (uint32_t)(x & 0x80000000);
  s2 = (uint32_t)(y & 0x80000000);
  x1 = (s1==0)?(uint32_t)x:(uint32_t)(~x) + 1 ;
  y1 = (s2==0)?(uint32_t)y:(uint32_t)(~y) + 1 ;
  // printf("I am here\n");
  // printf("x  = %08x y  = %08x",x,y);
  // printf("x1 = %04x y1 = %04x\n",x1,y1);
  a = uint32_t_mul(x1, y1);
  if ((s1 ^ s2) != 0)
  {
    // printf("s1 ^ s2 = %08x",s1 ^ s2);
    a.t[1] = ~a.t[1];
    a.t[0] = ~a.t[0];
    a.t[1] |= (s1 ^ s2);
    a = uint64_s_add(a,(uint64_s){1,0});
  }
  // print_uint64_s(&a,1);
  // printf("I am here\n");
  return a;
}

uint64_s shl(uint64_s x,uint32_t c)
{
  uint64_s a;
  if (c == 0)
  {
    a = x;
  }
  else if (c<32 && c>0)
  {
    a.t[0] = (x.t[0] << c);
    a.t[1] = (x.t[0] >> (32 - c)) | (x.t[1] << c);
  }
  else
  {
    a.t[0] = 0;
    a.t[1] = (x.t[0] << (c - 32));
  }
  return a;
}

uint64_s uint64_s_or(uint64_s x, uint64_s y)
{
  return uint64_s_bit_alu(x,y,BIT_ALU_OR);
}

uint64_s uint64_s_and(uint64_s x, uint64_s y)
{
  return uint64_s_bit_alu(x,y,BIT_ALU_AND);
}

uint64_s uint64_s_xor(uint64_s x, uint64_s y)
{
  return uint64_s_bit_alu(x,y,BIT_ALU_XOR);
}

uint64_s uint64_s_not(uint64_s x)
{
  return uint64_s_bit_alu(x,x,BIT_ALU_NOT);
}

uint64_s uint64_s_sub(uint64_s x, uint64_s y)
{
  y = uint64_s_add(uint64_s_not(y),(uint64_s){{1,0}});
  return uint64_s_add(x,y);
}

uint64_s uint64_s_neg(uint64_s x)
{
  return uint64_s_sub((uint64_s){{0,0}},x);
}

uint64_s uint64_s_mul(uint64_s x, uint64_s y)
{
  uint64_s x0y0, x0y1,x1y0;
  uint64_s tmp;
  
  x0y0 = uint32_t_mul(x.t[0],y.t[0]);
  x0y1 = uint32_t_mul(x.t[0],y.t[1]);
  x1y0 = uint32_t_mul(x.t[1],y.t[0]);
  x1y0 = shl(x1y0,32);
  x0y1 = shl(x0y1,32);
  tmp = uint64_s_add(x1y0,x0y1);
  tmp = uint64_s_add(x0y0,tmp);
  // if (x.t[0] == 0x58F2DD48 && y.t[0] == 0xffffe090 && y.t[1] == 0xffffffff)
  // {
  //   printf("\t\tmul:\n");
  //   print_uint64_s(&x0y0,1);
  //   print_uint64_s(&x0y1,1);
  //   print_uint64_s(&x1y0,1);
  //   print_uint64_s(&tmp,1);
  // }
  return tmp;
}

int64_s int64_s_mul(int64_s x, int64_s y)
{
  uint32_t sx,sy;
  uint64_s ux,uy,a;
  sx = x.t[1] >> 31;
  sy = y.t[1] >> 31;
  ux = (sx == 0)?x:uint64_s_neg(x);
  uy = (sy == 0)?y:uint64_s_neg(y);

  a = uint64_s_mul(ux, uy);

  if ((sx ^ sy) != 0)
  {
   a = uint64_s_neg(a);
  }
  return a;
}

int64_s int64_s_shr(int64_s x,uint32_t c)
{
  uint64_s ones = {{0xffffffff,0xffffffff}};
  uint64_s tmp;
  uint32_t s;
  s = x.t[1] & 0x80000000;
  tmp = uint64_s_shr(x,c);
  ones = uint64_s_shl(ones,64 - c);
  tmp = (s == 0)?tmp:uint64_s_or(tmp,ones);

  return tmp;
}

int uint64_s_eq(uint64_s x, uint64_s y)
{
  if (x.t[1] == y.t[1] && x.t[0] == y.t[0])
    return 1;
  else
    return 0;
}
