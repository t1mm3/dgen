#define BOOST_TEST_MODULE "CSV"

#include <boost/test/unit_test.hpp>

#include "conf.hpp"
#include "spec.hpp"
#include "generator.hpp"
#include "outputs.hpp"
#include "test/buildpaths.hpp"
#include <iostream>
#include <mutex>

std::atomic<int64_t> g_rows;

struct CsvChecker {
	size_t threads = 2;
	size_t card = 100;
	bool string = false;

	bool enforce_seq = false;

	RelSpec spec;

	CsvChecker(bool str);

	void operator()();

	void verify(const std::string& data, RelSpec& spec);
};

CsvChecker::CsvChecker(bool str = false)
  : string(str)
{
	spec.threads = threads;
	spec.card = card;

	spec.cols = {
		ColSpec {Integer {Sequential {}, 0, 101} },
		ColSpec {Integer {Uniform {}, 0, 10013} },
		ColSpec {Integer {Uniform {}, -100, 104200 } }, 
	};

	if (str) {
		spec.cols.emplace_back(ColSpec {
			String {
				nullptr,
				(std::string(g_path) + std::string("words1.txt")),
				Integer {Uniform {}, 0, 5 }
			}
		});
	}
}

void
CsvChecker::operator()() {
	spec.s_sep = "|";
	spec.s_newline = "\n";

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
	int64_t prev = -1;
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

					BOOST_REQUIRE(strlen(val.c_str()) == val.size());
					auto ival = std::stoll(val);

					if (ival < cint.min || ival > cint.max) {
						std::cerr << "col=" << col << " ival=" << ival << " from '" << val << "' min=" << cint.min << " max=" << cint.max << std::endl; 
					}
					BOOST_REQUIRE(ival >= cint.min);
					BOOST_REQUIRE(ival <= cint.max);

					if (enforce_seq) {
						BOOST_REQUIRE(prev < ival);
						prev = ival;
					}
				},
				[&] (String unused2) {
				}
			);

			col++;
		}

		if (ss.good()) {
			BOOST_REQUIRE_EQUAL(col, spec.cols.size());
			BOOST_REQUIRE(num_lines < spec.card);
			num_lines++;
			g_rows++;
		} else {
			BOOST_REQUIRE_EQUAL(col, 0);
		}
	}
}

BOOST_AUTO_TEST_CASE(csv_ints_backend) {
	g_chunk_size = 1024 + 128;
	CsvChecker csv;

	for (int threads=1; threads<=32; threads*=2) {
		for (size_t card = 100; card <= 10000; card *= 10) {
			csv.card = card;
			csv.spec.card = card;
			auto run = [&] () {
				g_rows = 0;
				csv.threads = threads;
				csv.spec.threads = threads;

				std::cerr << "card=" << csv.card << " threads=" << csv.threads << std::endl;
				csv();

				BOOST_REQUIRE_EQUAL(g_rows, card);
			};

			run();
			csv.card = card*5;
			run();
		}
	}
}


BOOST_AUTO_TEST_CASE(csv_strs_backend) {
	g_chunk_size = 1024 + 128;
	CsvChecker csv;

	csv.string = true;

	for (int threads=1; threads<=32; threads*=2) {
		for (size_t card = 100; card <= 10000; card *= 10) {
			csv.card = card;
			csv.spec.card = card;
			auto run = [&] () {
				g_rows = 0;
				csv.threads = threads;
				csv.spec.threads = threads;

				std::cerr << "card=" << csv.card << " threads=" << csv.threads << std::endl;
				csv();

				BOOST_REQUIRE_EQUAL(g_rows, card);
			};

			run();

			csv.card = card*5;
			run();
		}
	}
}

BOOST_AUTO_TEST_CASE(json_parse) {
	CsvChecker csv;

	csv.spec.cols.clear();
	csv.spec.card = 0;
	csv.spec.threads = 0;

	parse_config(std::string(g_path) + std::string("json_conf1.txt"),
		csv.spec);

	csv();
}

BOOST_AUTO_TEST_CASE(json_parse_str) {
	CsvChecker csv;

	csv.spec.cols.clear();
	csv.spec.card = 0;
	csv.spec.threads = 0;

	parse_config(std::string(g_path) + std::string("json_conf2.txt"),
		csv.spec);

	csv();
}


BOOST_AUTO_TEST_CASE(json_parse_str_inline) {
	CsvChecker csv;

	csv.spec.cols.clear();
	csv.spec.card = 0;
	csv.spec.threads = 0;

	parse_config(std::string(g_path) + std::string("json_conf3.txt"),
		csv.spec);

	csv();
}


BOOST_AUTO_TEST_CASE(json_parse_seq) {
	CsvChecker csv;

	g_chunk_size = 1024 + 128;

	csv.spec.cols.clear();
	csv.spec.card = 0;
	csv.spec.threads = 0;
	csv.enforce_seq = true;

	parse_config(std::string(g_path) + std::string("json_conf4.txt"),
		csv.spec);

	csv();
}
