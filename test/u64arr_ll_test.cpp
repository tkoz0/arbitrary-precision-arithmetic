
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <array>
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

// string equality
bool str_eq(const char *__restrict__ a, const char *__restrict__ b)
{
    return strcmp(a,b) == 0;
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

template <typename crc_t, crc_t poly, crc_t init, crc_t xorout>
crc_t crc(const uint8_t *buf, size_t len, crc_t h = 0)
{
#if 0 // cannot seem to get compile time table generation to work
    static constexpr std::array<crc_t,0x100> tab = []()
    {
        std::array<crc_t,0x100> tab;
        std::generate(tab.begin(),tab.end(),
            [n = crc_t{0}]() mutable
            {
                crc_t r = n++;
                for (size_t j = 0; j < 8; ++j)
                    r = (r >> 1) ^ ((r & 1) * poly);
                return r;
            });
        return tab;
    }();
#else // static variable for crc table
    static crc_t tab[0x100];
    static bool tab_gen = false;
    if (!tab_gen)
    {
        for (size_t i = 0; i < 0x100; ++i)
        {
            crc_t r = i;
            for (size_t j = 0; j < 8; ++j)
            {
                crc_t x = (r & 1) * poly;
                r = (r >> 1) ^ x;
            }
            tab[i] = r;
        }
        tab_gen = true;
    }
#endif
    h ^= init;
    for (const uint8_t *p = buf; p < buf+len; ++p)
        h = (h >> 8) ^ tab[(*p ^ h) & 0xFF];
    return h ^ xorout;
}

uint32_t crc32(const uint8_t *buf, size_t len)
{
    return crc<uint32_t,0xEDB88320,0xFFFFFFFF,0xFFFFFFFF>(buf,len);
}

uint32_t crc64(const uint8_t *buf, size_t len)
{
    return crc<uint64_t,0xC96C5795D7870F42uLL,0xFFFFFFFFFFFFFFFFuLL,
        0xFFFFFFFFFFFFFFFFuLL>(buf,len);
}
/*
uint32_t crc32(const uint8_t *buf, size_t len, uint32_t h = 0)
{
    static uint32_t tab[0x100];
    static bool tab_gen = false;
    if (!tab_gen)
    {
        for (size_t i = 0; i < 0x100; ++i)
        {
            uint32_t r = i;
            for (size_t j = 0; j < 8; ++j)
            {
                // polynomial 0x04C11DB7 reflected (iso 3309)
                // x^32+x^26+x^23+x^22+x^16+x^12+x^11
                // +x^10+x^8+x^7+x^5+x^4+x^2+x+1
                uint32_t x = (r & 1) * 0xEDB88320;
                r = (r >> 1) ^ x;
            }
            tab[i] = r;
        }
        tab_gen = true;
    }
    // invert before and after (iso standard)
    h = ~h;
    for (const uint8_t *p = buf; p < buf+len; ++p)
        h = (h >> 8) ^ tab[(*p ^ h) & 0xFF];
    return ~h;
}

uint64_t crc64(const uint8_t *buf, size_t len, uint64_t h = 0)
{
    static uint64_t tab[0x100];
    static bool tab_gen = false;
    if (!tab_gen)
    {
        for (size_t i = 0; i < 0x100; ++i)
        {
            uint64_t r = i;
            for (size_t j = 0; j < 8; ++j)
            {
                // polynomial 0x42F0E1EBA9EA3693 reflected (ecma-182)
                // x^64+x^62+x^57+x^55+x^54+x^53+x^52+x^47+x^46+x^45+x^40
                // +x^39+x^38+x^37+x^35+x^33+x^32+x^31+x^29+x^27+x^24+x^23
                // +x^22+x^21+x^19+x^17+x^13+x^12+x^10+x^9+x^7+x^4+x+1
                uint64_t x = (r & 1) * 0xC96C5795D7870F42uLL;
                r = (r >> 1) ^ x;
            }
            tab[i] = r;
        }
        tab_gen = true;
    }
    // invert before and after (as done with crc64 used in xz)
    h = ~h;
    for (const uint8_t *p = buf; p < buf+len; ++p)
        h = (h >> 8) ^ tab[(*p ^ h) & 0xFF];
    return ~h;
}
*/
uint64_t java_str_hash(const uint8_t *buf, size_t len, uint64_t h = 0)
{
    for (const uint8_t *p = buf; p < buf+len; ++p)
        h = (31*h) + (*p);
    return h;
}

// class for hashing numbers to check correctness (with high probability)
// TODO add java string hash (31*hash + nextbyte) and crc64
struct BUI_hash
{
    uint64_t h_add,h_xor,h_mod,h_str,h_crc;
    BUI_hash(BUI &n)
    {
        h_add = 0;
        h_xor = 0;
        h_str = 0;
        for (uint64_t i : n)
        {
            h_add += i; // sum of limbs
            h_xor ^= i; // xor of limbs
        }
        h_mod = _modm61arrle__v1(n.data(),n.size()); // mod by a prime
        h_str = java_str_hash((uint8_t*)n.data(),n.size()<<3);
        h_crc = crc64((uint8_t*)n.data(),n.size()<<3);
    }
    BUI_hash(uint64_t h_add, uint64_t h_xor, uint64_t h_mod,
             uint64_t h_str, uint64_t h_crc):
        h_add(h_add), h_xor(h_xor), h_mod(h_mod), h_str(h_str), h_crc(h_crc) {}
    bool operator==(const BUI_hash &o)
    { return (h_add == o.h_add) and (h_xor == o.h_xor) and (h_mod == o.h_mod)
            and (h_str == o.h_str) and (h_crc == o.h_crc); }
    bool operator!=(const BUI_hash &o)
    { return (h_add != o.h_add) or (h_xor != o.h_xor) or (h_mod != o.h_mod)
            or (h_str != o.h_str) or (h_crc != o.h_crc); }
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
    BUI a = {0};
    uint32_t ret = u64arr_ll_div_32(a.data(),1,75140);
    assert(ret == 0);
    assert(BUI_eq(a,{0}));
    a = {_m61};
    ret = u64arr_ll_div_32(a.data(),1,_m31);
    assert(ret == (1<<30)-1);
    assert(BUI_eq(a,{1<<30}));
    BUI primes100 = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,
                     53,59,61,67,71,73,79,83,89,97};
    a = {14005151959471558694uLL,124985089766135611uLL};
    for (uint64_t p : primes100)
    {
        ret = u64arr_ll_div_32(a.data(),a.size(),p);
        assert(ret == 0);
    }
    assert(a.back() == 0);
    a.pop_back();
    assert(BUI_eq(a,{1}));
    a = {14005151959471558694uLL,124985089766135611uLL};
    ret = u64arr_ll_div_32(a.data(),2,16777216);
    assert(ret == 11210790);
    assert(BUI_eq(a,{13481309715515015473uLL,7449691877uLL}));
}

