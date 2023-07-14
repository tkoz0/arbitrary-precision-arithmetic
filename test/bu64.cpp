#include "bu64.hpp"

#include "u64_math.hpp"

// multiply big uint by uint64_t
uint64_t bu64_mul64(uint64_t *input, size_t ilen, uint64_t m)
{
    _mul64_t mv;
    uint64_t c = 0; // carry
    size_t i;
    for (i = 0; i < ilen; ++i)
    {
        mv = _mul64(m,input[i]);
        uint64_t tmp = mv.lo + c;
        input[i] = tmp;
        c = mv.hi + (tmp < mv.lo); // handle overflow
    }
    return c;
}

// add uint64_t to big uint
bool bu64_add64(uint64_t *input, size_t ilen, uint64_t a)
{
    uint64_t c = a;
    size_t i = 0;
    while (c and i < ilen)
    {
        uint64_t tmp = input[i] + c;
        input[i++] = tmp;
        c = (tmp < c);
    }
    return (i == ilen) and c;
}

// divide big uint by uint32_t (uint64_t does not work as easily)
uint32_t bu64_div32(uint64_t *input, size_t ilen, uint32_t d)
{
    uint32_t *input32 = (uint32_t*)input;
    uint64_t v = 0;
    for (size_t i = 2*ilen; i--;)
    {
        v = (v << 32) | input32[i]; // < d * 2^32
        input32[i] = v / d; // will fit in 32 bits
        v %= d;
    }
    return v;
}

// subtract uint64_t from big uint
bool bu64_sub64(uint64_t *input, size_t ilen, uint64_t a)
{
    uint64_t s = a;
    size_t i = 0;
    while (s and i < ilen)
    {
        uint64_t tmp = input[i];
        input[i] -= s;
        s = (input[i++] > tmp);
    }
    return not s;
}

// type for handling diagonal sums in grid multiply method
struct _u192
{
    uint64_t u0, u1, u2;
    _u192(): u0(0), u1(0), u2(0) {}
    void add(uint64_t v0, uint64_t v1)
    {
        uint64_t tmp = u0 + v0;
        bool c = tmp < v0;
        u0 = tmp;
        tmp = u1 + v1 + c;
        c = (tmp < v1) or (c and tmp <= v1);
        u1 = tmp;
        u2 += c;
    }
};

// add bu64 objects
bool bu64_add(const uint64_t *__restrict__ arr1, size_t len1,
              const uint64_t *__restrict__ arr2, size_t len2,
              uint64_t *__restrict__ output)
{
    bool carry = 0;
    size_t i = 0;
    size_t lmin = len1 < len2 ? len1 : len2;
    uint64_t tmp;
    for (; i < lmin; ++i)
    {
        tmp = arr1[i] + arr2[i] + carry;
        carry = (tmp < arr1[i]) or (carry and tmp <= arr1[i]);
        output[i] = tmp;
    }
    for (; i < len1; ++i)
    {
        tmp = arr1[i] + carry;
        carry = (tmp == 0);
        output[i] = tmp;
    }
    for (; i < len2; ++i)
    {
        tmp = arr2[i] + carry;
        carry = (tmp == 0);
        output[i] = tmp;
    }
    return carry;
}

// subtract bu64 objects
bool bu64_sub(const uint64_t *__restrict__ arr1, size_t len1,
              const uint64_t *__restrict__ arr2, size_t len2,
              uint64_t *__restrict__ output)
{
    bool carry = 1;
    size_t i = 0;
    size_t lmin = len1 < len2 ? len1 : len2;
    uint64_t tmp;
    for (; i < lmin; ++i)
    {
        tmp = arr1[i] + (~arr2[i]) + carry;
        carry = (tmp < arr1[i]) or (carry and tmp <= arr1[i]);
        output[i] = tmp;
    }
    for (; i < len1; ++i)
    {
        tmp = arr1[i] + (-1) + carry;
        carry |= arr1[i];
        output[i] = tmp;
    }
    for (; i < len2; ++i)
    {
        tmp = 0 + (~arr2[i]) + carry;
        carry = (tmp == 0);
        output[i] = tmp;
    }
    return !carry;
}

