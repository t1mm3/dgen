#ifndef H_GEN_MAIN
#define H_GEN_MAIN

#include <stdint.h>
#include <string>
#include "utils.hpp"

struct RelSpec;
struct Output;

void generate(RelSpec& spec, Output& out);

extern size_t g_chunk_size;

#endif