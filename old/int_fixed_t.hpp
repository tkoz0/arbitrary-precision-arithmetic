// emulation for larger fixed width integers

#pragma once

#include <cassert>
#include <cstdint>
#include <cstdlib>

#include "u64_math.hpp"

class uint128_t
{
    uint64_t u[2]; // low, high
public:
    inline uint128_t(uint64_t u0 = 0, uint64_t u1 = 0)
    {
        u[0] = u0;
        u[1] = u1;
    }
    inline uint128_t &operator=(uint64_t a)
    {
        u[0] = a;
        u[1] = 0;
        return *this;
    }
    inline uint128_t &operator+=(const uint128_t &a)
    {
        uint64_t tmp = u[0] + a.u[0];
        u[0] = tmp;
        u[1] += a.u[1] + (tmp < a.u[0]);
        return *this;
    }
    inline uint128_t &operator-=(const uint128_t &a)
    {
        uint64_t tmp = u[0] + (~a.u[0]) + 1;
        u[0] = tmp;
        u[1] += (~a.u[1]) + (tmp <= a.u[0]);
        return *this;
    }
    inline uint128_t &operator*=(const uint128_t &a)
    {
        _mul64_t m = _mul64(u[0],a.u[0]);
        m.hi += u[0] * a.u[1];
        m.hi += u[1] * a.u[0];
        u[0] = m.lo;
        u[1] = m.hi;
        return *this;
    }
    inline uint128_t &operator/=(const uint128_t &a)
    {
        assert(0); // not implemented
        return *this;
    }
    inline uint128_t &operator%=(const uint128_t &a)
    {
        assert(0); // not implemented
        return *this;
    }
    inline uint128_t &operator^=(const uint128_t &a) { return u[0] ^= a.u[0], u[1] ^= a.u[1], *this; }
    inline uint128_t &operator&=(const uint128_t &a) { return u[0] &= a.u[0], u[1] &= a.u[1], *this; }
    inline uint128_t &operator|=(const uint128_t &a) { return u[0] |= a.u[0], u[1] |= a.u[1], *this; }
    inline uint128_t &operator<<=(size_t s)
    {
        if (s >= 64)
        {
            u[1] = u[0] << (s-64);
            u[0] = 0;
        }
        else
        {
            u[1] = (u[1] << s) | (u[0] >> (64-s));
            u[0] <<= s;
        }
        return *this;
    }
    inline uint128_t &operator>>=(size_t s)
    {
        if (s >= 64)
        {
            u[0] = u[1] >> (s-64);
            u[1] = 0;
        }
        else
        {
            u[0] = (u[0] >> s) | (u[1] << (64-s));
            u[1] >>= s;
        }
        return *this;
    }
    inline uint128_t operator+(const uint128_t &a) const
    {
        uint128_t ret = *this;
        ret += a;
        return ret;
    }
    inline uint128_t operator-(const uint128_t &a) const
    {
        uint128_t ret = *this;
        ret -= a;
        return ret;
    }
    inline uint128_t operator*(const uint128_t &a) const
    {
        uint128_t ret = *this;
        ret *= a;
        return ret;
    }
    inline uint128_t operator/(const uint128_t &a) const
    {
        uint128_t ret = *this;
        ret /= a;
        return ret;
    }
    inline uint128_t operator%(const uint128_t &a) const
    {
        uint128_t ret = *this;
        ret %= a;
        return ret;
    }
    inline void divmod(const uint128_t &a, uint128_t &quot, uint128_t &rem) const
    {
        quot = 0;
        rem = 0;
    }
    inline uint128_t operator^(const uint128_t &a) const { return uint128_t(u[0]^a.u[0],u[1]^a.u[1]); }
    inline uint128_t operator&(const uint128_t &a) const { return uint128_t(u[0]&a.u[0],u[1]&a.u[1]); }
    inline uint128_t operator|(const uint128_t &a) const { return uint128_t(u[0]|a.u[0],u[1]|a.u[1]); }
    inline uint128_t operator<<(size_t s) const
    {
        uint128_t ret = *this;
        ret <<= s;
        return ret;
    }
    inline uint128_t operator>>(size_t s) const
    {
        uint128_t ret = *this;
        ret >>= s;
        return ret;
    }
    inline uint128_t operator~() const { return uint128_t(~u[0],~u[1]); }
    // comparison
    inline bool operator==(const uint128_t &a) const { return u[0] == a.u[0] and u[1] == a.u[1]; }
    inline bool operator!=(const uint128_t &a) const { return u[0] != a.u[0] or u[1] != a.u[1]; }
    inline bool operator<=(const uint128_t &a) const { return u[1] < a.u[1] or (u[1] == a.u[1] and u[0] <= a.u[0]); }
    inline bool operator>=(const uint128_t &a) const { return u[1] > a.u[1] or (u[1] == a.u[1] and u[0] >= a.u[0]); }
    inline bool operator<(const uint128_t &a) const { return u[1] < a.u[1] or (u[1] == a.u[1] and u[0] < a.u[0]); }
    inline bool operator>(const uint128_t &a) const { return u[1] > a.u[1] or (u[1] == a.u[1] and u[0] > a.u[0]); }
    // logical (lose short circuit evaluation)
    inline bool operator&&(const uint128_t &a) const { return bool(*this) && bool(a); }
    inline bool operator||(const uint128_t &a) const { return bool(*this) || bool(a); }
    inline bool operator!() const { return !u[0] and !u[1]; }
    inline operator bool() const { return u[0] or u[1]; }
    // conversion
    inline operator uint64_t() const { return u[0]; }
    inline operator uint32_t() const { return u[0]; }
    inline operator uint16_t() const { return u[0]; }
    inline operator uint8_t() const { return u[0]; }
    inline operator int64_t() const { return u[0]; }
    inline operator int32_t() const { return u[0]; }
    inline operator int16_t() const { return u[0]; }
    inline operator int8_t() const { return u[0]; }
    // prefix
    inline uint128_t &operator++()
    {
        u[1] += !(++u[0]);
        return *this;
    }
    inline uint128_t &operator--()
    {
        u[1] -= !(u[0]--);
        return *this;
    }
    // postfix
    inline uint128_t operator++(int)
    {
        uint128_t ret = *this;
        operator++();
        return ret;
    }
    inline uint128_t operator--(int)
    {
        uint128_t ret = *this;
        operator--();
        return ret;
    }
    // access halfs
    inline uint64_t operator[](bool ind) const { return u[ind]; }
};
