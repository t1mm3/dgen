#define BOOST_TEST_MODULE "Config"

#include <boost/test/unit_test.hpp>

#include "conf.hpp"
#include "test/buildpaths.hpp"
#include <iostream>

BOOST_AUTO_TEST_CASE(json_parse) {
	RelSpec rel;

	parse_config(std::string(g_path) + std::string("json_conf1.txt"),
		rel);
}

BOOST_AUTO_TEST_CASE(json_parse2) {
	RelSpec rel;

	parse_config(std::string(g_path) + std::string("json_conf2.txt"),
		rel);
}