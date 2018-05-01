#define BOOST_TEST_MODULE "BaseClassModule"

#include <boost/test/unit_test.hpp>

#include "spec.hpp"
#include "generator.hpp"
#include "outputs.hpp"

BOOST_AUTO_TEST_CASE(assignment) {
	RelSpec spec;

	spec.threads = 1;
	spec.card = 100;
	spec.cols = { ColSpec {Integer, Sequential, 0, 100}, ColSpec {Integer, Random, 0, 10000} };

	CheckOutput outp([] (std::string& data) {
		BOOST_CHECK_EQUAL(true, true);
	});
	generate(spec, outp);
}