
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cstdlib>

#include <vector>

#include "../u64arr/u64arr_ll.hpp"
#include "../utils/fastmod.h"

// big unsigned integer
typedef std::vector<uint64_t> BUI;
#define UMAX 0xFFFFFFFFFFFFFFFFuLL

// test equality
bool BUI_eq(BUI a, BUI b)
{
    size_t la = a.size(), lb = b.size();
    while (la and a[la-1] == 0) --la;
    while (lb and b[lb-1] == 0) --lb;
    if (la != lb)
        return false;
    for (size_t i = 0; i < la; ++i)
        if (a[i] != b[i])
            return false;
    return true;
}

// generate from sequence
BUI BUI_gen_seq(uint64_t lo, uint64_t hi, uint64_t step)
{
    BUI ret;
    for (uint64_t n = lo; n < hi; n += step)
        ret.push_back(n);
    return ret;
}

// to restrict sizes for testing multiplication
// so there are plenty that do not overflow 64 bits
BUI masks_for_mul = {0xFFFFFFFFFFFFFFFFuLL,
                     0x00FFFFFFFFFFFFFFuLL,
                     0x0000FFFFFFFFFFFFuLL,
                     0x000000FFFFFFFFFFuLL,
                     0x00000000FFFFFFFFuLL,
                     0x0000000000FFFFFFuLL,
                     0x000000000000FFFFuLL,
                     0x00000000000000FFuLL};

BUI masks_for_add = {0xFFFFFFFFFFFFFFFFuLL,
                     0x0FFFFFFFFFFFFFFFuLL};

// generate from LCG
// bit masks allow better variation in limb magnitude
// otherwise almost all limbs are very big
// (>99% are bigger than 2^57 using pure LCG output)
BUI BUI_gen_lcg(uint64_t seed, size_t len,
                BUI bit_masks = {0xFFFFFFFFFFFFFFFFuLL},
                uint64_t mult = 0x5DEECE66DuLL, uint64_t add = 0xBuLL)
{
    BUI ret;
    for (size_t i = 0; i < len; ++i)
    {
        seed = seed*mult + add;
        uint64_t mask = bit_masks[(seed >> 32) % bit_masks.size()];
        seed = seed*mult + add;
        uint64_t tmp = seed >> 32;
        seed = seed*mult + add;
        tmp |= seed & 0xFFFFFFFF00000000uLL;
        ret.push_back(tmp & mask);
    }
    return ret;
}

// class for hashing numbers to check correctness (with high probability)
struct BUI_hash
{
    uint64_t h_add,h_xor,h_mod;
    BUI_hash(BUI &n)
    {
        h_add = 0;
        h_xor = 0;
        for (uint64_t i : n)
        {
            h_add += i; // sum of limbs
            h_xor ^= i; // xor of limbs
        }
        h_mod = _modm61arrle__v1(n.data(),n.size()); // mod by a prime
    }
    BUI_hash(uint64_t h_add, uint64_t h_xor, uint64_t h_mod):
        h_add(h_add), h_xor(h_xor), h_mod(h_mod) {}
    BUI_hash(uint64_t n): h_add(n), h_xor(n), h_mod(_modm61(n)) {}
    bool operator==(const BUI_hash &o)
    { return (h_add == o.h_add) and (h_xor == o.h_xor) and (h_mod == o.h_mod); }
    bool operator!=(const BUI_hash &o)
    { return (h_add != o.h_add) or (h_xor != o.h_xor) or (h_mod != o.h_mod); }
};

