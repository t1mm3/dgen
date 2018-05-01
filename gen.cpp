#include "spec.hpp"

#include <iostream>
#include <boost/program_options.hpp>

#include "conf.hpp"
#include "generator.hpp"
#include "build.hpp"
#include "outputs.hpp"

int main(int argc, char* argv[]) {
	std::ios_base::sync_with_stdio(false);
	RelSpec spec;

	std::cerr << "dgen " << Build::GetVersionStr() << std::endl;

	namespace po = boost::program_options;
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("conf", po::value<std::string>(), "Configuration file")
	;

	try {
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cerr << desc << "\n";
			return 1;
		}

		if (vm.count("conf")) {
			auto fname = vm["conf"].as<std::string>();

			parse_config(std::move(fname), spec);
		} else {
			std::cerr << "Configuration is required\n"; 
			std::cerr << desc << "\n";
			return 1;
		}
	} catch(std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

#if 0
	spec.card = 100;
	spec.cols = { ColSpec {Integer, Sequential, 0, 100}, ColSpec {Integer, Random, 0, 10000} };
#endif

    CoutOutput cout;

	generate(spec, cout);
}