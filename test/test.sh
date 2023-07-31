#!/bin/bash
g++ -g -Wall -Werror -Wextra \
    -march=native ../u64arr/u64arr_ll.cpp u64arr_ll_test.cpp \
    && valgrind ./a.out
