#ifndef H_GEN_SPEC
#define H_GEN_SPEC

#include <vector>
#include <stdint.h>
#include <mapbox/variant.hpp>

struct Dictionary;

enum ColGenType {
	Random,
	Sequential
};

struct Integer {
	ColGenType cgen;
	int64_t min;
	int64_t max;
};


struct String {
	Dictionary* dict = nullptr;
	std::string fname;
	Integer index;
};

using mapbox::util::variant;
using mapbox::util::recursive_wrapper;

using ColType = variant<recursive_wrapper<String>, Integer>;

struct ColSpec {
	ColType ctype;	
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