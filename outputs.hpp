#ifndef H_GEN_OUTPUT
#define H_GEN_OUTPUT

#include <string>
#include <mutex>

struct Output {
	virtual void operator()(std::string& data) = 0;
};

struct CoutCons : Output {
	std::mutex lock;

	void operator()(std::string& data) override;
};


#endif