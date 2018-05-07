#ifndef H_GEN_OUTPUT
#define H_GEN_OUTPUT

#include <vector>

struct StrBuffer {
	std::vector<char> data;
	size_t used = 0;

	StrBuffer() {

	}

	void init(size_t bytes) {
		data.resize(bytes);
	}

	size_t capacity() const {
		return data.capacity();
	}

	void resize(size_t bytes) {
		data.resize(bytes);
	}

	char* pointer() {
		return &data[0];
	}

	const char* pointer() const {
		return &data[0];
	}
};

struct Output {
	virtual void operator()(StrBuffer&& data) = 0;
};

struct CoutOutput : Output {
	void operator()(StrBuffer&& data) override;
};


#include <functional>

struct CheckOutput : Output {
	std::function<void(StrBuffer&&)> check;

	CheckOutput(std::function<void(StrBuffer&&)> check);

	void operator()(StrBuffer&& data) override;
};

#endif