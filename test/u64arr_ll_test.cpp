
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cstdlib>

#include <vector>

#include "../u64arr/u64arr_ll.hpp"

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
}

void test_u64arr_ll_sub_64()
{
    printf("test_u64arr_ll_sub_64()\n");
}

void test_u64arr_ll_mul_32()
{
    printf("test_u64arr_ll_mul_32()\n");
}

void test_u64arr_ll_mul_64()
{
    printf("test_u64arr_ll_mul_64()\n");
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
    test_u64arr_ll_inc();
    test_u64arr_ll_dec();
    test_u64arr_ll_add_64();
    test_u64arr_ll_sub_64();
    test_u64arr_ll_mul_32();
    test_u64arr_ll_mul_64();
    test_u64arr_ll_div_32();
    test_u64arr_ll_div_64();
    test_u64arr_ll_write_str();
    test_u64arr_ll_read_str();
    test_u64arr_ll_add_to();
    test_u64arr_ll_sub_from();
    test_u64arr_ll_add();
    test_u64arr_ll_sub();
    test_u64arr_ll_mul();
    test_u64arr_ll_div();
    return 0;
}
