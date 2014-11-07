/* 64-bit multiplication and division
   Copyright (C) 1989, 1992-1999, 2000, 2001, 2002, 2003
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <libchaos/defs.h>

#if __WORDSIZE == 32

#include <libchaos/divdi3.h>

typedef unsigned int uint128_t	__attribute__ ((mode (QI)));

#define W_TYPE_SIZE 32

#if __BYTE_ORDER == __BIG_ENDIAN
struct int64struct { int32_t high, low;};
#elif __BYTE_ORDER == __LITTLE_ENDIAN
struct int64struct { int32_t low, high;};
#else
#error "unhandled endianity"
#endif
typedef union { struct int64struct s; int64_t ll; } int64union_t;

/* Prototypes of exported functions.  */
extern int64_t __divdi3 (int64_t u, int64_t v);
extern int64_t __moddi3 (int64_t u, int64_t v);
extern uint64_t __udivdi3 (uint64_t u, uint64_t v);
extern uint64_t __umoddi3 (uint64_t u, uint64_t v);

static uint64_t
__udivmoddi4 (uint64_t n, uint64_t d, uint64_t *rp)
{
  int64union_t ww;
  int64union_t nn, dd;
  int64union_t rr;
  uint32_t d0, d1, n0, n1, n2;
  uint32_t q0, q1;
  uint32_t b, bm;

  nn.ll = n;
  dd.ll = d;

  d0 = dd.s.low;
  d1 = dd.s.high;
  n0 = nn.s.low;
  n1 = nn.s.high;

#if !UDIV_NEEDS_NORMALIZATION
  if(d1 == 0)
  {
    if(d0 > n1)
    {
      /* 0q = nn / 0D */
      
      udiv_qrnnd(q0, n0, n1, n0, d0);
      q1 = 0;
      
      /* Remainder in n0.  */
    }
    else
    {
      /* qq = NN / 0d */
      
      if(d0 == 0)
        d0 = 1 / d0;	/* Divide intentionally by zero.  */
      
      udiv_qrnnd(q1, n1, 0, n1, d0);
      udiv_qrnnd(q0, n0, n1, n0, d0);
      
      /* Remainder in n0.  */
    }
    
    if(rp != 0)
    {
      rr.s.low = n0;
      rr.s.high = 0;
      *rp = rr.ll;
    }
  }
  
#else /* UDIV_NEEDS_NORMALIZATION */

  if(d1 == 0)
  {
    if(d0 > n1)
    {
      /* 0q = nn / 0D */
      
      count_leading_zeros (bm, d0);
      
      if(bm != 0)
      {
        /* Normalize, i.e. make the most significant bit of the
         denominator set.  */
        
        d0 = d0 << bm;
        n1 = (n1 << bm) | (n0 >> (W_TYPE_SIZE - bm));
        n0 = n0 << bm;
      }
      
      udiv_qrnnd(q0, n0, n1, n0, d0);
      q1 = 0;
      
      /* Remainder in n0 >> bm.  */
    }
    else
    {
      /* qq = NN / 0d */
      
      if(d0 == 0)
        d0 = 1 / d0;	/* Divide intentionally by zero.  */
      
      count_leading_zeros(bm, d0);

      if(bm == 0)
      {
        /* From (n1 >= d0) /\ (the most significant bit of d0 is set),
         * conclude (the most significant bit of n1 is set) /\ (the
         * leading quotient digit q1 = 1).
         * 
         * This special case is necessary, not an optimization.
	 *	 (Shifts counts of W_TYPE_SIZE are undefined.) 
         */
        n1 -= d0;
        q1 = 1;
      }
      else
      {
        /* Normalize.  */
        b = W_TYPE_SIZE - bm;
        
        d0 = d0 << bm;
        n2 = n1 >> b;
        n1 = (n1 << bm) | (n0 >> b);
        n0 = n0 << bm;
        
        udiv_qrnnd(q1, n1, n2, n1, d0);
      }
      
      /* n1 != d0...  */
      
      udiv_qrnnd(q0, n0, n1, n0, d0);
      
      /* Remainder in n0 >> bm.  */
    }
    
    if(rp != 0)
    {
      rr.s.low = n0 >> bm;
      rr.s.high = 0;
      *rp = rr.ll;
    }
  }
#endif /* UDIV_NEEDS_NORMALIZATION */
  else
  {
    if(d1 > n1)
    {
      /* 00 = nn / DD */
      q0 = 0;
      q1 = 0;

      /* Remainder in n1n0.  */
      if(rp != 0)
      {
        rr.s.low = n0;
        rr.s.high = n1;
        *rp = rr.ll;
      }
    }
    else
    {
      /* 0q = NN / dd */
      count_leading_zeros (bm, d1);
      if(bm == 0)
      {
        /* From (n1 >= d1) /\ (the most significant bit of d1 is set),
         * conclude (the most significant bit of n1 is set) /\ (the
         * quotient digit q0 = 0 or 1).
         * 
         * This special case is necessary, not an optimization.  
         */
        /* The condition on the next line takes advantage of that
         * n1 >= d1 (true due to program flow).  
         */
        if(n1 > d1 || n0 >= d0)
        {
          q0 = 1;
          sub_ddmmss(n1, n0, n1, n0, d1, d0);
        }
        else
          q0 = 0;
        
        q1 = 0;
        
        if(rp != 0)
        {
          rr.s.low = n0;
          rr.s.high = n1;
          *rp = rr.ll;
        }
      }
      else
      {
        uint32_t m1, m0;
        /* Normalize.  */
        b = W_TYPE_SIZE - bm;
        
        d1 = (d1 << bm) | (d0 >> b);
        d0 = d0 << bm;
        n2 = n1 >> b;
        n1 = (n1 << bm) | (n0 >> b);
        n0 = n0 << bm;

        udiv_qrnnd(q0, n1, n2, n1, d1);
        umul_ppmm(m1, m0, q0, d0);

        if(m1 > n1 || (m1 == n1 && m0 > n0))
        {
          q0--;
          sub_ddmmss(m1, m0, m1, m0, d1, d0);
        }
        
        q1 = 0;

        /* Remainder in (n1n0 - m1m0) >> bm.  */
        if(rp != 0)
        {
          sub_ddmmss(n1, n0, n1, n0, m1, m0);
          rr.s.low = (n1 << b) | (n0 >> bm);
          rr.s.high = n1 >> bm;
          *rp = rr.ll;
        }
      }
    }
  }
  
  ww.s.low = q0;
  ww.s.high = q1;
  return ww.ll;
}

int64_t __divdi3(int64_t u, int64_t v)
{
  int32_t c = 0;
  int64_t w;

  if(u < 0)
  {
    c = ~c;
    u = -u;
  }
  
  if(v < 0)
  {
    c = ~c;
    v = -v;
  }
  
  w = __udivmoddi4(u, v, NULL);
  
  if(c)
    w = -w;
  
  return w;
}

int64_t __moddi3(int64_t u, int64_t v)
{
  int32_t c = 0;
  int64_t w;

  if(u < 0)
  {
    c = ~c;
    u = -u;
  }
  
  if(v < 0)
    v = -v;
  
  __udivmoddi4(u, v, &w);
  
  if(c)
    w = -w;
  
  return w;
}

uint64_t __udivdi3(uint64_t u, uint64_t v)
{
  return __udivmoddi4(u, v, NULL);
}

uint64_t __umoddi3(uint64_t u, uint64_t v)
{
  uint64_t w;

  __udivmoddi4(u, v, &w);
  
  return w;
}

#endif /* __WORDSIZE == 32 */
