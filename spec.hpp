#ifndef H_GEN_SPEC
#define H_GEN_SPEC

#include <vector>
#include <stdint.h>
#include <mapbox/variant.hpp>

struct String {

};

struct Integer {

};

using ColType = variant<String, Integer>;

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

	const char* kSep = "|";
	size_t kSepLen = 1;
	const char* kNewlineSep = "|\n";
	size_t kNewlineSepLen = 2;

	size_t GetSepLen(bool endl) const
	{
		if (endl) {
			return kNewlineSepLen;
		}
		return kSepLen;
	}


	const char* GetSep(bool endl) const
	{
		if (endl) {
			return kNewlineSep;
		}
		return kSep;
	}

};

#endif