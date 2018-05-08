#include "output_queue.hpp"
#include "task.hpp"
#include <cassert>

OutputQueue::OutputQueue(size_t capacity, size_t num_threads, Output& out)
 : m_out(out), m_num_threads(num_threads)
{
	assert(capacity > 0);
	assert(num_threads > 0);

	if (num_threads > 1) {
		m_todo = capacity;
		m_queue.resize(capacity);
		m_used = new std::atomic<bool>[capacity];
		for (size_t i=0; i<capacity; i++) {
			m_used[i] = false;
		}
		m_read_pos = 0;
	}
}

OutputQueue::~OutputQueue() {
	if (m_num_threads > 1) {
		// flush
		assert(m_read_pos == m_queue.size());

		delete[] m_used;
	}
}

void
OutputQueue::Push(Task& t, StrBuffer& final)
{
	if (m_num_threads == 1) {
		// single threaded 
		m_out(final);
		dealloc(final);
		return;
	}

	auto id = t.taskId;
	assert(id >= 0);
	assert(id <= m_queue.capacity());

	// copy
	m_queue[id] = &final;

	// commit
	bool bfalse = false;
	bool succeeded = m_used[id].compare_exchange_strong(bfalse, true);
	assert(succeeded);

	if (!succeeded) {
		throw std::bad_alloc();
	}

	// scan previous work & output if nobody did
	bool ordered = true;
	for (auto i = m_read_pos.load(); i <= id; i++) {
		ordered &= m_used[i].load();
	}

	if (ordered || --m_todo == 0) {
		flush(id);
	}
}

void
OutputQueue::flush(size_t npos)
{
	std::lock_guard<std::mutex> lock(m_print_lock);

	size_t i;
	for (i = m_read_pos; i < m_queue.size() && m_used[i].load(); i++) {
		auto& buf = m_queue[i];
		assert(buf);
		m_out(*buf);
		dealloc(*buf);
		m_read_pos++;
	}

	assert(i >= npos);
}

void
OutputQueue::dealloc(StrBuffer& buf)
{
	auto pool = buf.owner;
	assert(pool);
	pool->Push(buf);
}