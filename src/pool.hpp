#ifndef H_GEN_POOL
#define H_GEN_POOL

#include <vector>
#include <atomic>
#include "blockingconcurrentqueue.h"

template<typename Job, typename Fun>
class ThreadPool {
private:
	std::atomic<int64_t> m_num_tasks;
	std::vector<std::thread> m_threads;
	moodycamel::BlockingConcurrentQueue<Job> m_queue;

	const int64_t m_max_threads;

public:
	void Worker(size_t id) {
		while (m_num_tasks > 0) {
			Job item;
			if (m_queue.wait_dequeue_timed(item, std::chrono::milliseconds(20))) {
				Fun fun;
				fun(std::move(item));
				m_num_tasks.fetch_sub(1);
			}
		}
	}
public:
	ThreadPool(int64_t num_threads) : m_max_threads(num_threads) {
		m_num_tasks = 0;
	}

	~ThreadPool() {
		Worker(0);
		for (auto& t : m_threads) {
			t.join();
		}
	}

	void Push(Job&& job) {
		if (!m_num_tasks.fetch_add(1)) {
			m_threads.clear();

			const int64_t num = m_max_threads-1;
			for(int64_t i=0; i<num; i++) {
				m_threads.push_back(std::thread([this, i] {
					Worker(i+1);
				}));
			}
		}

		m_queue.enqueue(std::move(job));
	}
};

#endif