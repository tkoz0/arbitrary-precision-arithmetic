#include "u64arr_ll.hpp"

#include <cassert>

#include "../utils/u64ops.h"

bool u64arr_ll_inc(uint64_t *n, size_t l)
{
    bool c = true; // carry bit
    size_t i = 0;
    while (i < l and c) // propagate as long as numbers overflow
        c = (++n[i++] == 0);
    return c;
}

bool u64arr_ll_dec(uint64_t *n, size_t l)
{
    bool c = true; // means need to subtract 1
    size_t i = 0;
    while (i < l and c) // propagate as long as numbers underflow
        c = (n[i++]-- == 0);
    return c;
}

bool u64arr_ll_add_64(uint64_t *n, size_t l, uint64_t a)
{
    assert(l);
    uint64_t tmp = n[0]; // add to first limb
    n[0] += a;
    bool c = (n[0] < tmp); // propagate carry bit
    size_t i = 1;
    while (i < l and c)
        c = (++n[i++] == 0);
    return c;
}

bool u64arr_ll_sub_64(uint64_t *n, size_t l, uint64_t a)
{
    assert(l);
    uint64_t tmp = n[0]; // subtract from first limb
    n[0] -= a;
    bool c = (n[0] > tmp); // propagate the "borrow" process
    size_t i = 0;
    while (i < l and c)
        c = (n[i++]-- == 0);
    return c;
}

uint32_t u64arr_ll_mul_32(uint64_t *n, size_t l, uint32_t a)
{
    uint32_t *nn = (uint32_t*)n; // work with 32 bit half limbs
    uint32_t c = 0;
    uint64_t m;
    for (size_t i = 0; i < 2*l; ++i)
    {
        m = (uint64_t)(nn[i])*(uint64_t)a + c;
        nn[i] = m;
        c = m >> 32;
    }
    return c;
}

uint64_t u64arr_ll_mul_64(uint64_t *n, size_t l, uint64_t a)
{
    uint64_t c = 0, m0, m1, tmp;
    for (size_t i = 0; i < l; ++i)
    {
        _mul64full(a,n[i],&m0,&m1);
        tmp = m0 + c;
        n[i] = tmp;
        c = m1 + (tmp < m0);
    }
    return c;
}

uint32_t u64arr_ll_div_32(uint64_t *n, size_t l, uint32_t a)
{
    uint32_t *nn = (uint32_t*)n;
    uint64_t v = 0;
    for (size_t i = 2*l; i--;)
    {
        v = (v << 32) | nn[i];
        nn[i] = v / a;
        v %= a;
    }
    return v;
}

uint64_t u64arr_ll_div_64(uint64_t *n, size_t l, uint64_t a)
{
    uint64_t v0 = 0, v1;
    for (size_t i = l; i--;)
    {
        v1 = v0;
        v0 = n[i];
        _udiv64_1(v0,v1,a,n+i,&v0);
    }
    return v0;
}

// digits for bases 2-36
const char *_digits1 = "0123456789abcdefghijklmnopqrstuvwxyz";
const char *_digits2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// convert digit to numereic value
static inline uint8_t _digitval(char c)
{
    return c <= '9' ? c-'0' : (c >= 'a' ? c-'a'+10 : c-'A'+10);
}

size_t u64arr_ll_write_str(uint8_t base, bool uppercase,
                           uint64_t *__restrict__ n, size_t l,
                           char *__restrict__ s)
{
    const char *_digits = _digits1;
    if (uppercase)
        _digits = _digits2;
    char *sptr = s;
    while (l and n[l-1] == 0)
        --l;
    if (!l) // special case for 0
    {
        s[0] = '0';
        s[1] = '\0';
        return 1;
    }
    while (l) // write digits starting from least significant
    {
        *(sptr++) = _digits[u64arr_ll_div_32(n,l,base)];
        if (n[l-1] == 0)
            --l;
    }
    *sptr = '\0';
    size_t ret = sptr - s;
    --sptr;
    while (s < sptr) // reverse digit order
    {
        char tmp = *s;
        *(s++) = *sptr;
        *(sptr--) = tmp;
    }
    return ret;
}

