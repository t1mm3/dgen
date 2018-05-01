#include "spec.hpp"

#include <thread>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include "generator.hpp"

struct InvalidConf : std::exception {
	const char* msg;
	InvalidConf(const char* msg) : msg(msg) {
	}

	virtual const char* what() const throw() {
		return msg;
	}
};

void parse_config(std::string&& fname, RelSpec& spec) {
	// Short alias for this namespace
	namespace pt = boost::property_tree;

	// Create a root
	pt::ptree root;

	// Load the json file in this ptree
	pt::read_json(fname, root);

	spec.card = root.get<size_t>("tuples", 0);
	spec.threads = root.get<size_t>("threads", std::thread::hardware_concurrency());

	for (pt::ptree::value_type &col : root.get_child("columns")) {
		auto parse_ctype = [] (std::string& v) {
			if (boost::iequals(v, "integer") || boost::iequals(v, "int")) {
				return Integer;
			}
			if (boost::iequals(v, "string") || boost::iequals(v, "str")) {
				return String;
			}
			throw InvalidConf("invalid gdata.type");
		};

		auto parse_cgen = [] (std::string& v) {
			if (boost::iequals(v, "sequential")) {
				return Sequential;
			}
			if (boost::iequals(v, "random")) {
				return Random;
			}
			throw InvalidConf("invalid gen.type");
		};

		auto parse_col = [&] (pt::ptree& node) -> ColSpec {
			ColSpec r;
			std::string str;

			r.min = node.get<int64_t>("min", 0);
			r.max = node.get<int64_t>("max", 0);

			str = node.get<std::string>("data.type");
			r.ctype = parse_ctype(str);

			str = node.get<std::string>("gen.type");
			r.cgen = parse_cgen(str);

			return std::move(r);
		};

		spec.cols.push_back(parse_col(col.second));
	}
}

int main(int argc, char* argv[]) {
	std::ios_base::sync_with_stdio(false);
	RelSpec spec;

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

	generate(spec);
}