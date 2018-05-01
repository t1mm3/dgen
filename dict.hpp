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

struct Dictionary {
protected:
	struct Entry {
		char* str;
		size_t len;
	};

	std::vector<Entry> m_index;

	const size_t kIndexAllocAhead = 1024*1024;

	static void lookup(char** __restrict__ ptr, size_t* __restrict__ len,
		size_t* __restrict__ indices, size_t num, int* __restrict__ sel,
		const Entry* __restrict__ entries, size_t max);
public:
	Dictionary();

	size_t GetCount() const {
		return m_index.size();
	}

	void Insert(char* str, size_t len) {
		if (m_index.size() == m_index.capacity()) {
			m_index.reserve(m_index.size() + kIndexAllocAhead);
		}

		m_index.emplace_back(Entry { str, (size_t)len});
	}

	void Lookup(char** ptr, size_t* len, size_t* indices, size_t num, int* sel) const {
		lookup(ptr, len, indices, num, sel, &m_index[0], GetCount());
	}
};

#include <boost/iostreams/device/mapped_file.hpp>

struct FileDictionary : Dictionary {
private:
	boost::iostreams::mapped_file_source m_file;

public:
	FileDictionary(const std::string& file);
};

#endif