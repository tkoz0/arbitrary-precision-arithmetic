
#include <cstdio>
#include <vector>
#include <ctime>
#include "u64_math.hpp"
#include "bu64.hpp"
#define base 10
static inline uint64_t nanotime()
{
    timespec t;
    clock_gettime(CLOCK_MONOTONIC,&t);
    return t.tv_sec*1000000000L + t.tv_nsec;
}

typedef uint64_t u64;
typedef std::vector<u64> bu64v;

// class for hashing numbers as array of uint64_t
// detect errors with high probability without hardcoding lots of data
// - add limbs
// - xor limbs
// - mod 2^61-1
// TODO add word hash by 31*hash+nextword
struct bu64v_hash
{
    u64 h_add,h_xor,h_mod;
    bu64v_hash(bu64v &n)
    {
        while (n.size() > 0 and n.back() == 0)
            n.pop_back();
        h_add = 0;
        h_xor = 0;
        for (u64 i : n)
        {
            h_add += i; // sum limbs
            h_xor ^= i; // xor limbs
        }
        h_mod = _mod_m61(n.data(),n.size());
    }
    bu64v_hash(u64 h_add, u64 h_xor, u64 h_mod):
        h_add(h_add), h_xor(h_xor), h_mod(h_mod) {}
    bu64v_hash(u64 n): h_add(n), h_xor(n), h_mod(_mod_m61(n)) {}
    bool operator==(const bu64v_hash &a)
    {
        return (h_add == a.h_add) and (h_xor == a.h_xor) and (h_mod == a.h_mod);
    }
    bool operator!=(const bu64v_hash &a)
    {
        return (h_add != a.h_add) or (h_xor != a.h_xor) or (h_mod != a.h_mod);
    }
};

typedef std::vector<bu64v_hash> bu64v_hashv;

// factorial by multiplying in order
bu64v factorial_seq(u64 n)
{
    if (n == 0)
        return {1};
    bu64v arr;
    arr.push_back(1);
    for (u64 i = 1; i <= n; ++i)
    {
        u64 tmp = bu64_mul64(arr.data(),arr.size(),i);
        if (tmp)
            arr.push_back(tmp);
    }
    assert(arr.back() != 0);
    return arr;
}

// factorial by binary splitting
bu64v factorial_split_helper(u64 a, u64 b)
{
    if (a == b)
        return {a};
    u64 m = (a + b) >> 1;
    bu64v left = factorial_split_helper(a,m);
    bu64v right = factorial_split_helper(m+1,b);
    bu64v ret(left.size()+right.size());
    bu64_mul(left.data(),left.size(),right.data(),right.size(),ret.data());
    while (ret.back() == 0)
        ret.pop_back();
    return ret;
}

// factorial by binary splitting
bu64v factorial_split(u64 n)
{
    if (n < 2)
        return {1};
    return factorial_split_helper(2,n);
}

void test_factorial()
{
    printf("||| test_factorial() |||\n");
    //bu64v nvals = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 25, 30, 40, 50,
    //    100, 200, 300, 500, 1000, 2000, 3000, 4000, 5000, 6000, 8000,
    //    10000, 20000, 30000, 40000, 50000};
    bu64v nvals = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
        32, 50, 64, 100, 128, 256, 512};
    bu64v_hashv nhash =
    {
        bu64v_hash(1), // 0!
        bu64v_hash(1), // 1!
        bu64v_hash(2), // ...
        bu64v_hash(6),
        bu64v_hash(24),
        bu64v_hash(120),
        bu64v_hash(720),
        bu64v_hash(5040),
        bu64v_hash(40320),
        bu64v_hash(362880),
        bu64v_hash(3628800), // 10!
        bu64v_hash(39916800), // ...
        bu64v_hash(479001600),
        bu64v_hash(6227020800uLL),
        bu64v_hash(87178291200uLL),
        bu64v_hash(1307674368000uLL),
        bu64v_hash(20922789888000uLL),
        bu64v_hash(355687428096000uLL),
        bu64v_hash(6402373705728000uLL),
        bu64v_hash(121645100408832000uLL),
        bu64v_hash(2432902008176640000uLL), // 20!
        bu64v_hash(12415130045685458760uLL,12405751211500529480uLL,985765458384992837uLL), // 32!
        bu64v_hash(6589729573054611793uLL,13138209378248917993uLL,1073241748499649399uLL), // 50!
        bu64v_hash(1241560636448615309uLL,14726041283266335795uLL,85860879309046617uLL), // 64!
        bu64v_hash(15749613020236411213uLL,11356886691189310447uLL,549389702849517455uLL), // 100!
        bu64v_hash(16040779783429889426uLL,8059827708971936484uLL,811969475257955399uLL), // 128!
        bu64v_hash(4126120635465203972uLL,9636794159095227280uLL,2049759823180561440uLL), // 256!
        bu64v_hash(15505635027413656053uLL,2210342920938274031uLL,77083831934327881uLL) // 512!
    };
    assert(nvals.size() == nhash.size());
    bu64v arr;
    u64 t1,t2;
    for (size_t i = 0; i < nvals.size(); ++i)
    {
        u64 n = nvals[i];
        t1 = nanotime();
        arr = factorial_seq(n);
        //arr = factorial_split(n);
        t2 = nanotime();
        bu64v_hash h(arr);
        bu64v_hash he = nhash[i];
        if (he != h)
        {
            printf("computed %lu! (%lu limbs, %lu nsec)\n",
            n,arr.size(),t2-t1);
            printf("hash: sum(%lu) xor(%lu) mod(%lu)\n",h.h_add,h.h_xor,h.h_mod);
            printf("WRONG HASH, expected\n");
            printf("hash: sum(%lu) xor(%lu) mod(%lu)\n",he.h_add,he.h_xor,he.h_mod);
            assert(0);
        }
    }
    printf("test_factorial() passed\n");
}

void test_factorial_single(uint64_t n)
{
    u64 t1 = nanotime();
    //bu64v arr = factorial_seq(n);
    bu64v arr = factorial_split(n);
    u64 t2 = nanotime();
    printf("%lu limbs, %lu nsec\n",arr.size(),t2-t1);
}

int main(int argc, const char **argv)
{
    test_factorial();
    //test_factorial_single((u64)atoll(argv[1]));
    return 0;
};