size_t u64arr_ll_read_str(uint8_t base,
                          const char *__restrict__ s,
                          uint64_t *__restrict__ n)
{
    size_t l = 1;
    n[0] = 0;
    while (*s)
    {
        uint8_t d = _digitval(*(s++));
        uint64_t cm = u64arr_ll_mul_64(n,l,base);
        if (cm)
            n[l++] = cm;
        bool ca = u64arr_ll_add_64(n,l,d);
        if (ca)
            n[l++] = 1;
    }
    return l;
}

bool u64arr_ll_add_to(uint64_t *__restrict__ n1, size_t l1,
                      const uint64_t *__restrict__ n2, size_t l2)
{
    assert(l1 >= l2);
    bool c = false;
    uint64_t tmp;
    size_t i;
    for (i = 0; i < l2; ++i)
    {
        // TODO make this faster
        // it generates a jump instruction
        // see x86 ADC instruction (add with carry)
        tmp = n1[i] + n2[i] + c;
        c = (tmp < n1[i]) or (c and tmp <= n1[i]);
        n1[i] = tmp;
    }
    while (i < l1 and c)
        c = (++n1[i++] == 0);
    return c;
}

bool u64arr_ll_sub_from(uint64_t *__restrict__ n1, size_t l1,
                        const uint64_t *__restrict__ n2, size_t l2)
{
    assert(l1 >= l2);
    bool c = true;
    uint64_t tmp;
    size_t i;
    for (i = 0; i < l2; ++i)
    {
        // TODO make this faster
        // see x86 SBB instruction (subtract with borrow)
        tmp = n1[i] + (~n2[i]) + c;
        c = (tmp < n1[i]) or (c and tmp <= n1[i]);
        n1[i] = tmp;
    }
    if (c) // leaves rest of {n1,l1} unchanged
        return true;
    while (i < l1 and !c)
        c = (n1[i++]-- != 0);
    return c;
}

bool u64arr_ll_add(const uint64_t *__restrict__ x, size_t lx,
                   const uint64_t *__restrict__ y, size_t ly,
                   uint64_t *__restrict__ z)
{
    size_t i = 0, l = (lx < ly ? lx : ly);
    bool c = false;
    while (i < l) // add lower limbs both have
    {
        // TODO see comment in u64arr_ll_add_to
        z[i] = x[i] + y[i] + c;
        c = (z[i] < x[i]) or (c and z[i] <= x[i]);
        ++i;
    }
    while (i < lx) // finish {x,lx}
    {
        z[i] = x[i] + c;
        c &= (x[i++] == (uint64_t)(-1));
    }
    while (i < ly) // finish {y,ly}
    {
        z[i] = y[i] + c;
        c &= (y[i++] == (uint64_t)(-1));
    }
    return c;
}

bool u64arr_ll_sub(const uint64_t *__restrict__ x, size_t lx,
                   const uint64_t *__restrict__ y, size_t ly,
                   uint64_t *__restrict__ z)
{
    size_t i = 0, l = (lx < ly ? lx : ly);
    bool c = true;
    while (i < l) // add lower limbs using 2s complement subtraction
    {
        // TODO see comment in u64arr_ll_sub_from
        z[i] = x[i] + (~y[i]) + c;
        c = (z[i] < x[i]) or (c and z[i] <= x[i]);
        ++i;
    }
    while (i < lx) // finish {x,lx}
    {
        z[i] = x[i] - (!c);
        c |= z[i++];
    }
    while (i < ly) // finish {y,ly}
    {
        z[i] = (~y[i]) + c;
        c &= (y[i] == 0);
    }
    return !c;
}

// type for diagonal sums in grid multiplication
// handles sums of 128 bit numbers with 192 bits
struct _add128
{
    uint64_t _u0, _u1, _u2;
    _add128(): _u0(0), _u1(0), _u2(0) {}
    inline void _add(uint64_t _v0, uint64_t _v1)
    {
        uint64_t tmp = _u0 + _v0;
        bool c = tmp < _v0;
        _u0 = tmp;
        tmp = _u1 + _v1 + c;
        c = (tmp < _v1) or (c and tmp <= _v1);
        _u1 = tmp;
        _u2 += c;
    }
};

