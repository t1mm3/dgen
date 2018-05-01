#include "conf.hpp"
#include "build.hpp"

#include <iostream>
#include <thread>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>

struct InvalidConf : std::exception {
	const char* msg;
	InvalidConf(const char* msg) : msg(msg) {
	}

	virtual const char* what() const throw() {
		return msg;
	}
};

void check_version(const std::string& str)
{
	std::istringstream f(str);
	std::string s;

	int i = 0;
	bool good = true;

	while (getline(f, s, '.')) {
		auto ival = std::stoll(s);
		switch (i) {
		case 0:
			good &= Build::GetVersionMajor() >= ival;
			break;
		case 1:
			good &= Build::GetVersionMinor() >= ival;
			break;
		default:
			throw InvalidConf("Invalid version");
			break;
		}
		i++;
	}

	if (!good) {
		throw InvalidConf("Configuration is from a future version");
	}

	if (!i) {
		throw InvalidConf("Configuration requires a version");
	}

	std::cerr << "Version okay" << std::endl;
}

void parse_config(std::string&& fname, RelSpec& spec)
{
	// Short alias for this namespace
	namespace pt = boost::property_tree;

	// Create a root
	pt::ptree root;

	// Load the json file in this ptree
	pt::read_json(fname, root);

	check_version(root.get<std::string>("version"));


	spec.card = root.get<size_t>("tuples", 0);
	spec.threads = root.get<size_t>("threads", std::thread::hardware_concurrency());

	for (pt::ptree::value_type &col : root.get_child("columns")) {
		auto parse_ctype = [] (std::string& v) -> ColType {
			if (boost::iequals(v, "integer") || boost::iequals(v, "int")) {
				// return Integer();
			}
			if (boost::iequals(v, "string") || boost::iequals(v, "str")) {
				// return String();
			}
			throw InvalidConf("invalid gdata.type");
		};

		auto parse_cgen = [] (std::string& v) {
			if (boost::iequals(v, "sequential")) {
				//return Sequential;
			}
			if (boost::iequals(v, "random")) {
				//return Random;
			}
			throw InvalidConf("invalid gen.type");
		};

		auto parse_col = [&] (pt::ptree& node) -> ColSpec {
			ColSpec r;
			std::string str;

			//r.min = node.get<int64_t>("min", 0);
			//r.max = node.get<int64_t>("max", 0);

			str = node.get<std::string>("data.type");
			r.ctype = parse_ctype(str);

			str = node.get<std::string>("gen.type");
			//r.cgen = parse_cgen(str);

			return std::move(r);
		};

		spec.cols.push_back(parse_col(col.second));
	}
}
