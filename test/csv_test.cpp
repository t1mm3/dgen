#define BOOST_TEST_MODULE "CSV"

#include <boost/test/unit_test.hpp>

#include "spec.hpp"
#include "generator.hpp"
#include "outputs.hpp"
#include <iostream>

struct CsvChecker {
	size_t threads = 2;
	size_t card = 100;

	void operator()();

	void verify(const std::string& data, RelSpec& spec);
};

void
CsvChecker::operator()() {
	RelSpec spec;

	spec.threads = threads;
	spec.card = card;

	spec.cols = { ColSpec {Integer, Sequential, 0, 100}, ColSpec {Integer, Random, 0, 10000}, ColSpec {Integer, Random, -100, 10000} };

	spec.kSep = "|";
	spec.kSepLen = 1;
	spec.kNewlineSep = "\n";
	spec.kNewlineSepLen = 1;

	CheckOutput outp([&] (const std::string& data) {
		verify(data, spec);
	});
	generate(spec, outp);
}

void
CsvChecker::verify(const std::string& data, RelSpec& spec)
{
	std::stringstream ss(data);
	size_t num_lines = 0;
	while (ss.good()) {
		// get a line
		std::string line;
		getline(ss, line, '\n');

		std::stringstream ls(line);
		size_t col = 0;
		while (ls.good()) {
			// get a value
			std::string val;
			getline(ls, val, '|');

			// enforce schema
			BOOST_CHECK(col < spec.cols.size());

			auto& scol = spec.cols[col];

			// check a value
			switch (scol.ctype) {
			case Integer:
				{
					BOOST_CHECK(val.size() > 0);
					std::cerr << "val='"<< val <<"'" << std::endl;
					auto ival = std::stoi(val);
					BOOST_CHECK(ival >= scol.min);
					BOOST_CHECK(ival < scol.max);
				}
				break;

			default:
				BOOST_REQUIRE(false);
				break;
			}

			col++;
		}
		BOOST_CHECK(col == spec.cols.size());

		BOOST_CHECK(num_lines < card);
		std::cerr << "line " << num_lines << std::endl;
		num_lines++;
	}

	BOOST_CHECK(num_lines == card);
}

BOOST_AUTO_TEST_CASE(csv_ints_backend) {
	CsvChecker csv;

	for (size_t card = 100; card < 1000000; card *= 10) {
		csv.card = card;
		auto run = [&] () {
			for (int i=1; i<8; i*=2) {
				csv.threads = i;
				csv();
			}
		};

		run();
	}
}