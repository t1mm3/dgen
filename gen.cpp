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

	std::cerr << "dgen " << Build::GetVersionStr() << " " << Build::GetTypeStr() << std::endl;

	namespace po = boost::program_options;
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
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
			std::cerr << "Configuration is required\n"; 
			std::cerr << desc << "\n";
			return 1;
		}
	} catch(std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    CoutOutput cout;

	generate(spec, cout);
}