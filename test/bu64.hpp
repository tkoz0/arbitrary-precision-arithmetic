// low level test implementation of large unsigned integer type
// representation using 64 bit limbs (uint64_t)
// function prefix bu64_ (big unsigned integer with 64 bit limbs)

#pragma once

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

// multiply by m in place, returns the last carry amount
// (nonzero last carry means the number gets longer)
uint64_t bu64_mul64(uint64_t *input, size_t ilen, uint64_t m);

// add a in place, return true if carry propagates past end
// (carry propagation past end means number gets longer)
bool bu64_add64(uint64_t *input, size_t ilen, uint64_t a);

// divide by d in place, return remainder
uint32_t bu64_div32(uint64_t *input, size_t ilen, uint32_t d);

// divide by d in place, return remainder
uint64_t bu64_div64(uint64_t *input, size_t ilen, uint64_t d);

// subtract a in place, return false if a is bigger that the input
// (false return value indicates underflow)
bool bu64_sub64(uint64_t *input, size_t ilen, uint64_t a);

// add 2 big unsigned integers (extended to max(len1,len2) limbs)
// returns carry bit, output must have max(len1,len2) length
// result is written to new memory location
bool bu64_add(const uint64_t *__restrict__ arr1, size_t len1,
              const uint64_t *__restrict__ arr2, size_t len2,
              uint64_t *__restrict__ output);

// subtract 2 big unsigned integers, returns true if result stays nonnegative
// subtractiton is done as if numbers are extended to max(len1,len2) limbs
// output must have max(len1,len2) length
// return result means (input1 >= input2)
// result is written to new memory location
bool bu64_sub(const uint64_t *__restrict__ arr1, size_t len1,
              const uint64_t *__restrict__ arr2, size_t len2,
              uint64_t *__restrict__ output);

// multiply 2 big unsigned integers, output must have len1+len2 length
// result is written to new memory location
void bu64_mul(const uint64_t *__restrict__ arr1, size_t len1,
              const uint64_t *__restrict__ arr2, size_t len2,
              uint64_t *__restrict__ output);

// convert number to string (bases 2-36 supported, uses lowercase letters)
// output must be long enough to fit result with null character
// input is modified to have division done in place
size_t bu64_write_str(uint8_t base, uint64_t *__restrict__ input, size_t ilen, char *__restrict__ output);

// convert string to number, output must be long enough to fit result
// string must be null terminated
// characters must be 0-9 and a-z, any others lead to undefined behavior
// input is not modified
size_t bu64_read_str(uint8_t base, const char *__restrict__ input, uint64_t *__restrict__ output);

