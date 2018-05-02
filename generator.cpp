#include "generator.hpp"

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

size_t g_chunk_size = 16*1024;
constexpr size_t g_vector_size = 1024;

#include <random>

#define R __restrict__

template<typename T>
NO_INLINE void tgen_uni(T* R res, size_t num, int64_t seed, int64_t min, int64_t max) {
	std::mt19937 rng(seed);
	std::uniform_int_distribution<T> uni(min,max);

	for (size_t i=0; i<num; i++) {
		res[i] = uni(rng);
	}
}

NO_INLINE void gen_uni(int64_t* R res, size_t num, int64_t seed, int64_t min, int64_t max, BaseType t) {
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
NO_INLINE void tgen_poisson(T* R res, size_t num, int64_t seed, int64_t min, int64_t max, double mean) {
	std::mt19937 rng(seed);
	std::poisson_distribution<T> uni(mean);
	int64_t dom = max - min;

	for (size_t i=0; i<num; i++) {
		auto k = uni(rng);
		res[i] = (k % dom) + min;
	}
}

NO_INLINE void gen_poisson(int64_t* R res, size_t num, int64_t seed, int64_t min, int64_t max, double mean, BaseType t) {
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
tgen_seq(T* R res, size_t num, int64_t start, int64_t min, int64_t max) {
	assert(max >= min);
	T dom = max - min;

	for (size_t i=0; i<num; i++) {
		T k = start + i;
		res[i] = (k % dom) + min;
	}
}


NO_INLINE void gen_seq(int64_t* R res, size_t num, int64_t start, int64_t min, int64_t max, BaseType t) {
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

template<typename T>
NO_INLINE void str_int_round(char** R s, size_t* R len, T* R a, T* R div, size_t num, int* R sel) {
	VectorExec(sel, num, [&] (auto m) {
		const T dv = (T)div[m];
		T av = (T)a[m];

		char* R dst = s[m] + len[m];
		*dst = '0' + (av / dv);
		len[m]++;


		a[m] = static_cast<T>(av % dv);
		div[m] = static_cast<T>(dv / 10);
	});
}

template<typename T>
NO_INLINE void
tstr_int(char** R s, size_t* R len, T* R a, size_t num, T* R log10, bool* R tmp_pred, int* R tmp_sel, int* R tmp_sel2) {
	// handle 0 and forget about them
	{
		size_t curr = sel_0<T>(tmp_sel, num, a, nullptr);
		VectorExec(tmp_sel, curr, [&] (auto i) {
			char* d = s[i];
			*d = '0';
			d++;
			*d = '\0';
			len[i] = 1;
		});
	}

	// init and handle 0 and minus
	{
		num = sel_not0<T>(tmp_sel, num, a, nullptr);
		VectorExec(tmp_sel, num, [&] (auto i) {
			len[i] = 0;
			if (a[i] < 0) {
				*s[i] = '-';
				len[i]++;
				a[i] = -a[i];
			}
		});
	}

	trounddown_log10<T>(log10, a, num, tmp_sel);

	// divide and modulo
	{
		int* sel = tmp_sel;
		size_t curr = num;

		while (curr > 0) {
			str_int_round<T>(s, len, a, log10, curr, sel);

			int* newsel = tmp_sel;
			if (sel == tmp_sel) {
				newsel = tmp_sel2;
			}
			curr = sel_gt0<T>(newsel, curr, log10, sel);
			sel = newsel;			
		}
	}

	// terminate with \0
	for (size_t i=0; i<num; i++) {
		char* dst = s[i] + len[i];
		*dst = '\0';
	}
}

void
str_int(char** s, size_t* len, int64_t* a, size_t num, int64_t* R log10, bool* tmp_pred, int* tmp_sel, int* tmp_sel2)
{
	tstr_int<int64_t>(s, len, a, num, log10, tmp_pred, tmp_sel, tmp_sel2);
}

NO_INLINE void scatter_out(char* R dest, size_t* R pos, char** R strs, size_t* R lens, const char* R sep, size_t sep_len, size_t num) {
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

	switch(sep_len) {
	case 0: 	KERNEL(0); break;
	case 1: 	KERNEL(1); break;
	case 2: 	KERNEL(2); break;
	case 3: 	KERNEL(3); break;
	case 4: 	KERNEL(4); break;
	default: 	KERNEL(sep_len); break;
	}

#undef KERNEL
}

struct VData {
	int64_t a[g_vector_size];
	size_t len[g_vector_size];
	std::vector<char> buf;
	char* s[g_vector_size];

	bool tmp_pred[g_vector_size];
	int tmp_sel[g_vector_size];
	int tmp_sel2[g_vector_size];

	int64_t tmp_vals[g_vector_size];

	size_t pos[g_vector_size];

	BaseType res_type = I64;

	int64_t* res;
};

NO_INLINE size_t calc_positions(size_t pos, size_t num, size_t num_cols,
	VData* R cols, size_t len_sep, size_t len_nl)
{
	for (size_t i=0; i<num; i++) {
		for (size_t c = 0; c < num_cols; c++) {
			bool last_col = c == num_cols-1;

			cols[c].pos[i] = pos;

			pos += cols[c].len[i];
			
			if (last_col) {
				pos += len_nl;
			} else {
				pos += len_sep;
			}
		}
	}

	return pos;
}

struct WorkerState {
	std::vector<VData> cols;
};

thread_local WorkerState state;

constexpr size_t num_chars_int(int64_t x) {
	size_t r = 0;
	if (x < 0) {
		r++;
	}

	do {
		r++;
	} while (x /= 10);

	r++; // terminator

	return r;
}


struct Task {
	size_t start;
	size_t end;

	RelSpec* rel;
	Output* outp;
};

struct DoTask {
	void operator()(Task&& t);
	void append_vector(std::string& out, size_t start, size_t num, const RelSpec& rel);
};

static void
to_str(const ColSpec& col, size_t colid, size_t num)
{
	auto& scol = state.cols[colid];
	int64_t* a = &scol.res[0];
	char** s = &scol.s[0];

	col.ctype.match(
		[&] (Integer cint) {
			auto& buf = scol.buf;
			size_t max_chars = std::max(num_chars_int(cint.max), num_chars_int(cint.min));

			// null terminator
			max_chars++;

			// allocate
			if (buf.size() < num*max_chars) {
				buf.resize(num*max_chars);
			}

			fix_ptrs(s, num, max_chars, &buf[0]);

			switch (scol.res_type) {
#define A(C, B) case B: tstr_int<C>(s, &scol.len[0], (C*)a, num, \
							(C*)&scol.tmp_vals[0], &scol.tmp_pred[0], \
							&scol.tmp_sel[0], &scol.tmp_sel2[0]); \
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
		},
		[] (String cstr) {
			// already a string
		}
	);
}

static void
gen_col(const ColType& ctype, const ColSpec& col, size_t colid, size_t start,
	size_t num, bool top_level)
{
	auto& scol = state.cols[colid];
	int64_t* a = &scol.a[0];
	char** s = &scol.s[0];

	scol.res = nullptr;

	ctype.match(
		[&] (Integer cint) {
			scol.res_type = GetFittingType(cint.min, cint.max);

			cint.cgen.match(
				[&] (Sequential& gseq) {
					gen_seq(a, num, start, cint.min, cint.max, scol.res_type);
				},
				[&] (Uniform& guni) {
					gen_uni(a, num, start, cint.min, cint.max, scol.res_type);
				},
				[&] (Poisson& gpoisson) {
					gen_poisson(a, num, start, cint.min, cint.max, gpoisson.mean, scol.res_type);
				}
			);

			scol.res = a;
		},
		[&] (String cstr)  {
			gen_col(cstr.index, col, colid, start, num, false);

			assert(cstr.dict);
			assert(scol.res);

			cstr.dict->Lookup(s, &scol.len[0], (size_t*)scol.res, num, nullptr, scol.res_type);
		}
	);

	if (top_level) {
		to_str(col, colid, num);
	}
}


NO_INLINE void
DoTask::append_vector(std::string& out, size_t start, size_t num, const RelSpec& rel)
{
	size_t num_cols = rel.cols.size();
	if (state.cols.size() < num_cols) {
		state.cols.resize(num_cols);
	}

	assert(num_cols > 0);

	for (size_t c = 0; c < num_cols; c++) {
		gen_col(rel.cols[c].ctype, rel.cols[c], c, start, num, true);
	}

	// calculate positions
	size_t size = calc_positions(0, num, num_cols, &state.cols[0],
		rel.GetSepLen(false), rel.GetSepLen(true));
	out.resize(size);

	char* dst = (char*)out.c_str();

	// copy strings to final location
	for (size_t c = 0; c < num_cols; c++) {
		auto& scol = state.cols[c];
		const bool last_col = c == num_cols-1;

		const char* sep = rel.GetSep(last_col);
		const size_t sep_len = rel.GetSepLen(last_col);

		scatter_out(dst, &scol.pos[0], &scol.s[0], &scol.len[0], sep, sep_len, num);
	}
}

std::mutex lock;

NO_INLINE void DoTask::operator()(Task&& t) {
	// generate [start, end) from relation
	assert(t.rel);
	assert(t.end <= t.rel->card);

	size_t off = t.start;

	std::string final;
	final.reserve(1024*1024*10);

	while (off < t.end) {
		size_t num = std::min(g_vector_size, t.end - off);

		assert(t.rel);

		std::string tmp;
		append_vector(tmp, off, num, *t.rel);

		final += tmp;

		off += num;
	}

	(*t.outp)(final);
}

NO_INLINE void generate(RelSpec& spec, Output& out) {
	assert(spec.threads > 0);
	assert(g_chunk_size > 0);
	assert(g_vector_size > 0);

	std::vector<std::unique_ptr<Dictionary>> objs;

	// create dictionaries
	for (auto& col : spec.cols) {
		col.ctype.match(
			[&] (String& cstr) {
				if (!cstr.dict) {
					cstr.dict = new FileDictionary(cstr.fname);
				}
				objs.emplace_back(std::unique_ptr<Dictionary>(cstr.dict));
			},
			[] (Integer cint) {}
		);
	}

	auto num_threads = std::min(spec.threads, (spec.card / (g_chunk_size + g_chunk_size - 1)));
	if (num_threads < 0) {
		num_threads = 1;
	}

	ThreadPool<Task, DoTask> g_pool(num_threads);

	size_t num = spec.card;
	size_t todo = num;
	size_t offset = 0;

	while (todo > 0) {
		const size_t num = std::min(g_chunk_size, todo);

		g_pool.Push(Task {offset, offset + num, &spec, &out});
		todo -= num;
		offset += num;
	};
}
