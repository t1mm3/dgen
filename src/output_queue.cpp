#include "output_queue.hpp"
#include "task.hpp"
#include <cassert>

OutputQueue::OutputQueue(size_t capacity, size_t num_threads, Output& out)
 : m_out(out), m_num_threads(num_threads)
{
	assert(capacity > 0);
	assert(num_threads > 0);

	m_writer = nullptr;
	m_buffer = nullptr;
	m_buffer_capacity = 0;

	if (num_threads > 1) {
		m_todo = capacity;
		m_buffer_capacity = capacity;
		m_buffer = new std::atomic<StrBuffer*>[capacity];
		for (size_t i=0; i<capacity; i++) {
			m_buffer[i] = nullptr;
		}
		m_read_pos = 0;

		m_writer = new std::thread(&OutputQueue::writer, this);
	}
}

void
OutputQueue::writer()
{
	while (m_read_pos < m_buffer_capacity) {
		size_t id = 0;
		m_flushq.wait_dequeue(id);

		for (size_t i=m_read_pos.load(); i<m_buffer_capacity; i++) {
			auto pbuf = m_buffer[i].load();
			if (!pbuf) {
				break;
			}
			auto& buf = *pbuf;

			m_out(buf);
			dealloc(buf);
			m_read_pos++;
		}
	}
}

OutputQueue::~OutputQueue() {
	if (m_writer) {
		m_writer->join();
		delete m_writer;
	}

	if (m_num_threads > 1) {
		// flush
		assert(m_read_pos == m_queue.size());
	}

	if (m_buffer) {
		delete[] m_buffer;
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
	assert(id < m_buffer_capacity);

	// commit
	StrBuffer* buf0 = nullptr;
	bool succeeded = m_buffer[id].compare_exchange_strong(buf0, &final);
	assert(succeeded);

	if (!succeeded) {
		throw std::bad_alloc();
	}

	if (--m_todo > 0) {
		// scan previous work & output if nobody did
		for (auto i = m_read_pos.load(); i <= id; i++) {
			if (!m_buffer[i].load()) {
				// not ordered cannot flush
				return;
			}
		}
	}

	m_flushq.enqueue(id);
}

void
OutputQueue::dealloc(StrBuffer& buf)
{
	auto pool = buf.Owner();
	assert(pool);
	pool->Push(buf);
}