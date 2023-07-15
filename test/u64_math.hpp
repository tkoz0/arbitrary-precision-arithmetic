#pragma once

#include <cstdint>
#include <cstdlib>

// high bits of 64 bit multiplication to 128 bit result
static inline uint64_t _mul64hi(uint64_t a, uint64_t b)
{ return ((__uint128_t) a * (__uint128_t) b) >> 64; }

// low bits of 64 bit multiplication to 128 bit result
static inline uint64_t _mul64lo(uint64_t a, uint64_t b)
{ return a * b; }

// full 128 bit result of 64 bit multiplication
typedef struct { uint64_t lo, hi; } _mul64_t;

// should compile to 3 instructions with -O3
static inline _mul64_t _mul64(uint64_t a, uint64_t b)
{
    _mul64_t ret;
    __uint128_t c = (__uint128_t) a * (__uint128_t) b;
    ret.lo = c;
    ret.hi = c >> 64;
    return ret;
}

// mersenne prime 2^61-1
const uint64_t m61 = 0x1FFFFFFFFFFFFFFFuLL;

// fast modulus by mersenne prime m61
static inline uint64_t _mod_m61(uint64_t a)
{
    uint64_t tmp = a & m61;
    tmp += (a >> 61);
    return tmp - (tmp >= m61)*m61;
}

// fast modulus by mersenne prime m61 for big unsigned integer
static inline uint64_t _mod_m61(const uint64_t *arr, size_t len)
{
    uint64_t ret = 0;
    size_t p = 0; // p for 2^p mod m61 (all are powers of 2)
    for (size_t i = 0; i < len; ++i)
    {
        uint64_t limb = arr[i];
        // compute 2^p * limb (2^p <= 2^60) (128 bit result)
        uint64_t hi = p ? limb >> (64-p) : 0;
        uint64_t lo = limb << p;
        // step 1 for mod by m64, hi part guaranteed to become 0
        uint64_t low61 = lo & m61;
        lo >>= 61;
        lo |= hi << 3; // < 2^63
        lo += low61;
        ret = _mod_m61(ret + lo); // update with this limb
        p = (p + 64) % 61;
    }
    return ret;
}

// divide 128 bit number by 64 bit number, assumes quotient fits in 64 bits
// x86_64 only, uses divq instruction which compilers generally avoid
// sigfpe occurs if the quotient does not fit in 64 bits
static inline void _div64_1(uint64_t u0, uint64_t u1, uint64_t d,
                            uint64_t *q, uint64_t *r)
{
    uint64_t qq,rr;
    __asm__
    (
        "divq %[d]"
        : "=a"(qq), "=d"(rr) // quotient in rax, remainder in rdx
        : [d]"r"(d), "a"(u0), "d"(u1) // dividend in rdx:rax
    );
    *q = qq;
    *r = rr;
}

// divide 128 bit number by 64 bit number (x86_64 only, uses divq)
// allowing results that dont fit in 64 bits
static inline void _div64_2(uint64_t u0, uint64_t u1, uint64_t d,
                            uint64_t *q0, uint64_t *q1, uint64_t *r)
{
    // quotient to compute is (u1 * 2^64 + u0) / d
    // first divide u1 into u1 = u1q * d + u1r
    // then we have u1q * 2^64 + (u1r * 2^64 + u0) / d
    // 2nd division can be done with divq since u1r < d
    uint64_t u1q = u1 / d, u1r = u1 % d;
    *q1 = u1q;
    _div64_1(u0,u1r,d,q0,r);
}
