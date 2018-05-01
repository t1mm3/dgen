#define BOOST_TEST_MODULE "CSV"

#include <boost/test/unit_test.hpp>

#include "spec.hpp"
#include "generator.hpp"
#include "outputs.hpp"
#include "test/buildpaths.hpp"
#include <iostream>
#include <mutex>

struct CsvChecker {
	size_t threads = 2;
	size_t card = 100;
	bool string = false;

	void operator()();

	void verify(const std::string& data, RelSpec& spec);
};

void
CsvChecker::operator()() {
	RelSpec spec;

	spec.threads = threads;
	spec.card = card;

	spec.cols = {
		ColSpec {Integer {Sequential, 0, 101} },
		ColSpec {Integer {Random, 0, 10013} },
		ColSpec {Integer {Random, -100, 104200 } }, 
	};

	if (string) {
		spec.cols.emplace_back(ColSpec {
			String {
				nullptr,
				(std::string(g_path) + std::string("words1.txt")),
				Integer {Random, 0, 5 }
			}
		});
	}

	spec.kSep = "|";
	spec.kSepLen = 1;
	spec.kNewlineSep = "\n";
	spec.kNewlineSepLen = 1;

	std::mutex lock;

	CheckOutput outp([&] (const std::string& data) {
		std::lock_guard<std::mutex> guard(lock);
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
		while (ls.good() && ss.good()) {
			// get a value
			std::string val;
			getline(ls, val, '|');

			BOOST_REQUIRE(col < spec.cols.size());

			const auto& scol = spec.cols[col];

			// check a value
			scol.ctype.match(
				[&] (Integer cint) {
					BOOST_REQUIRE(val.size() > 0);

					BOOST_CHECK(strlen(val.c_str()) == val.size());
					auto ival = std::stoll(val);

					if (ival < cint.min || ival >= cint.max) {
						std::cerr << "ival=" << ival << " from '" << val << "' min=" << cint.min << " max=" << cint.max << std::endl; 
					}
					BOOST_CHECK(ival >= cint.min);
					BOOST_CHECK(ival < cint.max);
				},
				[&] (String unused2) {
					BOOST_REQUIRE(string);
				}
			);

			col++;
		}

		if (ss.good()) {
			BOOST_REQUIRE_EQUAL(col, spec.cols.size());
			BOOST_REQUIRE(num_lines < card);
			num_lines++;
		} else {
			BOOST_REQUIRE_EQUAL(col, 0);
			BOOST_REQUIRE_EQUAL(num_lines, card);
		}
	}

	BOOST_REQUIRE_EQUAL(num_lines, card);
}

BOOST_AUTO_TEST_CASE(csv_ints_backend) {
	CsvChecker csv;

	for (int threads=1; threads<=8; threads*=2) {
		for (size_t card = 100; card <= 1000000; card *= 10) {
			csv.card = card;
			auto run = [&] () {
				csv.threads = threads;

				std::cerr << "card=" << card << " threads=" << threads << std::endl;
				csv();
			};

			run();
		}
	}
}


BOOST_AUTO_TEST_CASE(csv_strs_backend) {
	CsvChecker csv;

	csv.string = true;

	for (int threads=1; threads<=8; threads*=2) {
		for (size_t card = 100; card <= 1000000; card *= 10) {
			csv.card = card;
			auto run = [&] () {
				csv.threads = threads;

				std::cerr << "card=" << card << " threads=" << threads << std::endl;
				csv();
			};

			run();
		}
	}
}