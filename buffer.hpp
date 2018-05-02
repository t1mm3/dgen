#ifndef H_GEN_BUFFER
#define H_GEN_BUFFER

#include <string>

struct BufferFactory;

struct Buffer {
	char* data;
	size_t capacity;
	size_t size;
	Buffer* next;

	char* Alloc(size_t bytes);
private:
	friend class BufferFactory;
	Buffer();
};

struct BufferFactory {
	Buffer* NewBuffer(size_t capacity);
	void FreeBuffer(Buffer* buf);
};

#endif