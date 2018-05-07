#include "primitives.hpp"
#include <stdint.h>

#include <vector>
#include <stdint.h>
#include <thread>
#include <string>

#include "outputs.hpp"
#include "spec.hpp"
#include "pool.hpp"
#include "build.hpp"
#include "utils.hpp"
#include "dict.hpp"

#include <memory>
#include <iostream>
#include <type_traits>
#include <random>

#define R __restrict__

template<typename T>
NO_INLINE void
tgen_uni(T* R res, size_t num, int64_t seed, int64_t min, int64_t max)
{
	std::mt19937 rng(seed);
	std::uniform_int_distribution<T> uni(min,max);

	for (size_t i=0; i<num; i++) {
		res[i] = uni(rng);
	}
}

NO_INLINE void
gen_uni(int64_t* R res, size_t num, int64_t seed, int64_t min, int64_t max, BaseType t)
{
	switch (t) {
#define A(C, B) case B: tgen_uni<C>((C*)res, num, seed, min, max); break;
	A(int8_t, I8);
	A(uint8_t, U8);

	A(int16_t, I16);
	A(uint16_t, U16);

	A(int32_t, I32);
	A(uint32_t, U32);

	A(int64_t, I64);

#undef A
	default:
		assert(false);
	}
}

template<typename T>
NO_INLINE void
tgen_poisson(T* R res, size_t num, int64_t seed, int64_t min,
	int64_t max, double mean)
{
	std::mt19937 rng(seed);
	std::poisson_distribution<T> uni(mean);
	int64_t dom = max - min;

	for (size_t i=0; i<num; i++) {
		auto k = uni(rng);
		res[i] = (k % dom) + min;
	}
}

NO_INLINE void
gen_poisson(int64_t* R res, size_t num, int64_t seed, int64_t min, int64_t max,
	double mean, BaseType t)
{
	switch (t) {
#define A(C, B) case B: tgen_poisson<C>((C*)res, num, seed, min, max, mean); break;
	A(int8_t, I8);
	A(uint8_t, U8);

	A(int16_t, I16);
	A(uint16_t, U16);

	A(int32_t, I32);
	A(uint32_t, U32);

	A(int64_t, I64);

#undef A
	default:
		assert(false);
	}
}

template<typename T>
NO_INLINE void
tgen_seq(T* R res, size_t num, int64_t start, int64_t min, int64_t max)
{
	assert(max >= min);
	const T dom = (int64_t)max - (int64_t)min;
	const T dmin = min;

	for (size_t i=0; i<num; i++) {
		const int64_t k = start + i;
		res[i] = (k % dom) + dmin;
	}
}


NO_INLINE void
gen_seq(int64_t* R res, size_t num, int64_t start, int64_t min,
	int64_t max, BaseType t)
{
	switch (t) {
#define A(C, B) case B: tgen_seq<C>((C*)res, num, start, min, max); break;
	A(int8_t, I8);
	A(uint8_t, U8);

	A(int16_t, I16);
	A(uint16_t, U16);

	A(int32_t, I32);
	A(uint32_t, U32);

	A(int64_t, I64);

#undef A
	default:
		assert(false);
	}

}

NO_INLINE void
fix_ptrs(char** R res, size_t num, size_t max_chars, char* R base)
{
	for (size_t i=0; i<num; i++) {
		res[i] = base + i*max_chars;
	}
}

template<typename T>
NO_INLINE size_t
sel_gt0(int* R out, size_t num, T* R pred, int* R sel)
{
	size_t res = 0;
	VectorExec(sel, num, [&] (auto m) {
		out[res] = m;
		res += pred[m] > 0;
	});
	return res;
}


template<typename T>
NO_INLINE size_t
sel_gt(int* R out, size_t num, T* R pred, T val, int* R sel)
{
	size_t res = 0;
	VectorExec(sel, num, [&] (auto m) {
		out[res] = m;
		res += pred[m] > val;
	});
	return res;
}

