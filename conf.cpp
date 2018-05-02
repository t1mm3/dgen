#include "conf.hpp"
#include "build.hpp"
#include "dict.hpp"

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
}

std::string
unescape(const std::string& s)
{
	// from https://stackoverflow.com/questions/5612182/convert-string-with-explicit-escape-sequence-into-relative-character
	std::string res;
	std::string::const_iterator it = s.begin();
	while (it != s.end()) {
		char c = *it++;
		if (c == '\\' && it != s.end()) {
			switch (*it++) {
				case '\\': c = '\\'; break;
				case 'n': c = '\n'; break;
				case 't': c = '\t'; break;
				// all other escapes
				default: 
					// invalid escape sequence - skip it. alternatively you can copy it as is, throw an exception...
					continue;
			}
		}
		res += c;
	}

	return res;
}

namespace pt = boost::property_tree;

Integer
parse_integer(pt::ptree& node)
{
	Integer r;

	r.min = node.get<int64_t>("min", 0);
	r.max = node.get<int64_t>("max");
	r.cgen = Uniform{};

	size_t matches = 0;
	for (pt::ptree::value_type &t : node) {
		if (t.first == "gen") {
			if (t.second.data() == "uniform" || t.second.data() == "random") {
				r.cgen = Uniform{};
				matches++;
			}

			if (t.second.data() == "sequential") {
				r.cgen = Sequential {};
				matches++;
			}

			if (t.second.data() == "poisson") {
				r.cgen = Poisson {};
				matches++;
			}

			for (pt::ptree::value_type &i : t.second) {
				if (i.first == "uniform" || i.first == "random") {
					r.cgen = Uniform {};
					matches++;
				}
				if (i.first == "sequential") {
					r.cgen = Sequential {};
					matches++;
				}
				if (i.first == "poisson") {
					auto n = i.second;

					auto p = Poisson {};
					p.mean = i.second.get<double>("mean", p.mean);
					r.cgen = p;

					matches++;
				}
			}
		}
	}

	assert(matches == 1);

	return std::move(r);
}

String
parse_string(pt::ptree& node)
{
	String r;
	size_t matches = 0;

	for (pt::ptree::value_type &t : node) {
		if (t.first == "index") {
			for (pt::ptree::value_type &i : t.second) {
				if (i.first == "integer") {
					r.index = parse_integer(i.second);
					matches++;
				}
			}
		}
	}

	assert(matches == 1);

	r.dict = nullptr;
	r.fname = "";

	for (pt::ptree::value_type &t : node) {
		if (t.first == "file") {
			r.fname = t.second.data();
			matches++;
		}
		if (t.first == "in") {
			auto in = new InlineDictionary();
			r.dict = in;
			for (pt::ptree::value_type &w : t.second) {
				in->Put(unescape(w.second.data()));
				// std::cerr << "word" << unescape(w.second.data()) << std::endl;
			}
			matches++;
		}
	}

	return r;
}

ColSpec
parse_column(pt::ptree& node)
{
	ColSpec r;

	assert(!node.empty());
	
	size_t matches = 0;

	for (pt::ptree::value_type &t : node) {
		if (t.first == "integer") {
			r.ctype = parse_integer(t.second);
			matches++;
		}
		if (t.first == "string") {
			r.ctype = parse_string(t.second);
			matches++;
		}
	}

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

	size_t matches = 0;
	for (pt::ptree::value_type &f : root.get_child("format")) {
		if (f.first == "csv") {
			spec.s_sep = unescape(f.second.get<std::string>("comma", "|"));
			spec.s_newline = unescape(f.second.get<std::string>("newline", "\\n"));

			matches++;
		}
	}
	assert(matches == 1);
}
