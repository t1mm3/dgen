#ifndef H_GEN_TASK
#define H_GEN_TASK

#include <string>

struct RelSpec;
struct OutputQueue;

struct Task {
	size_t start;
	size_t end;

	RelSpec* rel;
	OutputQueue* outp;
	size_t taskId;
};

#endif