#include "outputs.hpp"

#include <iostream>
#include <cassert>

void
StrBuffer::init(size_t bytes)
{
	assert(m_owner);
	m_data.reserve(bytes);
	m_used = 0;
}

void
StrBuffer::Resize(size_t bytes)
{
	if (bytes <= Capacity()) {
		return;
	}

	m_data.resize(bytes);
}

StrBuffer::StrBuffer(StrBufferPool* owner)
 : m_owner(owner) {

}


StrBufferPool::StrBufferPool(size_t init)
 : capacity(init)
{
	assert(init > 0);
	for (size_t i=0; i<init; i++) {
		m_queue.enqueue(new StrBuffer(this));
	}
}

StrBufferPool::~StrBufferPool()
{
	for (size_t i=0; i<capacity; i++) {
		StrBuffer* p = nullptr;
		m_queue.wait_dequeue(p);
		assert(p);
		delete p;
	}
}

StrBuffer*
StrBufferPool::Get(size_t bytes)
{
	StrBuffer* b = nullptr;
	m_queue.wait_dequeue(b);
	assert(b);
	b->init(bytes);
	return b;
}

void
StrBufferPool::Push(StrBuffer& b)
{
	m_queue.enqueue(std::move(&b));
}


void CoutOutput::operator()(const StrBuffer& data)
{
	std::cout.write(data.Pointer(), data.GetWritePos());
}


CheckOutput::CheckOutput(std::function<void(const StrBuffer&)> check)
 : check(check)
{
}


void
CheckOutput::operator()(const StrBuffer& data)
{
	check(data);
}
