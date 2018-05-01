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
};

void CsvChecker::operator()() {
	RelSpec spec;

	spec.threads = threads;
	spec.card = card;

	const size_t kCols = 3;
	const int64_t kColMin[kCols] = {0, 0, -100};
	const int64_t kColMax[kCols] = {100, 10000, 10000};

	spec.cols = { ColSpec {Integer, Sequential, 0, 100}, ColSpec {Integer, Random, 0, 10000}, ColSpec {Integer, Random, -100, 10000} };

	spec.kSep = "|";
	spec.kSepLen = 1;
	spec.kNewlineSep = "\n";
	spec.kNewlineSepLen = 1;

	CheckOutput outp([&] (const std::string& data) {
		std::stringstream ss(data);
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
				BOOST_CHECK(col < kCols);

				// check a value
				if (!val.empty()) {
					std::cerr << "val='"<< val <<"'" << std::endl;
					auto ival = std::stoi(val);
					BOOST_CHECK(ival >= kColMin[col]);
					BOOST_CHECK(ival < kColMax[col]);
				}

				
				col++;
			}
		}
	});
	generate(spec, outp);
}

BOOST_AUTO_TEST_CASE(csv_ints) {
	CsvChecker csv;

	for (size_t card = 100; card < 10000; card *= 10) {
		csv.card = card;
		for (int i=1; i<8; i*=2) {
			csv.threads = i;
			csv();
		}
	}
}