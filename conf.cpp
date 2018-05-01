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

namespace pt = boost::property_tree;

Integer
parse_integer(pt::ptree& node)
{
	Integer r;

	r.min = node.get<int64_t>("min", 0);
	r.max = node.get<int64_t>("max");

	std::cerr << "min=" << r.min << " max=" << r.max << std::endl;
	r.cgen = Random;

	return std::move(r);
}

String
parse_string(pt::ptree& node)
{
	String r;

	size_t matches = 0;
	for (pt::ptree::value_type &col : node.get_child("index")) {
		r.index = parse_integer(col.second);
		matches++;
	}

	assert(matches == 1);

	r.fname = node.get<std::string>("file");
	r.dict = nullptr;

	return r;
}

ColSpec
parse_column(pt::ptree& node)
{
	ColSpec r;
	
	size_t matches = 0;
	pt::ptree& n = node.get_child("integer");
	if (!n.empty()) {
		r.ctype = parse_integer(n);
		matches++;
	}
		
	assert(matches <= 1);

#if 0
	n = node.get_child("string");
	if (!n.empty()) {
		r.ctype = parse_string(n);
		matches++;
	}
#endif
	std::cerr << "column" << std::endl;

	assert(matches == 1);
	return r;
}


void
parse_config(std::string&& fname, RelSpec& spec)
{
	// Create a root
	pt::ptree root;

	// Load the json file in this ptree
	pt::read_json(fname, root);

	check_version(root.get<std::string>("version"));


	spec.card = root.get<size_t>("tuples");
	spec.threads = root.get<size_t>("threads", std::thread::hardware_concurrency());

	for (pt::ptree::value_type &col : root.get_child("columns")) {
		spec.cols.push_back(parse_column(col.second));
	}
}
