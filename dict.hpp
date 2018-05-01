#ifndef H_GEN_DICT
#define H_GEN_DICT

#include <string>
#include <vector>

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

#include <boost/iostreams/device/mapped_file.hpp>

struct Dictionary {
private:
	struct Entry {
		char* str;
		size_t len;
	};

	std::vector<Entry> m_index;

	const size_t kIndexAllocAhead = 1024*1024;

	boost::iostreams::mapped_file_source m_file;

	static void lookup(char** __restrict__ ptr, size_t* __restrict__ len,
		size_t* __restrict__ indices, size_t num, int* __restrict__ sel,
		const Entry* __restrict__ entries, size_t max);
public:
	Dictionary(const std::string& file);

	size_t GetCount() const {
		return m_index.size();
	}

	void Lookup(char** ptr, size_t* len, size_t* indices, size_t num, int* sel) const {
		lookup(ptr, len, indices, num, sel, &m_index[0], GetCount());
	}
};

#endif