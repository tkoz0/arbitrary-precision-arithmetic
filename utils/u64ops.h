// operations on 64 bit integers

#pragma once

#include <stdint.h>
#include <stdlib.h>

/*
multiplication (64 bit inputs and 128 bit result)
these compile as desired with GCC -O3
*/

// high 64 bits of 128 bit multiplication result (a*b)
static inline uint64_t _mul64hi(uint64_t a, uint64_t b)
{
    return ((__uint128_t)a * (__uint128_t)b) >> 64;
}

// low 64 bits of 128 bit multiplication result (a*b)
static inline uint64_t _mul64lo(uint64_t a, uint64_t b)
{
    return a * b;
}

// full 128 bit result of multiplication (a*b)
// low bits in *m0, high bits in *m1
static inline void _mul64full(uint64_t a, uint64_t b,
                              uint64_t *m0, uint64_t *m1)
{
    __uint128_t m = (__uint128_t)a * (__uint128_t)b;
    if (m0) *m0 = m;
    if (m1) *m1 = m >> 64;
}

/*
division (with 128 bit dividend and 64 bit divisor)
inline asm is needed to get this to compile to use the divq instruction
compilers avoid it because it raises sigfpe if quotient is too big
*/

// divide 128 bit number (u0 + u1*2^64) by 64 bit number (d)
// assumes quotient fits in 64 bits, otherwise sigfpe occurs
// (x86_64 only, uses divq instruction which compilers generally avoid)
static inline void _udiv64_1(uint64_t u0, uint64_t u1, uint64_t d,
                            uint64_t *q, uint64_t *r)
{
    uint64_t qq,rr;
    __asm__
    (
        "divq %[d]"
        : "=a"(qq), "=d"(rr) // quotient in rax, remainder in rdx
        : [d]"r"(d), "a"(u0), "d"(u1) // dividend in rdx:rax
    );
    if (q) *q = qq;
    if (r) *r = rr;
}

// divide 128 bit number (u0 + u1*2^64) by 64 bit number (d)
// stores the quotient in *q0 (low) and *q1 (high)
// stores the remainder in *r
// (x86_64 only, uses divq instruction which compilers generally avoid)
// this one does division twice so it can handle more than _div64_1
static inline void _udiv64_2(uint64_t u0, uint64_t u1, uint64_t d,
                            uint64_t *q0, uint64_t *q1, uint64_t *r)
{
    // quotient to compute is (u1*2^64 + u0) / d
    // first divide u1 into u1 = u1q*d + u1r
    // then we have u1q * 2^64 + (u1r*2^64 + u0) / d
    // 2nd division can be done with divq since u1r < d
    uint64_t u1q = u1 / d, u1r = u1 % d;
    if (q1) *q1 = u1q;
    _udiv64_1(u0,u1r,d,q0,r);
}
