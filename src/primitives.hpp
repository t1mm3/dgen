#ifndef H_GEN_PRIMITIVES
#define H_GEN_PRIMITIVES

#include <stdint.h>
#include "utils.hpp"

void
gen_uni(int64_t* res, size_t num, int64_t seed, int64_t min,
	int64_t max, BaseType t);

void
gen_poisson(int64_t* res, size_t num, int64_t seed, int64_t min,
	int64_t max, double mean, BaseType t);

void
gen_binomial(int64_t* res, size_t num, int64_t seed, int64_t min,
	int64_t max, int64_t p_t, double p, BaseType t);

void
gen_neg_binomial(int64_t* res, size_t num, int64_t seed, int64_t min,
	int64_t max, int64_t k, double p, BaseType t);

void
gen_geometric(int64_t* res, size_t num, int64_t seed, int64_t min,
	int64_t max, double p, BaseType t);

void
gen_zipf(int64_t* res, size_t num, int64_t seed, int64_t min,
	int64_t max, double alpha, BaseType t);

void
gen_seq(int64_t* res, size_t num, int64_t start, int64_t min,
	int64_t max, BaseType t);

void
fix_ptrs(char** res, size_t num, size_t max_chars, char* base);

void
rounddown_log10(uint64_t* res, uint64_t* x, size_t num, int* sel);

void
str_int(char** s, size_t* len, int64_t* a, size_t num, int64_t* log10,
	bool* tmp_pred, int* tmp_sel, int* tmp_sel2, BaseType t);

void
scatter_out(char* dest, size_t* pos, char** strs, size_t* lens,
	const char* sep, size_t sep_len, size_t num);

#endif