void test_u64arr_ll_inc()
{
    printf("test_u64arr_ll_inc()\n");
    BUI a = {0};
    bool ret = u64arr_ll_inc(a.data(),1);
    assert(!ret);
    assert(BUI_eq(a,{1}));
    ret = u64arr_ll_inc(a.data(),1);
    assert(!ret);
    assert(BUI_eq(a,{2}));
    a = {UMAX-1};
    ret = u64arr_ll_inc(a.data(),1);
    assert(!ret);
    assert(BUI_eq(a,{UMAX}));
    ret = u64arr_ll_inc(a.data(),1);
    assert(ret);
    a.push_back(1);
    assert(BUI_eq(a,{0,1}));
    ret = u64arr_ll_inc(a.data(),2);
    assert(!ret);
    assert(BUI_eq(a,{1,1}));
    a = {UMAX,UMAX,UMAX,UMAX};
    ret = u64arr_ll_inc(a.data(),4);
    assert(ret);
    assert(BUI_eq(a,{0,0,0,0}));
    a = {UMAX,UMAX,7};
    ret = u64arr_ll_inc(a.data(),3);
    assert(!ret);
    assert(BUI_eq(a,{0,0,8}));
}

void test_u64arr_ll_dec()
{
    printf("test_u64arr_ll_dec()\n");
    BUI a = {1};
    bool ret = u64arr_ll_dec(a.data(),1);
    assert(!ret);
    assert(BUI_eq(a,{0}));
    ret = u64arr_ll_dec(a.data(),1);
    assert(ret);
    assert(BUI_eq(a,{UMAX}));
    a = {0,0,1};
    ret = u64arr_ll_dec(a.data(),3);
    assert(!ret);
    assert(BUI_eq(a,{UMAX,UMAX}));
    ret = u64arr_ll_dec(a.data(),3);
    assert(BUI_eq(a,{UMAX-1,UMAX}));
    a = {0,0};
    ret = u64arr_ll_dec(a.data(),2);
    assert(ret);
    assert(BUI_eq(a,{UMAX,UMAX}));
}

void test_u64arr_ll_add_64()
{
    printf("test_u64arr_ll_add_64()\n");
    BUI a = {0};
    bool ret = u64arr_ll_add_64(a.data(),1,UMAX-5);
    assert(!ret);
    assert(BUI_eq(a,{UMAX-5}));
    ret = u64arr_ll_add_64(a.data(),1,UMAX-7);
    assert(ret);
    a.push_back(1);
    assert(BUI_eq(a,{UMAX-13,1}));
    a = {5,UMAX,UMAX,400};
    ret = u64arr_ll_add_64(a.data(),4,UMAX);
    assert(!ret);
    assert(BUI_eq(a,{4,0,0,401}));
    ret = u64arr_ll_add_64(a.data(),4,UMAX-1);
    assert(!ret);
    assert(BUI_eq(a,{2,1,0,401}));
}

void test_u64arr_ll_sub_64()
{
    printf("test_u64arr_ll_sub_64()\n");
    BUI a = {0};
    bool ret = u64arr_ll_sub_64(a.data(),1,0);
    assert(!ret);
    assert(BUI_eq(a,{0}));
    ret = u64arr_ll_sub_64(a.data(),1,1);
    assert(ret);
    assert(BUI_eq(a,{UMAX}));
    a = {17000000000uLL,0,0,1};
    ret = u64arr_ll_sub_64(a.data(),4,18000000000uLL);
    assert(!ret);
    assert(BUI_eq(a,{UMAX-999999999,UMAX,UMAX}));
    a = {5,0,0,0,0};
    ret = u64arr_ll_sub_64(a.data(),5,7);
    assert(ret);
    assert(BUI_eq(a,{UMAX-1,UMAX,UMAX,UMAX,UMAX}));
}