template<typename T>
NO_INLINE size_t
sel_not0(int* R out, size_t num, T* R pred, int* R sel)
{
	size_t res = 0;
	VectorExec(sel, num, [&] (auto m) {
		out[res] = m;
		res += !!pred[m];
	});
	return res;
}

template<typename T>
NO_INLINE size_t
sel_0(int* R out, size_t num, T* R pred, int* R sel)
{
	size_t res = 0;
	VectorExec(sel, num, [&] (auto m) {
		out[res] = m;
		res += !pred[m];
	});
	return res;
}


// inspired by http://coliru.stacked-crooked.com/a/16f8a901a31b9d73
static constexpr uint64_t powers[]= {
	uint64_t(1u),
	uint64_t(10u),
	uint64_t(100u),
	uint64_t(1000u),
	uint64_t(10000u),
	uint64_t(100000u),
	uint64_t(1000000u),
	uint64_t(10000000u),
	uint64_t(100000000u),
	uint64_t(1000000000u),
	uint64_t(10000000000u),
	uint64_t(100000000000u),
	uint64_t(1000000000000u),
	uint64_t(10000000000000u),
	uint64_t(100000000000000u),
	uint64_t(1000000000000000u),
	uint64_t(10000000000000000u),
	uint64_t(100000000000000000u),
	uint64_t(1000000000000000000u),
	uint64_t(10000000000000000000u),
};

// inspired by http://coliru.stacked-crooked.com/a/16f8a901a31b9d73
static constexpr unsigned guess[65]= {
	0 ,0 ,0 ,0 , 1 ,1 ,1 , 2 ,2 ,2 ,
	3 ,3 ,3 ,3 , 4 ,4 ,4 , 5 ,5 ,5 ,
	6 ,6 ,6 ,6 , 7 ,7 ,7 , 8 ,8 ,8 ,
	9 ,9 ,9 ,9 , 10,10,10, 11,11,11,
	12,12,12,12, 13,13,13, 14,14,14,
	15,15,15,15, 16,16,16, 17,17,17,
	18,18,18,18, 19
};

template<typename T>
NO_INLINE void
trounddown_log10(T* R res, T* R x, size_t num, int* R sel)
{

	VectorExec(sel, num, [&] (auto i) {
#if 0
#if 0
		res[i] = 1;
		while ((res[i]*10) <= x[i]) {
			res[i] *= 10;
		}
#else
		const auto val = x[i];
		static constexpr size_t powers_size = sizeof(powers) / sizeof(powers[0]);
		for (size_t k=0; k<powers_size; k++) {
			if (val >= powers[k]) {
				res[i] = powers[k];
			}
		}
#endif
#else
	auto digits=guess[64 - __builtin_clzll(x[i])];
	// std::cerr << "x=" << x[i] << " leading=" << __builtin_clzll(x[i]) << " guess[64-lead]=" << digits << " powers[guess]=" << powers[digits]<< std::endl;
	if (((uint64_t)x[i] < powers[digits]) & (digits > 0)) {
		digits--;
	}

	res[i] = powers[digits];
#endif
	});
}

NO_INLINE void
rounddown_log10(uint64_t* R res, uint64_t* R x, size_t num, int* R sel)
{
	trounddown_log10<uint64_t>(res, x, num, sel);
}


static constexpr char kDigits2[200] = {
    '0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7','0','8','0','9',
    '1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7','1','8','1','9',
    '2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7','2','8','2','9',
    '3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7','3','8','3','9',
    '4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7','4','8','4','9',
    '5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7','5','8','5','9',
    '6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7','6','8','6','9',
    '7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7','7','8','7','9',
    '8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7','8','8','8','9',
    '9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7','9','8','9','9'
};

