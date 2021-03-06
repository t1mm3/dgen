#define BOOST_TEST_MODULE "StringFormatting"

#include <boost/test/unit_test.hpp>

#include "primitives.hpp"
#include "utils.hpp"
#include <iostream>
#include <limits>

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
		str_int(&str1, &len1, &b, 1, &tmp, &pred, &sel1, &sel2, I64);
	}

	size_t len2 = snprintf(str2, 1024, "%ld", a);


	// BOOST_REQUIRE_EQUAL(len1, len2);
	BOOST_REQUIRE_EQUAL(str1, str2);
}

BOOST_AUTO_TEST_CASE(int2str_negative) {
	for (int64_t i=-100; i<0; i++) {
		check_str_int(i);
	}
}

BOOST_AUTO_TEST_CASE(int2str_seq_positive) {
	for (int64_t i=0; i<100000; i++) {
		check_str_int(i);
	}
}

BOOST_AUTO_TEST_CASE(int2str_power2_env) {
	int64_t max = std::numeric_limits<int64_t>::max()-1;
	int64_t min = 1;
	for (int64_t i=min; i<max; i*=2 ) {
		if (i >= std::numeric_limits<int64_t>::max() / 2) {
			break;
		}
		check_str_int(i);
		check_str_int(i-1);
		check_str_int(i+1);
	}
}
