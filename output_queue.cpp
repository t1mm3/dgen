#include "output_queue.hpp"
#include "task.hpp"
#include <cassert>

OutputQueue::OutputQueue(size_t capacity, Output& out)
 : m_out(out)
{
	assert(capacity > 0);
	m_queue.resize(capacity);
	m_used = new std::atomic<bool>[capacity];
	for (size_t i=0; i<capacity; i++) {
		m_used[i] = false;
	}
	m_read_pos = 0;
}

OutputQueue::~OutputQueue() {
	delete[] m_used;
}

void
OutputQueue::Push(Task& t, std::string&& final)
{
	auto id = t.taskId;
	assert(id >= m_read_pos.load());
	assert(id >= 0);
	assert(id <= m_queue.capacity());

	// copy
	m_queue[id] = std::move(final);

	// commit
	bool bfalse = false;
	bool succeeded = m_used[id].compare_exchange_strong(bfalse, true);
	assert(succeeded);

	if (!succeeded) {
		throw std::bad_alloc();
	}

	// scan previous work & output if nobody did
	auto rpos = m_read_pos.load();
	bool ordered = true;
	for (auto i = rpos; i <= id; i++) {
		ordered &= m_used[i].load();
	}

	if (ordered && m_read_pos.compare_exchange_strong(rpos, id+1)) {
		std::lock_guard<std::mutex> lock(m_print_lock);

		for (auto i = rpos; i <= id; i++) {
			assert(m_used[i].load());
			m_out(std::move(m_queue[id]));
		}
	}
}