template<typename T>
NO_INLINE void
str_int_round2(char** R s, size_t* R len, T* R a, T* R div, size_t num,
	int* R sel)
{
	VectorExec(sel, num, [&] (auto m) {
		const T dv = (T)div[m] / 10;
		const T av = (T)a[m];
		const T ndiv = 2*(av / dv);
		const T nrem = av % dv;

		char* R dst = s[m] + len[m];
		*dst = kDigits2[ndiv];
		*(dst+1) = kDigits2[ndiv+1];

		len[m]+=2;

		a[m] = nrem;
		div[m] = dv / 10;
	});
}

template<typename T>
NO_INLINE void
str_int_round1(char** R s, size_t* R len, T* R a, T* R div,
	size_t num, int* R sel)
{
	VectorExec(sel, num, [&] (auto m) {
		const T dv = (T)div[m];
		const T av = (T)a[m];
		const T ndiv = av / dv;
		const T nrem = av % dv;

		char* R dst = s[m] + len[m];
		*dst = '0' + ndiv;
		len[m]++;

		a[m] = nrem;
		div[m] = dv / 10;
	});
}

template<typename T>
NO_INLINE void
str_int_handle0(char** R s, size_t* R len, size_t num, int* R sel)
{
	VectorExec(sel, num, [&] (auto i) {
		char* d = s[i];
		*d = '0';
		d++;
		*d = '\0';
		len[i] = 1;
	});
}

template<typename T>
NO_INLINE void
str_int_init_and_minus(char** R s, T* R a, size_t* R len, size_t num, int* R sel)
{
	VectorExec(sel, num, [&] (auto i) {
		len[i] = 0;
		if (a[i] < 0) {
			*s[i] = '-';
			len[i]++;
			a[i] = -a[i];
		}
	});
}


template<typename T>
NO_INLINE void
str_int_terminate(char** R s, size_t* R len, size_t num, int* R sel)
{
	VectorExec(sel, num, [&] (auto i) {
		char* dst = s[i] + len[i];
		*dst = '\0';
	});
}

#include "gen_tables.hpp"

template<typename T>
NO_INLINE bool
tstr_direct_lookup(char** R s, size_t* R len, const T* R a, size_t num)
{
#define LUT(C) \
	if (std::is_same<T, C>::value) { \
		VectorExec(nullptr, num, [&] (size_t i) { \
			assert((int32_t)a[i] >= (int32_t)std::numeric_limits<C>::min()); \
			int32_t v = (int32_t)a[i] - (int32_t)std::numeric_limits<C>::min(); \
			assert(v >= 0); \
			assert(v <= C##_int2str_lut_len); \
			s[i] = (char*)C##_int2str_lut[v].s; \
			len[i] = C##_int2str_lut[v].l; \
		}); \
		return true; \
	}

	LUT(int8_t)
	LUT(uint8_t)
	LUT(int16_t)
	LUT(uint16_t)

#undef LUT
	return false;
}

template<typename T>
NO_INLINE void
tstr_int(char** R s, size_t* R len, T* R a, size_t num, T* R log10,
	bool* R tmp_pred, int* R tmp_sel, int* R tmp_sel2)
{
	if (tstr_direct_lookup<T>(s, len, a, num)) {
		return;
	}

	// handle 0 and forget about them
	{
		size_t curr = sel_0<T>(tmp_sel, num, a, nullptr);
		str_int_handle0<T>(s, len, curr, tmp_sel);
	}

	// init and handle 0 and minus
	{
		num = sel_not0<T>(tmp_sel, num, a, nullptr);
		str_int_init_and_minus<T>(s, a, len, num, tmp_sel);
	}

	trounddown_log10<T>(log10, a, num, tmp_sel);

	// divide and modulo
	{
		size_t curr = num;
		curr = sel_gt<T>(tmp_sel2, curr, log10, 99, tmp_sel);

		while (curr > 0) {
			str_int_round2<T>(s, len, a, log10, curr, tmp_sel2);

			curr = sel_gt<T>(tmp_sel2, curr, log10, 99, tmp_sel2);
		}

		// last digits
		curr = sel_gt0<T>(tmp_sel, num, log10, tmp_sel);
		while (curr > 0) {
			str_int_round1<T>(s, len, a, log10, curr, tmp_sel);
			curr = sel_gt0<T>(tmp_sel, num, log10, tmp_sel);
		}
	}

	// terminate with \0
	str_int_terminate<T>(s, len, num, nullptr);
}

void
str_int(char** R s, size_t* R len, int64_t* R a, size_t num, 
	int64_t* R log10, bool* R tmp_pred, int* R tmp_sel,
	int* R tmp_sel2, BaseType t)
{
	switch (t) {
#define A(C, B) case B: tstr_int<C>(s, len, (C*)a, num, (C*)log10, \
							tmp_pred, tmp_sel, tmp_sel2); \
						break;
		A(int8_t, I8);
		A(uint8_t, U8);

		A(int16_t, I16);
		A(uint16_t, U16);

		A(int32_t, I32);
		A(uint32_t, U32);

		A(int64_t, I64);
#undef A
		default:
			assert(false);
	}
}