void test_u64arr_ll_mul_32()
{
    printf("test_u64arr_ll_mul_32()\n");
    BUI a = {0,0,0};
    uint32_t ret = u64arr_ll_mul_32(a.data(),3,71);
    assert(ret == 0);
    assert(BUI_eq(a,{0,0,0}));
    a[0] = 1;
    ret = u64arr_ll_mul_32(a.data(),3,88);
    assert(ret == 0);
    assert(BUI_eq(a,{88,0,0}));
    ret = u64arr_ll_mul_32(a.data(),3,0xFFFFFFFF);
    assert(ret == 0);
    assert(BUI_eq(a,{377957121960,0,0}));
    ret = u64arr_ll_mul_32(a.data(),3,0xFFFFFFFF);
    assert(ret == 0);
    assert(BUI_eq(a,{18446743317795307608uLL,87,0}));
    a = {77,12000000000000uLL};
    ret = u64arr_ll_mul_32(a.data(),2,750000000);
    assert(ret == 487);
    assert(BUI_eq(a,{57750000000uLL,16435636103448363008uLL}));
    a = {UMAX,UMAX,UMAX,UMAX};
    ret = u64arr_ll_mul_32(a.data(),4,1103);
    assert(ret == 1102);
    assert(BUI_eq(a,{UMAX-1102,UMAX,UMAX,UMAX}));
}

void test_u64arr_ll_mul_64()
{
    printf("test_u64arr_ll_mul_64()\n");
    BUI a = {1};
    uint64_t ret = u64arr_ll_mul_64(a.data(),1,_m61);
    assert(ret == 0);
    assert(BUI_eq(a,{_m61}));
    ret = u64arr_ll_mul_64(a.data(),1,_m61);
    assert(ret == 288230376151711743uLL);
    a.push_back(ret);
    assert(BUI_eq(a,{13835058055282163713uLL,288230376151711743uLL}));
    ret = u64arr_ll_mul_64(a.data(),2,_m61);
    assert(ret == 36028797018963967uLL);
    assert(BUI_eq(a,{6917529027641081855uLL,17582052945254416384uLL}));
    BUI primes100 = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,
                     53,59,61,67,71,73,79,83,89,97};
    a = {1};
    for (uint64_t p : primes100)
    {
        ret = u64arr_ll_mul_64(a.data(),a.size(),p);
        if (ret)
            a.push_back(ret);
    }
    assert(BUI_eq(a,{14005151959471558694uLL,124985089766135611uLL}));
    ret = u64arr_ll_mul_64(a.data(),2,_m31);
    assert(ret == 14550179);
    assert(BUI_eq(a,{15454450193228165082uLL,8151940110614728324uLL}));
}

void test_u64arr_ll_div_32()
{
    printf("test_u64arr_ll_div_32()\n");
}

void test_u64arr_ll_div_64()
{
    printf("test_u64arr_ll_div_64()\n");
}

void test_u64arr_ll_write_str()
{
    printf("test_u64arr_ll_write_str()\n");
}

void test_u64arr_ll_read_str()
{
    printf("test_u64arr_ll_read_str()\n");
}

void test_u64arr_ll_add_to()
{
    printf("test_u64arr_ll_add_to()\n");
}

void test_u64arr_ll_sub_from()
{
    printf("test_u64arr_ll_sub_from()\n");
}

void test_u64arr_ll_add()
{
    printf("test_u64arr_ll_add()\n");
}

void test_u64arr_ll_sub()
{
    printf("test_u64arr_ll_sub()\n");
}

void test_u64arr_ll_mul()
{
    printf("test_u64arr_ll_mul()\n");
}

void test_u64arr_ll_div()
{
    printf("test_u64arr_ll_div()\n");
}

int main(int argc, const char **argv)
{
    //test_u64arr_ll_inc();
    //test_u64arr_ll_dec();
    //test_u64arr_ll_add_64();
    //test_u64arr_ll_sub_64();
    //test_u64arr_ll_mul_32();
    test_u64arr_ll_mul_64();
    //test_u64arr_ll_div_32();
    //test_u64arr_ll_div_64();
    //test_u64arr_ll_write_str();
    //test_u64arr_ll_read_str();
    //test_u64arr_ll_add_to();
    //test_u64arr_ll_sub_from();
    //test_u64arr_ll_add();
    //test_u64arr_ll_sub();
    //test_u64arr_ll_mul();
    //test_u64arr_ll_div();
    return 0;
}
