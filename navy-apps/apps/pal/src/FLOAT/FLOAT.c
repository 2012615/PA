#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  //assert(0);
  //return 0;
  //(a*b)/2^16
  //we need a longer type
  return ((int64_t)a*(int64_t)b)>>16;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  //assert(0);
  //return 0;
  //make sure that b is not 0
  assert(b!=0);
  //div we should consider about reminder
  FLOAT num1=Fabs(a),num2=Fabs(b);
  FLOAT res=num1/num2;
  num1%=num2;

  for(int i=0;i<16;i++)
  {
    res<<=1;
    num1<<=1;
    if(num1>num2)
    {
      num1-=num2;
      res++;
    }
  }
  //judge whether the sign is different
  if(((a^b)&0x80000000)==0x80000000)
  {
    res=-res;
  }
  return res;
}

FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */

  /*
    IEEE754's float
    31  30   23   0
    +----+-------------------+--------------------+
    |sign| exp | tail |
    +----+-------------------+--------------------+
  */

  union _float{
    struct 
    {
      uint32_t tail:23;
      uint32_t exp:8;
      uint32_t sign:1;
    };
    uint32_t val;
  };
  
  union _float f;
  f.val=*((uint32_t*)(void*)&a);
  int ex=f.exp-127;
  FLOAT res=0;
  if(ex==128)//overflow
  {
    assert(0);
  }
  if(ex>=0)
  {
    int mov=7-ex;
    if(mov>=0)
    {
      res=(f.tail|1<<23)>>mov;
    }
    else
    {
      res=(f.tail|1<<23)<<(-mov);
    }
  }
  else// we keep the integer
  {
    return 0;
  }

  return f.sign==0?res:-res;

  //assert(0);
  //return 0;
}

FLOAT Fabs(FLOAT a) {
  //assert(0);
  //return 0;
  if((a&0x80000000)==0)
  {
    return a;
  }
  else
  {
    return -a;
  }
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