static_assert(sizeof(_add128) == 24);

void u64arr_ll_mul(const uint64_t *__restrict__ x, size_t lx,
                   const uint64_t *__restrict__ y, size_t ly,
                   uint64_t *__restrict__ z)
{
    assert(lx > 0 and ly > 0);
    _add128 *sums = new _add128[lx+ly-1]();
    uint64_t v0, v1, tmp;
    for (size_t i = 0; i < lx; ++i)
        for (size_t j = 0; j < ly; ++j)
        {
            _mul64full(x[i],y[j],&v0,&v1);
            sums[i+j]._add(v0,v1);
        }
    // 3 pass addition
    for (size_t i = 0; i < lx+ly-1; ++i) // low
        z[i] = sums[i]._u0;
    bool c = false;
    for (size_t i = 0; i < lx+ly-1; ++i) // middle
    {
        // TODO see comment in u64arr_ll_add_to
        tmp = z[i+1] + sums[i]._u1 + c;
        c = (tmp < z[i+1]) or (c and tmp <= z[i+1]);
        z[i+1] = tmp;
    }
    assert(!c);
    for (size_t i = 0; i < lx+ly-2; ++i) // high
    {
        tmp = z[i+2] + sums[i]._u2 + c;
        c = (tmp < z[i+2]) or (c and tmp <= z[i+2]);
        z[i+2] = tmp;
    }
    assert(!c);
    assert(sums[lx+ly-2]._u2 == 0);
    delete[] sums;
}

// divide 2 equal length numbers {z,l} / {y,l}
// return quotient and store remainder in {z,l}
// requires highest limb in y is nonzero
static inline uint64_t u64arr_ll_div_helper(const uint64_t *__restrict__ y,
                                            uint64_t *__restrict__ z, size_t l)
{
    assert(l > 0 and y[l-1] and z[l-1]);
    uint64_t ret = 0;
    uint64_t *yy = new uint64_t[l];
    for (size_t i = 0; i < l; ++i)
        yy[i] = y[i];
    // shift yy
    size_t s = 0;
    while (yy[l-1] < z[l-1] and !(yy[l-1] >> 63))
    {
        yy[l-1] <<= 1;
        ++s;
    }
    if (s) // need to shift the rest
        for (size_t i = l-1; i--;)
        {
            yy[i+1] = (yy[i] >> (64-s));
            yy[i] <<= s;
        }
    ++s;
    while (s--) // repeatedly compare and shift to set bits in ret
    {
        // test if {z,l} >= {yy,l}
        size_t j = l-1;
        while (j and z[j] == yy[j])
            --j;
        if (z[j] >= yy[j]) // set bit in ret and subtract
        {
            ret |= (1uLL << s);
            bool o = u64arr_ll_sub_from(z,l,yy,l);
            assert(!o);
        }
        // shift right 1 bit
        for (size_t i = 0; i < l-1; ++i)
            yy[i] = (yy[i] >> 1) | (yy[i+1] << 63);
        yy[l-1] >>= 1;
    }
    delete[] yy;
    return ret;
}

void u64arr_ll_div(const uint64_t *__restrict__ x, size_t lx,
                   const uint64_t *__restrict__ y, size_t ly,
                   uint64_t *__restrict__ q,
                   uint64_t *__restrict__ r)
{
    assert(lx >= ly and ly > 0);
    while (y[ly-1] == 0)
        --ly; // will fail if {y,ly} is 0
    // create a copy of x to work with
    uint64_t *z = new uint64_t[lx];
    for (size_t i = 0; i < lx; ++i)
        z[i] = x[i];
    // create the quotient limb by limb
    size_t qi = lx-ly+1;
    while (qi--)
        q[qi] = u64arr_ll_div_helper(y,z+qi,ly);
    for (size_t i = 0; i < ly; ++i)
        r[i] = z[i];
    delete[] z;
}
