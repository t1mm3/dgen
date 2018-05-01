#ifndef H_GEN_OUTPUT
#define H_GEN_OUTPUT

#include <string>
#include <mutex>

struct Output {
	virtual void operator()(std::string& data) = 0;
};

struct CoutOutput : Output {
	std::mutex lock;

	void operator()(std::string& data) override;
};


#include <functional>

struct CheckOutput : Output {
	std::mutex lock;
	std::function<void(std::string&)> check;

	CheckOutput(std::function<void(std::string&)> check);

	void operator()(std::string& data) override;
};

#endif