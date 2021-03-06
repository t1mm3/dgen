#ifndef H_GEN_SPEC
#define H_GEN_SPEC

#include <vector>
#include <stdint.h>
#include <mapbox/variant.hpp>

using mapbox::util::variant;
using mapbox::util::recursive_wrapper;

struct Dictionary;
struct ZipfHelper;

struct Sequential {};
struct Uniform {};
struct Poisson { double mean = 1.0; };
struct Binomial { int64_t t = 1; double p = 0.5; };
struct NegBinomial { int64_t k = 1; double p = 0.5; };
struct Geometric { double p = 0.5; };
struct Zipf { double alpha = 0.5; ZipfHelper* helper = nullptr; };

using ColGenType = variant<Uniform, Sequential, Poisson, Binomial, NegBinomial, Geometric, Zipf>;


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

using ColType = variant<recursive_wrapper<String>, Integer>;

struct ColSpec {
	ColType ctype;	
};

struct RelSpec {
	std::vector<ColSpec> cols;
	size_t card;
	size_t threads;

	std::string s_sep;
	std::string s_newline;

	size_t GetSepLen(bool endl) const
	{
		if (endl) {
			return s_newline.size();
		}
		return s_sep.size();
	}


	const char* GetSep(bool endl) const
	{
		if (endl) {
			return s_newline.c_str();
		}
		return s_sep.c_str();
	}

};

#endif