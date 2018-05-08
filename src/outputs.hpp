#ifndef H_GEN_OUTPUT
#define H_GEN_OUTPUT

#include <vector>

struct StrBufferPool;

struct StrBuffer {
private:
	std::vector<char> m_data;
	size_t m_used = 0;
	StrBufferPool* m_owner = nullptr;

	friend class StrBufferPool;
	void init(size_t bytes);
public:
	size_t Capacity() const {
		return m_data.capacity();
	}

	void Resize(size_t bytes);

	char* Pointer() {
		return &m_data[0];
	}

	const char* Pointer() const {
		return &m_data[0];
	}

	StrBufferPool* Owner() {
		return m_owner;
	}

	size_t GetWritePos() const {
		return m_used;
	}

	void SetWritePos(size_t n) {
		m_used = n;
	}

	StrBuffer(StrBufferPool* owner = nullptr);
};

#include "blockingconcurrentqueue.h"

struct StrBufferPool {
private:
	moodycamel::BlockingConcurrentQueue<StrBuffer*> m_queue;
	const size_t capacity;

public:
	StrBufferPool(size_t init);

	~StrBufferPool();

	StrBuffer* Get(size_t bytes);

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