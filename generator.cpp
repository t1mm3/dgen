#include "generator.hpp"

#include <vector>
#include <thread>
#include <string>

#include "primitives.hpp"
#include "outputs.hpp"
#include "spec.hpp"
#include "pool.hpp"
#include "build.hpp"
#include "utils.hpp"
#include "dict.hpp"

#include <memory>
#include <iostream>
#include <type_traits>

size_t g_chunk_size = 16*1024;
constexpr size_t g_vector_size = 1024;

#define R __restrict__
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

NO_INLINE size_t
calc_positions(size_t pos, size_t num, size_t num_cols,
	VData* R cols, size_t len_sep, size_t len_nl)
{
#define KERNEL(LAST_COL)  { \
			cols[c].pos[i] = pos; \
			pos += cols[c].len[i]; \
			if (LAST_COL) { \
				pos += len_nl; \
			} else { \
				pos += len_sep; \
			} \
		}


	if (num_cols == 1) {
		for (size_t i=0; i<num; i++) {
			size_t c = 0;
			KERNEL(true);
		}

		return pos;
	}

	for (size_t i=0; i<num; i++) {
		size_t c;
		for (c = 0; c < num_cols-1; c++) {
			KERNEL(false);
		}
		c = num_cols-1;
		KERNEL(true);
	}
	
#undef KERNEL
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
			if (cint.max > 4611686018427387905ll || cint.min < -4611686018427387905ll) {
				throw std::invalid_argument("Integer ranges too high");
			}

			size_t max_chars = std::max(num_chars_int(cint.max), num_chars_int(cint.min));

			// null terminator
			max_chars++;

			// allocate
			if (buf.size() < num*max_chars) {
				buf.resize(num*max_chars);
			}

			fix_ptrs(s, num, max_chars, &buf[0]);

			str_int(s, (size_t*)&scol.len[0], a, num, &scol.tmp_vals[0],
				&scol.tmp_pred[0], &scol.tmp_sel[0], &scol.tmp_sel2[0],
				scol.res_type);
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

NO_INLINE void
DoTask::operator()(Task&& t)
{
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

NO_INLINE void
generate(RelSpec& spec, Output& out)
{
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
