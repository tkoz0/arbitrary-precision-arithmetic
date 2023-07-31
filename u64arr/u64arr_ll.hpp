/*
big unsigned integer with 64 bit limbs (binary format)
represented as an array of 64 bit unsigned integers
u64arr_ll_ prefix for low level operations
numbers represented as a sequence of uint64_t starting with least significant
array {a0,a1,a2,...} is a0 + a1*2^64 + a2*2^128 + ...
comments may use {pointer,length} to describe a big unsigned integer
length must be >= 1 otherwise behavior may be undefined
extra zeroes at the end may be included, but try to remove them to optimize
TODO support bases like 3^40<2^64 to compact base 3 digits
without needing slower base conversion
*/

#pragma once

#include <cstdint>
#include <cstdlib>

/*
low level in-place operations with small numbers (modify inputs)
*/

// increment {n,l} (add 1)
// returns carry bit
bool u64arr_ll_inc(uint64_t *n, size_t l);

// decrement {n,l} (subtract 1)
// returns true if input is all 0 (underflow occurs)
bool u64arr_ll_dec(uint64_t *n, size_t l);

// add a 64 bit integer to {n,l}
// returns carry bit
bool u64arr_ll_add_64(uint64_t *n, size_t l, uint64_t a);

// subtract a 64 bit integer from {n,l}
// returns true if underflow occurs
bool u64arr_ll_sub_64(uint64_t *n, size_t l, uint64_t a);

// multiply {n,l} by a 32 bit integer
// returns carry amount
// TODO test if this is faster than mul_64
uint32_t u64arr_ll_mul_32(uint64_t *n, size_t l, uint32_t a);

// multiply {n,l} by a 64 bit integer
// returns carry amount
uint64_t u64arr_ll_mul_64(uint64_t *n, size_t l, uint64_t a);

// divide {n,l} by a 32 bit integer
// returns remainder (modulus)
// TODO test if this is faster than div_64
uint32_t u64arr_ll_div_32(uint64_t *n, size_t l, uint32_t a);

// divide {n,l} by a 64 bit integer
// returns remainder (modulus)
uint64_t u64arr_ll_div_64(uint64_t *n, size_t l, uint64_t a);

/*
low level conversion to/from strings
*/

// convert number {n,l} to string (s)
// bases 2-36 supported using specified case
// s must be long enough to fit result and null
// input is modified for division in place
// returns length of result (not including null)
size_t u64arr_ll_write_str(uint8_t base, bool uppercase,
                           uint64_t *__restrict__ n, size_t l,
                           char *__restrict__ s);

// convert string (s) to number (n)
// only bases 2-36 are supported with lowercase/uppercase letters
// s must end with null and consist only of 0-9 and a-z and A-Z
// (limited by thet base chosen, use a-z and A-Z for digit values 10-35)
// if input contains other characters, result is undefined
// n must be long enough to fit result
// returns number of limbs in result
size_t u64arr_ll_read_str(uint8_t base,
                          const char *__restrict__ s,
                          uint64_t *__restrict__ n);

/*
larger in-place operations
*/

// sets {n1,l1} to {n1,l1}+{n2,l2}
// requires l1 >= l2
// returns carry bit if going past length l1
bool u64arr_ll_add_to(uint64_t *__restrict__ n1, size_t l1,
                      const uint64_t *__restrict__ n2, size_t l2);

// sets {n1,l1} to {n1,l1}-{n2,l2}
// requires l1 >= l2
// returns true if underflow occurs
bool u64arr_ll_sub_from(uint64_t *__restrict__ n1, size_t l1,
                        const uint64_t *__restrict__ n2, size_t l2);

/*
operations on same length inputs
*/

/*
operations on different length inputs
*/

// {z,} = {x,lx} + {y,ly}
// z must have length >= max(lx,ly)
// returns carry bit if needing length > max(lx,ly)
bool u64arr_ll_add(const uint64_t *__restrict__ x, size_t lx,
                   const uint64_t *__restrict__ y, size_t ly,
                   uint64_t *__restrict__ z);

// {z,} = {x,lx} - {y,ly}
// z must have length >= max(lx,ly)
// returns true if underflow occurs
bool u64arr_ll_sub(const uint64_t *__restrict__ x, size_t lx,
                   const uint64_t *__restrict__ y, size_t ly,
                   uint64_t *__restrict__ z);

// {z,} = {x,lx} * {y,ly}
// output must have length >= lx+ly
void u64arr_ll_mul(const uint64_t *__restrict__ x, size_t lx,
                   const uint64_t *__restrict__ y, size_t ly,
                   uint64_t *__restrict__ z);

// {q,} = {x,lx} / {y,ly}
// {r,} = {x,lx} % {y,ly}
// q must have length >= lx-ly+1
// r must have length >= ly
// must have lx >= ly
// behavior undefined if {y,ly} is 0
void u64arr_ll_div(const uint64_t *__restrict__ x, size_t lx,
                   const uint64_t *__restrict__ y, size_t ly,
                   uint64_t *__restrict__ q,
                   uint64_t *__restrict__ r);
