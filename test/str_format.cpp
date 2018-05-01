#define BOOST_TEST_MODULE "StringFormatting"

#include <boost/test/unit_test.hpp>

#include "generator.hpp"
#include "utils.hpp"
#include <iostream>


BOOST_AUTO_TEST_CASE(rd_log10) {
	auto check = [] (uint64_t x, uint64_t y) {
		uint64_t r;
		rounddown_log10(&r, &x, 1, nullptr);
		BOOST_CHECK_EQUAL(r, y);	
	};
	
	check(1, 1);
	check(9, 1);
	check(10, 10);
	check(11, 10);
	check(15, 10);
	check(16, 10);
	check(17, 10);
	check(99, 10);
	check(100, 100);
	check(9172, 1000);
	check(9999, 1000);
}


extern void str_int(char** s, size_t* len, int64_t* a, size_t num, bool* tmp_pred, int* tmp_sel, int* tmp_sel2);

void check_str_int(int64_t a) {
	char buf1[1024];
	char buf2[1024];

	size_t len1;
	char* str1 = &buf1[0];
	char* str2 = &buf2[0];

	{
		int64_t b = a;
		int64_t tmp;
		bool pred;
		int sel1, sel2;
		str_int(&str1, &len1, &b, 1, &tmp, &pred, &sel1, &sel2);
	}

	size_t len2 = snprintf(str2, 1024, "%ld", a);


	BOOST_REQUIRE_EQUAL(len1, len2);
	BOOST_REQUIRE_EQUAL(str1, str2);
}

BOOST_AUTO_TEST_CASE(int2str) {
	for (int64_t i=-100; i<0; i++) {
		check_str_int(i);
	}

	for (int64_t i=0; i<10000; i++) {
		check_str_int(i);
	}
}
