#pragma once

#include <stdint.h>
#include <stdlib.h>

// mersenne primes
const uint64_t _m61 = 0x1FFFFFFFFFFFFFFFuLL;
const uint32_t _m31 = 0x7FFFFFFFu;

// modulo m61 (2^61-1)
static inline uint64_t _modm61(uint64_t a)
{
    uint64_t tmp = a & _m61;
    tmp += (a >> 61);
    return tmp - (tmp >= _m61)*_m61;
}

// modulo m31 (2^31-1)
static inline uint32_t _modm31(uint64_t a)
{
    uint64_t tmp = a & _m31;
    tmp += (a >> 31); // may be up to 34 bits
    uint32_t tmp2 = tmp & _m31;
    tmp2 += (tmp >> 31);
    return tmp2 - (tmp2 >= _m31)*_m31;
}

/*
2 versions written for large modulo m61, need to test for speed
*/

// modulo m61 (2^61-1) for large integer, least limb first
// this one generates much shorter asm output
static inline uint64_t _modm61arrle__v1(const uint64_t *arr, size_t len)
{
    uint64_t ret = 0; // always fits in 61 bits
    for (size_t i = len; i--;)
    {
        // multiply by 2^64 and add arr[i]
        uint64_t limb = arr[i];
        // compute {limb,ret} (order: low,high) modulo m61
        uint64_t tmp = (limb & _m61);
        uint64_t tmp2 = tmp;
        tmp += ((limb >> 61) | (ret << 3));
        // tmp < 2^61 + 2^64 (may overflow)
        uint64_t carry = (tmp < tmp2);
        tmp2 = tmp & _m61;
        tmp2 += (tmp >> 61) | (carry << 3); // add 3 or 4 bit number
        ret = tmp2 - (tmp2 >= _m61)*_m61;
    }
    return ret;
}

// modulo m61 (2^61-1) for large integer, least limb first
// this one generates much longer asm output
static inline uint64_t _modm61arrle__v2(const uint64_t *arr, size_t len)
{
    uint64_t ret = 0;
    // in each limb position, use some 2^p mod m61 (p=0,64,128,...)
    // 2^p mod m61 for any integer p is in the cycle 2^0,2^1,...,2^60
    size_t p = 0; // p for 2^p mod m61 (all are powers of 2)
    for (size_t i = 0; i < len; ++i)
    {
        uint64_t limb = arr[i];
        // 2^p * limb (where 2^p <= 2^60, p <= 60)
        uint64_t hi = p ? limb >> (64-p) : 0; // highest 4 bits will be 0
        uint64_t lo = limb << p;
        // step 1 for mod m61, hi becomes 0 after
        uint64_t lo61 = lo & _m61;
        lo >>= 61;
        lo |= hi << 3; // < 2^63 since highest 4 bits in hi are 0
        lo += lo61; // fits in 64 bits, compute mod m61 with other function
        ret = _modm61(ret + lo);
        p += 64; // go to next power of 2 mod m61 for next limb
        p -= (p >= 61)*61;
    }
    return ret;
}