void test_u64arr_ll_div_64()
{
    printf("test_u64arr_ll_div_64()\n");
    BUI a = {0};
    uint64_t ret = u64arr_ll_div_64(a.data(),1,UMAX);
    assert(ret == 0);
    assert(BUI_eq(a,{0}));
    a = {UMAX-2};
    ret = u64arr_ll_div_64(a.data(),1,UMAX);
    assert(ret == UMAX-2);
    assert(BUI_eq(a,{0}));
    a = {14722052863563208240uLL,2844907266022922488uLL,
         15977678935670796422uLL,3967148191121uLL};
    ret = u64arr_ll_div_64(a.data(),4,73000000000000uLL);
    assert(ret == 67850737755696uLL);
    assert(BUI_eq(a,{12113468845911842103uLL,4722885315982888144uLL,
                     1002479005261710302uLL,0}));
    ret = u64arr_ll_div_64(a.data(),4,1000000000000037uLL);
    assert(ret == 296493693962529uLL);
    assert(BUI_eq(a,{16956912574872315102uLL,8836087472045989416uLL,1002,0}));
    ret = u64arr_ll_div_64(a.data(),4,142857);
    assert(ret == 111453);
    assert(BUI_eq(a,{13072324724826654137uLL,129447444992748109uLL,0,0}));
}

