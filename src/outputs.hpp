#ifndef H_GEN_OUTPUT
#define H_GEN_OUTPUT

#include <vector>

struct StrBufferPool;

struct StrBuffer {
	std::vector<char> data;
	size_t used = 0;
	StrBufferPool* owner = nullptr;

	void init(size_t bytes);

	size_t capacity() const {
		return data.capacity();
	}

	void resize(size_t bytes);

	char* pointer() {
		return &data[0];
	}

	const char* pointer() const {
		return &data[0];
	}

	StrBuffer(StrBufferPool* owner = nullptr);
};

#include "blockingconcurrentqueue.h"

struct StrBufferPool {
	moodycamel::BlockingConcurrentQueue<StrBuffer*> m_queue;
	const size_t capacity;

	StrBufferPool(size_t init);

	~StrBufferPool();

	StrBuffer* Get();

	void Push(StrBuffer& b);
};

struct Output {
	virtual void operator()(const StrBuffer& data) = 0;
};

struct CoutOutput : Output {
	void operator()(const StrBuffer& data) override;
};


#include <functional>

struct CheckOutput : Output {
	std::function<void(const StrBuffer&)> check;

	CheckOutput(std::function<void(const StrBuffer&)> check);

	void operator()(const StrBuffer& data) override;
};

#endif