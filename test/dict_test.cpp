#define BOOST_TEST_MODULE "Dictionary"

#include <boost/test/unit_test.hpp>

#include "dict.hpp"
#include "test/buildpaths.hpp"

BOOST_AUTO_TEST_CASE(dict_mem) {
	std::string prefix(g_path);
	Dictionary d;

	// do some lookups
	auto lookup = [&] (size_t idx, const char* val) {
		char* str;
		size_t len;

		d.Insert((char*)val, strlen(val));

		d.Lookup(&str, &len, &idx, 1, nullptr);

		size_t val_len = strlen(val);
		BOOST_CHECK_EQUAL(len, val_len);
		BOOST_CHECK(memcmp(str, val, len) == 0);
	};

	for (size_t i=0; i<2; i++) {
		lookup(0, "word1");
		lookup(1, "w2");
		lookup(2, "woord3");
	}
}


BOOST_AUTO_TEST_CASE(dict_file1) {
	std::string prefix(g_path);
	FileDictionary fd(prefix + "words1.txt");

	// do some lookups
	auto lookup = [&] (size_t idx, const char* val) {
		char* str;
		size_t len;

		fd.Lookup(&str, &len, &idx, 1, nullptr);

		size_t val_len = strlen(val);
		BOOST_CHECK_EQUAL(len, val_len);
		BOOST_CHECK(memcmp(str, val, len) == 0);
	};

	lookup(0, "word1");
	lookup(1, "w2");
	lookup(2, "woord3");
	lookup(3, "wort4");
	lookup(4, "w5");
	lookup(5, "w6");

	// wrap around
	lookup(6, "word1");
}
