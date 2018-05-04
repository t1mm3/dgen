#include "spec.hpp"

#include <iostream>
#include <thread>
#include <boost/program_options.hpp>

#include "conf.hpp"
#include "generator.hpp"
#include "build.hpp"
#include "outputs.hpp"

int main(int argc, char* argv[]) {
	std::ios_base::sync_with_stdio(false);
	RelSpec spec;

	std::cerr << "dgen " << Build::GetVersionStr() << " " << Build::GetTypeStr() << std::endl;

	int64_t overwr_num_threads = -1;
	int64_t overwr_num_tuples = -1;
	namespace po = boost::program_options;
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "produce help message")
		("threads,t", po::value<int64_t>(&overwr_num_threads)->default_value(overwr_num_threads),
				"overwrite number of threads used")
		("num,n", po::value<int64_t>(&overwr_num_tuples)->default_value(overwr_num_tuples),
				"overwrite output cardinality")
		("conf", po::value<std::string>(), "Configuration file")
	;

	po::positional_options_description p;
	p.add("conf", 1);

	try {
		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cerr << desc << "\n";
			return 1;
		}

		if (vm.count("conf")) {
			assert(vm.count("conf") == 1);
			auto fname = vm["conf"].as<std::string>();

			parse_config(std::move(fname), spec);
		} else {
			parse_stdin(spec);
		}

		if (overwr_num_threads >= 1) {
			spec.threads = overwr_num_threads;
		}

		if (overwr_num_tuples >= 1) {
			spec.card = overwr_num_tuples;
		}
	} catch(std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    CoutOutput cout;

	generate(spec, cout);
}