void test_u64arr_ll_write_str()
{
    printf("test_u64arr_ll_write_str()\n");
    char s[1000];
    BUI a = {0};
    size_t ret = u64arr_ll_write_str(2,true,a.data(),1,s);
    assert(ret == 1);
    assert(!strcmp(s,"0"));
    a = {13179439483193780233uLL,795447783920280270uLL,10302852741122617414uLL,
         4686237692481951503uLL,852376800724301uLL};
    ret = u64arr_ll_write_str(36,true,a.data(),5,s);
    assert(ret == 60);
    assert(!strcmp(s,
        "1HJTR9LZK0RTZFK81YK6LVBJK3E0TU6CIN22GVKP0OJUAHHIGG7U8WO5Y96X"));
    assert(BUI_eq(a,{0}));
    BUI a2 = {14996889397075187173uLL,16224389114002008162uLL,29004uLL};
    a = a2;
    ret = u64arr_ll_write_str(21,false,a.data(),3,s);
    assert(ret == 33);
    assert(!strcmp(s,"4h5h6d75d04backc05969222gbb910451"));
    a = a2;
    ret = u64arr_ll_write_str(10,true,a.data(),3,s);
    assert(ret == 43);
    assert(!strcmp(s,"9869849057328637468598619034897346872546789"));
    ret = u64arr_ll_write_str(2,false,a.data(),2,s);
    assert(ret == 1);
    assert(!strcmp(s,"0"));
    a = {12157665459056928801uLL}; // 3^40
    ret = u64arr_ll_write_str(2,false,a.data(),1,s);
    assert(ret == 64);
    assert(!strcmp(s,
        "1010100010111000101101000101001000101001000111111110100000100001"));
    a = {12157665459056928801uLL}; // 3^40
    ret = u64arr_ll_write_str(3,false,a.data(),1,s);
    assert(ret == 41);
    assert(!strcmp(s,"10000000000000000000000000000000000000000"));
    a = {12157665459056928801uLL}; // 3^40
    ret = u64arr_ll_write_str(6,false,a.data(),1,s);
    assert(ret == 25);
    assert(!strcmp(s,"2322113124155541030050213"));
    a = {12157665459056928801uLL,32}; // 3^40 + 2^69
    ret = u64arr_ll_write_str(11,true,a.data(),2,s);
    assert(ret == 20);
    assert(!strcmp(s,"993A16326A55A1898567"));
}

void test_u64arr_ll_read_str()
{
    printf("test_u64arr_ll_read_str()\n");
    BUI a = {0,0,0,0,0,0};
    size_t ret = u64arr_ll_read_str(36,
        "1HJTR9LZK0RTZFK81YK6LVBJK3E0TU6CIN22GVKP0OJUAHHIGG7U8WO5Y96X",
        a.data());
    assert(ret == 5);
    assert(BUI_eq(a,{13179439483193780233uLL,795447783920280270uLL,
        10302852741122617414uLL,4686237692481951503uLL,852376800724301uLL}));
    a = {0,0,0};
    ret = u64arr_ll_read_str(21,"4h5h6d75d04backc05969222gbb910451",a.data());
    assert(ret == 3);
    assert(BUI_eq(a,
        {14996889397075187173uLL,16224389114002008162uLL,29004uLL}));
    a = {0,0,0};
    ret = u64arr_ll_read_str(10,
        "9869849057328637468598619034897346872546789",a.data());
    assert(ret == 3);
    assert(BUI_eq(a,
        {14996889397075187173uLL,16224389114002008162uLL,29004uLL}));
    a = {0,0};
    ret = u64arr_ll_read_str(2,"0",a.data());
    assert(ret == 1);
    assert(BUI_eq(a,{0}));
    ret = u64arr_ll_read_str(2,
        "1010100010111000101101000101001000101001000111111110100000100001",
        a.data());
    assert(ret == 1);
    assert(BUI_eq(a,{12157665459056928801uLL}));
    ret = u64arr_ll_read_str(3,
        "10000000000000000000000000000000000000000",a.data());
    assert(ret == 1);
    assert(BUI_eq(a,{12157665459056928801uLL}));
    ret = u64arr_ll_read_str(6,"2322113124155541030050213",a.data());
    assert(ret == 1);
    assert(BUI_eq(a,{12157665459056928801uLL}));
    ret = u64arr_ll_read_str(11,"993A16326A55A1898567",a.data());
    assert(ret == 2);
    assert(BUI_eq(a,{12157665459056928801uLL,32}));
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
    (void)argc;
    (void)argv;
    //test_u64arr_ll_inc();
    //test_u64arr_ll_dec();
    //test_u64arr_ll_add_64();
    //test_u64arr_ll_sub_64();
    //test_u64arr_ll_mul_32();
    //test_u64arr_ll_mul_64();
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
