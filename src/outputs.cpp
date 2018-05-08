#include "outputs.hpp"

#include <iostream>
#include <cassert>

void
StrBuffer::init(size_t bytes)
{
	assert(owner);
	data.reserve(bytes);
	used = 0;
}

void
StrBuffer::resize(size_t bytes)
{
	data.resize(bytes);
}

StrBuffer::StrBuffer(StrBufferPool* owner)
 : owner(owner) {

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
StrBufferPool::Get()
{
	StrBuffer* b = nullptr;
	m_queue.wait_dequeue(b);
	assert(b);
	return b;
}

void
StrBufferPool::Push(StrBuffer& b)
{
	m_queue.enqueue(std::move(&b));
}


void CoutOutput::operator()(const StrBuffer& data)
{
	std::cout.write(&data.data[0], data.used);
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
