#include "generator.hpp"

#include <vector>
#include <stdint.h>
#include <thread>
#include <string>

#include "outputs.hpp"
#include "spec.hpp"
#include "pool.hpp"

#include <iostream>

size_t g_chunk_size = 10000;
constexpr size_t g_vector_size = 1024;

const char* kSep = "|";
const size_t kSepLen = 1;
const char* kNewlineSep = "|\n";
const size_t kNewlineSepLen = 2;

template<bool endl>
size_t GetSepLen()
{
	if (endl) {
		return kNewlineSepLen;
	}
	return kSepLen;
}


template<bool endl>
const char* GetSep()
{
	if (endl) {
		return kNewlineSep;
	}
	return kSep;
}


#include <random>

#define R __restrict__

#ifdef __GNUC__
#define NO_INLINE __attribute__ ((noinline))
#else
#define NO_INLINE
#endif

NO_INLINE void gen_rand(int64_t* R res, size_t num, int64_t seed, int64_t min, int64_t max) {
	std::mt19937 rng(seed);
	std::uniform_int_distribution<int64_t> uni(min,max);

	for (size_t i=0; i<num; i++) {
		res[i] = uni(rng);
	}
}


NO_INLINE void gen_seq(int64_t* R res, size_t num, int64_t start, int64_t min, int64_t max) {
	assert(max >= min);
	int64_t dom = max - min;

	for (size_t i=0; i<num; i++) {
		int64_t k = start + i;
		res[i] = (k % dom) + min;
	}
}

NO_INLINE void fix_ptrs(char** R res, size_t num, size_t max_chars, char* R base) {
	for (size_t i=0; i<num; i++) {
		res[i] = base + i*max_chars;
	}
}

NO_INLINE size_t sel_true(int* R out, size_t num, bool* pred, int* R sel) {
	size_t res = 0;
#define KERNEL(m) \
	out[res] = m; \
	res += !!pred[m];

	if (sel) {
		for (size_t i=0; i<num; i++) {
			KERNEL(sel[i]);
		}	
	} else {
		for (size_t i=0; i<num; i++) {
			KERNEL(i);
		}
	}	

	return res;
#undef KERNEL
}

NO_INLINE void str_int_round(char** R s, size_t* R len, int64_t* R a, size_t num, size_t max_chars, bool* R tmp_pred, int* R sel) {
#define KERNEL(m) \
	char* dst = s[m] + len[m]; \
	*dst = '0' + (a[m] % 10); \
	len[m]++; \
	a[m] /= 10; \
	tmp_pred[m] = a[m] != 0;

	size_t i;
	if (sel) {
		for (i=0; i+8<num; i+=8) {
			for (int k=0; k<8; k++) {
				KERNEL(sel[i+k]);
			}
		}

		while (i < num) {
			KERNEL(sel[i]);
			i++;
		}
	} else {
		for (i=0; i<num; i++) {
			KERNEL(i);
		}
	}	

#undef KERNEL
}

NO_INLINE void str_int(char** R s, size_t* R len, int64_t* R a, size_t num, size_t max_chars, bool* R tmp_pred, int* R tmp_sel) {
	// handle minus
	for (size_t i=0; i<num; i++) {
		len[i] = 0;
		if (a[i] < 0) {
			*s[i] = '-';
			a[i] = -a[i];
			len[i]++;
		}
	}

	int* sel = nullptr;
	size_t curr = num;

	while (curr > 0) {
		str_int_round(s, len, a, curr, max_chars, tmp_pred, sel);

		curr = sel_true(tmp_sel, curr, tmp_pred, sel);
	}
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

	size_t pos[g_vector_size];

	int64_t* res;
};

NO_INLINE size_t calc_positions(size_t pos, size_t num, size_t num_cols, VData* R cols) {
	for (size_t i=0; i<num; i++) {
		for (size_t c = 0; c < num_cols; c++) {
			bool last_col = c == num_cols-1;

			cols[c].pos[i] = pos;

			pos += cols[c].len[i];
			
			if (last_col) {
				pos += GetSepLen<true>();
			} else {
				pos += GetSepLen<false>();
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


NO_INLINE void DoTask::append_vector(std::string& out, size_t start, size_t num, const RelSpec& rel)
{
	auto get_max_chars = [] (const auto& col) -> size_t {
		switch (col.ctype) {
		case Integer:
			return std::max(num_chars_int(col.max), num_chars_int(col.min));
		case String:
			return 512;
		default:
			assert(false);
			return 0;
		}
	};

	size_t num_cols = rel.cols.size();
	if (state.cols.size() < num_cols) {
		state.cols.resize(num_cols);
	}

	assert(num_cols > 0);

	for (size_t c = 0; c < num_cols; c++) {
		auto& col = rel.cols[c];
		auto& scol = state.cols[c];
		int64_t* a = &scol.a[0];
		char** s = &scol.s[0];

		switch (col.cgen) {
		case Random:
			gen_rand(a, num, start, col.min, col.max);
			break;

		case Sequential:
			gen_seq(a, num, start, col.min, col.max);
			break;

		default:
			assert(false);
			break;
		}

		scol.res = a;

		switch (col.ctype) {
		case String:
			assert(col.min >= 0);
			assert(false && "not impl'd");

			throw std::bad_alloc();

			// lookup in dict
			// calc strlen
			break;

		case Integer:
			auto& buf = scol.buf;
			size_t max_chars = get_max_chars(col);

			// null terminator
			max_chars++;

			// allocate
			if (buf.size() < num*max_chars) {
				buf.resize(num*max_chars);
			}

			fix_ptrs(s, num, max_chars, &buf[0]);
			str_int(s, &scol.len[0], a, num, max_chars, &scol.tmp_pred[0], &scol.tmp_sel[0]);
			break;
		}
	}

	// calculate positions
	size_t size = calc_positions(0, num, num_cols, &state.cols[0]);
	out.resize(size+1);

	char* dst = (char*)out.c_str();

	// copy strings to final location
	for (size_t c = 0; c < num_cols; c++) {
		auto& scol = state.cols[c];
		const bool last_col = c == num_cols-1;

		const char* sep = last_col ? GetSep<true>() : GetSep<false>();
		const size_t sep_len = last_col ? GetSepLen<true>() : GetSepLen<false>();

		scatter_out(dst, &scol.pos[0], &scol.s[0], &scol.len[0], sep, sep_len, num);
	}

	dst[size] = '\0';
}

std::mutex lock;

NO_INLINE void DoTask::operator()(Task&& t) {
	// generate [start, end) from relation
	assert(t.rel);
	assert(t.end <= t.rel->card);

	size_t off = t.start;

	std::string final;
	final.reserve(1024*1024);

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
	ThreadPool<Task, DoTask> g_pool(spec.threads);

	size_t num = spec.card;
	// split sequential rang into chunks
	size_t todo = num;
	size_t offset = 0;

	while (todo > 0) {
		const size_t num = std::min(g_chunk_size, todo);

		g_pool.Push(Task {offset, offset + num, &spec, &out});
		todo -= num;
		offset += num;
	};
}
