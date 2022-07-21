#include "struct64.h"
#include "performance.h"

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
  if (c<=32)
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
  
  uint64_s a,b;
  uint16_t a1 = (uint16_t)(x >> 16), a2 = (uint16_t)x, b1 = (uint16_t)(y >> 16), b2 = (uint16_t)y;
  a.t[1] = (uint32_t)a1 * (uint32_t)b1;
  a.t[0] = (uint32_t)a2 * (uint32_t)b2;
  b.t[0] = 0;
  b.t[1] = (uint32_t)a1 * b2 + (uint32_t)a2 * b1;
  
  b = shr(b,16);
  a = uint64_s_add(a,b);
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