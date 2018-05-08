#ifndef H_GEN_OUTPUT_QUEUE
#define H_GEN_OUTPUT_QUEUE

#include <atomic>
#include <mutex>
#include <vector>
#include "outputs.hpp"
#include "blockingconcurrentqueue.h"
#include <thread>

struct Task;

struct OutputQueue {
private:
	std::atomic<StrBuffer*>* m_buffer;
	std::atomic<size_t> m_read_pos;
	size_t m_buffer_capacity;
	Output& m_out;
	const size_t m_num_threads;
	std::atomic<int64_t> m_todo;
	std::thread* m_writer;

	moodycamel::BlockingConcurrentQueue<size_t> m_flushq;
	void writer();
	void dealloc(StrBuffer& buf);
public:
	OutputQueue(size_t capacity, size_t num_threads, Output& out);

	~OutputQueue();

	void Push(Task& t, StrBuffer& final);

	void operator()(Task& t, StrBuffer& final) {
		Push(t, final);
	}
};


#endif