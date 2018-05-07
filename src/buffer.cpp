#include "buffer.hpp"
#include <cassert>

Buffer::Buffer()
 : size(0), next(nullptr)
{
}

char*
Buffer::Alloc(size_t bytes)
{
	assert(data);
	if (size + bytes > capacity) {
		return nullptr;
	}
	char* r = data;
	data += bytes;
	return r;
}


Buffer*
BufferFactory::NewBuffer(size_t capacity)
{
	char* b = new char[capacity + sizeof(Buffer) + alignof(Buffer)];
	if (!b) {
		return nullptr;
	}

	Buffer* buf = new (b) Buffer();

	buf->capacity = capacity;
	buf->data = (char*)buf + sizeof(Buffer);

	return buf;
}

void
BufferFactory::FreeBuffer(Buffer* buf)
{
	char* b = (char*)buf;
	delete[] b;
}

