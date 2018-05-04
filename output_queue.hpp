#ifndef H_GEN_OUTPUT_QUEUE
#define H_GEN_OUTPUT_QUEUE

#include <atomic>
#include <mutex>
#include <vector>
#include "outputs.hpp"

struct Task;

struct OutputQueue {
private:
	std::vector<std::string> m_queue;
	std::atomic<bool>* m_used;
	std::atomic<size_t> m_read_pos;
	Output& m_out;
	std::mutex m_print_lock;
	const size_t m_num_threads;
	std::atomic<int64_t> m_todo;

	void flush(size_t npos);
public:
	OutputQueue(size_t capacity, size_t num_threads, Output& out);

	~OutputQueue();

	void Push(Task& t, std::string&& final);

	void operator()(Task& t, std::string&& final) {
		Push(t, std::move(final));
	}
};


#endif