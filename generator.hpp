#ifndef H_GEN_MAIN
#define H_GEN_MAIN

#include <stdint.h>
#include <string>

struct RelSpec;
struct Output;

void generate(RelSpec& spec, Output& out);

extern void str_int(char** s, size_t* len, int64_t* a, size_t num, int64_t* log10, bool* tmp_pred, int* tmp_sel, int* tmp_sel2);
#endif