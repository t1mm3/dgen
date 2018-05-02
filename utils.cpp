#include "utils.hpp"
#include <iostream>

void vec_log2_64(int* __restrict__ res, uint64_t* __restrict__ x, size_t num)
{
	for (size_t i=0; i<num; i++) {
		res[i] = log2_64(x[i]);
	}	
}

void vec_log2_32(int* __restrict__ res, uint32_t* __restrict__ x, size_t num)
{
	for (size_t i=0; i<num; i++) {
		res[i] = log2_32(x[i]);
	}	
}

// http://coliru.stacked-crooked.com/a/16f8a901a31b9d73
template<typename UInt>
unsigned baseTwoDigits(UInt x)
{
    if(sizeof x>4)
        return x ? 8*sizeof x-__builtin_clzll(x) : 0;
    else
        return x ? 8*sizeof x-__builtin_clz  (x) : 0;
}

// http://coliru.stacked-crooked.com/a/16f8a901a31b9d73
template<typename UInt, bool No0 = false>
unsigned baseTenDigits(UInt x)
{
	if (!No0) {
		if(x==0) return 0;
	}
    static constexpr UInt tenToThe[]=
    {
        UInt(1u),
        UInt(10u),
        UInt(100u),
        UInt(1000u),
        UInt(10000u),
        UInt(100000u),
        UInt(1000000u),
        UInt(10000000u),
        UInt(100000000u),
        UInt(1000000000u),
        UInt(10000000000u),
        UInt(100000000000u),
        UInt(1000000000000u),
        UInt(10000000000000u),
        UInt(100000000000000u),
        UInt(1000000000000000u),
        UInt(10000000000000000u),
        UInt(100000000000000000u),
        UInt(1000000000000000000u),
        UInt(10000000000000000000u),
    };
    static constexpr unsigned guess[65]=
    {
        0 ,0 ,0 ,0 , 1 ,1 ,1 , 2 ,2 ,2 ,
        3 ,3 ,3 ,3 , 4 ,4 ,4 , 5 ,5 ,5 ,
        6 ,6 ,6 ,6 , 7 ,7 ,7 , 8 ,8 ,8 ,
        9 ,9 ,9 ,9 , 10,10,10, 11,11,11,
        12,12,12,12, 13,13,13, 14,14,14,
        15,15,15,15, 16,16,16, 17,17,17,
        18,18,18,18, 19
    };
    const auto digits=guess[baseTwoDigits(x)];
    return digits + (x>=tenToThe[digits]);
}

void vec_log10_64(int* __restrict__ res, uint64_t* __restrict__ x, size_t num, int* __restrict__ sel, bool no0)
{
	if (no0) {
		VectorExec(sel, num, [&] (auto i) {
			res[i] = baseTenDigits<uint64_t, true>(x[i]);
		});
	} else {
		VectorExec(sel, num, [&] (auto i) {
			res[i] = baseTenDigits<uint64_t, false>(x[i]);
		});
	}	
}