// multiply bu64 objects
void bu64_mul(const uint64_t *__restrict__ arr1, size_t len1,
              const uint64_t *__restrict__ arr2, size_t len2,
              uint64_t *__restrict__ output)
{
    assert(len1 > 0 and len2 > 0);
    // store the diagonal sums from the grid method
    _u192 *diagsums = new _u192[len1+len2-1]();
    for (size_t i1 = 0; i1 < len1; ++i1)
        for (size_t i2 = 0; i2 < len2; ++i2)
        {
            _mul64_t prod = _mul64(arr1[i1],arr2[i2]);
            diagsums[i1+i2].add(prod.lo,prod.hi);
        }
    // 3 pass addition to output
    for (size_t i = 0; i < len1+len2-1; ++i) // low value (store, shift 0)
        output[i] = diagsums[i].u0;
    output[len1+len2-1] = 0;
    bool carry = 0;
    uint64_t tmp;
    for (size_t i = 0; i < len1+len2-1; ++i) // middle value (shift 1)
    {
        tmp = output[i+1] + diagsums[i].u1 + carry;
        carry = (tmp < output[i+1]) or (carry and tmp <= output[i+1]);
        output[i+1] = tmp;
    }
    assert(!carry);
    carry = 0;
    for (size_t i = 0; i < len1+len2-2; ++i) // high values (shift 2)
    {
        tmp = output[i+2] + diagsums[i].u2 + carry;
        carry = (tmp < output[i+2]) or (carry and tmp <= output[i+2]);
        output[i+2] = tmp;
    }
    assert(!carry);
    delete[] diagsums;
}

// negate bu64 object
void bu64_neg(uint64_t *__restrict__ arr, size_t len)
{
    size_t i;
    for (i = 0; i < len; ++i)
        arr[i] = ~arr[i];
    bool c = 1;
    i = 0;
    while (c and i < len)
    {
        ++arr[i];
        c = (arr[i++] == 0);
    }
}

const char *_digits = "0123456789abcdefghijklmnopqrstuvwxyz";

// character to numeric value for base conversion
static inline uint8_t _conv_digit(char c)
{
    return c <= '9' ? c-'0' : c-'a'+10;
}

// convert binary big uint to string
size_t bu64_write_str(uint8_t base, uint64_t *__restrict__ input, size_t ilen,
                      char *__restrict__ output)
{
    char *optr = output;
    while (ilen and input[ilen-1] == 0)
        --ilen;
    if (!ilen) // special case for 0
    {
        output[0] = '0';
        output[1] = '\0';
        return 1;
    }
    while (ilen) // write most significant to least significant digits
    {
        *(optr++) = _digits[bu64_div32(input,ilen,base)];
        if (input[ilen-1] == 0)
            --ilen;
    }
    *optr = '\0';
    size_t ret = optr - output;
    --optr;
    while (output < optr) // reverse string order
    {
        char tmp = *output;
        *(output++) = *optr;
        *(optr--) = tmp;
    }
    return ret;
}

// convert string to binary big uint
size_t bu64_read_str(uint8_t base, const char *__restrict__ input,
                     uint64_t *__restrict__ output)
{
    size_t olen = 0;
    while (*input)
    {
        uint8_t d = _conv_digit(*(input++));
        uint64_t cm = bu64_mul64(output,olen,base);
        if (cm)
            output[olen++] = cm;
        bool ca = bu64_add64(output,olen,d);
        if (ca)
            output[olen++] = 1;
    }
    return olen;
}

/* possibly useful intrinsics (all values in "little endian" order)

[32 bit mult -> 64 bit result]
(AVX2) _mm256_mul_epu32([a0,0,a1,0,a2,0,a3,0],[b0,0,b1,0,b2,0,b3,0])
-> [a0*b0,a1*b1,a2*b2,a3*b3]
(SSE2) _mm_mul_epu32([a0,0,a1,0],[b0,0,b1,0]) -> [a0*b0,a1*b1]

[parallel addition/subtraction]
(AVX2) _mm256_sub_epi32([a0..a7],[b0..b7]) -> [ai-bi, ...]
(AVX2) _mm256_add_epi32([a0..a7],[b0..b7]) -> [ai+bi, ...]
(AVX2) _mm256_sub_epi64([a0..a3],[b0..b3]) -> [ai-bi, ...]
(AVX2) _mm256_add_epi64([a0..a3],[b0..b3]) -> [ai+bi, ...]
(SSE2) _mm_add_epi32([a0..a3],[b0..b3]) -> [ai+bi, ...]
(SSE2) _mm_add_epi64([a0,a1],[b0,b1]) -> [a0+b0,a1+b1]
(SSE2) _mm_sub_epi32([a0..a3],[b0..b3]) -> [ai-bi, ...]
(SSE2) _mm_sub_epi64([a0,a1],[b0,b1]) -> [a0-b0,a1-b1]

[set values]
(AVX) _mm256_set_epi32(a0,a1,a2,a3,a4,a5,a6,a7)
(AVX) _mm256_set_epi64x(a0,a1,a2,a3)
(SSE2) _mm_set_epi32(a0,a1,a2,a3)
(SSE2) _mm_set_epi64x(a0,a1)

*/
