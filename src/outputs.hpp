#ifndef H_GEN_OUTPUT
#define H_GEN_OUTPUT

#include <string>

struct Output {
	virtual void operator()(std::string&& data) = 0;
};

struct CoutOutput : Output {
	void operator()(std::string&& data) override;
};


#include <functional>

struct CheckOutput : Output {
	std::function<void(std::string&&)> check;

	CheckOutput(std::function<void(std::string&&)> check);

	void operator()(std::string&& data) override;
};

#endif