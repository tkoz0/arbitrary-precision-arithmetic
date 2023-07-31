
import numpy
import sys

UMAX = 0xFFFFFFFFFFFFFFFF

_digits1 = '0123456789abcdefghijklmnopqrstuvwxyz'
_digits2 = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'

def int_to_str(n: int, b: int = 10):
    assert n >= 0
    assert 2 <= b <= 36
    if n == 0:
        return '0'
    ret = ''
    while n:
        n,d = divmod(n,b)
        ret += _digits2[d]
    return ret[::-1]

n = int(sys.argv[1])
b = int(sys.argv[2])
s1 = int_to_str(n,b)
s2 = numpy.base_repr(n,b)
assert s1 == s2, f'{s1} != {s2}'
print(s1)
arr = []
while n:
    arr.append(n & UMAX)
    n >>= 64
print(','.join(map(str,arr)))
