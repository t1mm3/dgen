#ifndef H_GEN_UTILS
#define H_GEN_UTILS

#include <stdint.h>
#include <string>
#include <limits>
#include <cassert>

void vec_log2_64(int* res, uint64_t* x, size_t num);
void vec_log2_32(int* res, uint32_t* x, size_t num);
void vec_log10_64(int* res, uint64_t* x, size_t num, int* sel, bool no0);


enum BaseType {
	I8,
	U8,
	I16,
	U16,
	I32,
	U32,
	I64	
};

inline static BaseType
GetFittingType(int64_t min, int64_t max)
{
#define A(C, B) \
		if ((int64_t)std::numeric_limits<C>::max() >= max && \
			(int64_t)std::numeric_limits<C>::min() <= min) { \
				return B; \
		}

	A(int8_t, I8);
	A(uint8_t, U8);

	A(int16_t, I16);
	A(uint16_t, U16);

	A(int32_t, I32);
	A(uint32_t, U32);

	A(int64_t, I64);

#undef A

	assert(false);
	return I64;
}

template<typename F>
void VectorExec(int* sel, size_t num, F&& fun, bool strict = true)
{
	if (sel) {
		size_t k;
		for (k=0; k+8<num; k+=8) {
			for (int m=0; m<8; m++) {
				fun(sel[k+m]);
			}
		}

		while (k < num) {
			fun(sel[k]);
			k++;
		}
	} else {
		for (size_t i=0; i<num; i++) {
			fun(i);
		}
	}
}

// https://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
const int tab64[64] = {
    63,  0, 58,  1, 59, 47, 53,  2,
    60, 39, 48, 27, 54, 33, 42,  3,
    61, 51, 37, 40, 49, 18, 28, 20,
    55, 30, 34, 11, 43, 14, 22,  4,
    62, 57, 46, 52, 38, 26, 32, 41,
    50, 36, 17, 19, 29, 10, 13, 21,
    56, 45, 25, 31, 35, 16,  9, 12,
    44, 24, 15,  8, 23,  7,  6,  5};

// https://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
inline static int log2_64(uint64_t value)
{
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return tab64[((uint64_t)((value - (value >> 1))*0x07EDD5E59A4E28C2)) >> 58];
}

// https://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
const int tab32[32] = {
     0,  9,  1, 10, 13, 21,  2, 29,
    11, 14, 16, 18, 22, 25,  3, 30,
     8, 12, 20, 28, 15, 17, 24,  7,
    19, 27, 23,  6, 26,  5,  4, 31};

// https://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
inline static int log2_32 (uint32_t value)
{
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return tab32[(uint32_t)(value*0x07C4ACDD) >> 27];
}

#include <vector>

struct ZipfHelper {
	std::vector<double> sum_probs;
	double c;

	ZipfHelper(int64_t domain, double alpha);
};


#endif