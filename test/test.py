# make tests using python3 big integer capability

from functools import reduce

UMAX = 0xFFFFFFFFFFFFFFFF
M61 = 0x1FFFFFFFFFFFFFFF

masks_for_add = [UMAX,UMAX>>4]
masks_for_mul = [UMAX>>s for s in range(0,64,8)]

def BUI_to_arr(n:int) -> list[int]:
    ret = []
    while n:
        ret.append(n&UMAX)
        n >>= 64
    if len(ret) == 0:
        ret.append(0)
    return ret

def arr_to_BUI(a:list[int]) -> int:
    ret = 0
    for w in a[::-1]:
        ret <<= 64
        ret |= w
    return ret

def BUI_gen_seq(lo:int,hi:int,step:int) -> int:
    ret = []
    while lo < hi:
        ret.append(lo&UMAX)
        lo += step
    return arr_to_BUI(ret)

def BUI_gen_lcg(seed:int,len_:int,bit_masks:list[int]=[UMAX],mult:int=0x5DEECE66D,add:int=0xB) -> int:
    ret = []
    for _ in range(len_):
        seed = (seed*mult+add)&UMAX
        mask = bit_masks[(seed>>32)%len(bit_masks)]
        seed = (seed*mult+add)%UMAX
        tmp = seed>>32
        seed = (seed*mult+add)%UMAX
        tmp |= seed&0xFFFFFFFF00000000
        ret.append(tmp&mask)
    return arr_to_BUI(ret)

def BUI_hash(n:int) -> tuple[int,int,int]:
    r = BUI_to_arr(n)
    return sum(r)&UMAX,reduce(lambda x,y:x^y,r),n%M61

a = BUI_gen_lcg(5,100,masks_for_mul)
b = BUI_gen_lcg(11,100,masks_for_mul)
print(len(str(a)),len(str(b)),len(str(a*b)))
print(BUI_hash(a*b))