#define KERNEL(LEN) \
	for (size_t i=0; i<num; i++) { \
		char* d = &dest[pos[i]]; \
		const char* s = strs[i]; \
		size_t k, m; \
		for (k=0; k<lens[i]; k++) { \
			d[k] = s[k]; \
		} \
		for (m=0; m<LEN; m++) { \
			d[k+m] = sep[m]; \
		} \
	}

template<size_t sep_len>
NO_INLINE void
tscatter_out(char* R dest, size_t* R pos, char** R strs, size_t* R lens,
	const char* R sep, size_t num)
{
	KERNEL(sep_len);
}

NO_INLINE void
gscatter_out(char* R dest, size_t* R pos, char** R strs, size_t* R lens,
	const char* R sep, size_t sep_len, size_t num)
{
	KERNEL(sep_len);
}

NO_INLINE void
scatter_out(char* R dest, size_t* R pos, char** R strs, size_t* R lens,
	const char* R sep, size_t sep_len, size_t num)
{
	switch(sep_len) {
#define A(i) case i: 	tscatter_out<i>(dest, pos, strs, lens, sep, num); break;
	// generated from haskell: foldl (++) "" $ map (\x -> "A(" ++ show x  ++ ");") [0..128]
	A(0);A(1);A(2);A(3);A(4);A(5);A(6);A(7);A(8);A(9);A(10);A(11);A(12);A(13);A(14);A(15);A(16);A(17);A(18);A(19);A(20);A(21);A(22);A(23);A(24);A(25);A(26);A(27);A(28);A(29);A(30);A(31);A(32);A(33);A(34);A(35);A(36);A(37);A(38);A(39);A(40);A(41);A(42);A(43);A(44);A(45);A(46);A(47);A(48);A(49);A(50);A(51);A(52);A(53);A(54);A(55);A(56);A(57);A(58);A(59);A(60);A(61);A(62);A(63);A(64);A(65);A(66);A(67);A(68);A(69);A(70);A(71);A(72);A(73);A(74);A(75);A(76);A(77);A(78);A(79);A(80);A(81);A(82);A(83);A(84);A(85);A(86);A(87);A(88);A(89);A(90);A(91);A(92);A(93);A(94);A(95);A(96);A(97);A(98);A(99);A(100);A(101);A(102);A(103);A(104);A(105);A(106);A(107);A(108);A(109);A(110);A(111);A(112);A(113);A(114);A(115);A(116);A(117);A(118);A(119);A(120);A(121);A(122);A(123);A(124);A(125);A(126);A(127);A(128);
#undef A
	default: 	gscatter_out(dest, pos, strs, lens, sep, sep_len, num); break;
	}
}

#undef KERNEL
