#ifndef H_GEN_CONF
#define H_GEN_CONF

#include <string>
#include "spec.hpp"

void parse_config(std::string&& fname, RelSpec& spec);
void parse_stdin(RelSpec& spec);

#endif