#ifndef H_GEN_SPEC
#define H_GEN_SPEC

#include <vector>
#include <stdint.h>

enum ColType {
	String,
	Integer,
};

enum ColGenType {
	Random,
	Sequential
};

struct ColSpec {
	ColType ctype;
	ColGenType cgen;
	int64_t min;
	int64_t max;
};

struct RelSpec {
	std::vector<ColSpec> cols;
	size_t card;
	size_t threads;
};

#endif